/*
 * Read files in .edf-format (using edfpack).
 * Copyright (C) 2010, 2011 Daniel Franke <dfranke@users.sourceforge.net>
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

int saxs_image_edf_read(saxs_image *image, const char *filename, size_t frame) {
  int i, fd, edf_errno, status;
  float *data = NULL;
  long *dim = NULL;    /* allocated by edf_read_data, first element
                          holds number of elements to follow */
  size_t size;         /* number of data elements */

  /* EDF images have only one frame */
  if (frame != 1)
    return -2;

  fd = edf_open_data_file(filename, "old", &edf_errno, &status);
  if (status) {
    fd = -1;
    fprintf(stderr, "edf: error on open: %s", edf_report_data_error(edf_errno));
    return status;
  }

  /*
   * The image index seems to vary depending on what kind of images are
   * used; data frames seem to have index 0, mask frames index 1?!
   */
  status = -1;
  for (i = 0; i < 10 && status != 0; ++i)
    edf_read_data(fd,
                  i,               /* Image index 0 */
                  1,               /* Memory 1 (image data), -1 (variance) */
                  &dim,
                  &size,
                  (void**) &data, MFloat,
                  &edf_errno, &status);

  if (status == 0) {
    int x, y;
    saxs_image_set_size(image, dim[1], dim[2], 1, 1);
    for (x = 0; x < dim[1]; ++x)
      for (y = 0; y < dim[2]; ++y)
        saxs_image_set_value(image, x, y, *(data + y * dim[1] + x));

  } else
    fprintf(stderr, "edf: error on read: %s", edf_report_data_error(edf_errno));

  /* Also free's the data */
  edf_close_data_file(fd, &edf_errno, &status);

  return status;
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_edf(const char *filename, const char *format) {
  static saxs_image_format image_edf = { saxs_image_edf_read,
                                         NULL, /* write */ };

  if (!compare_format(format, "edf")
      || !compare_format(suffix(filename), "edf"))
    return &image_edf;

  return NULL;
}
