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
#include <errno.h>
#include <string.h>


int saxs_image_msk_read(saxs_image *image, const char *filename) {
  int width, height, padding;
  int row, col, bit, tmp;

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
   * and (probably) the number of bits of padding.
   */
  fread(&width, sizeof(width), 1, fd);
  fread(&height, sizeof(height), 1, fd);
  fread(&padding, sizeof(padding), 1, fd);

  /*
   * Data starts with an offset of 1024 bytes.
   */
  fseek(fd, 1024, SEEK_SET);

  /*
   * Read the padding bits, but short-circuit the column loop;
   * this simplifies reading as no further special boundary
   * conditions need to be checked.
   */
  saxs_image_set_size(image, width, height);
  for (row = 0; row < height; ++row)
    for (col = 0; col < width; col += sizeof(tmp) * CHAR_BIT) {
      fread(&tmp, sizeof(tmp), 1, fd);

      for (bit = 0; (unsigned)bit < sizeof(tmp) * CHAR_BIT && col + bit < width; ++bit)
        saxs_image_set_value(image, col + bit, row, (tmp & (1 << bit)) ? 1.0 : 0.0);
    }

  fclose(fd);
  return 0;
}


/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_msk(const char *filename, const char *format) {
  static saxs_image_format image_msk = { saxs_image_msk_read, NULL };

  int header[4];

  if (!compare_format(format, "msk")
      || !compare_format(suffix(filename), "msk")) {

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
  }

  return NULL;
}
