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

/* Parse a line that has been printed by OUTDNUM or a similar routine
 *
 * Example line:
 * Constant adjusted ...................................... : 0.6556
 *
 * Assumes that there are at least three dots between the key and the value
 */
static int
try_parse_OUTDNUM(struct saxs_document *doc, const char *line) {
  const char *dotdotcolon = strstr(line, ".. : ");
  if (!dotdotcolon)
    return -1;

  const char *spacedotdot = strstr(line, " ...");
  if (!spacedotdot)
    return -2;

  const char *value = dotdotcolon + strlen(".. : ");
  size_t keylen = spacedotdot - line;

  saxs_document_add_property_strn(doc, line, keylen, value, -1);
  return 0;
}

/* Parse a line with column headers and chi-squared
 *
 * Example line:
 * sExp  |  iExp |  Err | iFit(+Const) | Chi^2=   0.207
 */
static int
try_parse_colhdrs_chi2(struct saxs_document *doc, const char *line) {
  const char *sExp_pos = strstr(line, "sExp");
  if (!sExp_pos)
    return -1;
  const char *iExp_pos = strstr(sExp_pos, "iExp");
  if (!iExp_pos)
    return -2;
  const char *Err_pos = strstr(iExp_pos, "Err");
  if (!Err_pos)
    return -3;
  const char *iFit_pos = strstr(sExp_pos, "iFit");
  if (!iFit_pos)
    return -4;
  const char *Chi2_pos = strstr(line, "Chi^2");
  if (!Chi2_pos)
    return -5;

  if ((Chi2_pos <= iFit_pos) || (Chi2_pos <= Err_pos))
    return -6;

  const char *equals_pos = strchr(Chi2_pos, '=');
  if (!equals_pos)
    return -7;

  const char *value = equals_pos + 1;
  while (isspace(*value)) {++value;}

  /* Handle cases where the Chi^2 is too large to fit, so asterisks are written */
  if (strstr(value, "***")) {
    saxs_document_add_property(doc, "Chi^2", "NaN");
    return 0;
  }

  /* TODO check that the value can be parsed as a float */
  saxs_document_add_property(doc, "Chi^2", value);
  return 0;
}

/* Parse a line with several "key: value" or "key = value" pairs
 *
 * Example line:
 * T= 0.300E-03 Rf =0.13565  Los: 0.1744 DisCog: 0.0909 Scale =  0.249E-07
 *
 * Require at least three key-value pairs
 */
static int
try_parse_many_key_value(struct saxs_document *doc, const char *line) {
  if (strlen(line) < 11) /* absolute minimum "k:v k:v k:v" */
    return -1;

  const char *sep_pos = line;
  int nseparators = 0;
  while ((sep_pos = strpbrk(sep_pos+1, ":="))) {++nseparators;}
  if (nseparators < 3)
    return -2;

  const char *curr_pos = line;
  while(*curr_pos) {
    while (isspace(*curr_pos)) {++curr_pos;}
    const char *key = curr_pos;
    int key_len = 0;
    while ((key[key_len] != '\0') &&
           !isspace(key[key_len]) &&
           !strchr(":=", key[key_len])) {
      ++key_len;
    }
    if (key_len <= 0) {
      return -3;
    }
    sep_pos = key + key_len; /* one after the end of the key */
    while (isspace(*sep_pos)) {++sep_pos;}
    if ((*sep_pos == '\0') ||
        !strchr(":=", *sep_pos)) {
      /* this character must be a delimiter */
      return -4;
    }

    const char *value = sep_pos + 1;
    while (isspace(*value)) {++value;}
    int value_len = 0;
    while ((value[value_len] != '\0') &&
           !isspace(value[value_len]) &&
           (value[value_len] != ',')) {
      if (strchr(":=", value[value_len])) {
        /* The value should not be followed by another k:v delimiter */
        return -5;
      }
      ++value_len;
    }
    if (value_len <= 0) {
      return -6;
    }
    curr_pos = value + value_len;
    if (*curr_pos == ',') {++curr_pos;}
    while (isspace(*curr_pos)) {++curr_pos;}

    /* Check for strings of asterisks caused by overfilling the space available */
    if (strspn(value, "*") == value_len) {
      value = "NaN";
      value_len = 3;
    }
    saxs_document_add_property_strn(doc, key, key_len, value, value_len);
  }
  return 0;
}

