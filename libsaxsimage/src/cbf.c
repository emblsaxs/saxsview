/*
 * Read files in .cbf-format (using cbflib).
 * Copyright (C) 2009, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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


static int
saxs_image_cbf_read_high_level(cbf_handle cbf,
                               size_t *width, size_t *height, int **data) {

  if (cbf_get_image_size(cbf, 0, 0, height, width) != 0)
    return -1;

  *data = malloc((*width) * (*height) * sizeof(int));
  return cbf_get_image(cbf, 0, 0,
                       *data,
                       sizeof(int),
                       1,            /* elsign: signed */
                       *height,
                       *width);
}


static int
saxs_image_cbf_read_low_level(cbf_handle cbf,
                              size_t *width, size_t *height, int **data) {
  unsigned int compression;
  int is_signed, is_unsigned, is_real;
  size_t size, n, nread;

  cbf_select_datablock(cbf, 0);

  if (!cbf_find_category(cbf, "array_data"))
    return -1;

  if (!cbf_find_column(cbf, "data"))
    return -1;

  cbf_get_arrayparameters_wdims(cbf, &compression, NULL, &size, &is_signed,
                                &is_unsigned, &n, NULL, NULL, &is_real, NULL,
                                width, height, NULL, NULL);

  if (*height == 0 || *width == 0){
    cbf_get_image_size(cbf, 0, 0, width, height);

    if (*height == 0 || *width == 0)
      return -1;

    /* find the previous category and column */
    cbf_find_category(cbf, "array_data");
    cbf_find_column(cbf, "data");
  }

  *data = malloc((*width) * (*height) * sizeof(int));
  if (is_real)
    cbf_get_realarray(cbf, NULL, *data, sizeof(int), n, &nread);
  else
    cbf_get_integerarray(cbf, NULL, *data, sizeof(int), is_signed, n, &nread);

  return (n == nread) ? 0 : -1;
}


int saxs_image_cbf_read(saxs_image *image, const char *filename, size_t frame) {
  size_t width, height, res;
  int *data;
  cbf_handle cbf;
  FILE *fd;

  /* CBF images have only one frame */
  if (frame != 1)
    return -2;

  /* The 'b' is required for windows, overwise reading fails. */
  fd = fopen(filename, "rb");
  if (!fd)
    return -1;

  if (cbf_make_handle(&cbf)) {
    fclose(fd);
    return -1;
  }

  cbf_read_file(cbf, fd, MSG_DIGEST);

  res = (saxs_image_cbf_read_high_level(cbf, &width, &height, &data) == 0
         || saxs_image_cbf_read_low_level(cbf, &width, &height, &data) == 0) ? 0 : -1;

  /* Also closes 'fd'. */
  cbf_free_handle(cbf);

  if (res == 0) {
    size_t x, y;

    saxs_image_set_size(image, width, height, 1, 1);
    for (x = 0; x < width; ++x)
      for (y = 0; y < height; ++y)
        saxs_image_set_value(image, x, height - y - 1, *(data + y * width + x));

    free(data);
  }

  return res;
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_cbf(const char *filename, const char *format) {
  static saxs_image_format image_cbf = { saxs_image_cbf_read,
                                         NULL, /* write */ };

  if (!compare_format(format, "cbf")
      || !compare_format(suffix(filename), "cbf")

      /* The software of MAR345 writes images with suffix '.cbf2300' */
      || !compare_format(format, "cbf2300")
      || !compare_format(suffix(filename), "cbf2300"))
    return &image_cbf;

  return NULL;
}
