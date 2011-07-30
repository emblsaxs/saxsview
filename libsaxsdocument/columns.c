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

#include "columns.h"
#include "saxsdocument.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


/* Similar to the POSIX "character classification routines";
   check if the argument 'c' is a column separator. */
static int issep(int c) {
  return c == ',' || c == ';';
}


struct line* lines_create() {
  struct line *line;

  line = malloc(sizeof(struct line));
  if (line) {
    line->line_length = 80;
    line->line_buffer = malloc(line->line_length);
    line->next        = NULL;

    if (line->line_buffer) {
      memset(line->line_buffer, 0, line->line_length);

    } else {
      free(line);
      line = NULL;
    }
  }

  return line;
}


void
lines_append(struct line **lines, struct line *l) {
  if (l) {
    if (*lines) {
      struct line *tail = *lines;
      while (tail->next)
        tail = tail->next;

      tail->next = l;

    } else {
      *lines = l;
    }
  }
}


int lines_printf(struct line *l, const char *fmt, ...) {
  int n;

  va_list va;
  va_start(va, fmt);
  n = vsnprintf(l->line_buffer, l->line_length, fmt, va);
  if (n >= (signed)l->line_length) {
    l->line_length = n + 1;
    l->line_buffer = realloc(l->line_buffer, l->line_length);
    n = vsnprintf(l->line_buffer, l->line_length, fmt, va);
  }
  va_end(va);

  return n;
}


int lines_read(struct line **lines, const char *filename) {
  struct line *head, *tail;
  int c;
  char *line_ptr;

  FILE *fd = strcmp(filename, "-") ? fopen(filename, "r") : stdin;
  if (!fd)
    return errno;

  head = tail =  lines_create();
  line_ptr = tail->line_buffer;

  while (!feof(fd)) {
    c = fgetc(fd);
    switch (c) {
      case '\r':
      case EOF:
        break;

      /* Replace TAB, trim leading whitespace. */
      case ' ':
      case '\t':
        if (strlen(tail->line_buffer) > 0)
          *line_ptr++ = ' ';
        break;

      case '\n':
        *line_ptr++ = '\0';

        tail->next = lines_create();
        tail = tail->next;
        line_ptr = tail->line_buffer;
        break;

      default:
        *line_ptr++ = (char)c;
    }

    /* If necessary, increase line length. */
    if (line_ptr - tail->line_buffer == (signed)tail->line_length) {
      int old_line_length = tail->line_length;
      tail->line_length *= 2;
      tail->line_buffer = realloc(tail->line_buffer, tail->line_length);
      line_ptr = tail->line_buffer + old_line_length;
    }
  }

  if (strcmp(filename, "-"))
    fclose(fd);

  *lines = head;

  return 0;
}

int lines_write(struct line *lines, const char *filename) {
  struct line *line;

  FILE *fd = strcmp(filename, "-") ? fopen(filename, "w") : stdout;
  if (!fd)
    return errno;

  for (line = lines; line; line = line->next)
    fprintf(fd, "%s\n", line->line_buffer);

  if (strcmp(filename, "-"))
    fclose(fd);

  return 0;
}


void lines_free(struct line *lines) {
  struct line *line = lines, *oldline;

  while (line) {
    if (line->line_buffer)
      free(line->line_buffer);

    oldline = line;
    line = line->next;

    free(oldline);
  }
}


int saxs_reader_columns_count(struct line *l) {
  int cnt = 0;
  char *p;
  double value;

  if (!l || strlen(l->line_buffer) == 0)
    return 0;

  p = l->line_buffer;
  while (*p) {
    if (sscanf(p, "%lf", &value) != 1)
      break;

    cnt += 1;

    /*
     * Skip leading whitespace before skipping
     * the value up to the next whitespace.
     */
    while (*p && (isspace(*p) || issep(*p))) ++p;
    while (*p && !isspace(*p)) ++p;
  }

  return cnt;
}