static int
atsas_fir_fit_parse_header(struct saxs_document *doc,
                           const struct line *firstline,
                           const struct line *lastline) {
  int rc = 0;
  const struct line *currline;

  for (currline = firstline; currline != lastline; currline = currline->next) {
    rc = try_parse_OUTDNUM(doc, currline->line_buffer);
    if (0 == rc) continue;

    /* Some lines can only occur as the final header line */
    if (currline->next == lastline) {
      rc = try_parse_colhdrs_chi2(doc, currline->line_buffer);
      if (0 == rc) continue;

      rc = try_parse_many_key_value(doc, currline->line_buffer);
      if (0 == rc) continue;
    }
  }
  return 0;
}

static int
atsas_fir_fit_parse_footer(struct saxs_document *doc,
                           const struct line *firstline,
                           const struct line *lastline) {
  /* .fit and .fir files never have a footer */
  if (firstline != lastline)
    return ENOTSUP;

  return 0;
}


/**************************************************************************/
static int
atsas_fit_write_header(struct saxs_document *doc, struct line **lines) {
  struct line *line;

  saxs_property *title;
  title = saxs_document_property_find_first(doc, "title");

  /* First line, if no title is available, this line is empty. */
  line = lines_create();
  if (!line) {return ENOMEM;}

  if (title)
    lines_printf(line, "%s", saxs_property_value(title));
  lines_append(lines, line);

  return 0;
}

/**************************************************************************/
/* Special-case header parsing for programs' own esoteric formats */

/* BODIES output
 *
 * Example:
 * hollow-sphere: ro=133.901, ri=0.287176E-002, scale=0.883244E-008
 */
static int
bodies_fir_parse_header(struct saxs_document *doc,
                        const struct line *firstline,
                        const struct line *lastline) {

  if (lastline != firstline->next) {
    return ENOTSUP;  // header must be a single line
  }

  const char *line = firstline->line_buffer;
  const char *colon_pos = strchr(line, ':');
  if (!colon_pos)
    return ENOTSUP;

  size_t btypelen = colon_pos - line;
  // If all of the strncmp calls return non-zero, it is not a bodies file
  if (strncmp("ellipsoid", line, btypelen) &&
      strncmp("rotation-ellipsoid", line, btypelen) &&
      strncmp("cylinder", line, btypelen) &&
      strncmp("elliptic-cylinder", line, btypelen) &&
      strncmp("hollow-cylinder", line, btypelen) &&
      strncmp("parallelepiped", line, btypelen) &&
      strncmp("hollow-sphere", line, btypelen) &&
      strncmp("dumbbell", line, btypelen)) {
    return ENOTSUP;
  }

  saxs_document_add_property_strn(doc, "bodies-body", -1,  line, btypelen);
  if (0 == try_parse_many_key_value(doc, colon_pos+1)) {
    return 0;
  }
  // Ignore the return value of try_parse_many_key_value, just give ENOTSUP
  return ENOTSUP;
}

/* .fit files from CRYSOL and CRYSON 
 *
 * Example:
 * 4mld.pdb  Dro:0.075  Ra:1.400  RGT:28.10  Vol: 86422.  Chi^2:******
 */
static int
crysol_fit_parse_header(struct saxs_document *doc,
                        const struct line *firstline,
                        const struct line *lastline) {

  if (lastline != firstline->next) {
    return ENOTSUP;  // header must be a single line
  }

  const char *line = firstline->line_buffer;

  const char *Dro_pos = strstr(line, "Dro:");
  if (!Dro_pos)
    return ENOTSUP;

  // "Dro:" should be the first key:value pair
  if (strchr(line, ':') != Dro_pos+3)
    return ENOTSUP;

  const char *Chi2_pos = strstr(line, "Chi^2:");
  if (!Chi2_pos)
    return ENOTSUP;

  // "Chi^2:" should be the last key:value pair
  if (strchr(Chi2_pos+6, ':'))
    return ENOTSUP;

  // Beginning of the line is a file name
  int pdbnam_len = Dro_pos - line;
  while ((pdbnam_len > 0) && (isspace(line[pdbnam_len-1]))) {--pdbnam_len;}
  if (pdbnam_len <= 0)
    return ENOTSUP;

  saxs_document_add_property_strn(doc, "pdbnam", -1, line, pdbnam_len);

  if (0 == try_parse_many_key_value(doc, Dro_pos)) {
    return 0;
  }
  // Ignore the return value of try_parse_many_key_value, just give ENOTSUP
  return ENOTSUP;
}

/**************************************************************************/
static int
atsas_fir_4_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {

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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fir_4_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}


/**************************************************************************/
static int
atsas_fit_3_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {

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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fit_3_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}


