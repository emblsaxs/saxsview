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


/*************************************************************************/
saxs_document* saxs_document_create() {
  saxs_document *doc = malloc(sizeof(saxs_document));

  if (doc) {
    doc->doc_filename    = NULL;
    doc->doc_lines       = NULL;
    doc->doc_properties  = saxs_property_list_create();
    doc->doc_curve_count = 0;
    doc->doc_curves_head = NULL;
    doc->doc_curves_tail = NULL;
  }

  return doc;
}

int saxs_document_read(saxs_document *doc, const char *filename,
                       const char *format) {
  struct line *l;
  int res = ENOTSUP;

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
      res = handler->read(doc, l, NULL);
      if (res == 0)
        break;
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
        res = handler->read(doc, l, NULL);
        if (res == 0) {
          /* When looping through all handlers, only consider it a success
           * if at least one curve or property can be read */
          if (saxs_document_curve_count(doc) > 0 ||
              saxs_document_property_count(doc) > 0)
            break;
          else 
            res = ENOTSUP;
        }
      }
      handler = saxs_document_format_next(handler);
    }
  }

  if (res == 0) {
    /* Here everything was read in successfully, keep the info around. */
    if (doc->doc_lines) lines_free(doc->doc_lines);
    doc->doc_lines = l;
    l = NULL;

    if (doc->doc_filename) free(doc->doc_filename);
    doc->doc_filename = strdup(filename);
  }

  lines_free(l);
  return res;
}

int saxs_document_write(saxs_document *doc, const char *filename,
                        const char *format) {

  struct line *l = NULL;
  int res = ENOTSUP;

  /*
   * First we shall try to determine the file type according to the
   * specified format or the file extension. If that doesn't work,
   * iterate through all known formats to see if any can write the data.
   */
  saxs_document_format* handler = saxs_document_format_find_first(filename, format);
  while (handler) {
    if (handler->write) {
      res = handler->write(doc, &l);
      if (res == 0)
        break;
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
        if (res == 0)
          break;
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
    }
  }

  lines_free(l);
  return res;
}

void saxs_document_free(saxs_document *doc) {
  if (doc->doc_filename)
    free(doc->doc_filename);

  lines_free(doc->doc_lines);

  saxs_property_list_free(doc->doc_properties);

  while (doc->doc_curves_head) {
    saxs_curve *curve = doc->doc_curves_head;

    while (curve->curve_data_head) {
      saxs_data *d = curve->curve_data_head;
      curve->curve_data_head = curve->curve_data_head->next;
      free(d);
    }

    doc->doc_curves_head = doc->doc_curves_head->next;
    if (curve->curve_title)
      free(curve->curve_title);
    free(curve);
  }

  free(doc);
}


saxs_property*
saxs_document_property_first(saxs_document *doc) {
  return doc ? saxs_property_list_first(doc->doc_properties) : NULL;
}

saxs_property*
saxs_document_property_find_first(saxs_document *doc, const char *name) {
  return doc ? saxs_property_list_find_first(doc->doc_properties, name) : NULL;
}

int
saxs_document_property_count(saxs_document *doc) {
  return doc ? saxs_property_list_count(doc->doc_properties) : 0;
}

const char *
saxs_document_filename(saxs_document *doc) {
  return doc ? doc->doc_filename : NULL;
}

int
saxs_document_curve_count(saxs_document *doc) {
  return doc ? doc->doc_curve_count : 0;
}

saxs_curve*
saxs_document_curve(saxs_document *doc) {
  return doc ? doc->doc_curves_head : NULL;
}

saxs_curve*
saxs_curve_next(saxs_curve *curve) {
  return curve ? curve->next : NULL;
}

saxs_curve*
saxs_document_curve_find(saxs_document *doc, int type) {
  saxs_curve *curve = doc ? doc->doc_curves_head : NULL;

  while (curve && !(curve->curve_type & type))
    curve = curve->next;

  return curve;
}

saxs_curve*
saxs_curve_find_next(saxs_curve *curve, int type) {
  if (curve)
    curve = curve->next;

  while (curve && !(curve->curve_type & type))
    curve = curve->next;

  return curve;
}

const char*
saxs_curve_title(saxs_curve *curve) {
  return curve ? curve->curve_title : NULL;
}

void
saxs_curve_set_title(saxs_curve *curve, const char *title) {
  if (curve) {
    if (curve->curve_title)
      free(curve->curve_title);

    if (title)
      curve->curve_title = strdup(title);
    else
      curve->curve_title = NULL;
  }
}

int
saxs_curve_type(saxs_curve *curve) {
  return curve ? curve->curve_type : -1;
}

int
saxs_curve_compare(saxs_curve *a, saxs_curve *b) {
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
saxs_curve_data_count(saxs_curve *curve) {
  return curve ? curve->curve_data_count : 0;
}

saxs_data*
saxs_curve_data(saxs_curve *curve) {
  return curve ? curve->curve_data_head : NULL;
}

saxs_data*
saxs_data_next(saxs_data *data) {
  return data ? data->next : NULL;
}

double saxs_data_x(saxs_data *data) {
  return data ? data->x : 0.0;
}

double saxs_data_x_err(saxs_data *data) {
  return data ? data->y_err : 0.0;
}

double saxs_data_y(saxs_data *data) {
  return data ? data->y : 0.0;
}

double saxs_data_y_err(saxs_data *data) {
  return data ? data->y_err : 0.0;
}


/*************************************************************************/
saxs_property*
saxs_document_add_property(saxs_document *doc,
                           const char *name, const char *value) {
  if (!doc)
    return NULL;

  if (!name || strlen(name) == 0
      || !value || strlen(value) == 0)
    return NULL;

  saxs_property *property = saxs_property_create(name, value);
  if (!property)
    return NULL;

  saxs_property_list_insert(doc->doc_properties, property);
  return property;
}

saxs_curve*
saxs_document_add_curve(saxs_document *doc, const char *title, int type) {
  saxs_curve *curve = malloc(sizeof(saxs_curve));

  curve->curve_title        = title ? strdup(title) : NULL;
  curve->curve_type         = type;
  curve->curve_data_count   = 0;
  curve->curve_data_head    = NULL;
  curve->curve_data_tail    = NULL;
  curve->next               = NULL;

  if (doc->doc_curves_head == NULL)
    doc->doc_curves_head = curve;
  else
    doc->doc_curves_tail->next = curve;

  doc->doc_curves_tail = curve;
  doc->doc_curve_count += 1;

  return curve;
}

saxs_curve*
saxs_document_copy_curve(saxs_document *doc, saxs_curve *in) {
  saxs_curve *out = NULL;

  if (in) {
    out = saxs_document_add_curve(doc, in->curve_title, in->curve_type);

    saxs_data *data = saxs_curve_data(in);
    while (data) {
      saxs_curve_add_data(out, data->x, data->x_err, data->y, data->y_err);
      data = data->next;
    }
  }

  return out;
}

void saxs_curve_add_data(saxs_curve *curve,
                         double x, double x_err,
                         double y, double y_err) {

  saxs_data *data = malloc(sizeof(saxs_data));

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
}

int
saxs_curve_has_y_err(saxs_curve *curve) {
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
