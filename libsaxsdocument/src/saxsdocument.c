/*
 * Main API for SAXS document creation and access.
 * Copyright (C) 2009-2014 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of libsaxsdocument.
 *
 * libsaxsdocument is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * libsaxsdocument is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with libsaxsdocument. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "saxsdocument.h"
#include "saxsdocument_format.h"
#include "columns.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <locale.h>
#include <stdio.h>

#ifndef DBL_EPSILON
#define DBL_EPSILON 1e-16
#endif


struct saxs_document {
  char *doc_filename;

  struct line *doc_lines;

  saxs_property_list *doc_properties;

  int doc_curve_count;
  saxs_curve *doc_curves_head;
  saxs_curve *doc_curves_tail;

  const saxs_document_format *doc_format;
};

struct saxs_curve {
  char *curve_title;
  int curve_type;

  int curve_data_count;
  saxs_data *curve_data_head;
  saxs_data *curve_data_tail;

  struct saxs_curve *next;
};

struct saxs_data {
  double x, x_err;
  double y, y_err;

  struct saxs_data *next;
};

#ifndef LIBSAXSDOCUMENT_HEAVY_ASSERTS

#define assert_valid_data(data)
#define assert_valid_data_or_null(data)
#define assert_valid_curve(curve)
#define assert_valid_curve_or_null(curve)
#define assert_valid_document(doc)
#define assert_valid_document_or_null(doc)

#else

static void assert_valid_data(const saxs_data *data) {
  assert((data) != NULL);
  assert(!isinf((data)->x) && !isnan((data)->x));
  assert(!isinf((data)->y) && !isnan((data)->y));
  assert(!isinf((data)->x_err) && !isnan((data)->x_err));
  assert(!isinf((data)->y_err) && !isnan((data)->y_err));
#ifdef DO_NOT_ALLOW_NEGATIVE_ERRORS
  assert((data)->x_err >= 0);
  assert((data)->y_err >= 0);
#endif
}

#define assert_valid_data_or_null(data) { \
  if (data) assert_valid_data(data); \
}

static void assert_valid_curve(const struct saxs_curve *curve) {
  
  assert(curve != NULL);
  //assert(curve->curve_title != NULL);
  assert(curve->curve_data_count >= 0);
  
  assert(curve->curve_type == SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA
      || curve->curve_type == SAXS_CURVE_THEORETICAL_SCATTERING_DATA
      || curve->curve_type == SAXS_CURVE_SCATTERING_DATA
      || curve->curve_type == SAXS_CURVE_PROBABILITY_DATA
      || curve->curve_type == SAXS_CURVE_USER_DATA);
  
  if (curve->curve_data_count > 0) {
    assert_valid_data(curve->curve_data_head);
    // Yes, the tail is checked twice (also during the subsequent loop)
    // but this is only the case when everything is valid
    assert_valid_data(curve->curve_data_tail);
    
    assert(curve->curve_data_tail->next == NULL);
    
    const struct saxs_data *data = curve->curve_data_head;
    int count = 1;  // head counts as one
    while(data != curve->curve_data_tail) {
      data = data->next;
      assert_valid_data(data);
      ++count;
    }
    assert(count == curve->curve_data_count);
  } else {
    assert(curve->curve_data_head == NULL);
    assert(curve->curve_data_tail == NULL);
  }
}

#define assert_valid_curve_or_null(curve) { \
  if (curve) assert_valid_curve(curve); \
}

static void assert_valid_document(const saxs_document *doc) {
  assert(doc != NULL);

  /* TODO: check filename is either NULL or shorter than PATH_MAX */

  assert_valid_line_or_null(doc->doc_lines);

  /* TODO: check doc_properties */

  assert(doc->doc_curve_count >= 0);
  if (doc->doc_curve_count > 0) {
    assert_valid_curve(doc->doc_curves_head);
    assert_valid_curve(doc->doc_curves_tail);
    int check_count = 0;
    const saxs_curve *curve;
    for(curve = doc->doc_curves_head; curve != NULL; curve = curve->next) {
      assert_valid_curve(curve);
      ++check_count;
      if (curve->next == NULL){
        assert(curve == doc->doc_curves_tail);
      }
    }
    assert(check_count == doc->doc_curve_count);
  } else {
    assert(doc->doc_curves_head == NULL);
    assert(doc->doc_curves_tail == NULL);
  }

  /* TODO: Check doc_format */
}

