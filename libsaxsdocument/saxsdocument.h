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

#ifndef LIBSAXSDOCUMENT_SAXSDOCUMENT_H
#define LIBSAXSDOCUMENT_SAXSDOCUMENT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
  SAXS_CURVE_SCATTERING_DATA = 1,
  SAXS_CURVE_PROBABILITY_DATA,
  SAXS_CURVE_USER_DATA = 100
};


struct saxs_document;
typedef struct saxs_document saxs_document;

struct saxs_property;
typedef struct saxs_property saxs_property;

struct saxs_curve;
typedef struct saxs_curve saxs_curve;

struct saxs_data;
typedef struct saxs_data saxs_data;


/*
 * Creating and destroying a saxs_document
 */
saxs_document*
saxs_document_create();

int
saxs_document_read(saxs_document *doc, const char *infile,
                   const char *format);

int
saxs_document_write(saxs_document *doc, const char *outfile,
                    const char *format);

void
saxs_document_free(saxs_document *doc);

const char*
saxs_document_filename(saxs_document *doc);


/*
 * Adding and retrieving properties
 */
saxs_property*
saxs_document_add_property(saxs_document *doc,
                           const char *name, const char *value);

int
saxs_document_property_count(saxs_document *doc);

saxs_property*
saxs_document_property(saxs_document *doc);

saxs_property*
saxs_document_property_find(saxs_document *doc, const char *name);


saxs_property*
saxs_property_next(saxs_property *property);

saxs_property*
saxs_property_find_next(saxs_property *property, const char *name);

const char*
saxs_property_name(saxs_property *property);

const char*
saxs_property_value(saxs_property *property);


/*
 * About saxs curves
 */
saxs_curve*
saxs_document_add_curve(saxs_document *doc, const char *title, int type);

int
saxs_document_curve_count(saxs_document *doc);

saxs_curve*
saxs_document_curve(saxs_document *doc);

saxs_curve*
saxs_document_curve_find(saxs_document *doc, int type);

saxs_curve*
saxs_curve_next(saxs_curve *curve);

saxs_curve*
saxs_curve_find_next(saxs_curve *curve, int type);

const char*
saxs_curve_title(saxs_curve *curve);

int
saxs_curve_type(saxs_curve *curve);

void
saxs_curve_add_data(saxs_curve *curve,
                    double x, double x_err,
                    double y, double y_err);

int
saxs_curve_data_count(saxs_curve *curve);

saxs_data*
saxs_curve_data(saxs_curve *curve);

saxs_data*
saxs_data_next(saxs_data *data);

double
saxs_data_x(saxs_data *data);

double
saxs_data_x_err(saxs_data *data);

double
saxs_data_y(saxs_data *data);

double
saxs_data_y_err(saxs_data *data);


#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_SAXSDOCUMENT_H */

