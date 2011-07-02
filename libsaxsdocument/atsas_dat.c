/*
 * Read files in .dat-format (used by EMBL-Hamburg).
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
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
#include <string.h>

#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


/**************************************************************************/
static int
atsas_dat_parse_header(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {

  /*
   * The first non-empty line is the 'title' of the file.
   * Titles are often formatted as:
   *   "text with whitespaces: filename(s)"
   *
   * If available, the filenames usually correspond to the parents
   * of the current file.
   *
   * Historically, filenames are separated by whitespaces ^^
   */
  while (firstline != lastline && strlen(firstline->line_buffer) == 0)
    firstline = firstline->next;

  if (firstline != lastline) {
    saxs_document_add_property(doc, "title", firstline->line_buffer);
    firstline = firstline->next;
  }

  /*
   * If the file is a raw data file, then the second non-empty line
   * holds the description, the code and the sample concentration.
   *
   * Example:
   *    "Sample:           water  c=  0.000 mg/ml Code:      h2o"
   *
   * Here, "water" is the description, "h2o" the code and "0.000"
   * the concentration in mg/ml.
   *
   * The description may contain whitespaces, thus, anything between
   * the first ':' and the last 'c=' is assumed to be the description.
   */
  while (firstline != lastline && strlen(firstline->line_buffer) == 0)
    firstline = firstline->next;

  if (firstline != lastline) {
    char *colon_pos = strchr(firstline->line_buffer, ':');
    char *conc_pos  = strstr(firstline->line_buffer, "c=");

    if (conc_pos) {
      char desc[64] = { '\0' }, code[64] = { '\0' }, conc[64] = { '\0' };

      if (colon_pos)
        strncpy(desc, colon_pos + 1,
                MIN(conc_pos - colon_pos - 1, 64));

      /* Skip "c=". */
      sscanf(conc_pos + 2, "%s", conc);

      colon_pos = strstr(conc_pos, ":");
      if (colon_pos)
        strncpy(code, colon_pos + 1,
                MIN(64, firstline->line_length -
                    (colon_pos - firstline->line_buffer - 1)));

      saxs_document_add_property(doc, "sample-description", desc);
      saxs_document_add_property(doc, "sample-concentration", conc);
      saxs_document_add_property(doc, "sample-code", code);
    }

    firstline = firstline->next;
  }

  /*
   * All other lines/information (if any) are ignored for now.
   */

  return 0;
}

static int
atsas_dat_parse_footer(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {
  /*
   * Anything useful here?
   */
  return 0;
}


/**************************************************************************/
static int
atsas_dat_3_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse(doc,
                                   firstline, lastline, 
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
atsas_dat_3_column_check(const char *filename) {
  return saxs_reader_columns_count_file(filename) == 3;
}

int
atsas_dat_3_column_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        atsas_dat_parse_header,
                                        atsas_dat_3_column_parse_data,
                                        atsas_dat_parse_footer);
}


/**************************************************************************/
static int
atsas_dat_4_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse(doc,
                                   firstline, lastline, 
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
atsas_dat_4_column_check(const char *filename) {
  return saxs_reader_columns_count_file(filename) == 4;
}

int
atsas_dat_4_column_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        atsas_dat_parse_header,
                                        atsas_dat_4_column_parse_data,
                                        atsas_dat_parse_footer);
}


/**************************************************************************/
static int
atsas_dat_n_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  int i, n = saxs_reader_columns_count(firstline);

  for (i = 1; i < n; ++i)
    if (saxs_reader_columns_parse(doc,
                                  firstline, lastline, 
                                  0, 1.0, i, 1.0, -1,
                                  "data",
                                  SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA))
      return -1;

  return 0;
}

int
atsas_dat_n_column_check(const char *filename) {
  const int n = saxs_reader_columns_count_file(filename);
  return n == 0 || n > 1;
}

int
atsas_dat_n_column_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        atsas_dat_parse_header,
                                        atsas_dat_n_column_parse_data,
                                        atsas_dat_parse_footer);
}


/**************************************************************************/
void
saxs_document_format_register_atsas_dat() {
  /*
   * ATSAS .dat files come in multiple flavours.
   * There are files with three columns (s, I, poisson-error), four
   * columns (s, I, poisson-error, gaussian-error) and N columns,
   * including N=3 and N=4, without any errors (s, I1, ..., IN).
   *
   * The N-column case is often used as input file for programs
   * like OLIGOMER.
   */
  saxs_document_format atsas_dat_3_column = {
     "dat", "atsas-dat-3-column",
     "ATSAS experimental data, one data set with Poisson errors",
     atsas_dat_3_column_check, atsas_dat_3_column_read, NULL, NULL
  };

  saxs_document_format atsas_dat_4_column = {
     "dat", "atsas-dat-4-column",
     "ATSAS experimental data, one data set with Poisson and Gaussian errors",
     atsas_dat_4_column_check, atsas_dat_4_column_read, NULL, NULL
  };

  saxs_document_format atsas_dat_n_column = {
     "dat", "atsas-dat-n-column",
     "ATSAS experimental data, multiple data sets, no errors",
     atsas_dat_n_column_check, atsas_dat_n_column_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_dat_3_column);
  saxs_document_format_register(&atsas_dat_4_column);
  saxs_document_format_register(&atsas_dat_n_column);
}