#define assert_valid_document_or_null(doc) { \
  if (doc) assert_valid_document(doc); \
}

#endif

static void saxs_curve_free(struct saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  
  if (curve) {
    while (curve->curve_data_head) {
      saxs_data *d = curve->curve_data_head;
      curve->curve_data_head = curve->curve_data_head->next;
      free(d);
    }

    if (curve->curve_title)
      free(curve->curve_title);
    free(curve);
  }
}

static int test_locale() {
  float d;
  char s[64];
  int n;

  /* making sure to use numbers which have an exact 
   * floating-point representation */
  n = sscanf("5.25", "%f", &d);
  if (n != 1) return 1;
  if (d != 5.25) return 2;

  n = snprintf(s, 64, "%.1f", 3.5);
  if (n != 3) return 3;
  if (strcmp(s, "3.5")) return 4;

  return 0;
}

/*************************************************************************/
saxs_document* saxs_document_create() {
  saxs_document *doc = malloc(sizeof(saxs_document));

  if (doc) {
    doc->doc_filename    = NULL;
    doc->doc_format      = NULL;
    doc->doc_lines       = NULL;
    doc->doc_properties  = saxs_property_list_create();
    doc->doc_curve_count = 0;
    doc->doc_curves_head = NULL;
    doc->doc_curves_tail = NULL;
    if (doc->doc_properties == NULL) {
      free(doc);
      return NULL;
    }
  }

  assert_valid_document(doc);
  return doc;
}

int saxs_document_read(saxs_document *doc, const char *filename,
                       const char *format) {
  saxs_document *tmpdoc = NULL;
  struct line *l;
  int res = ENOTSUP;

  assert_valid_document(doc);

  /*
   * Read in the file contents and cache that in the document buffer.
   * Determining the file type if stdin is read may be tricky, also
   * if a known file type comes with an unknown extension. To be
   * prepared, we already cache the lines here so each format's
   * reader can access them in turn, if necessary.
   */
  res = lines_read(&l, filename);
  if (res != 0)
    return res;

  /* Set the locale to "C" and check that numbers are read correctly */
  const char *oldlocale = setlocale(LC_NUMERIC, "C");
  if (!oldlocale) {
    fprintf(stderr, "Warning: Could not set numeric locale to 'C', this may result in incorrect data being read\n");
  }

  if (test_locale()) {
    fprintf(stderr, "Warning: Input/output of floating point numbers is not working as expected, this may result in incorrect data\n");
  }

  /*
   * First we shall try to determine the file type according to the
   * specified format or the file extension. If that doesn't work,
   * iterate through all known formats to see if any can read the data.
   * We can't immediately iterate through all as, e.g. atsas-dat-n-column
   * would also read .fit files. And that is not what we want.
   */
  saxs_document_format* handler = saxs_document_format_find_first(filename, format);
  while (handler) {
    if (handler->read) {
      /*
       * On read we want to avoid keeping any partial data,
       * therefor, each handler shall read into a new temporary
       * document. Once we found a handler that can read the data
       * all the way, merge the temporary document into the actual
       * data.
       */
      tmpdoc = saxs_document_create();

      res = handler->read(tmpdoc, l, NULL);
      if (res == 0) {
        break;
      } else if (res == ENOMEM) {
        /* Error out immediately if there is a memory allocation failure
         * rather than trying to read with a different file format */
        break;
      }

      saxs_document_free(tmpdoc);
    }
    handler = saxs_document_format_find_next(handler, filename, format);
  }

  /*
   * If nothing found, start again, trying each data handler in turn.
   * First one to accept the data, i.e. returns a value of 0, wins.
   */
  if (!handler) {
    handler = saxs_document_format_first();
    while (handler) {
      if (handler->read) {
        tmpdoc = saxs_document_create();

        res = handler->read(tmpdoc, l, NULL);
        if (res == 0) {
          /* When looping through all handlers, only consider it a success
           * if at least one curve can be read */
          if (saxs_document_curve_count(tmpdoc) > 0)
            break;
          else 
            res = ENOTSUP;
        } else if (res == ENOMEM) {
          /* Error out immediately if there is a memory allocation failure
           * rather than trying to read with a different file format */
          break;
        }

        saxs_document_free(tmpdoc);
      }
      handler = saxs_document_format_next(handler);
    }
  }

  if (res == 0) {
    /*
     * Here everything was read in successfully to the temporary document,
     * now swap the information.
     */
    saxs_document swap_helper;

    tmpdoc->doc_filename = doc->doc_filename;
    doc->doc_filename = strdup(filename);

    tmpdoc->doc_lines = doc->doc_lines;
    doc->doc_lines = l;
    l = NULL;

    swap_helper.doc_properties = doc->doc_properties;
    doc->doc_properties = tmpdoc->doc_properties;
    tmpdoc->doc_properties = swap_helper.doc_properties;

    doc->doc_curve_count = tmpdoc->doc_curve_count;
    swap_helper.doc_curves_head = doc->doc_curves_head;
    doc->doc_curves_head = tmpdoc->doc_curves_head;
    tmpdoc->doc_curves_head = swap_helper.doc_curves_head;
    swap_helper.doc_curves_tail = doc->doc_curves_tail;
    doc->doc_curves_tail = tmpdoc->doc_curves_tail;
    tmpdoc->doc_curves_tail = swap_helper.doc_curves_tail;

    doc->doc_format = handler;

    saxs_document_free(tmpdoc);
  }

  lines_free(l);
  setlocale(LC_NUMERIC, oldlocale);
  assert_valid_document(doc);
  return res;
}

