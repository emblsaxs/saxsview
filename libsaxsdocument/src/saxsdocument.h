/*
 * Main API for SAXS document creation and access.
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
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

/*
 * Windows platforms do not define ENOTSUP (Operation not supported),
 * use ENOSYS (Function not implemented) instead.
 */
#ifndef ENOTSUP
#define ENOTSUP ENOSYS
#endif

#include "saxsproperty.h"

enum {
  SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA = 0x1,
  SAXS_CURVE_THEORETICAL_SCATTERING_DATA = 0x2,
  SAXS_CURVE_SCATTERING_DATA = SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA
                                |SAXS_CURVE_THEORETICAL_SCATTERING_DATA,
  SAXS_CURVE_PROBABILITY_DATA = 0x4,
  SAXS_CURVE_USER_DATA = 0x10000
};


struct saxs_document;
typedef struct saxs_document saxs_document;

struct saxs_curve;
typedef struct saxs_curve saxs_curve;

struct saxs_data;
typedef struct saxs_data saxs_data;


/**
 * @brief Create a new document.
 *
 * A document is a collection of key-value pairs (information about
 * document properties) and a collection of scattering- and probability
 * curves. Any of these curves may be empty, non-empty curves may be
 * traversed using the corresponding iterator functions.
 *
 * A document is either populated by reading a file in a specified format
 * (@ref saxs_document_read), or by adding properties pairs
 * (@ref saxs_document_add_property) and curves (@ref saxs_document_add_curve)
 * manually. Modified documents may be written in a specfied format by
 * @ref saxs_document_write.
 *
 * @returns An opaque pointer to a newly allocated document. The pointer
 *          must be free'd with @ref saxs_document_free.
 */
saxs_document*
saxs_document_create();

/**
 * @brief Read data from a file or stdin.
 *
 * Reads data from a named file or stdin into the specified document.
 *
 * @param doc     A non-NULL document-pointer created by @ref saxs_document_create.
 * @param infile  Input-filename; reads from stdin if @c -.
 * @param format  A known format (e.g. "atsas-dat-3-column"). An attempt is
 *                made to deduce the format from the input filename if NULL.
 *
 * @returns 0 on success, a non-null error code on error; ENOTSUP if no format
 *          handler could successfully read the file.
 */
int
saxs_document_read(saxs_document *doc, const char *infile,
                   const char *format);

/**
 * @brief Write data to a file or stdout.
 *
 * Writes the specfied document to a named file or stdout.
 *
 * @param doc      A non-NULL document-pointer created by @ref saxs_document_create.
 * @param outfile  Output-filename; writes to stdout if @c -.
 * @param format   A known format (e.g. "dat"). An attempt is made to deduce the
 *                 format from the output filename if NULL.
 *
 * @returns 0 on success, a non-null error code on error; ENOTSUP if no format
 *          handler could successfully read the file.
 */
int
saxs_document_write(saxs_document *doc, const char *outfile,
                    const char *format);

/**
 * @brief Free's allocated memory.
 * Free's memory allocated by @ref saxs_document_create.
 */
void
saxs_document_free(saxs_document *doc);

/**
 * @brief The document's file name.
 *
 * @param doc A non-NULL document-pointer created by @ref saxs_document_create.
 *
 * @returns NULL if no file was read with @ref saxs_document_read or written with
 *          @ref saxs_document_write, the corresponding filename otherwise.
 */
const char*
saxs_document_filename(saxs_document *doc);


/**
 * @brief Add a new property to a document.
 *
 * Properties are name-value pairs stored together with the document.
 * The names are not necessarily unique and may appear multiple times.
 *
 * Available properties may be traversed by @ref saxs_document_property
 * and @ref saxs_property_next, properties with a particular key may be
 * traversed by @ref saxs_document_property_find and
 * @ref saxs_property_find_next. The number of overall properties is
 * available via @ref saxs_document_property_count.
 *
 * To access the properties fields, use @ref saxs_property_name and
 * @ref saxs_property_value.
 *
 * Depending on the output format, properties may be written into a
 * file with @ref saxs_document_write.
 *
 * @param doc    A non-NULL document-pointer created by @ref saxs_document_create.
 * @param name   The name of the property, shall not be NULL.
 * @param value  The value of the property, may be NULL.
 *
 * @returns A pointer to the newly created property if succesfull,
 *          NULL otherwise.
 */
saxs_property*
saxs_document_add_property(saxs_document *doc,
                           const char *name, const char *value);

/**
 * @brief The number of properties of a document.
 *
 * @param doc A non-NULL document-pointer created by @ref saxs_document_create.
 *
 * @returns The number of properties in the current document.
 */
int
saxs_document_property_count(saxs_document *doc);

/**
 * @brief Traverse all properties of a document.
 *
@verbatim
  saxs_property *property = saxs_document_property_first(doc);
  while (property) {
    // use property
    property = saxs_property_next(property);
  }
@endverbatim
 *
 * @param doc A non-NULL document-pointer created by @ref saxs_document_create.
 *
 * @returns The property, or NULL if there is none.
 *
 * @see @ref saxs_document_property_find_first
 *      @ref saxs_property_next
 */
saxs_property*
saxs_document_property_first(saxs_document *doc);


/**
 * @brief Traverse named properties of a document.
 *
 * To find all properties with a specific name of a document:
@verbatim
  saxs_property *parent = saxs_document_property_find(doc, "parent");
  while (parent) {
    // use parents
    parent = saxs_property_find_next(property, "parent");
  }
@endverbatim
 *
 * @param doc  A non-NULL document-pointer created by @ref saxs_document_create.
 * @param name The name of the property to find.
 *
 * @returns The first property whose name matches the specified name,
 *          or NULL if there is none.
 *
 * @see @ref saxs_document_property,
 *      @ref saxs_property_next
 */
saxs_property*
saxs_document_property_find_first(saxs_document *doc, const char *name);




/*
 * About saxs curves
 */
saxs_curve*
saxs_document_add_curve(saxs_document *doc, const char *title, int type);

/**
 * @brief The number of curves in a document.
 *
 * @param doc  A non-NULL document-pointer created by @ref saxs_document_create.
 *
 * @returns The number of curves in the document.
 */
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

