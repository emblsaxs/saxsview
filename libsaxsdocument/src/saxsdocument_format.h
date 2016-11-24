/*
 * Format handling of SAXS documents.
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

#ifndef LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H
#define LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

struct saxs_document;
struct line;

/**
 * @brief File format descriptor.
 */
struct saxs_document_format {
  /**
   * The filename extension of the format, if any.
   * Example: @a dat.
   */
  const char *extension;

  /**
   * A short descriptive and unique name for the format.
   * If multiple format descriptions have the same extension,
   * a particular one can be picked by name.
   */
  const char *name;

  /** Free form descriptive text, e.g. to be used in a GUI. */
  const char *description;

  /**
   * @returns 0 if read successfully, an error code on error.
   *          Shall return ENOTSUP if the file can not be read.
   */
  int (*read)(struct saxs_document *doc, const struct line*, const struct line*);

  /**
   * @returns 0 if written successfully, an error code on error.
   *          Shall return ENOTSUP if the file can not be written.
   */
  int (*write)(struct saxs_document *doc, struct line**);

  struct saxs_document_format *next;
};
typedef struct saxs_document_format saxs_document_format;


saxs_document_format*
saxs_document_format_create();

void
saxs_document_format_free(saxs_document_format*);

/**
 * Shall be called at by the user of libsaxsdocument at startup of the
 * application.
 *
 * Or, shall be called by one of @ref saxs_document_format_first,
 * @ref saxs_document_format_next, @ref saxs_document_format_find_first
 * or @ref saxs_document_format_find_next if the list of formats is empty.
 */
void
saxs_document_format_init();

/**
 * De-register all registered formats, frees all allocated memory.
 */
void
saxs_document_format_clear();

void
saxs_document_format_register(const saxs_document_format *format);

saxs_document_format*
saxs_document_format_first();


saxs_document_format*
saxs_document_format_next(const saxs_document_format*);


/**
 * @brief Find a specific format by name or by determined from file name.
 *
 * Convenience Function.
 *
 * The @a formatname takes precedence over the @a filename as the
 * @a filename uses the @a extension field to determine a suitable
 * format. If a @a formatname is specified, the format descriptor
 * with this name will be returned. If no such format descriptor
 * is found, the @a filename extension will be examined and the
 * first match will be returned.
 *
 * @param filename    A filename, may be NULL.
 * @param formatname  The name of a format, may be NULL.
 *
 * @returns A format description or NULL if no format could be found.
 */
saxs_document_format*
saxs_document_format_find_first(const char *filename,
                                const char *formatname);


saxs_document_format*
saxs_document_format_find_next(const saxs_document_format*,
                               const char *filename,
                               const char *formatname);


/**
 * @brief Case-insensitive string comparison.
 * @returns 0 if the format identifiers are equal, 1 otherwise.
 */
int
compare_format(const char *a, const char *b);

/**
 * @brief Extract the suffix of a filename.
 * Example: 'bsa.dat' has suffix 'dat'.
 */
const char*
suffix(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H */
