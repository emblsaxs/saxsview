/*
 * Read files in .dat-format (used by BioXTAS RAW).
 * Copyright (C) 2021 Al Kikhney <plmnnk@users.sourceforge.net>
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

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


static int
parse_key_value_pair(struct saxs_document *doc, const struct line *l) {

  /*
   * Keys and values are separated by ':' and a key may be any string.
   */
  const char *colon_pos = strchr(l->line_buffer, ':');
  if (colon_pos) {
    const char *key = l->line_buffer;
    int keylen = colon_pos - l->line_buffer;
    int keystart = 0;
    int valuestart = 0;

    const char *value = colon_pos+1;
    while (isspace(*value)) {++value;}
    int valuelen = strlen(value);

    if (valuelen == 0) {
      /* Skip empty values. */
      return 0;
    }


    /*
     * The properties are written in JSON. Here we cannot interpret the
     * JSON tree, we are interested only in whatever can be identified as
     * key-value pairs. The quotes will be removed from the key names and
     * string values.
     */
    if (!strcmp(value, "{") || !strcmp(value, "}") || !strcmp(value, "[") || !strcmp(value, "]") ) {
      /* Skip {}[] values. */
      return 0;
    }


    /* Remove the '"' around the key. */
    if (key[keystart] == '"' && key[keylen - 1] == '"') {
      ++keystart;
      keylen -= 2;
    }


    /* Remove '",' from the end of the value. */
    if (value[valuelen - 1] == ',') {
      --valuelen;
    }
    if (value[valuelen - 1] == '"') {
      --valuelen;
    }

    /* Remove '"' if it is the first character of the value. */
    if (value[valuestart] == '"') {
      ++valuestart;
      --valuelen;
    }

    saxs_document_add_property_strn(doc, key + keystart, keylen, value + valuestart, valuelen);

    /*
     * Attempt to set the "sample-concentration" and "sample-code" properties which are interpreted 
     * by some programs (e.g. primus).
     */
    if ((keylen == 20) && (strncmp(key + keystart, "sample_concentration", keylen) == 0)
         && saxs_document_property_find_first(doc, "sample-concentration") == NULL) {
      saxs_document_add_property_strn(doc, "sample-concentration", 20, value + valuestart, valuelen);
    }
    if ((keylen == 15) && (strncmp(key + keystart, "sample_bio_code", keylen) == 0)
         && saxs_document_property_find_first(doc, "sample-code") == NULL) {
      saxs_document_add_property_strn(doc, "sample-code", 11, value + valuestart, valuelen);
    }

    return 0;
  }
  return 1;
}




/* Files written by RAW always start with something like
 *
 * ### DATA:
 * #
 * # 391
 * #      Q             I(Q)           Error     
 *
 * followed by three columns of numbers. Here '391' is the number of data points.
 */
static int
raw_dat_parse_header(struct saxs_document *doc,
                     const struct line *firstline,
                     const struct line *lastline) {

  const char *line = firstline->line_buffer;
  if (strcmp(line, "DATA:")) {
    return ENOTSUP;  // header must start with '### DATA:' but hash symbols are trimmed
  }
  
  return 0;
}


static int
raw_dat_parse_footer(struct saxs_document *doc,
                     const struct line *firstline,
                     const struct line *lastline) {
  /*
   * The footer seems to be in JSON format but each line is preceded by '#'.
   * For now, parse it as 'name: value' pairs.
   */
  while (firstline != lastline) {
    parse_key_value_pair(doc, firstline);
    firstline = firstline->next;
  }

  return 0;
}


static int
raw_dat_parse_data(struct saxs_document *doc,
                   const struct line *firstline,
                   const struct line *lastline) {
  if (saxs_reader_columns_count(firstline) != 3)
    return ENOTSUP;

  return saxs_reader_columns_parse(doc,
                                   firstline, lastline,
                                   0, 1.0, 1, 1.0, 2,
                                   "data",
                                   SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
}

int
raw_dat_read(struct saxs_document *doc,
             const struct line *firstline,
             const struct line *lastline) {
  return saxs_reader_columns_parse_lines(doc, firstline, lastline,
                                         raw_dat_parse_header,
                                         raw_dat_parse_data,
                                         raw_dat_parse_footer);
}


/**************************************************************************/
void
saxs_document_format_register_raw_dat() {
  saxs_document_format raw_dat = {
     "dat", "raw-dat",
     "BioXTAS RAW three column scattering profile data",
     raw_dat_read, NULL, NULL
  };

  saxs_document_format_register(&raw_dat);
}
