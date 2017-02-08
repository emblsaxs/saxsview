/*
 * Read/write files in .dat-format (used by EMBL-Hamburg).
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
#include "saxsdocument_format.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static int
parse_basic_information(struct saxs_document *doc, const struct line *l) {
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
   *
   * Interesting effects may be observed if a random string contains
   * a "c=" and a ":", but in any order, e.g.
   *
   *     "Extrapolation to c=0 from: [...]"
   *
   * So, double check at least that the ":" comes before the "c=".
   */

  const char *colon_pos = strchr(l->line_buffer, ':');
  const char *conc_pos  = strstr(l->line_buffer, "c=");
  const char *p;

  if (conc_pos && conc_pos > colon_pos) {
    char desc[64] = { '\0' }, code[64] = { '\0' }, conc[64] = { '\0' };

    if (colon_pos)
      strncpy(desc, colon_pos + 1,
              MIN(conc_pos - colon_pos - 1, 63));

    /* Skip "c=", read the concentration string up to the next whitespace. */
    sscanf(conc_pos, "c=%63s", conc);
    /* TODO: read concentration units */

    colon_pos = strstr(conc_pos, ":");
    if (colon_pos)
      strncpy(code, colon_pos + 1,
              MIN(63, l->line_length -
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

      /*
       * There may be cases where the description is empty.
       * If this is the case, reuse the code as description
       * to avoid issues later on (see atsas_dat_parse_footer
       * where it is assumed that all three values are present).
       */
      if (saxs_document_property_find_first(doc, "sample-concentration")
           && saxs_document_property_find_first(doc, "sample-code")
           && !saxs_document_property_find_first(doc, "sample-description")) {
        saxs_document_add_property(doc, "sample-description", p);
      }
    }
  }
  return 0;
}


static int
parse_key_value_pair(struct saxs_document *doc, const struct line *l) {

  char buf1[31], buf2[31];
  /* Special case for a line containing both Beamstop_X and Beamstop_Y */
  if (2 == sscanf(l->line_buffer, "BeamCenter_X: %20[.0-9] BeamCenter_Y: %20[.0-9]", buf1, buf2)) {
    saxs_document_add_property(doc, "BeamCenter_X", buf1);
    saxs_document_add_property(doc, "BeamCenter_Y", buf2);
    return 0;
  }

  /*
   * Keys and values are separated by ':' and a key may be any string.
   */
  const char *colon_pos = strchr(l->line_buffer, ':');
  if (colon_pos) {
    char *key;
    const char *value;

    key = malloc(l->line_length);
    if (!key)
      return ENOMEM;

    memset(key, 0, l->line_length);
    strncpy(key, l->line_buffer, colon_pos - l->line_buffer);

    colon_pos += 1;
    while (isspace(*colon_pos))
      colon_pos += 1;

    /* no need to copy the value because saxs_property_create calls strdup */
    value = colon_pos;

    saxs_document_add_property(doc, key, value);

    /*
     * There may be files, e.g. from BM29, that specify the code only
     * in a key-value pair, use that if no other code has already been
     * identified.
     */
    if (strcmp(key, "Code") == 0
         && saxs_document_property_find_first(doc, "sample-code") == NULL) {
      saxs_document_add_property(doc, "sample-code", value);
    }

    free(key);
    return 0;
  }

  /*
   * In averaged raw data sets there may be a line indicating how many frames
   * where used to compute this data set. Something like:
   *
   * Example:
   *   Channels from 1 to 2449 Number of frames averaged =    8 from total    8 frames
   */
  if (strstr(l->line_buffer, "frames averaged =")) {
    const char *equal_pos = strchr(l->line_buffer, '=') + 1;

    if (2 == sscanf(equal_pos, "%20[0-9] from total %20[0-9] frames", buf1, buf2)) {
      saxs_document_add_property(doc, "averaged-number-of-frames", buf1);
      saxs_document_add_property(doc, "total-number-of-frames", buf2);
      return 0;
    }
  }
  return 1;
}


