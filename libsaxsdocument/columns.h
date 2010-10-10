/*
 * Common code to read columnized data in text files.
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

#ifndef LIBSAXSDOCUMENT_COLUMNS_H
#define LIBSAXSDOCUMENT_COLUMNS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct saxs_document;

struct line {
  size_t line_number;
  size_t line_length;
  char *line_buffer;

  struct line *next;
};

/**
 * @brief Copy the contents of a file to a list of lines.
 * The lines are allocated by the function and must be free'd by @ref lines_free.
 *
 * @param filename
 * @param lines
 *
 * @returns 0 on success, -1 otherwise.
 */
int
lines_read(struct line **l, const char *filename);

void
lines_free(struct line *l);


/**
 * @brief Separate header, data and footer in a previously filled list of lines.
 */
int
saxs_reader_columns_scan(struct line *lines,
                         struct line **header,
                         struct line **data,
                         struct line **footer);

/**
 * @brief Parse identified columns in a list of lines.
 */
int
saxs_reader_columns_parse(struct saxs_document *doc,
                          struct line *firstline, struct line *lastline,
                          int scol, double sfactor,
                          int icol, double ifactor,
                          int errcol,
                          const char *title, int type);


/**
 * @brief Count the number of data values in a given line.
 */
int
saxs_reader_columns_count(struct line *l);


/**
 * @brief Count the number of data values in a file.
 *
 * Convenience function.
 *
 * Scans a file, splits it in header, data, footer and counts the
 * data columns.
 */
int
saxs_reader_columns_count_file(const char *filename);


/**
 * @brief Count the number of data values in a file.
 *
 * Convenience function.
 *
 * Scans a file, splits it in header, data, footer and counts the
 * data columns.
 */
int
saxs_reader_columns_parse_file(struct saxs_document *doc,
                               const char *filename,
                               int (*parse_header)(struct saxs_document*,
                                                   struct line *,
                                                   struct line *),
                               int (*parse_data)(struct saxs_document*,
                                                 struct line *,
                                                 struct line *),
                               int (*parse_footer)(struct saxs_document*,
                                                   struct line *,
                                                   struct line *));

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_COLUMNS_H */
