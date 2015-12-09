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
#include <stdint.h>
/* endian.h is not present on some platforms
 * Instead declare htole32 and le32toh myself
#include <endian.h>
#define MSK_PUT(x) htole32(x)
#define MSK_GET(x) le32toh(x)
*/

#ifndef DBL_EPSILON
#define DBL_EPSILON 1e-16
#endif

typedef uint32_t msk_word;
#define MSK_WORD_SIZE (sizeof(msk_word))
#define MSK_WORD_BITS 32

static msk_word MSK_PUT(msk_word hword) {
  union {
    msk_word leword;
    uint8_t lechars[4];
  } mskput;

  mskput.lechars[0] = (hword & 0x000000FF) >>  0;
  mskput.lechars[1] = (hword & 0x0000FF00) >>  8;
  mskput.lechars[2] = (hword & 0x00FF0000) >> 16;
  mskput.lechars[3] = (hword & 0xFF000000) >> 24;

  return mskput.leword;
}

static msk_word MSK_GET(msk_word leword) {
  union {
    msk_word leword;
    uint8_t lechars[4];
  } mskget;
  msk_word hword;

  mskget.leword = leword;
  hword = 0;

  hword |= ((msk_word) mskget.lechars[0]) <<  0;
  hword |= ((msk_word) mskget.lechars[1]) <<  8;
  hword |= ((msk_word) mskget.lechars[2]) << 16;
  hword |= ((msk_word) mskget.lechars[3]) << 24;

  return hword;
}

int saxs_image_msk_read(saxs_image *image, const char *filename, size_t frame) {
  msk_word magic[4] = { 0 };
  msk_word *data = NULL, *tmp;
  msk_word lewidth, leheight, lepadding;
  unsigned int width, height;
  unsigned int row, col, bit;

  /* msk images have only one frame */
  if (frame != 1)
    return -2;

  /* msk images have only one frame */
  if (frame != 1)
    return -2;

  FILE *fd = fopen(filename, "rb");
  if (!fd)
    return errno;

  /* Check the magic header. */
  if (fread(magic, MSK_WORD_SIZE, 4, fd) != 4)
    goto error;

  /*
   * The first four times four bytes shall contain
   * the characters 'MASK':
   */
  if (MSK_GET(magic[0]) != 'M'
   || MSK_GET(magic[1]) != 'A'
   || MSK_GET(magic[2]) != 'S'
   || MSK_GET(magic[3]) != 'K')
    goto error;

  /*
   * Next three four-byte blocks are width, height
   * and (probably) the number of bits of padding.
   */
  if (fread(&lewidth, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;
  width = MSK_GET(lewidth);
  if (fread(&leheight, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;
  height = MSK_GET(leheight);
  if (fread(&lepadding, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;
  // Ignore the padding number

  /*
   * Data starts with an offset of 1024 bytes.
   */
  if (fseek(fd, 1024, SEEK_SET) != 0)
    goto error;

  const size_t rowsize = ceil(width / (float)MSK_WORD_BITS) * MSK_WORD_SIZE;
  const size_t datasize = rowsize * height;
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
    for (col = 0; col < width; col += MSK_WORD_BITS) {
      const msk_word leword = *tmp;
      const msk_word word = MSK_GET(leword);
      for (bit = 0; bit < MSK_WORD_BITS && col + bit < width; ++bit)
        saxs_image_set_value(image, col + bit, row, (word & (1 << bit)) ? 1.0 : 0.0);

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
  const msk_word magic[4] = { MSK_PUT('M'),
                              MSK_PUT('A'),
                              MSK_PUT('S'),
                              MSK_PUT('K') };

  unsigned int width, height, padding;
  msk_word lewidth, leheight, lepadding;
  int row, col, bit;

  FILE *fd = fopen(filename, "wb");
  if (!fd)
    return errno;

  width   = saxs_image_width(image);
  height  = saxs_image_height(image);
  padding = MSK_WORD_SIZE * (width / MSK_WORD_SIZE + 1) - width;

  lewidth = MSK_PUT(width);
  leheight = MSK_PUT(height);
  lepadding = MSK_PUT(padding);
  /* Header: any endianess issues here? */
  if (fwrite(magic, MSK_WORD_SIZE, 4, fd) != 4)
    goto error;
  if (fwrite(&lewidth, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;
  if (fwrite(&leheight, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;
  if (fwrite(&lepadding, MSK_WORD_SIZE, 1, fd) != 1)
    goto error;

  /* Data starts with an offset of 1024 bytes. */
  if (fseek(fd, 1024, SEEK_SET) != 0)
    goto error;

  for (row = 0; row < height; ++row)
    for (col = 0; col < width; col += MSK_WORD_BITS) {
      msk_word word = 0;

      for (bit = 0; (unsigned)bit < MSK_WORD_BITS && col + bit < width; ++bit) {
        double value = saxs_image_value(image, col + bit, row);
        if (fabs(value) > DBL_EPSILON)
          word |= (1 << bit);
      }

      const msk_word leword = MSK_PUT(word);
      if (fwrite(&leword, MSK_WORD_SIZE, 1, fd) != 1)
        goto error;
    }

  /* Any footer? */

  fclose(fd);
  return 0;

error:
  fclose(fd);

  if (errno)
    return errno;
  else
    return ENOTSUP;
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