/**************************************************************************/
static int
atsas_dat_parse_header(struct saxs_document *doc,
                       const struct line *firstline,
                       const struct line *lastline) {

  int res;
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

    if (strstr(firstline->line_buffer, "Description:")
        || strstr(firstline->line_buffer, "Sample description:")) {
      const char *p = strchr(firstline->line_buffer, ':') + 1;
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
    res = parse_basic_information(doc, firstline);
    if (res)
      return res;
    firstline = firstline->next;
  }

  /*
   * Following, here may be key-value pairs of some kind. 
   */
  while (firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    /* Ignore return value if no key-value pair can be found */
    firstline = firstline->next;
  }

  return 0;
}

static int
atsas_dat_parse_footer(struct saxs_document *doc,
                       const struct line *firstline,
                       const struct line *lastline) {

  /*
   * In subtracted files, the "real" information is in the footer.
   * Try to read the basic information from there, the sample usually
   * comes first.
   */
  if (!saxs_document_property_find_first(doc, "sample-description")
       || !saxs_document_property_find_first(doc, "sample-code")
       || !saxs_document_property_find_first(doc, "sample-concentration")) {

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
/* N.B. Error handling here aims only to avoid crashes; outputting malformed
 * data in the event of running out of memory is OK */
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
  if (!line)
    return ENOMEM;
  if (description)
    lines_printf(line, "Sample description: %s", saxs_property_value(description));
  lines_append(lines, line);

  /* Second line, if neither code nor concentration
     are available, this line is skipped. */
  if (code || concentration) {
    line = lines_create();
    if (!line)
      return ENOMEM;

    lines_printf(line, "Sample: %.15s  c= %s mg/ml  Code: %s",
                 description ? saxs_property_value(description) : "",
                 concentration ? saxs_property_value(concentration): "0.0",
                 code ? saxs_property_value(code) : "");

    lines_append(lines, line);
  }

  /* Third line, if no parents are available, this line is skipped. */
  parent = saxs_document_property_find_first(doc, "parent");
  if (parent) {
    line = lines_create();
    if (!line)
      return ENOMEM;

    lines_printf(line, "Parent(s):");
    while (parent) {
      /* N.B. lines_printf reallocates memory internally in
       * case the line buffer is passed as an argument so
       * there is no need to duplicate the line buffer here */
      lines_printf(line, "%s %s", line->line_buffer, saxs_property_value(parent));
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
      if (!line)
        return ENOMEM;

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
                              const struct line *firstline,
                              const struct line *lastline) {
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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_dat_parse_header,
                                         atsas_dat_3_column_parse_data,
                                         atsas_dat_parse_footer);
}

static int
atsas_dat_3_column_write_data(struct saxs_document *doc,
                              struct line **lines) {

  saxs_curve *curve;
  saxs_data *data;

  if (saxs_document_curve_count(doc) != 1)
    return ENOTSUP;

  curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  if (!saxs_curve_has_y_err(curve))
    return ENOTSUP;

  data = saxs_curve_data(curve);
  while (data) {
    struct line *l = lines_create();
    if (!l) {return ENOMEM;}
    lines_printf(l, "%14e %14e %14e",
                 saxs_data_x(data), saxs_data_y(data), saxs_data_y_err(data));
    lines_append(lines, l);

    data = saxs_data_next(data);
  }

  return 0;
}

int
atsas_dat_3_column_write(struct saxs_document *doc, struct line **l) {
  return saxs_writer_columns_write_lines(doc, l,
                                         atsas_dat_write_header,
                                         atsas_dat_3_column_write_data,
                                         atsas_dat_write_footer);
}

/**************************************************************************/
static int
atsas_dat_4_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {
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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_dat_parse_header,
                                         atsas_dat_4_column_parse_data,
                                         atsas_dat_parse_footer);
}

static int
atsas_dat_4_column_write_data(struct saxs_document *doc, struct line **lines) {
  saxs_curve *curve1, *curve2;
  saxs_data *data1, *data2;

  if (saxs_document_curve_count(doc) != 2)
    return ENOTSUP;

  curve1 = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  curve2 = saxs_curve_next(curve1);
  if (!saxs_curve_has_y_err(curve1) || !saxs_curve_has_y_err(curve2))
    return ENOTSUP;

  data1 = saxs_curve_data(curve1);
  data2 = saxs_curve_data(curve2);

  while (data1 && data2) {
    struct line *l = lines_create();
    if (!l) {return ENOMEM;}

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
atsas_dat_4_column_write(struct saxs_document *doc, struct line **l) {
  return saxs_writer_columns_write_lines(doc, l,
                                         atsas_dat_write_header,
                                         atsas_dat_4_column_write_data,
                                         atsas_dat_write_footer);
}


/**************************************************************************/
static int
atsas_dat_n_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {

  /* Catch all version. Accept anything with at least two columns. */

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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_dat_parse_header,
                                         atsas_dat_n_column_parse_data,
                                         atsas_dat_parse_footer);
}

static int
atsas_dat_n_column_write_data(struct saxs_document *doc, struct line **lines) {
  struct line *firstline = NULL;

  if (saxs_document_curve_count(doc) < 1)
    return ENOTSUP;

  /* Write the first column with 's' values, create lines in the process. */
  saxs_curve *curve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  saxs_data *data = saxs_curve_data(curve);
  while (data) {
    struct line *l = lines_create();
    if (!l) {return ENOMEM;}
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
atsas_dat_n_column_write(struct saxs_document *doc, struct line **l) {
  return saxs_writer_columns_write_lines(doc, l,
                                         atsas_dat_write_header,
                                         atsas_dat_n_column_write_data,
                                         atsas_dat_write_footer);
}

/**************************************************************************/
int
atsas_header_txt_read(struct saxs_document *doc,
                      const struct line *firstline,
                      const struct line *lastline) {

  /*
   * A header text is special in the sense that each and every line
   * must be formatted as "key : value". To make sure that this is
   * the case, verify that each line has a ':' or is empty - but
   * avoid completely empty files.
   *
   * This is particularly useful when reading buffers of unknown
   * format (e.g. stdin) and this reader is not the right one to
   * handle things.
   */
  int key_value_pairs = 0;

  const struct line *l = firstline;
  while (l && l != lastline) {
    if (strchr(l->line_buffer, ':'))
      key_value_pairs += 1;

    else if (strlen(l->line_buffer) > 0)
      break;

    l = l->next;
  }

  if (key_value_pairs == 0 || l != lastline)
    return ENOTSUP;
  
  while (firstline && firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    firstline = firstline->next;
  }

  return 0;
}

/**************************************************************************/

static int
autosub_dat_parse_header(struct saxs_document *doc,
                         const struct line *firstline,
                         const struct line *lastline) {
  /*
   * Old .dat file format from AUTOSUB
   * The first line has the date and the parent file names
   * Following lines have more information about the parent files
   *
   * This parser is deliberately restrictive in the files it accepts.
   * Those which do not fit the AUTOSUB format will be parsed as atsas_dat.
   */
  int nscan;

  if (!firstline) { return ENOTSUP; }

  /* I would use strptime here, but apparently it is only available on POSIX systems */
  unsigned int day, year;
  char month[4];
  nscan = sscanf(firstline->line_buffer, "%2u-%3[A-Za-z]-%4u ", &day, month, &year);
  if (nscan != 3)
    { return errno?errno:ENOTSUP; }
  char datebuf[21];
  nscan = sscanf(firstline->line_buffer, "%20s ", datebuf);
  if (nscan != 1) { return errno?errno:ENOTSUP; }
  saxs_document_add_property(doc, "date", datebuf);

  const char *rest_of_line = firstline->line_buffer + strlen(datebuf);
  while(isspace(*rest_of_line)) {++rest_of_line;}
  /* only accept it as an AUTOSUB line if it contains at three mathematical special characters */
  int nmathchars = 0;
  if (strchr(rest_of_line, '+'))
    { ++nmathchars; }
  if (strchr(rest_of_line, '-'))
    { ++nmathchars; }
  if (strchr(rest_of_line, '*'))
    { ++nmathchars; }
  if (strchr(rest_of_line, '/'))
    { ++nmathchars; }
  if (strchr(rest_of_line, '('))
    { ++nmathchars; }
  if (strchr(rest_of_line, ')'))
    { ++nmathchars; }
  if (nmathchars < 3)
    { return ENOTSUP; }

  saxs_document_add_property(doc, "autosub-operation", rest_of_line);

  const struct line *currline = firstline;
  while((currline = currline->next) && (currline != lastline)) {
    /* All subsequent lines should look like:
     * [filename]  Conc = [conc]  N1 =    [n]  N2 = [n]
     * 
     * The concentration sometimes contains fortran-style '1.0d+2' which is not parsed by %f
     */
    unsigned int n1, n2;
    nscan = sscanf(currline->line_buffer, "%*s Conc = %*[-+.0-9dDeE] N1 = %u N2 = %u", &n1, &n2);
    if (nscan != 2) {
      /* Some files contain a Chi value here */
      if ((currline->next == lastline) &&
          !strncmp(currline->line_buffer, "Chi(from original file)=     ", 29)) {
        saxs_document_add_property(doc, "Chi(from original file)", currline->line_buffer+29);
        break;
      } else {
        return ENOTSUP;
      }
    }
    if (n2 <= n1)
      { return ENOTSUP; }
    /* TODO - get the list of parents and the concentration from here */
  }
  return 0;
}

static int is_equals_marker_line(const struct line *l) {
  int nequals = 0;  /* number of '=' characters, need at least 3 */
  const char *c;
  for (c = l->line_buffer; *c; ++c) {
    if (*c == '=') {
      ++nequals;
    } else if (!isspace(*c)) {
      return 0;
    }
  }
  return (nequals >= 3);
}

static int
autosub_dat_parse_footer(struct saxs_document *doc,
                         const struct line *firstline,
                         const struct line *lastline) {
  if (!firstline) {return ENOTSUP;}
  if (firstline == lastline) {return ENOTSUP;}
  if (!is_equals_marker_line(firstline))
    { return ENOTSUP; }

  /* Get the sample description */
  const struct line *currline = firstline;

  if (! ((currline = currline->next) && (currline != lastline)))
    { return ENOTSUP; }
  if (0 != strncmp(currline->line_buffer, "Description:                            ", 40)) {
    return ENOTSUP;
  }
  saxs_document_add_property(doc, "sample-description", currline->line_buffer+40);

  /* Get the sample code and concentration */
  if (! ((currline = currline->next) && (currline != lastline)))
    { return ENOTSUP; }
  char sample_code[31], conc[21], code[21];
  float fconc = 0;
  int nscan = sscanf(currline->line_buffer, "Sample: %30s c= %20s mg/ml Code: %20s", sample_code, conc, code);
  if (nscan != 3) {
    return errno?errno:ENOTSUP;
  }
  nscan = sscanf(conc, "%f", &fconc);
  if (nscan != 1 || fconc < 0) {
    return errno?errno:ENOTSUP;
  }
  saxs_document_add_property(doc, "sample-code", sample_code);
  saxs_document_add_property(doc, "sample-concentration", conc);
  saxs_document_add_property(doc, "code", code);

  /* Read any more properties up until the next equals marker line */
  while (currline = currline->next) {
    if (currline == lastline)
      {break;}
    if (is_equals_marker_line(currline))
      {break;}
    if (strchr(currline->line_buffer, ':')) {
      int res = parse_key_value_pair(doc, currline);
      if (res) return res;
      continue;
    }
    if (!strncmp(currline->line_buffer, "Channels from ", 14)) {
      saxs_document_add_property(doc, "channels", currline->line_buffer+14);
      continue;
    }
    return ENOTSUP;
  }
  return 0;
}

int
autosub_dat_read(struct saxs_document *doc,
                 const struct line *firstline,
                 const struct line *lastline) {
  /*
   * Old .dat file format from AUTOSUB
   */
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         autosub_dat_parse_header,
                                         atsas_dat_3_column_parse_data,
                                         autosub_dat_parse_footer);
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
  saxs_document_format autosub_dat = {
     "dat", "autosub-dat",
     "Experimental data from AUTOSUB",
     autosub_dat_read, NULL, NULL
  };

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

  saxs_document_format_register(&autosub_dat);
  saxs_document_format_register(&atsas_dat_3_column);
  saxs_document_format_register(&atsas_dat_4_column);
  saxs_document_format_register(&atsas_dat_n_column);
  saxs_document_format_register(&atsas_header_txt);
}
