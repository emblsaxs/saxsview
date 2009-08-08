/*
 * Callbacks to format-specific parsers.
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
#include "formats.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

static struct saxs_format_callback_map {
  const char *format;
  saxs_format_callback callback;

} reader_map[] = {
  { "dat", saxs_reader_dat },
  { "fir", saxs_reader_fir_fit },
  { "fit", saxs_reader_fir_fit },
  { "int", saxs_reader_int },
  { "out", saxs_reader_out },
  { NULL, NULL }

}, writer_map[] = {
  { "dat", saxs_writer_dat },
  { NULL, NULL }
};

static saxs_format_callback callback_find(const char *filename,
                                          const char *format,
                                          struct saxs_format_callback_map *map) {

  char fmt[32] = { '\0' };
  const char *p;
  char *q;
  struct saxs_format_callback_map *cb;

  if (format == NULL || format[0] == '\0') {
    /* need format if reading from stdin */
    if (!strcmp(filename, "-"))
      return NULL;

    p =  strrchr(filename, '.');
    if (p)
      p += 1;

  } else
    p = format;

  if (!p)
    return NULL;

  q = fmt;
  while (p && *p)
    *q++ = tolower(*p++);

  for (cb = map; cb->format; ++cb)
    if (strcmp(fmt, cb->format) == 0)
      return cb->callback;

  return NULL;
}

saxs_format_callback saxs_reader_find(const char *filename,
                                      const char *format) {

  return callback_find(filename, format, reader_map);
}

saxs_format_callback saxs_writer_find(const char *filename,
                                      const char *format) {

  return callback_find(filename, format, writer_map);
}
