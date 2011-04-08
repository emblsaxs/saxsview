/*
 * Read files in .cbf-format (using cbflib).
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

#include "saxsimage.h"
#include "saxsimage_format.h"

#include "cbf.h"
#include "cbf_simple.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct image_cbf_private {
  cbf_handle cbf;

  size_t width;
  size_t height;
  size_t bpp;
  unsigned int *data;

} image_cbf_private;

#define PRIVATE_DATA(p) ((image_cbf_private*)(p))


/**************************************************************************/
int saxs_image_cbf_open(void **data) {
  image_cbf_private *private_data;

  assert(!*data);     /* Private data must not be allocated yet. */

  private_data         = malloc(sizeof(image_cbf_private));
  private_data->width  = 0;
  private_data->height = 0;
  private_data->bpp    = 0;
  private_data->data   = NULL;

  if (cbf_make_handle(&private_data->cbf))
    return -1;

  *data = private_data;
  return 0;
}

static int
saxs_image_cbf_read_high_level(image_cbf_private *p) {
  if (cbf_get_image_size(p->cbf, 0, 0, &p->height, &p->width) != 0)
    return -1;

  if (p->data)
    free(p->data);

  p->data = malloc(p->width * p->height * sizeof(int));
  p->bpp = 32;

  return cbf_get_image(p->cbf, 0, 0,
                       p->data,
                       sizeof(int),
                       1,            /* elsign: signed */
                       p->height,
                       p->width);
}

static int
saxs_image_cbf_read_low_level(image_cbf_private *p) {
  unsigned int compression;
  int is_signed, is_unsigned, is_real;
  size_t size, n, nread;

  cbf_select_datablock(p->cbf, 0);

  cbf_find_category(p->cbf, "array_data");
  cbf_find_column(p->cbf, "data");

  cbf_get_arrayparameters_wdims(p->cbf, &compression, NULL, &size, &is_signed,
                                &is_unsigned, &n, NULL, NULL, &is_real, NULL,
                                &p->width, &p->height, NULL, NULL);

  if (p->height == 0 || p->width == 0){
    cbf_get_image_size(p->cbf, 0, 0, &p->width, &p->height);

    if (p->height == 0 || p->width == 0)
      return -1;

    /* find the previous category and column */
    cbf_find_category(p->cbf, "array_data");
    cbf_find_column(p->cbf, "data");
  }

  p->bpp = size * 8;
  p->data = malloc(p->width * p->height * sizeof(int));

  if (p->data == NULL)
    return -1;

  if (is_real) {
    cbf_get_realarray(p->cbf,
                      0L,
                      p->data,
                      sizeof(int),
                      n,
                      &nread);
  } else {
    cbf_get_integerarray(p->cbf,
                         0L,
                         p->data,
                         sizeof(int),
                         is_signed,
                         n,
                         &nread);
  }

  return (n == nread) ? 0 : -1;
}

int saxs_image_cbf_read(void *data, const char *filename) {
  image_cbf_private *p = PRIVATE_DATA(data);

  /* open() should have allocated memory already ... */
  assert(p);

  /* The 'b' is required for windows, overwise reading fails. */
  FILE *fd = fopen(filename, "rb");
  if (!fd)
    return -1;

  cbf_read_file(p->cbf, fd, MSG_DIGEST);

  if (saxs_image_cbf_read_high_level(p) == 0)
    return 0;
  else if (saxs_image_cbf_read_low_level(p) == 0)
    return 0;
  else
    return -1;
}

int saxs_image_cbf_close(void *data) {
  image_cbf_private *p = PRIVATE_DATA(data);
  assert(p);

  cbf_free_handle(p->cbf);
  free(p->data);
  free(p);

  return 0;
}

size_t saxs_image_cbf_width(void *data) {
  return PRIVATE_DATA(data)->width;
}

size_t saxs_image_cbf_height(void *data) {
  return PRIVATE_DATA(data)->height;
}

long saxs_image_cbf_value(void *data, int x, int y) {
  image_cbf_private *p = PRIVATE_DATA(data);

  if (x < 0 || x >= (signed)p->width
      || y < 0 || y > (signed)p->height)
    return 0;

  return *(p->data + y * p->width + x);
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_cbf(const char *filename, const char *format) {
  static saxs_image_format image_cbf = { saxs_image_cbf_open,
                                         saxs_image_cbf_read,
                                         NULL, /* write */
                                         saxs_image_cbf_close,
                                         saxs_image_cbf_value,
                                         saxs_image_cbf_width,
                                         saxs_image_cbf_height,
                                         NULL, /* value_min */
                                         NULL  /* value_max */ };

  if (!compare_format(format, "cbf")
      || !compare_format(suffix(filename), "cbf")

      /* The software of MAR345 writes images with suffix '.cbf2300' */
      || !compare_format(format, "cbf2300")
      || !compare_format(suffix(filename), "cbf2300"))
    return &image_cbf;

  return NULL;
}