int saxs_reader_columns_scan(struct line *lines, struct line **header,
                             struct line **data, struct line **footer) {

  struct line *currentline;

  /*
   * Parse all the lines and try to determine the data format
   * Heuristic:
   *   find a number of similar lines that might contain
   *   data columns, here 5 lines that contain 'colcnt'
   *   columns of floating point values.
   */
  int datalines = 0, datacolumns = 0;
  int datafound = 0;

  /*
   * Initial assumption: data only, no header, no footer
   */
  *header = *data = lines;
  *footer = NULL;

  for (currentline = lines; currentline; currentline = currentline->next) {
    int colcnt;

    /*
     * Empty lines are assumed to have the same format
     * as the previous line.
     */
    if (strlen(currentline->line_buffer) == 0) {
      if (datalines > 0)
        ++datalines;
      continue;
    }

    /*
     * Try to read everything as floating-point numbers.
     * If this succeeds, we probably have a data line.
     */
    colcnt = saxs_reader_columns_count(currentline);

    if (colcnt == 0 || (datalines > 0 && datacolumns != colcnt)) {
      if (datafound) {
        *footer = currentline;
        break;
      }

      datalines   = 0;
      datacolumns = 0;

    } else if (datalines == 0) {
      *data        = currentline;
      datalines   = 1;
      datacolumns = colcnt;

    } else {
      datalines += 1;
    }

    if (datalines > 5 && !datafound)
      datafound = 1;
  }

  return 0;
}


static int columns_parse(struct line *l, double *values) {
  int cnt = 0;
  char *p;
  double *value = values;

  if (!l || !l->line_buffer)
    return 0;

  p = l->line_buffer;
  while (*p) {
    if (sscanf(p, "%lf", value) != 1)
      break;

    cnt += 1;

    ++value;
    while (*p && (isspace(*p) || issep(*p))) ++p;
    while (*p && !isspace(*p)) ++p;
  }

  return cnt;
}

int saxs_reader_columns_parse(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline,
                              int xcol, double xfactor,
                              int ycol, double yfactor,
                              int y_errcol,
                              const char *title, int type) {

  int colcnt;
  double *values;
  struct saxs_curve *curve;

  if (firstline == lastline)
    return 0;

  /*
   * First, check that there are enough columns;
   * 'errcol' may be -1 to indicate absence of errors.
   */
  colcnt = saxs_reader_columns_count(firstline);

  if (xcol < 0
      || ycol < 0
      || (colcnt < xcol || colcnt < ycol || colcnt < y_errcol))
    return EINVAL;

  curve = saxs_document_add_curve(doc, title, type);

  values = malloc(colcnt * sizeof(double));
  while (firstline != lastline) {
    if (saxs_reader_columns_count(firstline) == colcnt) {
      columns_parse(firstline, values);
      saxs_curve_add_data (curve, values[xcol] * xfactor, 0.0,
                           values[ycol] * yfactor,
                           y_errcol >= 0 ? values[y_errcol] : 0.0);
    }

    firstline = firstline->next;
  }
  free(values);

  return 0;
}


int saxs_reader_columns_count_file(const char *filename) {
  struct line *lines, *header, *data, *footer;
  int count, res;

  if ((res = lines_read(&lines, filename)) != 0)
    goto error;

  if ((res = saxs_reader_columns_scan(lines, &header, &data, &footer)) != 0)
    goto error;

  count = saxs_reader_columns_count(data);

  lines_free(lines);
  return count;

error:
  lines_free(lines);
  return -res;
}


int saxs_reader_columns_parse_file(struct saxs_document *doc,
                                   const char *filename,
                                   int (*parse_header)(struct saxs_document*,
                                                       struct line *,
                                                       struct line *),
                                   int (*parse_data)(struct saxs_document*,
                                                     struct line *,
                                                     struct line *),
                                   int (*parse_footer)(struct saxs_document*,
                                                       struct line *,
                                                       struct line *)) {
  int res;
  struct line *lines, *header, *data, *footer;

  if ((res = lines_read(&lines, filename)) != 0)
    goto error;

  if ((res = saxs_reader_columns_scan(lines, &header, &data, &footer)) != 0)
    goto error;

  if ((res = parse_header && parse_header(doc, header, data)) != 0)
    goto error;

  if ((res = parse_data && parse_data(doc, data, footer)) != 0)
    goto error;

  if ((res = parse_footer && parse_footer(doc, footer, NULL)) != 0)
    goto error;

  lines_free(lines);
  return 0;

error:
  lines_free(lines);
  return res;
}


int saxs_writer_columns_write_file(struct saxs_document *doc,
                                   const char *filename,
                                   int (*write_header)(struct saxs_document*,
                                                       struct line **),
                                   int (*write_data)(struct saxs_document*,
                                                     struct line **),
                                   int (*write_footer)(struct saxs_document*,
                                                       struct line **)) {
  int res;
  struct line *lines = NULL;

  if ((res = write_header && write_header(doc, &lines)) != 0)
    goto error;

  if ((res = write_data && write_data(doc, &lines)) != 0)
    goto error;

  if ((res = write_footer && write_footer(doc, &lines)) != 0)
    goto error;

  if ((res = lines_write(lines, filename)) != 0)
    goto error;

  lines_free(lines);
  return 0;

error:
  lines_free(lines);
  return res;
}
