/*
 * Read files in 32-bit .tiff-format.
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

#include <tiffio.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct image_tiff_private {
  uint32 width;
  uint32 height;
  uint16 bpp;           /* bits per pixel */
  uint16 spp;           /* samples per pixel */
  unsigned int *data;

} image_tiff_private;

#define PRIVATE_DATA(p) ((image_tiff_private*)(p))


/**************************************************************************/
int saxs_image_tiff_open(void **data) {
  image_tiff_private *private_data;

  assert(!*data);     /* Private data must not be allocated yet. */

  private_data       = malloc(sizeof(image_tiff_private));
  private_data->data = NULL;

  *data = private_data;
  return 0;
}

int saxs_image_tiff_read(void *data, const char *filename) {
  image_tiff_private *p = PRIVATE_DATA(data);
  tstrip_t strip;
  TIFF *tiff;
  uint16 samplesPerPixel = 0;

  /* open() should have allocated memory already ... */
  assert(p);

  /* The 'b' is required for windows, overwise reading fails. */
  tiff = TIFFOpen(filename, "rb");
  if (!tiff)
    return -1;

  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &p->width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &p->height);
  TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &p->bpp);

  if (TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &p->spp) == 0)
    p->spp = 1;

  p->data = _TIFFmalloc(p->width * p->height * p->bpp/8 * p->spp);

  for (strip = 0; strip < TIFFNumberOfStrips(tiff); ++strip)
    TIFFReadEncodedStrip(tiff,
                         strip,
                         ((char*)p->data) + strip * TIFFStripSize(tiff),
                         (tsize_t) -1);

  TIFFClose(tiff);
  return 0;
}

int saxs_image_tiff_close(void *data) {
  image_tiff_private *p = PRIVATE_DATA(data);

  _TIFFfree(p->data);
  free(p);

  return 0;
}

size_t saxs_image_tiff_width(void *data) {
  return PRIVATE_DATA(data)->width;
}

size_t saxs_image_tiff_height(void *data) {
  return PRIVATE_DATA(data)->height;
}

long saxs_image_tiff_value(void *data, int x, int y) {
  image_tiff_private *p = PRIVATE_DATA(data);

  if (x < 0 || x >= p->width
      || y < 0 || y > p->height)
    return 0;

  if (p->spp == 4) {
    unsigned char *rgba = (unsigned char *)(p->data + y * p->width + x);
    return (rgba[0]*11 + rgba[1]*16 + rgba[2]*5)/32;
/*  return (rgba[3]*11 + rgba[2]*16 + rgba[1]*5)/32; */

  } else
    return *(p->data + y * p->width + x);
}


/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_tiff(const char *filename, const char *format) {
  static saxs_image_format image_tiff = { saxs_image_tiff_open,
                                          saxs_image_tiff_read,
                                          NULL, /* write */
                                          saxs_image_tiff_close,
                                          saxs_image_tiff_value,
                                          saxs_image_tiff_width,
                                          saxs_image_tiff_height,
                                          NULL, /* value_min */
                                          NULL  /* value_max */ };

  if (!compare_format(format, "tiff")
      || !compare_format(suffix(filename), "tiff")
      || !compare_format(format, "tif")
      || !compare_format(suffix(filename), "tif"))
    return &image_tiff;

  return NULL;
}