int saxs_document_write(saxs_document *doc, const char *filename,
                        const char *format) {
  assert_valid_document(doc);
  struct line *l = NULL;
  int res = ENOTSUP;

  /* Set the locale to "C" and check that numbers are read correctly */
  const char *oldlocale = setlocale(LC_NUMERIC, "C");
  if (!oldlocale) {
    fprintf(stderr, "Could not set numeric locale to 'C', this may result in incorrect data being read");
  }

  if (test_locale()) {
    fprintf(stderr, "Input/output of floating point numbers is not working as expected, this may result in incorrect data");
  }
  /*
   * First we shall try to determine the file type according to the
   * specified format or the file extension. If that doesn't work,
   * iterate through all known formats to see if any can write the data.
   */
  saxs_document_format* handler = saxs_document_format_find_first(filename, format);
  while (handler) {
    if (handler->write) {
      res = handler->write(doc, &l);
      if (res == 0){
        break;
      } else if (res == ENOMEM) {
        /* Error out immediately if there is a memory allocation failure
          * rather than trying to write with a different file format */
        break;
      }
    }
    handler = saxs_document_format_find_next(handler, filename, format);
  }

  /*
   * If nothing found, start again, trying each data handler in turn.
   * First one to accept the data, i.e. returns a value of 0, wins.
   */
  if (!handler) {
    handler = saxs_document_format_first();
    while (handler) {
      if (handler->write) {
        res = handler->write(doc, &l);
        if (res == 0) {
          break;
        } else if (res == ENOMEM) {
          /* Error out immediately if there is a memory allocation failure
           * rather than trying to write with a different file format */
          break;
        }
      }
      handler = saxs_document_format_next(handler);
    }
  }

  if (res == 0) {
    res = lines_write(l, filename);
    if (res == 0) {
      if (doc->doc_lines) lines_free(doc->doc_lines);
      doc->doc_lines = l;
      l = NULL;

      if (doc->doc_filename) free(doc->doc_filename);
      doc->doc_filename = strdup(filename);

      doc->doc_format = handler;
    }
  }

  lines_free(l);
  setlocale(LC_NUMERIC, oldlocale);
  assert_valid_document(doc);
  return res;
}

