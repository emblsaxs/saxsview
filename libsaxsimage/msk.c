/*
 * Read files in Fit2D .msk format.
 * Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
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
#include <assert.h>
#include <errno.h>
#include <string.h>


typedef struct image_msk_private {
  int width;
  int height;
  int padding;    /* rows are padded to align on 32-bit boundaries */
  char *data;

} image_msk_private;

#define PRIVATE_DATA(p) ((image_msk_private*)(p))


/**************************************************************************/
int saxs_image_msk_open(void **data) {
  image_msk_private *private_data;

  assert(!*data);     /* Private data must not be allocated yet. */

  private_data         = malloc(sizeof(image_msk_private));
  private_data->data   = NULL;
  private_data->width  = 0;
  private_data->height = 0;

  *data = private_data;
  return 0;
}

int saxs_image_msk_read(void *data, const char *filename) {
  int row, col, bit, tmp;
  char *dp;

  image_msk_private *p = PRIVATE_DATA(data);

  FILE *fd = fopen(filename, "rb");
  if (!fd) {
    printf("%s: %s\n", filename, strerror(errno));
    return -1;
  }

  /*
   * Skip the identifier (see saxs_image_format_msk()).
   */
  fseek(fd, 16, SEEK_SET);

  /*
   * Next three four-byte blocks are width, height
   * and (possibly) the number of bits of padding.
   */
  fread(&p->width, sizeof(p->width), 1, fd);
  fread(&p->height, sizeof(p->height), 1, fd);
  fread(&p->padding, sizeof(p->padding), 1, fd);

  /*
   * Data starts with an offset of 1024 bytes.
   */
  fseek(fd, 1024, SEEK_SET);

  if (p->data)
    free (p->data);

  /*
   * Read and keep the padding bits, but do not report 
   * them as width. This simplifies reading as no special
   * boundary conditions need to be checked.
   */
  p->data = (char*)malloc((p->width + p->padding) * p->height * sizeof(char));

  for (row = 0; row < p->height; ++row)
    for (col = 0; col < p->width; col += sizeof(tmp) * CHAR_BIT) {
      fread(&tmp, sizeof(tmp), 1, fd);

      dp = p->data + row * (p->width + p->padding) + col;
      for (bit = 0; bit < sizeof(tmp) * CHAR_BIT; ++bit)
        dp[bit] = (tmp & (1 << bit)) ? 0x01 : 0x00;
    }

  fclose(fd);
  return 0;
}

int saxs_image_msk_close(void *data) {
  image_msk_private *p = PRIVATE_DATA(data);

  free(p->data);
  free(p);

  return 0;
}

size_t saxs_image_msk_width(void *data) {
  image_msk_private *p = PRIVATE_DATA(data);
  return p->width;
}

size_t saxs_image_msk_height(void *data) {
  image_msk_private *p = PRIVATE_DATA(data);
  return p->height;
}

long saxs_image_msk_value(void *data, int x, int y) {
  image_msk_private *p = PRIVATE_DATA(data);
  return *(p->data + y * (p->width + p->padding) + x) ? 1 : 0;
}

long saxs_image_msk_value_min(void *data) {
  return 0;
}

long saxs_image_msk_value_max(void *data) {
  return 1;
}


/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_msk(const char *filename, const char *format) {
  static saxs_image_format image_msk = { saxs_image_msk_open,
                                         saxs_image_msk_read,
                                         NULL, /* write */
                                         saxs_image_msk_close,
                                         saxs_image_msk_value,
                                         saxs_image_msk_width,
                                         saxs_image_msk_height,
                                         saxs_image_msk_value_min,
                                         saxs_image_msk_value_max };

  int header[4];

  FILE *fd = fopen(filename, "rb");
  if (!fd)
    return NULL;

  fread(header, sizeof(int), 4, fd);
  fclose(fd);

  /*
   * The first four times four bytes shall contain
   * the characters 'MASK':
   */
  if (   header[0] == 'M'
      && header[1] == 'A'
      && header[2] == 'S'
      && header[3] == 'K')
    return &image_msk;

  return NULL;
}
