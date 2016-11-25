/*
 * Common code to read columnized data in text files.
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

#include "columns.h"
#include "saxsdocument.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <assert.h>

/*
 * Similar to the POSIX "character classification routines";
 * check if the argument 'c' is a column separator.
 * As whitespaces usually are column separators as well, include them here.
 */
static int issep(int c) {
  return isspace(c) || c == ',' || c == ';';
}

static int is_utf8(const unsigned char *data) {
  return data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF;
}

static int is_utf16_le(const unsigned char *data) {
  return data[0] == 0xFE && data[1] == 0xFF;
}

static int is_utf16_be(const unsigned char *data) {
  return data[0] == 0xFF && data[1] == 0xFE;
}

static int is_utf32_le(const unsigned char *data) {
  return data[0] == 0x00 && data[1] == 0x00 && data[2] == 0xFE && data[3] == 0xFF;
}

static int is_utf32_be(const unsigned char *data) {
  return data[0] == 0xFF && data[1] == 0xFE && data[2] == 0x00 && data[3] == 0x00;
}

#ifdef LIBSAXSDOCUMENT_HEAVY_ASSERTS

void assert_valid_lineset(const struct line* first, const struct line* last) {
  assert_valid_line(first);
  assert_valid_line_or_null(last);
  const struct line *current = first;
  while (current != last) {
    assert_valid_line(current);
    current = current->next;
  }
}

#define assert_valid_tokenised_line(l) { \
  assert_valid_line(l); \
  assert(l->line_column_count >= 0); \
}

#else

#define assert_valid_tokenised_line(l)

#endif

static int columns_tokenize(struct line *l);

struct line* lines_create() {
  struct line *line;

  line = malloc(sizeof(struct line));
  if (line) {
    line->line_column_count  = -1;
    line->line_column_values = NULL;
    line->line_length        = 80;
    line->line_buffer        = malloc(line->line_length);
    line->next               = NULL;

    if (line->line_buffer) {
      memset(line->line_buffer, 0, line->line_length);

    } else {
      free(line);
      line = NULL;
    }
  }

  assert_valid_line_or_null(line);
  return line;
}


void
lines_append(struct line **lines, struct line *l) {
  assert_valid_lineset_or_null(*lines);
  assert_valid_line_or_null(l);
  if (l) {
    if (*lines) {
      struct line *tail = *lines;
      while (tail->next)
        tail = tail->next;

      tail->next = l;

    } else {
      *lines = l;
    }
    assert_valid_lineset(*lines, l);
  }
}


int lines_printf(struct line *l, const char *fmt, ...) {
  assert_valid_line(l);
  int n;
  char *line_buffer = 0L;
  size_t line_length = l->line_length;

  va_list va;

  while (1) {
    /*
     * In case the line buffer is also used in the argument list,
     * write to a temporary location to avoid messing up the
     * original line buffer.
     */
    line_buffer = realloc(line_buffer, line_length);
    if (!line_buffer)
      return -ENOMEM;

    va_start(va, fmt);
    n = vsnprintf(line_buffer, line_length, fmt, va);
    va_end(va);

    if (n >= 0 && n < (signed) line_length)
      break;

    /*
     * The meaning of the return value depends on the underlying, platform
     * dependent, C runtime:
     *  - on UNIX, n >= line_length means 'a line_length of at least n+1 is
     *    needed'
     *  - on WINDOWS, n < 0 indicates 'buffer too small' without any further
     *    indication of the required size.
     *
     * Instead of doing any platform specifc things, simply keep
     * doubling the line_length until it is either large enough or we
     * run out of memory.
     */
    line_length = line_length * 2;
  }

  l->line_length = line_length;
  free(l->line_buffer);
  l->line_buffer = line_buffer;

  l->line_column_count = -1;
  if (l->line_column_values) {
    free(l->line_column_values);
    l->line_column_values = NULL;
  }

  assert_valid_line(l);
  return n;
}

