/*
 * Callbacks to format-specific parsers.
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

#ifndef LIBSAXSDOCUMENT_FORMAT_H
#define LIBSAXSDOCUMENT_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

struct saxs_document;

typedef int (*saxs_format_callback)(struct saxs_document *doc, 
                                    const char *filename);

/**
 * @brief Find a parser/reader for a given format.
 * @returns a callback if an appropriate reader was found, NULL otherwise.
 */
saxs_format_callback
saxs_reader_find(const char *filename, const char *format);

/**
 * @brief Find a generator/writer for a given format.
 * @returns a callback if an appropriate writer was found, NULL otherwise.
 */
saxs_format_callback
saxs_writer_find(const char *filename, const char *format);



/*
 * Private prototypes of callback functions.
 */
int saxs_reader_dat(struct saxs_document *doc, const char *filename);
int saxs_writer_dat(struct saxs_document *doc, const char *filename);
int saxs_reader_int(struct saxs_document *doc, const char *filename);
int saxs_reader_fir_fit(struct saxs_document *doc, const char *filename);
int saxs_reader_out(struct saxs_document *doc, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_FORMAT_H */