void saxs_document_free(saxs_document *doc) {
  assert_valid_document(doc);
  if (doc->doc_filename)
    free(doc->doc_filename);

  lines_free(doc->doc_lines);

  saxs_property_list_free(doc->doc_properties);

  while (doc->doc_curves_head) {
    saxs_curve *curve = doc->doc_curves_head;
    doc->doc_curves_head = doc->doc_curves_head->next;
    saxs_curve_free(curve);
  }

  free(doc);
}


saxs_property*
saxs_document_property_first(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  return saxs_property_list_first(doc->doc_properties);
}

saxs_property*
saxs_document_property_find_first(const saxs_document *doc, const char *name) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  return saxs_property_list_find_first(doc->doc_properties, name);
}

int
saxs_document_property_count(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return 0; // Maintain compatibility with old code that expects this undocumented behaviour
  return saxs_property_list_count(doc->doc_properties);
}

const char *
saxs_document_filename(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  return doc->doc_filename;
}

const char *
saxs_document_format_id(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  if (doc->doc_format == NULL) return NULL;
  return doc->doc_format->name;
}

int
saxs_document_curve_count(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return 0; // Maintain compatibility with old code that expects this undocumented behaviour
  return doc->doc_curve_count;
}

saxs_curve*
saxs_document_curve(const saxs_document *doc) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  return doc->doc_curves_head;
}

saxs_curve*
saxs_curve_next(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  return curve ? curve->next : NULL;
}

