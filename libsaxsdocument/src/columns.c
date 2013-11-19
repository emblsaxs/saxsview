/*
 * Common code to read columnized data in text files.
 * Copyright (C) 2009, 2011, 2012, 2013
 *  Daniel Franke <dfranke@users.sourceforge.net>
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


/*
 * Similar to the POSIX "character classification routines";
 * check if the argument 'c' is a column separator.
 * As whitespaces usually are column separators as well, include them here.
 */
static int issep(int c) {
  return isspace(c) || c == ',' || c == ';';
}


struct line* lines_create() {
  struct line *line;

  line = malloc(sizeof(struct line));
  if (line) {
    line->line_column_count = -1;
    line->line_length       = 80;
    line->line_buffer       = malloc(line->line_length);
    line->next              = NULL;

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

  /*
   * In case the line buffer is also used in the argument list,
   * write to a temporary location to avoid messing up the
   * original line buffer.
   */
  char *buffer = malloc(l->line_length);

  va_list va;
  va_start(va, fmt);

  n = vsnprintf(buffer, l->line_length, fmt, va);
  if (n >= (signed)l->line_length) {
    va_end(va);

    l->line_length = n + 1;
    buffer = realloc(buffer, l->line_length);

    va_start(va, fmt);
    n = vsnprintf(buffer, l->line_length, fmt, va);
  }
  va_end(va);

  free(l->line_buffer);
  l->line_buffer       = buffer;
  l->line_column_count = -1;

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

      /*
       * Trim leading whitespace and hash symbols
       * (the latter are often used as 'comment' indicators).
       */
      case ' ':
      case '\t':
      case '#':
        if (strlen(tail->line_buffer) > 0)
          *line_ptr++ = (char)c;
        break;

      case '\n':
        /*
         * Filling the current position with a 'space' instead of '\0'
         * makes trimming any trailing whitespace simpler as '\0' does
         * not need to be handled specifically.
         */
        *line_ptr = ' ';

        /* Trim trailing whitespace. */
        while (line_ptr > tail->line_buffer && isspace(*line_ptr))
          *line_ptr-- = '\0';

        tail->next = lines_create();
        tail = tail->next;
        line_ptr = tail->line_buffer;
        break;

      default:
        *line_ptr++ = (char)c;
    }

    /* If necessary, increase line length. */
    if (line_ptr - tail->line_buffer == (signed)tail->line_length) {
      char *old_line = tail->line_buffer;
      int old_line_length = tail->line_length;
     
      /*
       * One could use realloc() here, but that leaves the trailing bytes uninitialized
       * (which may throw off string searches later).
       */
      tail->line_length *= 2;
      tail->line_buffer = calloc(sizeof(char), tail->line_length);
      strncpy(tail->line_buffer, old_line, old_line_length);
      free(old_line);

      line_ptr = tail->line_buffer + old_line_length;
    }
  }

  if (strcmp(filename, "-"))
    fclose(fd);

  *lines = head;

  return 0;
}

int lines_write(struct line *lines, const char *filename) {
  int res = 0;
  struct line *line;

  FILE *fd = strcmp(filename, "-") ? fopen(filename, "w") : stdout;
  if (fd) {
    for (line = lines; line; line = line->next)
      if (fprintf(fd, "%s\n", line->line_buffer) < 0) {
        res = errno;
        break;
      }

    if (strcmp(filename, "-"))
      fclose(fd);

  } else
    res = errno;

  return res;
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

  /* If we already counted the columns before, use the cached value. */
  if (l->line_column_count >= 0)
    return l->line_column_count;

  p = l->line_buffer;
  while (*p) {
    if (sscanf(p, "%lf", &value) != 1)
      break;

    cnt += 1;

    /* Skip leading whitespace, if any. */
    while (*p && isspace(*p)) ++p;

    /* Skip the floating point value until the next separator is found. */
    while (*p && !issep(*p)) ++p;
    
    /* Skip all consecutive separators up to the next value (think " , "). */
    while (*p && issep(*p)) ++p;
  }

  /* Skip any trailing whitespace up to either a character or end of line. */
  while (*p && isspace(*p)) ++p;

  /*
   * If a line starts with one or more numbers which is followed by text,
   * then this is not a data line. A data line contains only numbers and
   * whitespaces.
   */
  l->line_column_count = *p ? 0 : cnt;

  return l->line_column_count;
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
      if (!datafound) {
        /* Reject the current preliminary data block and start a new one. */
        *data        = currentline;
        datalines   = 1;
        datacolumns = colcnt;

      } else {
        /*
         * A sufficiently large data block has been found.
         * Any remaining lines are part of the footer by definition.
         */
        *footer = currentline;
        break;
      }

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

    /* Skip leading whitespace, if any. */
    while (*p && isspace(*p)) ++p;

    /* Skip the floating point value until the next separator is found. */
    while (*p && !issep(*p)) ++p;
    
    /* Skip all consecutive separators up to the next value (think " , "). */
    while (*p && issep(*p)) ++p;
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


int saxs_reader_columns_parse_lines(struct saxs_document *doc,
                                    struct line *firstline, struct line *lastline,
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
  struct line *header = NULL, *data = NULL, *footer = NULL;

  if ((res = saxs_reader_columns_scan(firstline, &header, &data, &footer)) != 0)
    return res;

  /*
   * If we can't identify anything, then this is probably not
   * columnized data.
   */
  if (!header && !data && !footer)
    return ENOTSUP;

  if (parse_header && (res = parse_header(doc, header, data) != 0))
    return res;

  if (parse_data && (res = parse_data(doc, data, footer) != 0))
    return res;

  if (parse_footer && (res = parse_footer(doc, footer, lastline) != 0))
    return res;

  return 0;
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
