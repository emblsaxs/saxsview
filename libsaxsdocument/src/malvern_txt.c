/*
 * Copyright (C) 2012, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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
#include "saxsdocument_format.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#ifndef DBL_EPSILON
#define DBL_EPSILON 1e-16
#endif


/*
 * Build a list of column names. The labels may contain spaces,
 * but the columns are TAB separated.
 */
static void
malvern_txt_parse_column_headers(struct line *l, char ***columns, int *n) {
  int i = 0;
  char *c, *buffer, *p;

  /*
   * Count the TABs to allocate the number of entries in
   * the 'columns' vector.
   */
  *n = 1;
  c = l->line_buffer;
  while (*c)
    *n += (*c++ == '\t');

  *columns = (char**)malloc(*n * sizeof(char*));

  /* No column label is larger than the whole header string. */
  buffer = (char*)malloc((l->line_length + 1) * sizeof(char));

  c = l->line_buffer;
  for (i = 0; i < *n; ++i, ++c) {
    memset(buffer, 0, l->line_length + 1);
    p = buffer;

    while (*c != '\t' && *c != '\0')
      *p++ = *c++;

    (*columns)[i] = strdup(buffer);
  }

  free(buffer);
}

static int
malvern_txt_parse_column_values(struct line *l, double *values, int n) {
  int cnt = 0;
  char *p;

  if (!l || !l->line_buffer)
    return 0;

  p = l->line_buffer;
  while (*p) {
    assert(cnt < n);

    if (sscanf(p, "%lf", &values[cnt]) != 1)
      break;

    /*
     * It was found that some files may have more data columns than 
     * header entries. However, this seems not to be extra data, but
     * simply a duplicated column. Thus, if two or more consecutive
     * values are exactly identical, we ignore every one but the first.
     */
    if (cnt == 0 || fabs(values[cnt-1] - values[cnt]) > DBL_EPSILON)
      cnt += 1;

    /* Skip leading whitespace, if any. */
    while (*p && isspace(*p)) ++p;

    /* Skip the floating point value until the next separator is found. */
    while (*p && !isspace(*p)) ++p;

    /* Skip all consecutive separators up to the next value (think " , "). */
    while (*p && isspace(*p)) ++p;
  }

  return cnt;
}


/**************************************************************************/
static int
malvern_txt_parse_header(struct saxs_document *doc,
                         struct line *firstline, struct line *lastline) {

  /* TODO: Anything useful here? Ignore this for now. */

  return 0;
}

static int
malvern_txt_parse_data(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {
  int i, n, m;
  char **headers = NULL;
  struct saxs_curve **curves;
  double *values;

  /* The list of column names. */
  malvern_txt_parse_column_headers(firstline, &headers, &n);
  firstline = firstline->next;

  /* The list of curves. */
  curves = (struct saxs_curve **) malloc(n * sizeof(struct saxs_curve*));
  for (i = 0; i < n; ++i)
    curves[i] = saxs_document_add_curve(doc, headers[i],
                                        SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  /*
   * There may be missing values in the columns, but only on the right
   * hand side, i.e. not "VALUE N/A VALUE", but "VALUE VALUE N/A" only.
   *
   * Read as many values as there are, then fill them into the curves.
   * It is assumed that the first column represents the x-axis; here
   * it should be the retention volume.
   */
  values = (double*) malloc(n * sizeof(double));
  while (firstline != lastline) {
    m = malvern_txt_parse_column_values(firstline, values, n);
    for (i = 1; i < m; ++i)
      saxs_curve_add_data(curves[i], values[0], 0.0, values[i], 0.0);

    firstline = firstline->next;
  }

  for (i = 0; i < n; ++i)
    free(headers[i]);

  free(headers);
  free(values);
  free(curves);

  return 0;
}

int
malvern_txt_read(struct saxs_document *doc,
                 struct line *firstline, struct line *lastline) {

  static const char* columns[] = {
    "Ret. Vol.", "RI", "RALS", "UV",
    "Adjusted RI", "Adjusted RALS", "Adjusted UV",
    "Molecular Weight", "Conc.", NULL
  };
  const char **col;

  struct line *header, *data, *footer;

  /*
   * The header starts at the first line and ends when the data begins with
   * a number of column labels. The columns are not fixed an depend on the
   * settings for the analysis. Try to find at least four of the known
   * column labels in one line to decide that this is the start of the data.
   */
  header = firstline;
  data   = header;
  while (data) {
    int count = 0;
    for (col = columns; *col; *col++)
      if (strstr(data->line_buffer, *col) != NULL)
        count += 1;

    if (count >= 4)
      break;

    data = data->next;
  }

  /* There is no spoon, sorry, footer. */
  footer = lastline;

  /*
   * If the beginning of the data was not found,
   * this is not a malvern file.
   */
  if (!data)
    return ENOTSUP;

  malvern_txt_parse_header(doc, header, data);
  malvern_txt_parse_data(doc, data, footer);

  return 0;
}


/**************************************************************************/
void
saxs_document_format_register_malvern_txt() {
  saxs_document_format malvern_txt = {
     "txt", "malvern-txt",
     "Data from Malvern OmniSEC text files.",
     malvern_txt_read, NULL, NULL
  };

  saxs_document_format_register(&malvern_txt);
}
