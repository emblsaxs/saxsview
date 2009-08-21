/*
 * Format handling of SAXS documents.
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

#ifndef LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H
#define LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H

struct saxs_document;

struct saxs_document_format {
  int (*read)(struct saxs_document *doc, const char *filename);
  int (*write)(struct saxs_document *doc, const char *filename);
};
typedef struct saxs_document_format saxs_document_format;


saxs_document_format*
saxs_document_format_find(const char *filename, const char *format);


/** Case-insensitive string comparison. */
int compare_format(const char *a, const char *b);

/** Extract the suffix of a filename, e.g. 'bsa.dat' has suffix 'dat'. */
const char* suffix(const char *filename);

#endif /* !LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H */
