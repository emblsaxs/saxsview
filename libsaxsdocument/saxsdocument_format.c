/*
 * Format handling of SAXS documents.
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

#include "saxsdocument_format.h"
#include "saxsdocument.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef saxs_document_format* (*format_handler)(const char *, const char*);

saxs_document_format*
saxs_document_format_atsas_dat(const char *, const char*);
saxs_document_format*
saxs_document_format_atsas_fir_fit(const char *, const char*);
saxs_document_format*
saxs_document_format_atsas_out(const char *, const char*);


saxs_document_format*
saxs_document_format_find(const char *filename,
                          const char *format) {

  format_handler known_formats[] = {
    saxs_document_format_atsas_dat,
    saxs_document_format_atsas_fir_fit,
    saxs_document_format_atsas_out,
    NULL
  };

  format_handler *fmt;

  for (fmt = known_formats; *fmt; ++fmt) {
    saxs_document_format *handler = (*fmt)(filename, format);
    if (handler)
      return handler;
  }

  return NULL;
}


int compare_format(const char *a, const char *b) {
  while (a && b) {
    if ((*a == '\0' && *b != '\0')
        || (*a != '\0' && *b == '\0'))
      return 1;

    else if (*a == '\0' && *b == '\0')
      return 0;

    else if (tolower(*a) != tolower(*b))
      return 1;

    a++;
    b++;
  }

  return 1;
}

const char* suffix(const char *filename) {
  const char *p;

  if (!strcmp(filename, "-"))
    return NULL;

  p =  strrchr(filename, '.');
  if (!p)
    return NULL;

  return p + 1;
}
