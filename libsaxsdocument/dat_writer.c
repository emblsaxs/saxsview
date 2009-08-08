/*
 * Write files in .dat-format (used by EMBL-Hamburg).
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

#include "saxsdocument.h"

#include <stdio.h>
#include <string.h>


static void write_header(FILE *fd, saxs_document *doc) {
  saxs_property *title = saxs_document_property_find(doc, "title");
  saxs_property *desc = saxs_document_property_find(doc, "sample-description");
  saxs_property *code = saxs_document_property_find(doc, "sample-code");
  saxs_property *conc = saxs_document_property_find(doc, "sample-concentration");

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


int saxs_writer_dat(struct saxs_document *doc, const char *filename) {
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
