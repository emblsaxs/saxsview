/*
 * Common code to read columnized data in text files.
 * Copyright (C) 2009, 2011 Daniel Franke <dfranke@users.sourceforge.net>
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
  size_t line_length;
  char *line_buffer;

  /* Number of data columns found. */
  int line_column_count;

  /* The values found. */
  double *line_column_values;

  struct line *next;
};

/**
 * @brief Allocate and initialize a new line.
 * @returns a pointer to the allocated memory or NULL if out of memory.
 */
struct line*
lines_create();


/**
 * @brief 
 * @param lines 
 * @param l
 */
void
lines_append(struct line **lines, struct line *l);


/**
 * @brief Formatted output to line, similar to printf.
 *
 * If the current line length is too short, the line buffer is increased
 * to hold the full output.
 *
 * @returns The number of characters printed to the line.
 */
int
lines_printf(struct line *l, const char *fmt, ...);


/**
 * @brief Copy the contents of a file to a list of lines.
 *
 * The lines are allocated by the function and must be free'd by @ref lines_free.
 *
 * @param lines
 * @param filename The target file name. If the filename is '-', the
 *                 input will be read from stdin.
 *
 * @returns 0 on success, a non-null error number (i.e. an @a errno) otherwise.
 */
int
lines_read(struct line **lines, const char *filename);


/**
 * @brief Write a list of lines into a named file.
 *
 * @param lines
 * @param filename The target file name. Any existing file will
 *                 be overwritten. If the filename is '-', the
 *                 output will be redirected to stdout.
 *
 * @returns 0 on success, a non-null error number (i.e. an @a errno) otherwise.
 */
int
lines_write(struct line *lines, const char *filename);


/**
 * @brief Free the set of lines.
 * @param lines A pointer to the first lines, also free's all following lines.
 */
void
lines_free(struct line *lines);


/**
 * @brief Separate header, data and footer in a previously filled list of lines.
 *
 * @param lines
 * @param header
 * @param data
 * @param footer
 *
 * @returns 0 on success, a non-null error number (i.e. an @a errno) otherwise.
 */
int
saxs_reader_columns_scan(struct line *lines,
                         struct line **header,
                         struct line **data,
                         struct line **footer);

/**
 * @brief Parse specified columns into a list of lines.
 *
 * The first data line is used as a template. If any other data
 * line has more or less data values than the first line, that
 * line is ignored.
 *
 * @param doc
 * @param firstline
 * @param lastline
 * @param scol
 * @param sfactor
 * @param icol
 * @param ifactor
 * @param errcol
 * @param title
 * @param type
 *
 * @returns 0 on success, EINVAL on invalid column selection.
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
 * @param l
 * @returns The number of data values in line @a l.
 */
int
saxs_reader_columns_count(struct line *l);

/**
 * @brief Get the data values in a given line.
 * @param l
 * @returns The data values in line @a l.
 */
double* saxs_reader_columns_values(struct line *l);

/**
 * @brief Count the number of data values in a file.
 *
 * Convenience function.
 *
 * Scans a file, splits it in header, data, footer and counts the
 * data columns.
 *
 * @param doc
 * @param firstline
 * @param lastline
 * @param parse_header
 * @param parse_data
 * @param parse_footer
 *
 * @returns
 */
int
saxs_reader_columns_parse_lines(struct saxs_document *doc,
                                struct line *firstline, struct line *lastline,
                                int (*parse_header)(struct saxs_document*,
                                                    struct line*,
                                                    struct line*),
                                int (*parse_data)(struct saxs_document*,
                                                  struct line*,
                                                  struct line*),
                                int (*parse_footer)(struct saxs_document*,
                                                    struct line*,
                                                    struct line*));

/**
 * @brief 
 *
 * @param doc
 * @param filename
 * @param write_header
 * @param write_data
 * @param write_footer
 *
 * @returns
 */
int
saxs_writer_columns_write_file(struct saxs_document *doc,
                               const char *filename,
                               int (*write_header)(struct saxs_document*,
                                                   struct line **),
                               int (*write_data)(struct saxs_document*,
                                                 struct line **),
                               int (*write_footer)(struct saxs_document*,
                                                   struct line **));


#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_COLUMNS_H */
