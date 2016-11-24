/*
 * Read files in .int-format (e.g. written by CRYSOL).
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

#include <string.h>
#include <errno.h>

/**************************************************************************/
static int
atsas_int_parse_header(struct saxs_document *doc,
                       const struct line *firstline,
                       const struct line *lastline) {
  /*
   * .int-files may have a 'title', but we simply ignore any
   * information that might be available for now ...
   */
  return 0;
}

static int
atsas_int_parse_data(struct saxs_document *doc,
                     const struct line *firstline,
                     const struct line *lastline) {
  int res;

  if (saxs_reader_columns_count(firstline) != 5)
    return ENOTSUP;

  /* s vs I_final */
  res = saxs_reader_columns_parse(doc, firstline, lastline,
                                  0, 1.0, 1, 1.0, -1, "final",
                                  SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
  if (res != 0) return res;

  /* s vs I_atomic */
  res = saxs_reader_columns_parse(doc, firstline, lastline,
                                  0, 1.0, 2, 1.0, -1, "atomic",
                                  SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
  if (res != 0) return res;

  /* s vs I_excluded_volume */
  res = saxs_reader_columns_parse(doc, firstline, lastline,
                                  0, 1.0, 3, 1.0, -1, "excluded volume",
                                  SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
  if (res != 0) return res;

  /* s vs I_hydration_shell */
  res = saxs_reader_columns_parse(doc, firstline, lastline,
                                  0, 1.0, 4, 1.0, -1, "hydration shell",
                                  SAXS_CURVE_THEORETICAL_SCATTERING_DATA);
  if (res != 0) return res;

  return 0;
}

static int
atsas_int_parse_footer(struct saxs_document *doc,
                       const struct line *firstline,
                       const struct line *lastline) {
  /*
   * This should be empty?
   */
  return 0;
}

int
atsas_int_read(struct saxs_document *doc,
               struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         atsas_int_parse_header,
                                         atsas_int_parse_data,
                                         atsas_int_parse_footer);
}


/**************************************************************************/
void
saxs_document_format_register_atsas_int() {
  /*
   * .int-files are usually written by CRYSOL and contain 5 columns:
   *   s, I_final, I_atomic, I_excluded_volume, I_hydration_shell
   * where I_final is a function of the others.
   */
  saxs_document_format atsas_int = {
     "int", "atsas-int", "ATSAS theoretical intensities (by CRYSOL)",
     atsas_int_read, NULL, NULL
  };

  saxs_document_format_register(&atsas_int);
}
