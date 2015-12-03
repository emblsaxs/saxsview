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

#include "saxsimage.h"
#include "saxsimage_format.h"
#include "saxsproperty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <assert.h>

struct saxs_image {
  char *image_filename;

  size_t image_width;
  size_t image_height;
  double *image_data;

  size_t image_frame_count;
  size_t image_current_frame;

  /* Callback functions to deal with the current format. */
  const saxs_image_format *image_format;

  saxs_property_list *image_properties;

  /* pre computed and cached values */
  int cache_valid;
  double cache_min_value, cache_max_value;
};


static void
saxs_image_update_cache(saxs_image *image) {
  if (image && !image->cache_valid) {
    size_t i, j;
    const size_t width = saxs_image_width(image);
    const size_t height = saxs_image_height(image);

    image->cache_min_value = DBL_MAX;
    image->cache_max_value = DBL_MIN;

    for (i = 0; i < width; ++i)
      for (j = 0; j < height; ++j) {
        double value = saxs_image_value(image, i, j);
        if (image->cache_min_value > value)
          image->cache_min_value = value;
        if (image->cache_max_value < value)
          image->cache_max_value = value;
      }

    image->cache_valid = 1;
  }
}



saxs_image*
saxs_image_create() {
  saxs_image *image = malloc(sizeof(saxs_image));
  if (!image)
    return NULL;

  image->image_filename      = NULL;
  image->image_width         = 0;
  image->image_height        = 0;
  image->image_data          = NULL;
  image->image_frame_count   = 0;
  image->image_current_frame = 0;
  image->cache_valid         = 0;
  image->cache_min_value     = DBL_MAX;
  image->cache_max_value     = DBL_MIN;
  image->image_format        = NULL;
  image->image_properties    = saxs_property_list_create();
  if (!image->image_properties) {
    free(image);
    return NULL;
  }

  return image;
}

saxs_image*
saxs_image_copy(saxs_image *image) {
  if (!image)
    return NULL;

  saxs_image *copy = malloc(sizeof(saxs_image));
  if (!copy)
    return NULL;

  if (image->image_filename) {
    copy->image_filename      = strdup(image->image_filename);
    if (!copy->image_filename) {
      saxs_image_free(copy);
      return NULL;
    }
  }
  copy->image_data          = NULL;
  copy->image_format        = NULL;
  copy->image_frame_count   = image->image_frame_count;
  copy->image_current_frame = image->image_current_frame;
  copy->cache_valid         = image->cache_valid;
  copy->cache_min_value     = image->cache_min_value;
  copy->cache_max_value     = image->cache_max_value;

  copy->image_properties    = saxs_property_list_create();
  if (!copy->image_properties) {
    saxs_image_free(copy);
    return NULL;
  }
  const saxs_property *prop;
  for (prop = saxs_property_list_first(image->image_properties);
       prop != NULL;
       prop = saxs_property_next(prop)) {
    saxs_property *prop_copy = saxs_property_create(saxs_property_name(prop), saxs_property_value(prop));
    if (!prop_copy) {
      saxs_image_free(copy);
      return NULL;
    }
    saxs_property_list_insert(copy->image_properties, prop_copy);
  }

  if (image->image_data) {
    saxs_image_set_size(copy, image->image_width, image->image_height,
                        image->image_frame_count, image->image_current_frame);
    if (!copy->image_data) {
      saxs_image_free(copy);
      return NULL;
    }

    memcpy(copy->image_data, image->image_data,
           image->image_width * image->image_height * sizeof(double));
  }

  assert(copy);
  if(image->image_filename){
    assert(copy->image_filename);
    assert(!strcmp(image->image_filename, copy->image_filename));
  }
  assert(copy->image_properties);
  assert(saxs_property_list_count(copy->image_properties) == saxs_property_list_count(image->image_properties));
  if (image->image_data)
    assert(copy->image_data);
  return copy;
}

int
saxs_image_read(saxs_image *image, const char *filename, const char *format) {
  assert(image);
  assert(filename);

  const saxs_image_format* handler = saxs_image_format_find(filename, format);
  if (!handler)
    return -1;
  if (!handler->read)
    return -2;

  free(image->image_filename);
  image->image_filename = strdup(filename);
  if (!image->image_filename)
    return -3;
  image->image_format   = handler;

  int res = image->image_format->read(image, filename, 1);

  if (res == 0) {
    assert(image->image_data);
    assert(image->image_frame_count > 0);
    assert(image->image_current_frame == 1);
    assert(image->image_height > 0);
    assert(image->image_width > 0);
  }
  return res;
}