saxs_curve*
saxs_document_curve_find(const saxs_document *doc, int type) {
  assert_valid_document(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour
  saxs_curve *curve = doc->doc_curves_head;

  while (curve && !(curve->curve_type & type))
    curve = curve->next;

  assert_valid_curve_or_null(curve);
  return curve;
}

saxs_curve*
saxs_curve_find_next(const saxs_curve *curve, int type) {
  assert_valid_curve_or_null(curve);

  saxs_curve* nextcurve = (curve) ? curve->next : NULL;

  while (nextcurve && !(nextcurve->curve_type & type))
    nextcurve = nextcurve->next;

  assert_valid_curve_or_null(nextcurve);
  return nextcurve;
}

const char*
saxs_curve_title(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  return curve ? curve->curve_title : NULL;
}

void
saxs_curve_set_title(saxs_curve *curve, const char *title) {
  assert_valid_curve_or_null(curve);

  if (curve) {
    if (curve->curve_title)
      free(curve->curve_title);

    if (title)
      curve->curve_title = strdup(title);
    else
      curve->curve_title = NULL;
  }
  
  assert_valid_curve_or_null(curve);
}

int
saxs_curve_type(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  return curve ? curve->curve_type : -1;
}

int
saxs_curve_compare(const saxs_curve *a, const saxs_curve *b) {
  
  assert_valid_curve(a);
  assert_valid_curve(b);
  
  saxs_data *da, *db;

  if (saxs_curve_data_count(a) != saxs_curve_data_count(b))
    return 1;

  da = saxs_curve_data(a);
  db = saxs_curve_data(b);
  while (da && db) {
    if (fabs(da->x - db->x) > DBL_EPSILON
         || fabs(da->x_err - db->x_err) > DBL_EPSILON
         || fabs(da->y - db->y) > DBL_EPSILON
         || fabs(da->y_err - db->y_err) > DBL_EPSILON)
      return 1;

    da = da->next;
    db = db->next;
  }

  return 0;
}

int
saxs_curve_data_count(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  return curve ? curve->curve_data_count : 0;
}

saxs_data*
saxs_curve_data(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  return curve ? curve->curve_data_head : NULL;
}

saxs_data*
saxs_data_next(const saxs_data *data) {
  assert_valid_data_or_null(data);
  return data ? data->next : NULL;
}

double saxs_data_x(const saxs_data *data) {
  assert_valid_data_or_null(data);
  return data ? data->x : 0.0;
}

double saxs_data_x_err(const saxs_data *data) {
  assert_valid_data_or_null(data);
  return data ? data->y_err : 0.0;
}

double saxs_data_y(const saxs_data *data) {
  assert_valid_data_or_null(data);
  return data ? data->y : 0.0;
}

double saxs_data_y_err(const saxs_data *data) {
  assert_valid_data_or_null(data);
  return data ? data->y_err : 0.0;
}


/*************************************************************************/
saxs_property*
saxs_document_add_property(saxs_document *doc,
                           const char *name, const char *value) {
  return saxs_document_add_property_strn(doc, name, -1, value, -1);
}

saxs_property*
saxs_document_add_property_strn(saxs_document *doc,
                                const char *name, int namelen,
                                const char *value, int valuelen) {
  assert_valid_document_or_null(doc);
  if (!doc) return NULL; // Maintain compatibility with old code that expects this undocumented behaviour

  if (!name)
    return NULL;
  if (namelen == 0)
    return NULL;
  if ((namelen < 0) && (strlen(name) == 0))
    return NULL;

  /* According to the comments in saxsdocument.h, value is allowed to be NULL
   * Replacing NULL here with the empty string */
  if (!value) {
    value = "";
    valuelen = 0;
  }

  saxs_property *property = saxs_property_create_strn(name, namelen, value, valuelen);
  if (!property)
    return NULL;

  saxs_property_list_insert(doc->doc_properties, property);
  return property;
}


static saxs_curve* saxs_curve_init(const char *title, int type) {
  saxs_curve *curve = malloc(sizeof(saxs_curve));
  if (!curve)
    return NULL;

  curve->curve_title        = title ? strdup(title) : NULL;
  curve->curve_type         = type;
  curve->curve_data_count   = 0;
  curve->curve_data_head    = NULL;
  curve->curve_data_tail    = NULL;
  curve->next               = NULL;

  assert_valid_curve(curve);
  return curve;
}

/* Append the curve to the document. Cannot fail.
 * Takes over ownership of the saxs_curve object */
static void saxs_document_append_curve(saxs_document *doc, saxs_curve *curve) {
  assert_valid_document(doc);
  assert_valid_curve(curve);

  if (doc->doc_curves_head == NULL)
    doc->doc_curves_head = curve;
  else
    doc->doc_curves_tail->next = curve;

  doc->doc_curves_tail = curve;
  doc->doc_curve_count += 1;
  assert_valid_document(doc);
}

saxs_curve*
saxs_document_add_curve(saxs_document *doc, const char *title, int type) {
  assert_valid_document(doc);
  saxs_curve *curve = saxs_curve_init(title, type);
  if (!curve)
    return NULL;
  saxs_document_append_curve(doc, curve);
  assert_valid_document(doc);
  assert_valid_curve(curve);
  return curve;
}

saxs_curve*
saxs_document_copy_curve(saxs_document *doc, const saxs_curve *in) {
  assert_valid_document(doc);
  assert_valid_curve_or_null(in);

  saxs_curve *out = NULL;

  if (in) {
    out = saxs_curve_init(in->curve_title, in->curve_type);
    if (!out)
      return NULL;

    saxs_data *data = saxs_curve_data(in);
    while (data) {
      int res = saxs_curve_add_data(out, data->x, data->x_err, data->y, data->y_err);
      if (res != 0) {
        saxs_curve_free(out);
        return NULL;
      }
      data = data->next;
    }
  }
  saxs_document_append_curve(doc, out);

  assert_valid_document(doc);
  assert_valid_curve(out);
  return out;
}

int saxs_curve_add_data(saxs_curve *curve,
                         double x, double x_err,
                         double y, double y_err) {

  assert_valid_curve(curve);

  saxs_data *data = malloc(sizeof(saxs_data));
  if (!data)
    return ENOMEM;

  data->x     = x;
  data->x_err = x_err;
  data->y     = y;
  data->y_err = y_err;
  data->next  = NULL;

  if (curve->curve_data_head == NULL)
    curve->curve_data_head = data;
  else
    curve->curve_data_tail->next = data;

  curve->curve_data_tail = data;
  curve->curve_data_count += 1;

  assert_valid_curve(curve);
  return 0;
}

int
saxs_curve_has_y_err(const saxs_curve *curve) {
  assert_valid_curve_or_null(curve);
  if (curve) {
    saxs_data *data = saxs_curve_data(curve);

    while (data) {
      if (saxs_data_y_err(data) > 0.0)
        return 1;

      data = saxs_data_next(data);
    }
  }

  return 0;
}
