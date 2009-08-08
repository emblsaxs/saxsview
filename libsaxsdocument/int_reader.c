/*
 * Read files in .int-format (e.g. written by CRYSOL).
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

#include <string.h>

static int parse_header(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  /*
   * .int-files may have a 'title', but we simply ignore any 
   * information that might be available for now ...
   */
  return 0;
}

static int parse_data(struct saxs_document *doc,
                      struct line *firstline, struct line *lastline) {

  /*
   * .int-files are usually written by CRYSOL and contain 5 columns:
   *   s, I_final, I_atomic, I_excluded_volume, I_hydration_shell
   * where I_final is a function of the others.
   */
  if (saxs_reader_columns_count(firstline) != 5)
    return -1;

  /* s vs I_final */
  if (saxs_reader_columns_parse(doc, firstline, lastline, 
                                0, 1.0, 1, 1.0, -1, "final",
                                SAXS_CURVE_SCATTERING_DATA) != 0)
    return -1;

  /* s vs I_atomic */
  if (saxs_reader_columns_parse(doc, firstline, lastline, 
                                0, 1.0, 2, 1.0, -1, "atomic",
                                SAXS_CURVE_SCATTERING_DATA) != 0)

    return -1;

  /* s vs I_excluded_volume */
  if (saxs_reader_columns_parse(doc, firstline, lastline, 
                                0, 1.0, 3, 1.0, -1, "excluded volume",
                                SAXS_CURVE_SCATTERING_DATA) != 0)
    return -1;

  /* s vs I_hydration_shell */
  if (saxs_reader_columns_parse(doc, firstline, lastline, 
                                0, 1.0, 4, 1.0, -1, "hydration shell",
                                SAXS_CURVE_SCATTERING_DATA) != 0)
    return -1;

  return 0;
}

static int parse_footer(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  /*
   * This should be empty?
   */
  return 0;
}


int saxs_reader_int(struct saxs_document *doc, const char *filename) {
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
