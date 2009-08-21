/*
 * Read files in .dat-format (used by EMBL-Hamburg).
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
#include <string.h>

#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/**************************************************************************/
static int parse_header(struct saxs_document *doc,
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
   * All other lines/information (if any) is ignored for now.
   */

  return 0;
}

static int parse_data(struct saxs_document *doc,
                      struct line *firstline, struct line *lastline) {

  /*
   * A four-column .dat-file has two different error
   * estimates: the first based on Poisson-statistics,
   * the second are Gaussian error estimates.
   *
   * Here we read and handle the Poisson-statistics.
   */
  switch (saxs_reader_columns_count(firstline)) {
    case 4:
    case 3:
      return saxs_reader_columns_parse(doc,
                                       firstline, lastline, 
                                       0, 1.0, 1, 1.0, 2,
                                       "data",
                                       SAXS_CURVE_SCATTERING_DATA) ? -1 : 0;

    case 2:
      return saxs_reader_columns_parse(doc,
                                       firstline, lastline, 
                                       0, 1.0, 1, 1.0, -1,
                                       "data",
                                       SAXS_CURVE_SCATTERING_DATA) ? -1 : 0;

    default:
      return -1;
  }

}

static int parse_footer(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  /*
   * Anything useful here?
   */
  return 0;
}

int atsas_dat_read(struct saxs_document *doc, const char *filename) {
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


/**************************************************************************/
static void write_header(FILE *fd, saxs_document *doc) {
  saxs_property *title = saxs_document_property_find_first(doc, "title");
  saxs_property *desc = saxs_document_property_find_first(doc, "sample-description");
  saxs_property *code = saxs_document_property_find_first(doc, "sample-code");
  saxs_property *conc = saxs_document_property_find_first(doc, "sample-concentration");

  if (title && saxs_property_value(title))
    fprintf(fd, "%s", saxs_property_value(title));
  fprintf(fd, "\n");

  if (desc && saxs_property_value(desc)
      && code && saxs_property_value(code)
      && conc && saxs_property_value(conc))
    fprintf(fd, "Sample: %15s c= %s mg/ml code: %s\n",
            saxs_property_value(desc), saxs_property_value(conc),
            saxs_property_value(code));
}

static void write_data(FILE* fd, saxs_document *doc) {
  saxs_curve *curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);

  saxs_data *data = saxs_curve_data(curve);
  while (data) {
    fprintf(fd, "%14e %14e %14e\n",
            saxs_data_x(data), saxs_data_y(data), saxs_data_y_err(data));
    data = saxs_data_next(data);
  }
}

static void write_footer(FILE *fd, saxs_document *doc) {
}

int atsas_dat_write(struct saxs_document *doc, const char *filename) {
  FILE *fd;
  fd = !strcmp(filename, "-") ? stdout : fopen(filename, "w");
  if (!fd)
    return -1;

  write_header(fd, doc);
  write_data(fd, doc);
  write_footer(fd, doc);

  if (fd != stdout)
    fclose(fd);

  return 0;
}


/**************************************************************************/
#include "saxsdocument_format.h"

saxs_document_format*
saxs_document_format_atsas_dat(const char *filename, const char *format) {
  static saxs_document_format atsas_dat = { atsas_dat_read, atsas_dat_write };

  if (!compare_format(format, "dat")
      || !compare_format(suffix(filename), "dat"))
    return &atsas_dat;

  return NULL;
}
