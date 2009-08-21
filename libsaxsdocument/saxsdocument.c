/*
 * Main API for SAXS document creation and access.
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

struct saxs_document {
  char *doc_filename;

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

  doc->doc_filename = NULL;
  doc->doc_properties  = saxs_property_list_create();
  doc->doc_curve_count = 0;
  doc->doc_curves_head = NULL;
  doc->doc_curves_tail = NULL;

  return doc;
}

int saxs_document_read(saxs_document *doc, const char *filename,
                       const char *format) {
  saxs_document_format* handler = saxs_document_format_find(filename, format);

  doc->doc_filename = strdup(filename);
  return handler && handler->read ? handler->read(doc, filename) : -1;
}

int saxs_document_write(saxs_document *doc, const char *filename,
                        const char *format) {
  saxs_document_format* handler = saxs_document_format_find(filename, format);
  return handler && handler->write ? handler->write(doc, filename) : -1;
}

void saxs_document_free(saxs_document *doc) {
  if (doc->doc_filename)
    free(doc->doc_filename);

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

  while (curve && curve->curve_type != type)
    curve = curve->next;

  return curve;
}

saxs_curve*
saxs_curve_find_next(saxs_curve *curve, int type) {
  if (curve)
    curve = curve->next;

  while (curve && curve->curve_type != type)
    curve = curve->next;

  return curve;
}

const char*
saxs_curve_title(saxs_curve *curve) {
  return curve ? curve->curve_title : NULL;
}

int
saxs_curve_type(saxs_curve *curve) {
  return curve ? curve->curve_type : -1;
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
