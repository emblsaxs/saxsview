/*
 * Read files in .out-format (e.g. written by GNOM).
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

/**************************************************************************/
/*
 * Find TEXT in "DELIM__TEXT__" where '__' is one or more whitespace
 * or newline characters, but TEXT may contain whitespaces itself.
 *
 * Add the TEXT as a property with the given name
 */
static int
extract_property(struct saxs_document *doc,
                 const char *name,
                 const struct line *l,
                 const char *delim) {
  assert_valid_line(l);

  const char *line = l->line_buffer;

  const char *value = strstr(line, delim);
  if (!value)
    return ENOTSUP;
  value += strlen(delim);

  while (isspace(*value)) {++value;}

  const char *valueend = value;
  while (*valueend && !isspace(*valueend)) {++valueend;}

  const saxs_property *p = saxs_document_add_property_strn(doc, name, -1, value, valueend-value);
  return (p)?0:ENOTSUP;
}

/* like `extract_property`, but take all text up to the end of the line */
static int
extract_property_line(struct saxs_document *doc,
                      const char *name,
                      const struct line *l,
                      const char *delim) {
  assert_valid_line(l);

  const char *line = l->line_buffer;

  const char *value = strstr(line, delim);
  if (!value)
    return ENOTSUP;
  value += strlen(delim);

  while (isspace(*value)) {++value;}

  const char *valueend = value + strlen(value);
  while ((valueend > value) && isspace(*(valueend-1))) {--valueend;}

  const saxs_property *p = saxs_document_add_property_strn(doc, name, -1, value, valueend-value);
  return (p)?0:ENOTSUP;
}

static int parse_header(struct saxs_document *doc,
                        const struct line *firstline,
                        const struct line *lastline) {

  assert_valid_lineset(firstline, lastline);

  while (firstline != lastline) {
    /*
     * Example line:
     * "           ####    G N O M   ---   Version 4.6                       ####"
     *                                             ^^^
     */
    if (strstr(firstline->line_buffer, "G N O M")) {
      saxs_document_add_property(doc, "creator", "GNOM");
      extract_property(doc, "creator-version", firstline, "Version");
    }

    /*
     * Example line:
     * "Run title:   sphere"
     * "Run title:  Lysozyme, high angles (>.22) 46 mg/ml, small angles (<.22) 15 mg/"
     */
    else if (strstr(firstline->line_buffer, "Run title"))
      /*
       * Contrary to any other place, here we want everything after the delimiter,
       * not just the token up to the next whitespace.
       */
      extract_property_line(doc, "title", firstline, ":");

    /*
     * Example lines:
     * "  Number of points omitted at the beginning:           9"
     *                                                         ^
     * "  Number of points omitted at the end:        1100"
     *                                                ^^^^
     * These lines are not present if '0' points are omitted.
     */
    else if (strstr(firstline->line_buffer, "omitted at the beginning"))
      extract_property(doc, "leading-points-omitted", firstline, ":");

    else if (strstr(firstline->line_buffer, "omitted at the end"))
      extract_property(doc, "trailing-points-omitted", firstline, ":");

    /*
     * Example line:
     * "   *******    Input file(s) : lyz_014.dat"
     *                                ^^^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Input file"))
      extract_property(doc, "parent", firstline, ":");

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
     * Example lines:
     * "Number of real space points  is too large! Modified to NR = 215"
     *                                                              ^^^
     * If the number of points was not modified, no line is printed.
     */
    else if (strstr(firstline->line_buffer, "Number of real space points"))
      extract_property(doc, "real-space-points", firstline, "=");

    /*
     * Example line:
     * " Warning: Dmax*Smin =  4.090   is greater than Pi"
     */
    else if (strstr(firstline->line_buffer, "greater than Pi"))
      saxs_document_add_property(doc, "warning-dmax*smin-greater-than-pi",
                                 "true");

    /*
     * Example line:
     * "  Real space range   :     from      0.00   to     10.00"
     *
     * Assumption: 'from' is always 0.0, then 'to' denotes Dmax.
     */
    else if (strstr(firstline->line_buffer, "Real space range"))
      extract_property(doc, "real-space-range", firstline, "to");

    /*
     * Example line:
     * "  Highest ALPHA (theor) :   0.182E+03                 JOB = 0"
     *                              ^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Highest ALPHA (theor)"))
      extract_property(doc, "highest-alpha-theor", firstline, ":");

    /*
     * Example line:
     * "  Current ALPHA         :   0.195E-18   Rg :  0.118E+01   I(0) :   0.332E+02"
     *                              ^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Current ALPHA"))
      extract_property(doc, "current-alpha", firstline, ":");

    /*
     * Example line:
     * "           Total  estimate : 0.251  which is     A BAD      solution"
     *                               ^^^^^
     */
    else if (strstr(firstline->line_buffer, "Total  estimate"))
      extract_property(doc, "total-estimate", firstline, ":");

// FIXME-1: properly handle 4.6 and 5.0 file versions.
// FIXME-2: first-point, last-point only work if there was only one input file,
//          if there are multiple, things get messy.
    else if (strstr(firstline->line_buffer, "First data point used"))
      extract_property(doc, "first-point", firstline, ":");

    else if (strstr(firstline->line_buffer, "Last data point used"))
      extract_property(doc, "last-point", firstline, ":");

    else if (strstr(firstline->line_buffer, "Reciprocal space Rg"))
      extract_property(doc, "reciprocal-space-rg", firstline, ":");

    else if (strstr(firstline->line_buffer, "Reciprocal space I(0)"))
      extract_property(doc, "reciprocal-space-I0", firstline, ":");

    else if (strstr(firstline->line_buffer, "Real space Rg"))
      extract_property(doc, "real-space-rg", firstline, ":");

    else if (strstr(firstline->line_buffer, "Real space I(0)"))
      extract_property(doc, "real-space-I0", firstline, ":");

    else if (strstr(firstline->line_buffer, "Total Estimate"))
      extract_property(doc, "total-estimate", firstline, ":");

    firstline = firstline->next;
  }

  return 0;
}