int lines_read(struct line **lines, const char *filename) {
  struct line *head, *tail;
  int c;
  int retcode = 0;
  char *line_ptr;

  FILE *fd = strcmp(filename, "-") ? fopen(filename, "r") : stdin;
  if (!fd) {
    return errno;
  }

  head = tail = lines_create();
  if (!head) {
    retcode = ENOMEM;
    goto read_fail;
  }

  line_ptr = tail->line_buffer;

  while (!feof(fd)) {
    c = fgetc(fd);
    switch (c) {
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

      case '\r':
        /* Check if the next character is '\n'
         * If so, this is a \r\n line ending
         * so the \n should be ignored 
         */
        c = fgetc(fd);
        if (c != '\n') {
          ungetc(c, fd);
        }
        /* Deliberate fall-through (no break):
         * Handle the newline as if a '\n' had been found
         */
      case '\n':
        /*
         * Filling the current position with a 'space' instead of '\0'
         * makes trimming any trailing whitespace simpler as '\0' does
         * not need to be handled specifically.
         */
        *line_ptr = ' ';

        /* Trim trailing whitespace. */
        while (line_ptr >= tail->line_buffer && isspace(*line_ptr))
          *line_ptr-- = '\0';

        /* Tokenise the line here so it can later be used as const */
        retcode = columns_tokenize(tail);
        if (retcode != 0) {
          goto read_fail;
        }

        tail->next = lines_create();
        if (tail->next == NULL) {
          retcode = ENOMEM;
          goto read_fail;
        }
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
      if (!(tail->line_buffer)){
        free(old_line);
        retcode = errno;
        goto read_fail;
      }
      strncpy(tail->line_buffer, old_line, old_line_length);
      free(old_line);

      line_ptr = tail->line_buffer + old_line_length;
    }
  }
  /* Tokenise the line here so it can later be used as const */
  retcode = columns_tokenize(tail);
  if (retcode != 0) {
    goto read_fail;
  }

  /*
   * Check if we have a unicode file. We can deal with UTF-8,
   * although some text may be garbled.
   *
   * Error out on all other unicode formats for now.
   * Later we may fully support unicode, if needed.
   */
  if (    is_utf16_le((unsigned char*)head->line_buffer)
       || is_utf16_be((unsigned char*)head->line_buffer)
       || is_utf32_le((unsigned char*)head->line_buffer)
       || is_utf32_be((unsigned char*)head->line_buffer)) {

    retcode = EILSEQ;
    goto read_fail;

  } else if (is_utf8((unsigned char*)head->line_buffer)) {
    /* Do nothing? */
  }

  if (strcmp(filename, "-"))
    fclose(fd);

  *lines = head;
  assert_valid_lineset(*lines, NULL);
  return 0;

read_fail:
  lines_free(head);
  if (strcmp(filename, "-")) {
    fclose(fd);
  }
  if (retcode == 0) {
    retcode = errno;
  }
  return retcode;
}

int lines_write(const struct line *lines, const char *filename) {
  assert_valid_lineset(lines, NULL);
  int res = 0;
  const struct line *line;

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
  assert_valid_lineset_or_null(lines);
  struct line *line = lines, *oldline;

  while (line) {
    if (line->line_buffer)
      free(line->line_buffer);

    if (line->line_column_values)
      free(line->line_column_values);

    oldline = line;
    line = line->next;

    free(oldline);
  }
}


