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

/*
 * Count the TABs to determine the number of columns
 * in a line.
 */
static int
malvern_txt_count_columns(struct line *l) {
  if (l && l->line_buffer) {
    int n = 1;
    char *c = l->line_buffer;

    while (*c)
      n += (*c++ == '\t');

    return n;

  } else
    return 0;
}


/*
 * Build a list of column names. The labels may contain spaces,
 * but the columns are TAB separated.
 */
static void
malvern_txt_parse_column_headers(struct line *l, char ***columns, int *n) {
  int i = 0;
  char *c, *buffer, *p;

  *n = malvern_txt_count_columns(l);
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

static void
malvern_txt_parse_column_values(struct line *l, double **values, int *n) {
  *n = malvern_txt_count_columns(l);
  *values = (double*)malloc(*n * sizeof(double));

  if (*n > 0) {
    int k = 0;
    char *p = l->line_buffer;

    while (*p) {
      if (sscanf(p, "%lf", &(*values)[k]) != 1)
        break;

      k += 1;

      /* Skip leading whitespace, if any. */
      while (*p && isspace(*p)) ++p;

      /* Skip the floating point value until the next separator is found. */
      while (*p && !isspace(*p)) ++p;

      /* Skip all consecutive separators up to the next value (think " , "). */
      while (*p && isspace(*p)) ++p;
    }
  }
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

  int i, j, n;
  char **headers = NULL;
  double *values;

  /*
   * Read everything into a temporary document first.
   * It was found that some files may have more data columns than 
   * header entries. However, this seems not to be extra data, but
   * simply a duplicated column. Thus, once everything is parsed
   * into the temporary document, compare the curves, drop duplicates
   * and copy unique ones into the actual document with the correct
   * header title.
   */
  struct saxs_document *tmpdoc;
  struct saxs_curve **curves;

  /* The list of column names. */
  malvern_txt_parse_column_headers(firstline, &headers, &n);
  firstline = firstline->next;

  /*
   * An oversized list of curves to accomodate for duplicate columns
   * (with temporary titles).
   */
  tmpdoc = saxs_document_create();
  curves = (struct saxs_curve **) malloc(2 * n * sizeof(struct saxs_curve*));
  for (i = 0; i < 2*n; ++i)
    curves[i] = saxs_document_add_curve(tmpdoc, "tmp",
                                        SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  /*
   * There may be missing values in the columns, but only on the right
   * hand side, i.e. not "VALUE N/A VALUE", but "VALUE VALUE N/A" only.
   *
   * Read as many values as there are, then fill them into the curves.
   * It is assumed that the first column represents the x-axis; here
   * it should be the retention volume.
   */
  while (firstline != lastline) {
    malvern_txt_parse_column_values(firstline, &values, &n);
    for (i = 1; i < n; ++i)
      saxs_curve_add_data(curves[i], values[0], 0.0, values[i], 0.0);

    free(values);
    firstline = firstline->next;
  }

  /*
   * Compare the curves, copy the unique ones to the original document.
   */
  j = 1;
  for (i = 0; i < saxs_document_curve_count(tmpdoc); ++i) {
    int duplicate = 0;

    saxs_curve *c = saxs_document_curve(doc);
    while (!duplicate && c) {
      if (saxs_curve_compare(curves[i], c) == 0)
        duplicate = 1;

      c = saxs_curve_next(c);
    }

    if (!duplicate && saxs_curve_data_count(curves[i]) > 0) {
      c = saxs_document_copy_curve(doc, curves[i]);
      saxs_curve_set_title(c, headers[j++]);
    }
  }

  saxs_document_free(tmpdoc);

  for (i = 0; i < n; ++i)
    free(headers[i]);

  free(headers);
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
    for (col = columns; *col; ++col)
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
