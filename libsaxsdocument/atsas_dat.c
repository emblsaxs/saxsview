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
  /*
   * TODO: everything after the last "/" (if present) should be treated as
   *       concentration (overridden by "c="). Example:
   * " 02-Mar-2009       (al_011.dat - 1.0*Aver(al_010.dat,al_012.dat) /  4.37"
   *
   * TODO: should read "description" if present. Examples:
   *    "Description:                            Bovine Serum Al"
   *    "Sample description:                     Bovine Serum Albumin"
   */

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
      /* TODO: read concentration units */

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
   * TODO: read the description and code
   */
  return 0;
}


/**************************************************************************/
static int
atsas_dat_write_header(struct saxs_document *doc, struct line **lines) {
  struct line *line;

  saxs_property *title;
  saxs_property *description, *code, *concentration;
  saxs_property *parent;

  /* First line, if no title is available, this line is empty. */
  title = saxs_document_property_find_first(doc, "title");
  line = lines_create();
  if (title)
    lines_printf(line, "%s", saxs_property_value(title));
  lines_append(lines, line);

  /* Second line, if concentration, code and concentration
     are not available, this line is skipped. */
  description   = saxs_document_property_find_first(doc, "sample-description");
  code          = saxs_document_property_find_first(doc, "sample-code");
  concentration = saxs_document_property_find_first(doc, "sample-concentration");
  if (description && code && concentration) {
    line = lines_create();
    lines_printf(line, "Sample: %15s c= %s mg/ml Code: %8s",
                 saxs_property_value(description),
                 saxs_property_value(concentration),
                 saxs_property_value(code));
    lines_append(lines, line);
  }

  /* Third line, if no parents are available, this line is skipped. */
  parent = saxs_document_property_find_first(doc, "parent");
  if (parent) {
    line = lines_create();
    lines_printf(line, "Parent(s): TODO");

    /* FIXME */

    lines_append(lines, line);
  }

  return 0;
}

static int
atsas_dat_write_footer(struct saxs_document *doc, struct line **lines) {
  return 0;
}

/**************************************************************************/
static int
atsas_dat_3_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {
  if (saxs_reader_columns_count(firstline) != 3)
    return -1;

  return saxs_reader_columns_parse(doc,
                                   firstline, lastline,
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
atsas_dat_3_column_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        atsas_dat_parse_header,
                                        atsas_dat_3_column_parse_data,
                                        atsas_dat_parse_footer);
}

static int
atsas_dat_3_column_write_data(struct saxs_document *doc,
                              struct line **lines) {

  saxs_curve *curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  saxs_data *data = saxs_curve_data(curve);
  while (data) {
    struct line *l = lines_create();
    lines_printf(l, "%14e %14e %14e",
                saxs_data_x(data), saxs_data_y(data), saxs_data_y_err(data));
    lines_append(lines, l);

    data = saxs_data_next(data);
  }

  return 0;
}

int
atsas_dat_3_column_write(struct saxs_document *doc, const char *filename) {
  if (saxs_document_curve_count(doc) != 1)
    return -1;

  return saxs_writer_columns_write_file(doc, filename,
                                        atsas_dat_write_header,
                                        atsas_dat_3_column_write_data,
                                        atsas_dat_write_footer);
}

/**************************************************************************/
static int
atsas_dat_4_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {
  if (saxs_reader_columns_count(firstline) != 4)
    return -1;

  return saxs_reader_columns_parse(doc,
                                   firstline, lastline,
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
atsas_dat_4_column_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        atsas_dat_parse_header,
                                        atsas_dat_4_column_parse_data,
                                        atsas_dat_parse_footer);
}

static int
atsas_dat_4_column_write_data(struct saxs_document *doc, struct line **lines) {

  saxs_curve *curve1 = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  saxs_data *data1 = saxs_curve_data(curve1);

  saxs_curve *curve2 = saxs_curve_next(curve1);
  saxs_data *data2 = saxs_curve_data(curve2);

  while (data1 && data2) {
    struct line *l = lines_create();
    lines_printf(l, "%14e %14e %14e %14e",
                saxs_data_x(data1), saxs_data_y(data1),
                saxs_data_y_err(data1), saxs_data_y_err(data2));
    lines_append(lines, l);

    data1 = saxs_data_next(data1);
    data2 = saxs_data_next(data2);
  }

  return 0;
}

int
atsas_dat_4_column_write(struct saxs_document *doc, const char *filename) {
  if (saxs_document_curve_count(doc) != 2)
    return -1;

  return saxs_writer_columns_write_file(doc, filename,
                                        atsas_dat_write_header,
                                        atsas_dat_4_column_write_data,
                                        atsas_dat_write_footer);
}


/**************************************************************************/
static int
atsas_dat_n_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {

  /* Catch all version. Accept anything, including empty files. */

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
     atsas_dat_3_column_read, atsas_dat_3_column_write, NULL
  };

  saxs_document_format atsas_dat_4_column = {
     "dat", "atsas-dat-4-column",
     "ATSAS experimental data, one data set with Poisson and Gaussian errors",
     atsas_dat_4_column_read, atsas_dat_4_column_write, NULL
  };

  saxs_document_format atsas_dat_n_column = {
     "dat", "atsas-dat-n-column",
     "ATSAS experimental data, multiple data sets, no errors",
     atsas_dat_n_column_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_dat_3_column);
  saxs_document_format_register(&atsas_dat_4_column);
  saxs_document_format_register(&atsas_dat_n_column);
}
