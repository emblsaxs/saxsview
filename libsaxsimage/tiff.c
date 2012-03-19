	/*
 * Read/write files in 32-bit .tiff-format.
 * Copyright (C) 2009, 2010, 2011 Daniel Franke <dfranke@users.sourceforge.net>
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
#include <limits.h>
#include <assert.h>

/*
 * Define a set of custom fields.
 * See:
 *    http://remotesensing.org/libtiff/addingtags.html
 */
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Custom fields defined and written by DECTRIS camserver */
#define DECTRIS_OFFSET                 0x9000
#define DECTRIS_TITLE_TAG              (DECTRIS_OFFSET + 0x0000)
#define DECTRIS_NUM_EXPOSURE_TAG       (DECTRIS_OFFSET + 0x0001)
#define DECTRIS_NUM_BACKGROUND_TAG     (DECTRIS_OFFSET + 0x0002)
#define DECTRIS_EXPOSURE_TIME_TAG      (DECTRIS_OFFSET + 0x0003)
#define DECTRIS_BACKGROUND_TIME_TAG    (DECTRIS_OFFSET + 0x0004)
#define DECTRIS_TELEMETRY_TAG          (DECTRIS_OFFSET + 0x0006)
#define DECTRIS_BLACK_LEVEL_TAG        (DECTRIS_OFFSET + 0x000c)
#define DECTRIS_DARK_CURRENT_TAG       (DECTRIS_OFFSET + 0x000d)
#define DECTRIS_READ_NOISE_TAG         (DECTRIS_OFFSET + 0x000e)
#define DECTRIS_DARK_CURRENT_NOISE_TAG (DECTRIS_OFFSET + 0x000f)
#define DECTRIS_BEAM_MONITOR_TAG       (DECTRIS_OFFSET + 0x0010)
#define DECTRIS_USER_VARIABLES_TAG     (DECTRIS_OFFSET + 0x0100)