int
atsas_fit_3_column_write_data(struct saxs_document *doc,
                              struct line **lines) {

  saxs_curve *expcurve, *fitcurve;
  saxs_data *expdata, *fitdata;

  if (saxs_document_curve_count(doc) != 2)
    return ENOTSUP;

  expcurve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  fitcurve = saxs_curve_next(expcurve);

  expdata = saxs_curve_data(expcurve);
  fitdata = saxs_curve_data(fitcurve);
  while (expdata && fitdata) {
    struct line *l = lines_create();
    if (!l) {return ENOMEM;}
    lines_printf(l, "%14e %14e %14e",
                 saxs_data_x(expdata), saxs_data_y(expdata), saxs_data_y(fitdata));
    lines_append(lines, l);

    expdata = saxs_data_next(expdata);
    fitdata = saxs_data_next(fitdata);
  }

  return 0;
}

int
atsas_fit_3_column_write(struct saxs_document *doc, struct line **l) {
  return saxs_writer_columns_write_lines(doc, l,
                                         atsas_fit_write_header,
                                         atsas_fit_3_column_write_data,
                                         NULL);
}


/**************************************************************************/
static int
atsas_fit_4_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {

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
                                    const struct line *header,
                                    const struct line *data,
                                    const struct line *footer) {
  const struct line *nextfit;

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
                        const struct line *firstline,
                        const struct line *lastline) {

  int res;
  const struct line *header = NULL, *data = NULL, *footer = NULL;

  if ((res = saxs_reader_columns_scan(firstline, &header, &data, &footer)) != 0)
    goto error;

  if ((data == NULL) || saxs_reader_columns_count(data) != 4) {
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

int
atsas_fit_4_column_write_data(struct saxs_document *doc,
                              struct line **lines) {

  saxs_curve *expcurve, *fitcurve;
  saxs_data *expdata, *fitdata;

  if (saxs_document_curve_count(doc) != 2)
    return ENOTSUP;

  expcurve = saxs_document_curve_find(doc, SAXS_CURVE_SCATTERING_DATA);
  fitcurve = saxs_curve_next(expcurve);

  if (!saxs_curve_has_y_err(expcurve))
    return ENOTSUP;

  expdata = saxs_curve_data(expcurve);
  fitdata = saxs_curve_data(fitcurve);
  while (expdata && fitdata) {
    struct line *l = lines_create();
    if (!l) {return ENOMEM;}
    lines_printf(l, "%14e %14e %14e %14e",
                 saxs_data_x(expdata), saxs_data_y(expdata), saxs_data_y_err(expdata), saxs_data_y(fitdata));
    lines_append(lines, l);

    expdata = saxs_data_next(expdata);
    fitdata = saxs_data_next(fitdata);
  }

  return 0;
}

int
atsas_fit_4_column_write(struct saxs_document *doc, struct line **l) {
  return saxs_writer_columns_write_lines(doc, l,
                                         atsas_fit_write_header,
                                         atsas_fit_4_column_write_data,
                                         NULL);
}

/**************************************************************************/
static int
atsas_fit_5_column_parse_data(struct saxs_document *doc,
                              const struct line *firstline,
                              const struct line *lastline) {

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
                        const struct line *firstline,
                        const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_fir_fit_parse_header,
                                         atsas_fit_5_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}

/**************************************************************************/
/* Special-case formats */

int
bodies_fir_read(struct saxs_document *doc,
                const struct line *firstline,
                const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         bodies_fir_parse_header,
                                         atsas_fit_4_column_parse_data,
                                         atsas_fir_fit_parse_footer);
}

int
crysol_fit_read(struct saxs_document *doc,
                const struct line *firstline,
                const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         crysol_fit_parse_header,
                                         atsas_fit_4_column_parse_data,
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
     atsas_fit_3_column_read, atsas_fit_3_column_write, NULL
  };

  saxs_document_format atsas_fit_4_column = {
     "fit", "atsas-fit-4-column",
     "ATSAS fit against data (4 column; SASREF, ...)",
     atsas_fit_4_column_read, atsas_fit_4_column_write, NULL
  };

  saxs_document_format atsas_fit_5_column = {
     "fit", "atsas-fit-5-column",
     "ATSAS fit against data (5 column; OLIGOMER, ...)",
     atsas_fit_5_column_read, NULL, NULL
  };

  saxs_document_format bodies_fir = {
     "fir", "bodies-fir",
     ".fir file from bodies --fit",
     bodies_fir_read, NULL, NULL
  };

  saxs_document_format crysol_fit = {
     "fit", "crysol-fit",
     ".fit files from CRYSOL or CRYSON fit mode",
     crysol_fit_read, NULL, NULL
  };

  saxs_document_format_register(&bodies_fir);
  saxs_document_format_register(&crysol_fit);
  saxs_document_format_register(&atsas_fir_4_column);
  saxs_document_format_register(&atsas_fit_3_column);
  saxs_document_format_register(&atsas_fit_4_column);
  saxs_document_format_register(&atsas_fit_5_column);
}
