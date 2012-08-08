/*
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**************************************************************************/
static int
malvern_txt_parse_data(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {

  int i, n = saxs_reader_columns_count(firstline);
  if (n != 10)
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
malvern_txt_read(struct saxs_document *doc,
                 struct line *firstline, struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         NULL,
                                         malvern_txt_parse_data,
                                         NULL);
}


/**************************************************************************/
void
saxs_document_format_register_malvern_txt() {
  saxs_document_format malvern_txt = {
     "txt", "malvern-txt",
     "Data from Malvern OmniSEC text files.",
     malvern_txt_read, NULL, NULL
  };

  saxs_document_format_register(&malvern_txt);
}