static const TIFFFieldInfo dectris_custom_fields[12] = {
  { DECTRIS_TITLE_TAG, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_ASCII,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisTitleTag" },
  { DECTRIS_NUM_EXPOSURE_TAG, 1, 1, TIFF_LONG,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisNumExposureTag" },
  { DECTRIS_NUM_BACKGROUND_TAG, 1, 1, TIFF_LONG,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisNumBackgroundTag" },
  { DECTRIS_EXPOSURE_TIME_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisExposureTimeTag" },
  { DECTRIS_BACKGROUND_TIME_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisBackgroundTimeTag" },
  { DECTRIS_TELEMETRY_TAG, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_ASCII,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisTelemetryTag" },
  { DECTRIS_BLACK_LEVEL_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisBlackLevelTag" },
  { DECTRIS_DARK_CURRENT_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisDarkCurrentTag" },
  { DECTRIS_READ_NOISE_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisReadNoiseTag" },
  { DECTRIS_DARK_CURRENT_NOISE_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisDarkCurrentNoiseTag" },
  { DECTRIS_BEAM_MONITOR_TAG, 1, 1, TIFF_FLOAT,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisBeamMonitorTag" },
  { DECTRIS_USER_VARIABLES_TAG, 20, 20, TIFF_LONG,
    FIELD_CUSTOM, FALSE, FALSE, "DectrisUserVariablesTag" }
};

static TIFFExtendProc tiff_parent_extender = NULL;

static void tiff_default_directory(TIFF *tiff) {
  /* Install the extended Tag field info */
  TIFFMergeFieldInfo(tiff, dectris_custom_fields, 12);

  /*
   * Since an XTIFF client module may have overridden
   * the default directory method, we call it now to
   * allow it to set up the rest of its own methods.
   */
  if (tiff_parent_extender) 
    (*tiff_parent_extender)(tiff);
}

static void tiff_initialize(void) {
  static int first_time = 1;

  if (first_time) {
    /* Grab the inherited method and install */
    tiff_parent_extender = TIFFSetTagExtender(tiff_default_directory);

    /*
     * Even with the definition of the PILATUS specific custom
     * fields, a bunch of warnings is printed to stdout whenever
     * a PILATUS TIFF is opened. As the warnings are the same 
     * for every image, they are meaningless - ignore them.
     */
    TIFFSetWarningHandler(NULL);

    first_time = 0;
  }
}

/**************************************************************************/
int saxs_image_tiff_read(saxs_image *image, const char *filename) {
  TIFF *tiff;
  tstrip_t strip;
  uint16 bpp, spp;
  uint32 width, height, x, y;
  int *data;

  tiff_initialize();

  tiff = TIFFOpen(filename, "r");
  if (!tiff)
    return -1;

  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bpp);

  if (TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &spp) == 0)
    spp = 1;

  data = _TIFFmalloc(width * height * bpp/CHAR_BIT * spp);

  for (strip = 0; strip < TIFFNumberOfStrips(tiff); ++strip)
    TIFFReadEncodedStrip(tiff,
                         strip,
                         ((char*)data) + strip * TIFFStripSize(tiff),
                         (tsize_t) -1);

  saxs_image_set_size(image, width, height);
  for (x = 0; x < width; ++x)
    for (y = 0; y < height; ++y)
      if (spp == 1) {
        saxs_image_set_value(image, x, height - y - 1, *(data + y * width + x) * 1.0);

      } else if (spp == 3) {
        unsigned char *rgb = (char *)(data) + (y * width + x)*3;
        saxs_image_set_value(image, x, height - y - 1, (rgb[0]*11 + rgb[1]*16 + rgb[2]*5)/32);
      }

  TIFFClose(tiff);
  _TIFFfree(data);

  return 0;
}

int saxs_image_tiff_write(saxs_image *image, const char *filename) {
  TIFF *tiff;
  tstrip_t strip;
  uint32 width, height, x, y;
  int *data;

  tiff_initialize();

  /*
   * Do NOT pass a "binary mode 'b'" here as for other formats (see cbf.c)
   * the 'b' does have a different meaning for TIFF, namely:
   * "When creating a new file force information be written with Big-Endian
   *  byte order (but see below). By default the library will create new
   *  files using the native CPU byte order." (man TiffOpen)
   */
  tiff = TIFFOpen(filename, "w");
  if (!tiff)
    return -1;

  width  = saxs_image_width(image);
  height = saxs_image_height(image);

  data = _TIFFmalloc(width * height * 4 * 1);
  for (x = 0; x < width; ++x)
    for (y = 0; y < height; ++y)
      *(data + y * width + x) = saxs_image_value(image, x, height - y - 1);

  /*
   * Tags need to be sorted in ascending order.
   *
   * Do not compress as Fit2D can not handle compressed images ...
   * TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW );
   */
  TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 32);
  TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
  TIFFSetField(tiff, TIFFTAG_XRESOLUTION, 0.0);
  TIFFSetField(tiff, TIFFTAG_YRESOLUTION, 0.0);

  for (strip = 0; strip < TIFFNumberOfStrips(tiff); ++strip)
    TIFFWriteEncodedStrip(tiff,
                          strip,
                          ((char*)data) + strip * TIFFStripSize(tiff),
                          height * width * 4);

  TIFFClose(tiff);
  _TIFFfree(data);

  return 0;
}

/**************************************************************************/
#include "saxsimage_format.h"

saxs_image_format*
saxs_image_format_tiff(const char *filename, const char *format) {
  static saxs_image_format image_tiff = { saxs_image_tiff_read,
                                          saxs_image_tiff_write };

  if (!compare_format(format, "tiff")
      || !compare_format(suffix(filename), "tiff")
      || !compare_format(format, "tif")
      || !compare_format(suffix(filename), "tif"))
    return &image_tiff;

  return NULL;
}
