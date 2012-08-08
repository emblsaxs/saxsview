/*
 * Read files in .fir/.fit-formats (e.g. written by DAMMIN, OLIGOMER, ...).
 * Copyright (C) 2009, 2010, 2011
 * Daniel Franke <dfranke@users.sourceforge.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

/**************************************************************************/
static int
atsas_fir_fit_parse_header(struct saxs_document *doc,
                           struct line *firstline, struct line *lastline) {
  /*
   * .fir-files may have a 'title', but we simply ignore any
   * information that might be available for now ...
   */
  return 0;
}

static int
atsas_fir_fit_parse_footer(struct saxs_document *doc,
                           struct line *firstline, struct line *lastline) {
  /*
   * This should be empty?
   */
  return 0;
}


/**************************************************************************/
static int
atsas_fir_4_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  if (saxs_reader_columns_count(firstline) != 4)
    return ENOTSUP;

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 1, 1.0, 2, "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 3, 1.0, -1, "fit",
                            SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

  return 0;
}

int
atsas_fir_4_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fir_4_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}


/**************************************************************************/
static int
atsas_fit_3_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  if (saxs_reader_columns_count(firstline) != 3)
    return ENOTSUP;

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 1, 1.0, -1, "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 2, 1.0, -1, "fit",
                            SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

  return 0;
}

int
atsas_fit_3_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fit_3_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}


/**************************************************************************/
static int
atsas_fit_4_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 1, 1.0, 2, "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 3, 1.0, -1, "fit",
                            SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

  return 0;
}

static int
atsas_fit_4_column_parse_monsa_data(struct saxs_document *doc,
                                    struct line *header,
                                    struct line *data,
                                    struct line *footer) {
  struct line *nextfit;

  while (header) {
    char filename[PATH_MAX] = { '\0' }, *p, *q;
    char label[PATH_MAX + 6] = { '\0' };

    /*
     * The first header has at least two lines, every following only one.
     */
    while (header->next != data)
      header = header->next;

    /*
     * Sections are preceded by
     *   "File arc1p_mer.dat   Chi:   3.470 Weight: 1.000 RelSca: 0.396"
     */
    p = strstr(header->line_buffer, "File");
    q = strstr(header->line_buffer, "Chi");
    if (!p || !q)
      break;

    /* Grab the filename ... */
    strncpy(filename, p + 4, q - p - 4);

    /* ... and trim leading ... */
    p = filename;
    while (isspace(*p)) ++p;

    /* ... and trailing whitespace. */
    q = filename + sizeof(filename) - 1;
    while (q > p && !isalnum(*q))
      *q-- = '\0';

    sprintf(label, "%s, data", p);
    saxs_reader_columns_parse(doc, data, footer,
                              0, 1.0, 1, 1.0, 2, label,
                              SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

    sprintf(label, "%s, fit", p);
    saxs_reader_columns_parse(doc, data, footer,
                              0, 1.0, 3, 1.0, -1, label,
                              SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

    /* Additional fits are listed in the "footer". */
    if (!footer)
      break;

    nextfit = footer;
    if (saxs_reader_columns_scan(nextfit, &header, &data, &footer))
      break;
  }

  return 0;
}

int
atsas_fit_4_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {

  int res;
  struct line *header = NULL, *data = NULL, *footer = NULL;

  if ((res = saxs_reader_columns_scan(firstline, &header, &data, &footer)) != 0)
    goto error;

  if (saxs_reader_columns_count(data) != 4) {
    res = ENOTSUP;
    goto error;
  }

  /*
   * Check the first line if it has "MONSA" - if yes, this is a .fit file
   * with (possibly) multiple fits stacked over each other.
   */
  if (strstr(firstline->line_buffer, "MONSA"))
    res = atsas_fit_4_column_parse_monsa_data(doc, header, data, footer);
  else
    res = atsas_fit_4_column_parse_data(doc, data, footer);

error:
  return res;
}


/**************************************************************************/
static int
atsas_fit_5_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  if (saxs_reader_columns_count(firstline) != 5)
    return ENOTSUP;

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 1, 1.0, 3, "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 2, 1.0, -1, "fit",
                            SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

  return 0;
}

int
atsas_fit_5_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fit_5_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}


/**************************************************************************/
void
saxs_document_format_register_atsas_fir_fit() {
  /*
   * Generally, .fit-files come with 3 columns (s, I, Ifit) and
   * .fir-files with 4 columns (s, I, err, Ifit). However, SASREF
   * writes .fit-files with 4 columns (identical to .fir-files
   * for other apps).
   *
   * To make matters even more fun, MONSA writes the same kind of .fit
   * files as SASREF in terms of columns, but stacks multiple fits
   * above each other.
   *
   * Further, OLIGOMER seems to write files with a fifth column (the
   * difference of I and Ifit). Also, the column order is different
   * (s, I, Ifit, err, diff).
   */
  saxs_document_format atsas_fir_4_column = {
     "fir", "atsas-fir-4-column",
     "ATSAS fit against experimental data",
     atsas_fir_4_column_read, NULL, NULL
  };

  saxs_document_format atsas_fit_3_column = {
     "fit", "atsas-fit-3-column",
     "ATSAS fit against data (3 column; DAMMIN, DAMMIF, ...)",
     atsas_fit_3_column_read, NULL, NULL
  };

  saxs_document_format atsas_fit_4_column = {
     "fit", "atsas-fit-4-column",
     "ATSAS fit against data (4 column; SASREF, ...)",
     atsas_fit_4_column_read, NULL, NULL
  };

  saxs_document_format atsas_fit_5_column = {
     "fit", "atsas-fit-5-column",
     "ATSAS fit against data (5 column; OLIGOMER, ...)",
     atsas_fit_5_column_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_fir_4_column);
  saxs_document_format_register(&atsas_fit_3_column);
  saxs_document_format_register(&atsas_fit_4_column);
  saxs_document_format_register(&atsas_fit_5_column);
}
