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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct image_cbf_private {
  cbf_handle *cbf_libhandle;

  size_t width;
  size_t height;
  size_t bpp;
  unsigned int *data;

} image_cbf_private;

image_cbf_private*
private_data(struct saxs_image_format *format) {
  if (!format->private_data) {
    image_cbf_private *private_data = malloc(sizeof(image_cbf_private));
    printf("CBF: init private data\n");
    private_data->cbf_libhandle = NULL;
    private_data->data = NULL;

    format->private_data = private_data;
  }

  return (image_cbf_private*)format->private_data;
}

/**************************************************************************/
int saxs_image_cbf_open(struct saxs_image_format *format) {
  cbf_handle *cbf = malloc(sizeof(cbf_handle));

  if (cbf_make_handle(cbf))
    return -1;

  printf("CBF: open\n");
  private_data(format)->cbf_libhandle = cbf;
  return 0;
}

static int
saxs_image_cbf_read_high_level(struct saxs_image_format *format) {
  image_cbf_private* p = private_data(format);

  printf("CBF: high level api, get size: %d\n", cbf_get_image_size(*p->cbf_libhandle, 0, 0, &p->height, &p->width));
  if (cbf_get_image_size(*p->cbf_libhandle, 0, 0, &p->height, &p->width) != 0)
    return -1;

  if (p->data)
    free(p->data);

  p->data = malloc(p->width * p->height * sizeof(int));
  p->bpp = 32;

  printf("CBF: high level api, get image (%d, %d)\n", p->width, p->height);
  return cbf_get_image(*p->cbf_libhandle, 0, 0,
                       p->data,
                       sizeof(int),
                       0,
                       p->height,
                       p->width);
}

static int
saxs_image_cbf_read_low_level(struct saxs_image_format *format) {
  image_cbf_private* p = private_data(format);

  unsigned int compression;
  int is_signed, is_unsigned, is_real;
  size_t size, n, nread;

  cbf_select_datablock(*p->cbf_libhandle, 0);

  cbf_find_category(*p->cbf_libhandle, "array_data");
  cbf_find_column(*p->cbf_libhandle, "data");

  cbf_get_arrayparameters_wdims(*p->cbf_libhandle, &compression, NULL, &size, &is_signed,
                                &is_unsigned, &n, NULL, NULL, &is_real, NULL,
                                &p->width, &p->height, NULL, NULL);

  if (p->height == 0 || p->width == 0){
    cbf_get_image_size(*p->cbf_libhandle, 0, 0, &p->width, &p->height);

    if (p->height == 0 || p->width == 0)
      return -1;

    /* find the previous category and column */
    cbf_find_category(*p->cbf_libhandle, "array_data");
    cbf_find_column(*p->cbf_libhandle, "data");
  }

  p->bpp = size * 8;
  p->data = malloc(p->width * p->height * sizeof(int));

  if (p->data == NULL)
    return -1;

  if (is_real) {
    cbf_get_realarray(*p->cbf_libhandle,
                      0L,
                      p->data,
                      sizeof(int),
                      n,
                      &nread);
  } else {
    cbf_get_integerarray(*p->cbf_libhandle,
                         0L,
                         p->data,
                         sizeof(int),
                         is_signed,
                         n,
                         &nread);
  }

  return (n == nread) ? 0 : -1;
}

int saxs_image_cbf_read(struct saxs_image_format *format, const char *filename) {
  FILE *fd = fopen(filename, "rb");
  if (!fd)
    return -1;

  cbf_read_file(*private_data(format)->cbf_libhandle, fd, MSG_DIGEST);

  if (saxs_image_cbf_read_high_level(format) == 0)
    return 0;
  else if (saxs_image_cbf_read_low_level(format) == 0)
    return 0;
  else
    return -1;
}

int saxs_image_cbf_close(struct saxs_image_format *format) {
  image_cbf_private* p = private_data(format);

  cbf_free_handle(*p->cbf_libhandle);

  free(p->cbf_libhandle);
  free(p->data);

  printf("CBF: close\n");
  return 0;
}

size_t saxs_image_cbf_width(struct saxs_image_format *format) {
  return private_data(format)->width;
}

size_t saxs_image_cbf_height(struct saxs_image_format *format) {
  return private_data(format)->height;
}

size_t saxs_image_cbf_value(struct saxs_image_format *format, int x, int y) {
  const image_cbf_private* p = private_data(format);
    if (x >= p->width)
      printf("X too big (%d/%d)\n", x, p->width);
    if (y >= p->height)
      printf("Y too big (%d/%d)\n", y, p->height);


/* printf("value(%d,%d) = %ud\n", x, y, *(p->data + x * p->width + y * p->height)); */
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
                                         NULL, /* value_max */
                                         NULL /* private data */ };

  if (!compare_format(format, "cbf")
      || !compare_format(suffix(filename), "cbf")

      /* The software of MAR345 writes images with suffix '.cbf2300' */
      || !compare_format(format, "cbf2300")
      || !compare_format(suffix(filename), "cbf2300"))
    return &image_cbf;

  return NULL;
}
