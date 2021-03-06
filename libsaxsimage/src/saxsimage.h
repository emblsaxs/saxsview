/*
 * Main API for SAXS image creation and access.
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

#ifndef LIBSAXSDOCUMENT_SAXSIMAGE_H
#define LIBSAXSDOCUMENT_SAXSIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/*
 * Windows platforms do not define ENOTSUP (Operation not supported),
 * use ENOSYS (Function not implemented) instead.
 */
#ifndef ENOTSUP
#define ENOTSUP ENOSYS
#endif


#if (defined(__GNUC__) || defined(__clang__))
#  define SAXSIMAGE_PURE __attribute__((pure))
#else
#  define SAXSIMAGE_PURE
#endif


struct saxs_property;

struct saxs_image;
typedef struct saxs_image saxs_image;

saxs_image*
saxs_image_create();

saxs_image*
saxs_image_copy(saxs_image* image);

int
saxs_image_read(saxs_image *image, const char *filename, const char *format);

int
saxs_image_read_frame(saxs_image *image, size_t frameid);

int
saxs_image_write(saxs_image *image, const char *filename, const char *format);

void
saxs_image_free(saxs_image *image);

size_t
saxs_image_width(saxs_image *image) SAXSIMAGE_PURE;

size_t
saxs_image_height(saxs_image *image) SAXSIMAGE_PURE;

int
saxs_image_frame_count(saxs_image *image) SAXSIMAGE_PURE;

int
saxs_image_current_frame(saxs_image *image) SAXSIMAGE_PURE;

void
saxs_image_set_size(saxs_image *image,
                    size_t width, size_t height,
                    size_t frame_count, size_t current_frame);


/**
 * Filters out values below 0.
 */
double
saxs_image_value(saxs_image *image, int x, int y) SAXSIMAGE_PURE;

void
saxs_image_set_value(saxs_image *image, int x, int y, double value);

double
saxs_image_value_min(saxs_image *image);

double
saxs_image_value_max(saxs_image *image);


struct saxs_property*
saxs_image_add_property(saxs_image *image, const char *name, const char *value);

int
saxs_image_property_count(saxs_image *image) SAXSIMAGE_PURE;

struct saxs_property*
saxs_image_property_first(saxs_image *image) SAXSIMAGE_PURE;

struct saxs_property*
saxs_image_property_find_first(saxs_image *image, const char *name) SAXSIMAGE_PURE;

#undef SAXSIMAGE_PURE

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_SAXSIMAGE_H */
