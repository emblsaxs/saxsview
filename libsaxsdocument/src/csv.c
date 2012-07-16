/*
 * Read files in .csv-format.
 * Copyright (C) 2011, 2012 Daniel Franke <dfranke@users.sourceforge.net>
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

static int
csv_parse_data(struct saxs_document *doc,
               struct line *firstline, struct line *lastline) {

  int i, n = saxs_reader_columns_count(firstline);

  for (i = 1; i < n; ++i)
    saxs_reader_columns_parse(doc, firstline, lastline,
                              0, 1.0, i, 1.0, -1,
                              "data",
                              SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  return 0;
}

int
csv_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        NULL, csv_parse_data, NULL);
}


static int
csv_write_header(struct saxs_document *doc, struct line **lines) {
  struct line *line;

  /* TODO: Add column headers?! */
  line = lines_create();
  lines_append(lines, line);

  return 0;
}

/*
 * TODO: besides the ',' in the output, this is identical
 *       to atsas_dat_n_column_write_data() - consolidate?
 */
static int
csv_write_data(struct saxs_document *doc, struct line **lines) {
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
      lines_printf(l, "%s, %14e", l->line_buffer, saxs_data_y(data));

      data = saxs_data_next(data);
      l = l->next;
    }

    curve = saxs_curve_next(curve);
  }

  lines_append(lines, firstline);
  return 0;
}

int
csv_write(struct saxs_document *doc, const char *filename) {
  return saxs_writer_columns_write_file(doc, filename,
                                        csv_write_header,
                                        csv_write_data,
                                        0L);
}


void
saxs_document_format_register_csv() {
  saxs_document_format csv = {
     "csv", "csv", "Columns of data, separated by a common separator",
     csv_read, csv_write, NULL
  };

  saxs_document_format_register(&csv);
}
