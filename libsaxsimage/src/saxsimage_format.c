/*
 * Format handling of SAXS images.
 * Copyright (C) 2009-2015 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsimage_format.h"
#include "saxsimage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef saxs_image_format* (*format_handler)(const char*, const char*);

saxs_image_format*
saxs_image_format_cbf(const char*, const char*);
#ifdef HAVE_EDF
saxs_image_format*
saxs_image_format_edf(const char*, const char*);
#endif
saxs_image_format*
saxs_image_format_msk(const char*, const char*);
#ifdef HAVE_TIFF
saxs_image_format*
saxs_image_format_tiff(const char*, const char*);
#endif
#ifdef HAVE_HDF5
saxs_image_format*
saxs_image_format_hdf5(const char*, const char*);
#endif

saxs_image_format*
saxs_image_format_find(const char *filename, const char *format) {

  format_handler known_formats[] = {
    saxs_image_format_cbf,
#ifdef HAVE_EDF
    saxs_image_format_edf,
#endif
    saxs_image_format_msk,
#ifdef HAVE_TIFF
    saxs_image_format_tiff,
#endif
#ifdef HAVE_HDF5
    saxs_image_format_hdf5,
#endif
    NULL
  };

  format_handler *fmt;

  for (fmt = known_formats; *fmt; ++fmt) {
    saxs_image_format *handler = (*fmt)(filename, format);
    if (handler)
      return handler;
  }

  return NULL;
}
