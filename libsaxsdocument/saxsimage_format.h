/*
 * Format handling of SAXS images.
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

#ifndef LIBSAXSDOCUMENT_SAXSIMAGE_FORMAT_H
#define LIBSAXSDOCUMENT_SAXSIMAGE_FORMAT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct saxs_image;

struct saxs_image_format {
  int (*open)(void **private_data);
  int (*read)(void *private_data, const char *filename);
  int (*write)(void *private_data, const char *filename);
  int (*close)(void *private_data);

  size_t (*value)(void *private_data, int x, int y);
  size_t (*width)(void *private_data);
  size_t (*height)(void *private_data);
  size_t (*value_min)(void *private_data);
  size_t (*value_max)(void *private_data);
};
typedef struct saxs_image_format saxs_image_format;


saxs_image_format*
saxs_image_format_find(const char *filename, const char *format);


/*
 * Also in saxsdocument_format.h, but to keep things separate ...
 */

/** Case-insensitive string comparison. */
int compare_format(const char *a, const char *b);

/** Extract the suffix of a filename, e.g. 'bsa.dat' has suffix 'dat'. */
const char* suffix(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_SAXSIMAGE_FORMAT_H */
