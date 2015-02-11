/*
 * Read/write files in EIGER HDF5 Format.
 * Copyright (C) 2015 Daniel Franke <dfranke@users.sourceforge.net>
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

#include <hdf5.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


int saxs_image_hdf5_read(saxs_image *image, const char *filename, size_t frame) {

  hid_t file_id, dataset_id, filespace, memspace;
  hsize_t dim[3], offset[3], size[3]; /* frames, xdim, ydim */
  herr_t res;
  int rank, x, y;
  int *mem;

  /* Open an existing file. */
  file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);

  /* Open an existing dataset. */
  dataset_id = H5Dopen2(file_id, "/entry/data", H5P_DEFAULT);

  /* Obtain the dataspace for whole dataset */
  filespace = H5Dget_space(dataset_id);

  /* Get rank and dimension of dataset */
  rank = H5Sget_simple_extent_ndims(filespace);
  H5Sget_simple_extent_dims(filespace, dim, NULL);

  mem = (int*)malloc(dim[1] * dim[2] * sizeof(int));
  if (!mem)
    return -1;

  /*
   * Create a description of the memory block we want to read, i.e. one frame.
   */
  size[0] = 1;
  size[1] = dim[1];
  size[2] = dim[2];
  memspace = H5Screate_simple(rank, size, NULL);

  /* 
   * Define the offset from the beginning of the data to the selected frame (hyperslab). 
   */
  offset[0] = frame - 1;   /* this frame (zero-offset) */
  offset[1] = 0;           /* beginning at (0,0) */
  offset[2] = 0;
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL /* stride */ , size /* count */, NULL /* block */);

  res = H5Dread(dataset_id, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, mem);

  if (res >= 0) {
    saxs_image_set_size(image, dim[1], dim[2], dim[0], frame);

    for(x = 0; x < dim[1]; x++)
       for(y = 0; y < dim[2]; y++)
         saxs_image_set_value(image, x, y, *(mem + x * dim[2] + y));
  }

  H5Sclose(memspace);
  H5Sclose(filespace);
  H5Dclose(dataset_id);
  H5Fclose(file_id);
  free(mem);

  return res < 0;
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_hdf5(const char *filename, const char *format) {
  static saxs_image_format image_hdf5 = { saxs_image_hdf5_read,
                                          NULL };

  if (!compare_format(format, "h5")
      || !compare_format(suffix(filename), "h5"))
    return &image_hdf5;

  return NULL;
}
