/*
 * Read files in .fir/.fit-formats (e.g. written by DAMMIN, OLIGOMER, ...).
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
   * .fir-files may have a 'title', but we simply ignore any 
   * information that might be available for now ...
   */
  return 0;
}

static int parse_data(struct saxs_document *doc,
                      struct line *firstline, struct line *lastline) {

  /*
   * Generally, .fit-files come with 3 columns (s, I, Ifit) and .fir-files
   * with 4 columns (s, I, err, Ifit). However, SASREF writes .fit-files
   * with 4 columns (identical to .fir-files for other apps).
   *
   * Further, OLIGOMER seems to write files with a fifth column (the difference
   * of I and Ifit). Also, the column order is different (s, I, Ifit, err, diff).
   *
   * Hence, regardless of the file extension, count the columns of the first
   * data line to decide which column is which.
   */
  switch (saxs_reader_columns_count(firstline)) {
    case 3:
      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 1, 1.0, -1, "data",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 2, 1.0, -1, "fit",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      return 0;

    case 4:
      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 1, 1.0, 2, "data",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 3, 1.0, -1, "fit",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      return 0;

    case 5:
      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 1, 1.0, 3, "data",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      if (saxs_reader_columns_parse(doc, firstline, lastline,
                                    0, 1.0, 2, 1.0, -1, "fit",
                                    SAXS_CURVE_SCATTERING_DATA) != 0)
        return -1;

      return 0;

    default:
      return -1;
  }

  return 0;
}

static int parse_footer(struct saxs_document *doc,
                        struct line *firstline, struct line *lastline) {
  /*
   * This should be empty?
   */
  return 0;
}


int saxs_reader_fir_fit(struct saxs_document *doc, const char *filename) {
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
