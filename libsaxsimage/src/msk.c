/*
 * Read and write files in Fit2D .msk format.
 * Copyright (C) 2011, 2013 Daniel Franke <dfranke@users.sourceforge.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#ifndef DBL_EPSILON
#define DBL_EPSILON 1e-16
#endif


int saxs_image_msk_read(saxs_image *image, const char *filename, size_t frame) {
  int magic[4] = { 0 }, datasize, *data = NULL, *tmp;
  int width, height, padding;
  int row, col, bit;

  /* msk images have only one frame */
  if (frame != 1)
    return -2;

  FILE *fd = fopen(filename, "rb");
  if (!fd)
    return errno;

  /* Check the magic header. */
  if (fread(magic, sizeof(magic), 1, fd) != 1)
    goto error;

  /*
   * The first four times four bytes shall contain
   * the characters 'MASK':
   */
  if (magic[0] != 'M' || magic[1] != 'A' || magic[2] != 'S' || magic[3] != 'K')
    goto error;

  /*
   * Next three four-byte blocks are width, height
   * and (probably) the number of bits of padding.
   */
  if (    fread(&width, sizeof(width), 1, fd) != 1
       || fread(&height, sizeof(height), 1, fd) != 1
       || fread(&padding, sizeof(padding), 1, fd) != 1)
    goto error;

  /*
   * Data starts with an offset of 1024 bytes.
   */
  if (fseek(fd, 1024, SEEK_SET) != 0)
    goto error;

  datasize = ceil(width / (double)(sizeof(int) * CHAR_BIT)) * sizeof(int) * height;
  data     = malloc(datasize);
  if (fread(data, datasize, 1, fd) != 1)
    goto error;

  fclose(fd);

  /*
   * Read the padding bits, but short-circuit the column loop;
   * this simplifies reading as no further special boundary
   * conditions need to be checked.
   */
  saxs_image_set_size(image, width, height, 1, 1);

  tmp = data;
  for (row = 0; row < height; ++row)
    for (col = 0; col < width; col += sizeof(int) * CHAR_BIT) {
      for (bit = 0; (unsigned)bit < sizeof(int) * CHAR_BIT && col + bit < width; ++bit)
        saxs_image_set_value(image, col + bit, row, ((*tmp) & (1 << bit)) ? 1.0 : 0.0);

      ++tmp;
    }

  free(data);
  return 0;

error:
  fclose(fd);
  if (data)
    free(data);

  return ENOTSUP;
}


int saxs_image_msk_write(saxs_image *image, const char *filename) {
  const int magic[4] = { 'M', 'A', 'S', 'K' };

  int width, height, padding;
  int row, col, bit;

  FILE *fd = fopen(filename, "wb");
  if (!fd)
    return errno;

  width   = saxs_image_width(image);
  height  = saxs_image_height(image);
  padding = sizeof(int) * (width / sizeof(int) + 1) - width;

  /* Header: any endianess issues here? */
  fwrite(magic, sizeof(magic), 1, fd);
  fwrite(&width, sizeof(width), 1, fd);
  fwrite(&height, sizeof(height), 1, fd);
  fwrite(&padding, sizeof(padding), 1, fd);

  /* Data starts with an offset of 1024 bytes. */
  fseek(fd, 1024, SEEK_SET);

  for (row = 0; row < height; ++row)
    for (col = 0; col < width; col += sizeof(int) * CHAR_BIT) {
      int tmp = 0;

      for (bit = 0; (unsigned)bit < sizeof(tmp) * CHAR_BIT && col + bit < width; ++bit) {
        double value = saxs_image_value(image, col + bit, row);
        if (fabs(value) > DBL_EPSILON)
          tmp |= (1 << bit);
      }

      fwrite(&tmp, sizeof(tmp), 1, fd);
    }

  /* Any footer? */

  fclose(fd);
  return 0;
}


/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_msk(const char *filename, const char *format) {
  static saxs_image_format image_msk = { saxs_image_msk_read,
                                         saxs_image_msk_write };

  if (!compare_format(format, "msk")
      || !compare_format(suffix(filename), "msk"))
    return &image_msk;

  return NULL;
}
