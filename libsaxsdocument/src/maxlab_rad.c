/*
 * Read/write files in .rad-format (used by MAXLAB).
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
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
#include <errno.h>

static int
maxlab_rad_parse_data(struct saxs_document *doc,
                      struct line *firstline, struct line *lastline) {
  if (saxs_reader_columns_count(firstline) != 4)
    return ENOTSUP;

  return saxs_reader_columns_parse(doc,
                                   firstline, lastline,
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
maxlab_rad_read(struct saxs_document *doc, const char *filename) {
  return saxs_reader_columns_parse_file(doc, filename,
                                        NULL,
                                        maxlab_rad_parse_data,
                                        NULL);
}

/**************************************************************************/
void
saxs_document_format_register_maxlab_rad() {
  saxs_document_format maxlab_rad = {
     "rad", "maxlab-rad",
     "MAXLAB experimental data",
     maxlab_rad_read, NULL, NULL
  };

  saxs_document_format_register(&maxlab_rad);
}
