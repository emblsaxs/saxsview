/*
 * Read/write files in .dat-format (used by EMBL-Hamburg).
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

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static void
parse_basic_information(struct saxs_document *doc, struct line *l) {
  /*
   * Basic Information:
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

  char *colon_pos = strchr(l->line_buffer, ':');
  char *conc_pos  = strstr(l->line_buffer, "c=");
  char *p;

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
              MIN(64, l->line_length -
                  (colon_pos - l->line_buffer - 1)));


    /*
     * If there is a description in line 1, do not add a (possibly truncated)
     * second description here.
     */
    if (!saxs_document_property_find_first(doc, "sample-description")) {
      p = desc;
      while (isspace(*p)) ++p;
      saxs_document_add_property(doc, "sample-description", p);
    }

    if (!saxs_document_property_find_first(doc, "sample-concentration")) {
      p = conc;
      while (isspace(*p)) ++p;
      saxs_document_add_property(doc, "sample-concentration", p);
    }

    if (!saxs_document_property_find_first(doc, "sample-code")) {
      p = code;
      while (isspace(*p)) ++p;
      saxs_document_add_property(doc, "sample-code", p);
    }
  }
}


static void
parse_key_value_pair(struct saxs_document *doc, struct line *l) {
  /*
   * Keys and values are separated by ':' and a key may be any string.
   */
  char *colon_pos = strchr(l->line_buffer, ':');
  if (colon_pos) {
    char *key, *value;

    key = malloc(l->line_length);
    memset(key, 0, l->line_length);
    strncpy(key, l->line_buffer, colon_pos - l->line_buffer);

    colon_pos += 1;
    while (isspace(*colon_pos))
      colon_pos += 1;

    value = malloc(l->line_length);
    memset(value, 0, l->line_length);
    strncpy(value, colon_pos, l->line_buffer + l->line_length - colon_pos);

    saxs_document_add_property(doc, key, value);

    /*
     * There may be files, e.g. from BM29, that specify the code only
     * in a key-value pair.
     */
    if (strcmp(key, "Code") == 0)
      saxs_document_add_property(doc, "sample-code", value);

    free(value);
    free(key);
  }

  /*
   * In averaged raw data sets there may be a line indicating how many frames
   * where used to compute this data set. Something like:
   *
   * Example:
   *   Channels from 1 to 2449 Number of frames averaged =    8 from total    8 frames
   */
  if (strstr(l->line_buffer, "frames averaged =")) {
    int averaged, total;
    char *equal_pos = strchr(l->line_buffer, '=') + 1;

    if (sscanf(equal_pos, "%d from total %d frames", &averaged, &total) == 2) {
      char buffer[64] = { '\0' };

      sprintf(buffer, "%d", averaged);
      saxs_document_add_property(doc, "averaged-number-of-frames", buffer);

      sprintf(buffer, "%d", total);
      saxs_document_add_property(doc, "total-number-of-frames", buffer);
    }
  }
}


