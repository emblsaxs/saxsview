/*
 * Read files in .edf-format (using edfpack).
 * Copyright (C) 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of libsaxsimage.
 *
 * libsaxsimage is free software: you can redistribute it 
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * libsaxsimage is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with libsaxsimage. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "saxsimage.h"
#include "saxsimage_format.h"

#include "edfio.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct image_edf_private {
  int fd;
  long *dim;           /* allocated by edf_read_data, first element
                          holds number of elements to follow */
  size_t size;         /* number of data elements */

  float* data;

} image_edf_private;

#define PRIVATE_DATA(p) ((image_edf_private*)(p))


/**************************************************************************/
int saxs_image_edf_open(void **data) {
  image_edf_private *private_data;

  assert(!*data);     /* Private data must not be allocated yet. */

  private_data = malloc(sizeof(image_edf_private));
  private_data->fd   = -1;
  private_data->dim  = NULL;
  private_data->size = 0;
  private_data->data = NULL;

  *data = private_data;
  return 0;
}

int saxs_image_edf_read(void *data, const char *filename) {
  int edf_errno, status;

  image_edf_private *p = PRIVATE_DATA(data);

  /* open() should have allocated memory already ... */
  assert(p);

  p->fd = edf_open_data_file (filename, "old", &edf_errno, &status);
  if (status) {
    p->fd = -1;
    fprintf(stderr, "edf: error on open: %s", edf_report_data_error(edf_errno));
    return status;
  }

  edf_read_data(p->fd,
                1,               /* Image 1 */
                1,               /* Memory 1 (image data), -1 (variance) */
                &p->dim,
                &p->size,
                (void**) &p->data, MFloat,
                &edf_errno, &status);

  if (status) {
    fprintf(stderr, "edf: error on read: %s", edf_report_data_error(edf_errno));
    return status;
  }

  return 0;
}

int saxs_image_edf_close(void *data) {
  int edf_errno, status;

  image_edf_private *p = PRIVATE_DATA(data);
  assert(p);

  edf_close_data_file(p->fd, &edf_errno, &status);
  free(p);

  return 0;
}

size_t saxs_image_edf_width(void *data) {
  return PRIVATE_DATA(data)->dim[1];
}

size_t saxs_image_edf_height(void *data) {
  return PRIVATE_DATA(data)->dim[2];
}

long saxs_image_edf_value(void *data, int x, int y) {
  image_edf_private *p = PRIVATE_DATA(data);

  if (x < 0 || x >= p->dim[1]
      || y < 0 || y > p->dim[2])
    return 0;

  return (long)*(p->data + y * p->dim[1] + x);
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_edf(const char *filename, const char *format) {
  static saxs_image_format image_edf = { saxs_image_edf_open,
                                         saxs_image_edf_read,
                                         NULL, /* write */
                                         saxs_image_edf_close,
                                         saxs_image_edf_value,
                                         saxs_image_edf_width,
                                         saxs_image_edf_height,
                                         NULL, /* value_min */
                                         NULL  /* value_max */ };

  if (!compare_format(format, "edf")
      || !compare_format(suffix(filename), "edf"))
    return &image_edf;

  return NULL;
}