int
saxs_image_write(saxs_image *image, const char *filename, const char *format) {
  assert(image);
  assert(filename);
  assert(image->image_data);
  assert(image->image_frame_count > 0);
  assert(image->image_height > 0);
  assert(image->image_width > 0);

  const saxs_image_format* handler = saxs_image_format_find(filename, format);
  if (!handler)
    return -1;
  if (!handler->write)
    return -2;

  free(image->image_filename);
  image->image_filename = strdup(filename);
  if (!image->image_filename)
    return -3;
  image->image_format   = handler;

  return image->image_format->write(image, filename);
}

void
saxs_image_free(saxs_image *image) {
  if (image) {
    if (image->image_filename)
      free(image->image_filename);

    if (image->image_data)
      free(image->image_data);

    saxs_property_list_free(image->image_properties);

    free(image);
  }
}

const char *
saxs_image_filename(saxs_image *image) {
  return image ? image->image_filename : NULL;
}

size_t
saxs_image_width(saxs_image *image) {
  return image ? image->image_width : 0;
}

size_t
saxs_image_height(saxs_image *image) {
  return image ? image->image_height : 0;
}

int
saxs_image_frame_count(saxs_image *image) {
  return image ? image->image_frame_count : 0;
}

int
saxs_image_current_frame(saxs_image *image) {
  return image ? image->image_current_frame : 0;
}

void
saxs_image_set_size(saxs_image *image, size_t width, size_t height,
                    size_t frame_count, size_t current_frame) {

  if (image->image_data)
    free(image->image_data);

  image->image_width         = width;
  image->image_height        = height;
  image->image_frame_count   = frame_count;
  image->image_current_frame = current_frame;
  image->image_data          = (double*) calloc(width * height, sizeof(double));
}

int
saxs_image_read_frame(saxs_image *image, size_t frame) {
  assert(image);
  assert(frame >= 1);

  if (frame > saxs_image_frame_count(image))
    return EINVAL;

  if (frame == image->image_current_frame)
    return 0;

  int res = image->image_format->read(image, image->image_filename, frame);

  assert(image->image_data);
  assert(frame == image->image_current_frame);
  return res;
}


double
saxs_image_value(saxs_image *image, int x, int y) {
  assert(image != NULL);
  assert(image->image_data != NULL);
  assert(x >= 0);
  assert(x < (signed)image->image_width);
  assert(y >= 0);
  assert(y < (signed)image->image_height);

  return image->image_data[y * image->image_width + x];
}

void
saxs_image_set_value(saxs_image *image, int x, int y, double value) {
  assert(image != NULL);
  assert(image->image_data != NULL);
  assert(x >= 0);
  assert(x < (signed)image->image_width);
  assert(y >= 0);
  assert(y < (signed)image->image_height);

  image->image_data[y * image->image_width + x] = value;

  if (image->cache_valid)
    image->cache_valid = 0;
}


double
saxs_image_value_min(saxs_image *image) {
  if (image) {
    saxs_image_update_cache(image);
    return image->cache_min_value;

  } else
    return 0.0;
}

double
saxs_image_value_max(saxs_image *image) {
  if (image) {
    saxs_image_update_cache(image);
    return image->cache_max_value;

  } else
    return 0.0;
}



saxs_property*
saxs_image_add_property(saxs_image *image, const char *name, const char *value) {
  if (!image)
    return NULL;

  saxs_property *property = saxs_property_create(name, value);
  if (!property)
    return NULL;

  saxs_property_list_insert(image->image_properties, property);
  return property;
}

int
saxs_image_property_count(saxs_image *image) {
  return saxs_property_list_count(image->image_properties);
}

saxs_property*
saxs_image_property_first(saxs_image *image) {
  return saxs_property_list_first(image->image_properties);
}

saxs_property*
saxs_image_property_find_first(saxs_image *image, const char *name) {
  return saxs_property_list_find_first(image->image_properties, name);
}
