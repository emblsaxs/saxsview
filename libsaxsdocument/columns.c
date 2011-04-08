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

#include "columns.h"
#include "saxsdocument.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int lines_read(struct line **lines, const char *filename) {
  struct line *head, *tail;
  int c;
  char *line_ptr;

  FILE *fd = fopen(filename, "r");
  if (!fd)
    return -1;

  head = tail = malloc(sizeof(struct line));
  head->line_number = 1;
  head->line_length = 80;
  head->line_buffer = line_ptr = malloc(head->line_length);
  memset(head->line_buffer, 0, head->line_length);
  head->next   = NULL;

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

        tail->next = malloc(sizeof(struct line));
        tail->next->line_number = tail->line_number + 1;

        tail = tail->next;
        tail->line_length = 80;
        tail->line_buffer = line_ptr = malloc(tail->line_length);
        memset(tail->line_buffer, 0, tail->line_length);
        tail->next = NULL;
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

  fclose(fd);
  *lines = head;

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
  char *p = l->line_buffer;
  double value;

  while (*p) {
    if (sscanf(p, "%lf", &value) != 1)
      break;

    cnt += 1;

    /*
     * Skip leading whitespace before skipping 
     * the value up to the next whitespace.
     */
    while (*p && isspace(*p)) ++p;
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
  char *p = l->line_buffer;
  double *value = values;

  while (*p) {
    if (sscanf(p, "%lf", value) != 1)
      break;

    cnt += 1;

    ++value;
    while (*p && isspace(*p)) ++p;
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
    return -1;

  curve = saxs_document_add_curve(doc, title, type);

  values = malloc(colcnt * sizeof(double));
  while (firstline != lastline) {
    if (columns_parse(firstline, values) == colcnt)
      saxs_curve_add_data (curve, values[xcol] * xfactor, 0.0,
                           values[ycol] * yfactor,
                           y_errcol >= 0 ? values[y_errcol] : 0.0);

    firstline = firstline->next;
  }
  free(values);

  return 0;
}


int saxs_reader_columns_count_file(const char *filename) {
  struct line *lines, *header, *data, *footer;
  int count;

  if (lines_read(&lines, filename) != 0)
    return -1;

  if (saxs_reader_columns_scan(lines, &header, &data, &footer) != 0)
    return -1;

  count = saxs_reader_columns_count(data);

  lines_free(lines);
  return count;
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
  struct line *lines, *header, *data, *footer;

  if (lines_read(&lines, filename) != 0)
    return -1;

  if (saxs_reader_columns_scan(lines, &header, &data, &footer) != 0)
    return -1;

  parse_header(doc, header, data);
  parse_data(doc, data, footer);
  parse_footer(doc, footer, NULL);

  lines_free(lines);
  return 0;
}