static int parse_scattering_data(struct saxs_document *doc,
                                 const struct line *firstline,
                                 const struct line *lastline) {

  assert_valid_lineset(firstline, lastline);

  saxs_curve *curve_exp, *curve_reg, *curve_des;
  curve_exp = saxs_document_add_curve(doc, "data", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
  curve_reg = saxs_document_add_curve(doc, "fit", SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
  curve_des = saxs_document_add_curve(doc, "desmeared", SAXS_CURVE_THEORETICAL_SCATTERING_DATA);

  /*
   * Skip empty and header lines until extrapolated data is
   * found. The data block generally is 5 columns wide, but
   * at the beginning, the extrapolated part is 2 columns only.
   */
  for (;firstline != lastline;firstline = firstline->next) {
    double s, jexp, err, jreg, ireg;

    if (sscanf(firstline->line_buffer, "%lf %lf %lf %lf %lf",
               &s, &jexp, &err, &jreg, &ireg) == 5) {
      if (isinf(s) || isnan(s))
        continue;
      if (isinf(jexp) || isnan(jexp)
#ifdef DO_NOT_ALLOW_NEGATIVE_ERRORS
        || (err < 0)
#endif
        || isinf(err) || isnan(err))
        continue;
      if (isinf(jreg) || isnan(jreg))
        continue;
      if (isinf(ireg) || isnan(ireg))
        continue;
      saxs_curve_add_data(curve_exp, s, 0.0, jexp, err);
      saxs_curve_add_data(curve_reg, s, 0.0, jreg, 0.0);
      saxs_curve_add_data(curve_des, s, 0.0, ireg, 0.0);

    } else if (sscanf(firstline->line_buffer, "%lf %lf",
                      &s, &ireg) == 2) {
      if (isinf(s) || isnan(s))
        continue;
      if (isinf(ireg) || isnan(ireg))
        continue;
      saxs_curve_add_data(curve_des, s, 0.0, ireg, 0.0);

    }
  }

  return 0;
}

static int parse_probability_data(struct saxs_document *doc,
                                  const struct line *firstline,
                                  const struct line *lastline) {

  assert_valid_lineset(firstline, lastline);

  /*
   * Skip empty and header lines until data is found ...
   */
  while (firstline != lastline
          && saxs_reader_columns_count(firstline) != 3)
    firstline = firstline->next;

  /* Ensure there is at least one line remaining */
  if (firstline == lastline)
    return ENOTSUP;

  /* distance distribution (r vs. p(r), r vs GammaC(r)) */
  saxs_reader_columns_parse(doc, firstline, lastline,
                            0, 1.0, 1, 1.0, 2, "p(r)",
                            SAXS_CURVE_PROBABILITY_DATA);

  return 0;
}

static int parse_footer(struct saxs_document *doc,
                        const struct line *firstline,
                        const struct line *lastline) {

  assert_valid_lineset(firstline, lastline);

  while (firstline != lastline) {
    /*
     * Example line:
     * "          Reciprocal space: Rg =    1.18     , I(0) =   0.3321E+02"
     *                                      ^^^^                ^^^^^^^^^^
     */
    if (strstr(firstline->line_buffer, "Reciprocal space")) {
      extract_property(doc, "reciprocal-space-rg", firstline, "Rg =");
      extract_property(doc, "reciprocal-space-I0", firstline, "I(0) =");
    }

    /*
     * Example line:
     * "     Real space: Rg =    1.31 +- 0.000  I(0) =   0.3330E+02 +-  0.5550E-01"
     *                           ^^^^                    ^^^^^^^^^^
     */
    else if (strstr(firstline->line_buffer, "Real space")) {
      extract_property(doc, "real-space-rg", firstline, "Rg =");
      extract_property(doc, "real-space-I0", firstline, "I(0) =");
    }

    firstline = firstline->next;
  }

  return 0;

}

int atsas_out_read(struct saxs_document *doc,
                   const struct line *firstline,
                   const struct line *lastline) {

  assert_valid_lineset(firstline, lastline);

  const struct line *current;
  const struct line *header = NULL, *scattering_begin = NULL, *probability_begin = NULL, *footer = NULL;
  int res;

  /*
   * .out files may come with multiple repeated data sections,
   * i.e. multiple full GNOM runs are being appended into the
   * same file. Some people would call it a feature, others
   * would consider it a bug. As per popular request, these
   * multi-segment files should be accepted and only the last
   * section should be read (if complete); ignore the rest.
   *
   * .out-files were meant to be human readable and are thus
   * "nicely" formatted for this purpose.
   *
   * Scan the lines, separate sections (header, scattering data,
   * probability data, footer) to be parsed later.
   */
  current = firstline;
  while(current) {
    /*
     * The header starts with the program name and version:
     * "           ####    G N O M   ---   Version 4.6                       ####"
     */
    if (strstr(current->line_buffer, "G N O M")) {
      header            = current;
      scattering_begin  = NULL;
      probability_begin = NULL;
      footer            = NULL;
    }

    /*
     * The scattering data (experimental and regularized) starts at the
     * first line and ends with:
     *     "S          J EXP       ERROR       J REG       I REG"
     */
    if (strstr(current->line_buffer, "J EXP")
         && strstr(current->line_buffer, "ERROR")
         && strstr(current->line_buffer, "J REG")
         && strstr(current->line_buffer, "I REG")) {
      scattering_begin = current;
      probability_begin = NULL;
      footer = NULL;
    }

    /*
     * Scattering data ends with:
     *     "Distance distribution  function of particle"       (gnom jobtype 0)
     *     "Volume distribution function of hard spheres"      (gnom jobtype 1)
     *     "Characteristic function of particle thickness"     (gnom jobtype 3)
     *     "Distance distribution function of cross-section"   (gnom jobtype 4)
     *     "Length distribution function of long cylinders"    (gnom jobtype 5)
     *     "Surface distribution function of spherical shells" (gnom jobtype 6)
     */
    if (strstr(current->line_buffer, "function of particle")
         || strstr(current->line_buffer, "function of hard spheres")
         || strstr(current->line_buffer, "particle thickness")
         || strstr(current->line_buffer, "function of cross-section")
         || strstr(current->line_buffer, "function of long cylinders")
         || strstr(current->line_buffer, "function of spherical shells")) {
      probability_begin = current;
      footer = NULL;
    }

    /*
     * Probability data ends with (v4.x):
     *     "Reciprocal space: Rg =   xx.xx     , I(0) =   x.xxxxxx"
     * or nothing at all (v5.x).
     */
    if (strstr(current->line_buffer, "Reciprocal space")
         && strstr(current->line_buffer, "Rg")
         && strstr(current->line_buffer, "I(0)")) {
      footer = current;
    }

    current = current->next;
  }

  /*
   * If any of the sections (except the footer) was not found, 
   * the lines do not come from a GNOM .out file.
   */
  if (!header || !scattering_begin || !probability_begin)
    return ENOTSUP;

  /*
   * Now parse the individual sections and extract the data.
   */
  res = parse_header(doc, header, scattering_begin);
  if (res)
    return res;
  res = parse_scattering_data(doc, scattering_begin, probability_begin);
  if (res)
    return res;
  res = parse_probability_data(doc, probability_begin, footer);
  if (res)
    return res;
  if (footer)
    res = parse_footer(doc, footer, lastline);
  return res;
}


/**************************************************************************/
void
saxs_document_format_register_atsas_out() {
  /*
   * .out-files are usually written by GNOM, DATGNOM or AUTOGNOM.
   */
  saxs_document_format atsas_out = {
     "out", "atsas-out", "ATSAS p(r) files (by GNOM)",
     atsas_out_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_out);
}