static int columns_tokenize(struct line *l) {
  assert_valid_line(l);
  char *p;
  double value;
  double *values = NULL;
  int nreserved = 0, nvalues = 0;

  if (l->line_column_count >= 0)
    return 0;

  p = l->line_buffer;
  while (*p) {
    if (sscanf(p, "%lf", &value) != 1)
      break;

    /* 
     * Reject any value that is not finite, i.e. NAN or INF, as hardly
     * anything can deal with those anyway.
     */
    if (!isnan(value) && !isinf(value)) {
      /*
       * This possibly reserves more memory than needed for a given
       * line, but the benefit is a severely reduced number of
       * malloc/free compared to an increase-length-by-one approach.
       */
      if (nvalues + 1 > nreserved) {
        nreserved = nreserved + 4;
        values = realloc(values, nreserved * sizeof(double));
        if (!values)
          return ENOMEM;
      }

      values[nvalues] = value;
      nvalues += 1;

    } else {
      /* Reset the line, cleanup done below. */
      p = l->line_buffer;
      break;
    }

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
  if (*p) {
    l->line_column_count = 0;
    l->line_column_values = NULL;
    free(values);

  } else {
    l->line_column_count  = nvalues;
    l->line_column_values = values;
  }
  assert_valid_tokenised_line(l);
  return 0;
}


int saxs_reader_columns_count(const struct line *l) {
  assert_valid_tokenised_line(l);

  return l->line_column_count;
}


const double* saxs_reader_columns_values(const struct line *l) {
  assert_valid_tokenised_line(l);

  return l->line_column_values;
}


int saxs_reader_columns_scan(const struct line *lines,
                             const struct line **header,
                             const struct line **data,
                             const struct line **footer) {

  assert_valid_lineset(lines, NULL);
  const struct line *currentline, *tmpdata;

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
  *header = tmpdata = lines;
  *data = *footer = NULL;

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
        tmpdata     = currentline;
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
      tmpdata     = currentline;
      datalines   = 1;
      datacolumns = colcnt;

    } else {
      datalines += 1;
    }

    if (datalines > 5 && !datafound)
      datafound = 1;
  }

  if (datafound)
    *data = tmpdata;

  return 0;
}

int saxs_reader_columns_parse(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline,
                              int xcol, double xfactor,
                              int ycol, double yfactor,
                              int y_errcol,
                              const char *title, int type) {

  assert_valid_lineset(firstline, lastline);
  int colcnt;
  const double *values;
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
      || (colcnt <= xcol || colcnt <= ycol || colcnt <= y_errcol))
    return EINVAL;

  curve = saxs_document_add_curve(doc, title, type);
  if (!curve) {return ENOMEM;}

  while (firstline != lastline) {
    if (saxs_reader_columns_count(firstline) == colcnt) {
      values = saxs_reader_columns_values(firstline);
      if (!values) {return ENOMEM;}
      saxs_curve_add_data (curve, values[xcol] * xfactor, 0.0,
                           values[ycol] * yfactor,
                           y_errcol >= 0 ? values[y_errcol] : 0.0);
    }

    firstline = firstline->next;
  }

  return 0;
}


int saxs_reader_columns_parse_lines(struct saxs_document *doc,
                                    const struct line *firstline,
                                    const struct line *lastline,
                                    int (*parse_header)(struct saxs_document*,
                                                        const struct line *,
                                                        const struct line *),
                                    int (*parse_data)(struct saxs_document*,
                                                      const struct line *,
                                                      const struct line *),
                                    int (*parse_footer)(struct saxs_document*,
                                                        const struct line *,
                                                        const struct line *)) {

  assert_valid_lineset(firstline, lastline);
  int res;
  const struct line *header = NULL, *data = NULL, *footer = NULL;

  if ((res = saxs_reader_columns_scan(firstline, &header, &data, &footer)) != 0)
    return res;

  /*
   * If we can't identify anything, then this is probably not
   * columnized data.
   */
  if (!header && !data && !footer)
    return ENOTSUP;

  if (parse_header && header && (res = parse_header(doc, header, data) != 0))
    return res;

  if (parse_data && data && (res = parse_data(doc, data, footer) != 0))
    return res;

  if (parse_footer && footer && (res = parse_footer(doc, footer, lastline) != 0))
    return res;

  return 0;
}


int saxs_writer_columns_write_lines(struct saxs_document *doc, struct line **lines,
                                    int (*write_header)(struct saxs_document*,
                                                        struct line **),
                                    int (*write_data)(struct saxs_document*,
                                                      struct line **),
                                    int (*write_footer)(struct saxs_document*,
                                                        struct line **)) {
  int res = 0;
  struct line *l = NULL;

  if (res == 0 && write_header)
    res = write_header(doc, &l);

  if (res == 0 && write_data)
    res = write_data(doc, &l);

  if (res == 0 && write_footer)
    res = write_footer(doc, &l);

  if (res == 0) {
    assert_valid_lineset(l, NULL);
    *lines = l;
  } else {
    lines_free(l);
  }
  return res;
}
