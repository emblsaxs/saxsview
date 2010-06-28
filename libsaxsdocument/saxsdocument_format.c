/*
 * Format handling of SAXS documents.
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

#include "saxsdocument_format.h"
#include "saxsdocument.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

saxs_document_format*
saxs_document_format_register_atsas_dat();
saxs_document_format*
saxs_document_format_register_atsas_fir_fit();
saxs_document_format*
saxs_document_format_register_atsas_int();
saxs_document_format*
saxs_document_format_register_atsas_out();

#ifdef HAVE_LIBXML2
saxs_document_format*
saxs_document_format_register_cansas_xml();
#endif


static int saxs_document_format_initialized = 0;
static saxs_document_format *format_head = 0L, *format_tail = 0L;


saxs_document_format*
saxs_document_format_create() {
  saxs_document_format *format = malloc(sizeof(saxs_document_format));

  format->name = NULL;
  format->description = NULL;
  format->check = NULL;
  format->read = NULL;
  format->write = NULL;
  format->next = NULL;

  return format;
}


void
saxs_document_format_free(saxs_document_format *format) {
  if (format->name)
    free((void*)format->name);

  if (format->description)
    free((void*)format->description);

  free(format);
}


void
saxs_document_format_init() {
  if (saxs_document_format_initialized)
    return;

  /*
   * TODO: is there a way (simple) to register known formats in a
   *       platform independent way without enumerating them?
   */
  saxs_document_format_register_atsas_dat();
  saxs_document_format_register_atsas_fir_fit();
  saxs_document_format_register_atsas_int();
  saxs_document_format_register_atsas_out();
#ifdef HAVE_LIBXML2
  saxs_document_format_register_cansas_xml;
#endif

  saxs_document_format_initialized = 1;
}


void
saxs_document_format_register(const saxs_document_format *format) {
  saxs_document_format *fmt = saxs_document_format_create();

  if (format->name)
    fmt->name = strdup(format->name);

  if (format->description)
    fmt->description = strdup(format->description);

  fmt->check = format->check;
  fmt->read = format->read;
  fmt->write = format->write;
  fmt->next = NULL;

  if (format_tail) {
    format_tail->next = fmt;
    format_tail = fmt;

  } else
    format_head = format_tail = fmt;
}


saxs_document_format*
saxs_document_format_first() {
  if (!saxs_document_format_initialized)
    saxs_document_format_init();

  return format_head;
}


saxs_document_format*
saxs_document_format_next(saxs_document_format *format) {
  return format ? format->next : NULL;
}


saxs_document_format*
saxs_document_format_find(const char *filename,
                          const char *formatname) {

  saxs_document_format *format;

  if (!saxs_document_format_initialized)
    saxs_document_format_init();

  for (format = format_head; format != NULL; format = format->next)
    if (format->check && format->check(filename, formatname))
      return format;

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

  if (!filename)
    return NULL;

  if (!strcmp(filename, "-"))
    return NULL;

  p =  strrchr(filename, '.');
  if (!p)
    return NULL;

  return p + 1;
}
