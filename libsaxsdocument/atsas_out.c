/*
 * Read files in .out-format (e.g. written by GNOM).
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
#include <ctype.h>

/*
 * Find VALUE in "DELIM__VALUE__" where '__' is one or more 
 * whitespace or newline characters.
 *
 * Note: not reentrant, uses static buffer for convenience.
 */
static const char* extract(struct line *l, const char *delim) {
  static char substr[1024];
  char *p = substr, *q;

  memset(p, 0, 1024);
  q = strstr(l->line_buffer, delim);
  if (q) {
    q += strlen(delim);

    while (q && isspace(*q))
      ++q;

    while (q && *q && !isspace(*q))
      *p++ = *q++;
  }

  return substr;
}


static int parse_header(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {

  while (firstline != lastline) {
    /*
     * Example line:
     * "           ####    G N O M   ---   Version 4.6                       ####"
     *                                             ^^^
     */
    if (strstr(firstline->line_buffer, "G N O M")) {
      saxs_document_add_property(doc, "creator", "GNOM");
      saxs_document_add_property(doc, "creator-version",
                                 extract(firstline, "Version"));
    }

    /*
     * Example lines:
     * "  Number of points omitted at the beginning:           9"
     *                                                         ^
     * "  Number of points omitted at the end:        1100"
     *                                                ^^^^
     * These lines are not present if '0' points are omitted.
     */
    else if (strstr(firstline->line_buffer, "omitted at the beginning"))
      saxs_document_add_property(doc, "leading-points-omitted",
                                 extract(firstline, ":"));

    else if (strstr(firstline->line_buffer, "omitted at the end"))
      saxs_document_add_property(doc, "trailing-points-omitted",
                                 extract(firstline, ":"));

    /*
     * Example line:
     * "   *******    Input file(s) : lyz_014.dat"
     *                                ^^^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Input file"))
      saxs_document_add_property(doc, "parent",
                                 extract(firstline, ":"));

    /*
     * Example lines:
     * "           Condition P(rmin) = 0 is used. "
     * "           Condition P(rmax) = 0 is used. "
     *
     * No need to extract anything, the lines are omitted if not used.
     */
    else if (strstr(firstline->line_buffer, "Condition P(rmin)"))
      saxs_document_add_property(doc, "condition-r-min-zero", "true");

    else if (strstr(firstline->line_buffer, "Condition P(rmax)"))
      saxs_document_add_property(doc, "condition-r-max-zero", "true");

    /*
     * Example line:
     * " Warning: Dmax*Smin =  4.090   is greater than Pi"
     */
    else if (strstr(firstline->line_buffer, "greater than Pi"))
      saxs_document_add_property(doc, "warning-dmax*smin-greater-than-pi", "true");

    /*
     * Example line:
     * "  Highest ALPHA (theor) :   0.182E+03                 JOB = 0"
     *                              ^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Highest ALPHA (theor)"))
      saxs_document_add_property(doc, "highest-alpha-theor",
                                 extract(firstline, ":"));

    /*
     * Example line:
     * "  Current ALPHA         :   0.195E-18   Rg :  0.118E+01   I(0) :   0.332E+02"
     *                              ^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Current ALPHA"))
      saxs_document_add_property(doc, "current-alpha",
                                 extract(firstline, ":"));

    /*
     * Example line:
     * "           Total  estimate : 0.251  which is     A BAD      solution"
     *                               ^^^^^
     */
    else if (strstr(firstline->line_buffer, "Total  estimate"))
      saxs_document_add_property(doc, "total-estimate",
                                 extract(firstline, ":"));

    firstline = firstline->next;
  }

  return 0;
}



static int parse_scattering_data(struct saxs_document *doc,
                                 struct line *firstline,
                                 struct line *lastline) {

  saxs_curve *curve_exp, *curve_reg;
  curve_exp = saxs_document_add_curve(doc, "data", SAXS_CURVE_SCATTERING_DATA);
  curve_reg = saxs_document_add_curve(doc, "fit", SAXS_CURVE_SCATTERING_DATA);

  /*
   * Skip empty and header lines until extrapolated data is
   * found. The data block generally is 5 columns wide, but
   * at the beginning, the extrapolated part is 2 columns only.
   */
  while (firstline != lastline) {
    double s, jexp, err, jreg, ireg;

    firstline = firstline->next;

    if (sscanf(firstline->line_buffer, "%lf %lf %lf %lf %lf",
               &s, &jexp, &err, &jreg, &ireg) == 5) {
      saxs_curve_add_data(curve_exp, s, 0.0, jexp, err);
      saxs_curve_add_data(curve_reg, s, 0.0, ireg, 0.0);

    } else if (sscanf(firstline->line_buffer, "%lf %lf",
                      &s, &ireg) == 2) {
      saxs_curve_add_data(curve_reg, s, 0.0, ireg, 0.0);

    }  else
      return -1;

    firstline = firstline->next;
  }

  return 0;
}

static int parse_probability_data(struct saxs_document *doc,
                                  struct line *firstline,
                                  struct line *lastline) {

  /*
   * Skip empty and header lines until data is found ...
   */
  while (saxs_reader_columns_count(firstline) != 3)
    firstline = firstline->next;

  /* distance distribution (r vs. P(r)) */
  if (saxs_reader_columns_parse(doc, firstline, lastline, 
                                0, 1.0, 1, 1.0, 2, "distance distribution",
                                SAXS_CURVE_PROBABILITY_DATA) != 0)
    return -1;

  return 0;
}

static int parse_footer(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {

  while (firstline != lastline) {
    /*
     * Example line:
     * "          Reciprocal space: Rg =    1.18     , I(0) =   0.3321E+02"
     *                                      ^^^^                ^^^^^^^^^^
     */
    if (strstr(firstline->line_buffer, "Reciprocal space")) {
      saxs_document_add_property(doc, "reciprocal-space-rg",
                                 extract(firstline, "Rg ="));
      saxs_document_add_property(doc, "reciprocal-space-I0",
                                 extract(firstline, "I(0) ="));
    }

    /*
     * Example line:
     * "     Real space: Rg =    1.31 +- 0.000  I(0) =   0.3330E+02 +-  0.5550E-01"
     *                           ^^^^                    ^^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Real space")) {
      saxs_document_add_property(doc, "real-space-rg",
                                 extract(firstline, "Rg ="));
      saxs_document_add_property(doc, "real-space-I0",
                                 extract(firstline, "I(0) ="));
    }

    firstline = firstline->next;
  }

  return 0;

}


int atsas_out_read(struct saxs_document *doc, const char *filename) {
  struct line *lines, *current;
  struct line *header, *scattering_begin, *probability_begin, *footer;

  /*
   * .out-files were meant to be human readable and are thus
   * "nicely" formatted for this purpose.
   *
   * Scan the lines, separate sections (header, scattering data,
   * probability data, footer) to be parsed later.
   */

  if (lines_read(&lines, filename) != 0)
    return -1;

  /*
   * The header starts at the first line and ends with:
   *     "S          J EXP       ERROR       J REG       I REG"
   */
  header = current = lines;
  while(current
        && !strstr(current->line_buffer,
                   "S          J EXP       ERROR       J REG       I REG"))
    current = current->next;

  scattering_begin = current;

  /*
   * Scattering data ends with:
   *     "Distance distribution  function of particle"
   */
  while (current
         && !strstr(current->line_buffer,
                    "Distance distribution  function of particle"))
    current = current->next;

  probability_begin = current;

  /*
   * Probability data ends with:
   *     "Reciprocal space"
   */
  while (current
         && !strstr(current->line_buffer, "Reciprocal space"))
    current = current->next;

  footer = current;

  /*
   * Now parse the individual sections and extract the data.
   */
  parse_header(doc, header, scattering_begin);
  parse_scattering_data(doc, scattering_begin, probability_begin);
  parse_probability_data(doc, probability_begin, footer);
  parse_footer(doc, footer, NULL);

  lines_free(lines);
  return 0;
}


/**************************************************************************/
#include "saxsdocument_format.h"

saxs_document_format*
saxs_document_format_atsas_out(const char *filename, const char *format) {
  static saxs_document_format atsas_out = { atsas_out_read, NULL };

  if (!compare_format(format, "out")
      || !compare_format(suffix(filename), "out"))
    return &atsas_out;

  return NULL;
}