/**************************************************************************/
static int
atsas_dat_parse_header(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {

  /*
   * The first non-empty line may contain the 'description' of the data.
   * Examples:
   *    "Description:                            Bovine Serum Al"
   *    "Sample description:                     Bovine Serum Albumin"
   *
   * Other files may provide a description without a "Description" key.
   * If there is nothing, use the line as is and hope for the best.
   * Example:
   *    "BSA calibration sample"
   */
  while (firstline != lastline && strlen(firstline->line_buffer) == 0)
    firstline = firstline->next;

  if (firstline != lastline) {

    /* Trim trailing whitespaces */
    char *q;
    q = firstline->line_buffer + firstline->line_length - 1;
    while (q > firstline->line_buffer && !isalnum(*q))
      *q-- = '\0';

    if (strstr(firstline->line_buffer, "Description:")
        || strstr(firstline->line_buffer, "Sample description:")) {
      char *p = strchr(firstline->line_buffer, ':') + 1;
      while (isspace(*p)) ++p;
      saxs_document_add_property(doc, "sample-description", p);

    } else if (strchr(firstline->line_buffer, ':') == NULL) {
      saxs_document_add_property(doc, "sample-description", firstline->line_buffer);
    }

    firstline = firstline->next;
  }

  /*
   * If the file is a raw data file, then the second non-empty line
   * holds the description, the code and the sample concentration.
   */
  while (firstline != lastline && strlen(firstline->line_buffer) == 0)
    firstline = firstline->next;

  if (firstline != lastline) {
    parse_basic_information(doc, firstline);
    firstline = firstline->next;
  }

  /*
   * Following, here may be key-value pairs of some kind. 
   */
  while (firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    firstline = firstline->next;
  }

  return 0;
}

static int
atsas_dat_parse_footer(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {

  /*
   * In subtracted files, the "real" information is in the footer.
   * Try to read the basic information from there, the sample usually
   * comes first.
   */
  if (!saxs_document_property_find_first(doc, "sample-description")) {
    while (firstline != lastline) {
      parse_basic_information(doc, firstline);
      firstline = firstline->next;
    }

    return 0;
  }

  /*
   * Alternatively, especially in raw frame data, there may be key-value
   * pairs of some kind. 
   */
  while (firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    firstline = firstline->next;
  }

  return 0;
}


/**************************************************************************/
static int
atsas_dat_write_header(struct saxs_document *doc, struct line **lines) {
  struct line *line;

  saxs_property *description, *code, *concentration;
  saxs_property *parent;

  description   = saxs_document_property_find_first(doc, "sample-description");
  code          = saxs_document_property_find_first(doc, "sample-code");
  concentration = saxs_document_property_find_first(doc, "sample-concentration");

  /* First line, if no description is available, this line is empty. */
  line = lines_create();
  if (description)
    lines_printf(line, "Sample description: %s", saxs_property_value(description));
  lines_append(lines, line);

  /* Second line, if neither code nor concentration
     are available, this line is skipped. */
  if (code || concentration) {
    line = lines_create();

    if (description) {
      lines_printf(line, "Sample: %.15s", saxs_property_value(description));
    }
    if (concentration) {
      char *oldline = strdup(line->line_buffer);
      lines_printf(line, "%s  c= %s mg/ml", oldline, saxs_property_value(concentration));
      free(oldline);
    }
    if (code) {
      char *oldline = strdup(line->line_buffer);
      lines_printf(line, "%s  Code: %.8s", oldline, saxs_property_value(code));
      free(oldline);
    }
    lines_append(lines, line);
  }

  /* Third line, if no parents are available, this line is skipped. */
  parent = saxs_document_property_find_first(doc, "parent");
  if (parent) {
    line = lines_create();
    lines_printf(line, "Parent(s):");
    while (parent) {
      char *oldline = strdup(line->line_buffer);
      lines_printf(line, "%s %s", oldline, saxs_property_value(parent));
      free(oldline);
      parent = saxs_property_find_next(parent, "parent");
    }
    lines_append(lines, line);
  }

  return 0;
}

static int
atsas_dat_write_footer(struct saxs_document *doc, struct line **lines) {
  struct line *line;

  saxs_property *property = saxs_document_property_first(doc);
  while (property) {
    const char *name  = saxs_property_name(property);
    const char *value = saxs_property_value(property);
  
    if (strcmp(name, "sample-description")
         && strcmp(name, "sample-code")
         && strcmp(name, "sample-concentration")) {

      line = lines_create();
      /* FIXME: columns should be aligned on output */
      lines_printf(line, "%s: %s", name, value);
      lines_append(lines, line);
    }

    property = saxs_property_next(property);
  }

  return 0;
}

/**************************************************************************/
static int
atsas_dat_3_column_parse_data(struct saxs_document *doc,
                              struct line *firstline, struct line *lastline) {
  if (saxs_reader_columns_count(firstline) != 3)
    return ENOTSUP;

  return saxs_reader_columns_parse(doc,
                                   firstline, lastline,
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
atsas_dat_3_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
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
  if (saxs_document_curve_count(doc) < 1)
    return ENOTSUP;

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
    return ENOTSUP;

  saxs_reader_columns_parse(doc,
                            firstline, lastline,
                            0, 1.0, 1, 1.0, 2,
                            "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  saxs_reader_columns_parse(doc,
                            firstline, lastline,
                            0, 1.0, 1, 1.0, 3,
                            "data",
                            SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  return 0;
}

int
atsas_dat_4_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
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
  if (saxs_document_curve_count(doc) < 2)
    return ENOTSUP;

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
  if (n < 2)
    return ENOTSUP;

  for (i = 1; i < n; ++i)
    saxs_reader_columns_parse(doc,
                              firstline, lastline,
                              0, 1.0, i, 1.0, -1,
                              "data",
                              SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  return 0;
}

int
atsas_dat_n_column_read(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_dat_parse_header,
                                         atsas_dat_n_column_parse_data,
                                         atsas_dat_parse_footer);
}

static int
atsas_dat_n_column_write_data(struct saxs_document *doc, struct line **lines) {
  struct line *firstline = NULL;

  /* Write the first column with 's' values, create lines in the process. */
  saxs_curve *curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  saxs_data *data = saxs_curve_data(curve);
  while (data) {
    struct line *l = lines_create();
    lines_printf(l, "%14e", saxs_data_x(data));
    lines_append(&firstline, l);

    data = saxs_data_next(data);
  }

  /*
   * For each curve, append a new column of 'I' values
   * to the previous line contents.
   */
  while (curve) {
    struct line *l = firstline;

    saxs_data *data = saxs_curve_data(curve);
    while (data) {
      lines_printf(l, "%s %14e", l->line_buffer, saxs_data_y(data));

      data = saxs_data_next(data);
      l = l->next;
    }

    curve = saxs_curve_next(curve);
  }

  lines_append(lines, firstline);
  return 0;
}

int
atsas_dat_n_column_write(struct saxs_document *doc, const char *filename) {
  return saxs_writer_columns_write_file(doc, filename,
                                        atsas_dat_write_header,
                                        atsas_dat_n_column_write_data,
                                        atsas_dat_write_footer);
}

/**************************************************************************/
int
atsas_header_txt_read(struct saxs_document *doc,
                      struct line *firstline, struct line *lastline) {

  /*
   * A header text is special in the sense that each and every line
   * must be formatted as "key : value". To make sure that this is
   * the case, verify that each line has a ':' or is empty.
   *
   * This is particularly useful when reading buffers of unknown
   * format (e.g. stdin) and this reader is not the right one to
   * handle things.
   */
  struct line *l = firstline;
  while (l && l != lastline
           && (strchr(l->line_buffer, ':') || strlen(l->line_buffer) == 0))
    l = l->next;

  if (l != lastline)
    return ENOTSUP;
  
  while (firstline && firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    firstline = firstline->next;
  }

  return 0;
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
     atsas_dat_n_column_read, atsas_dat_n_column_write, NULL
  };

  /*
   * Header information for raw radially averaged data files.
   * Information may be added to processed .dat files.
   */
  saxs_document_format atsas_header_txt = {
     "txt", "atsas-header-txt",
     "ATSAS header information for experimental data",
     atsas_header_txt_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_dat_3_column);
  saxs_document_format_register(&atsas_dat_4_column);
  saxs_document_format_register(&atsas_dat_n_column);
  saxs_document_format_register(&atsas_header_txt);
}
