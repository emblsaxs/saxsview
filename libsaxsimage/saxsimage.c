/*
 * Main API for SAXS image creation and access.
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
#include "saxsimage_format.h"
#include "../libsaxsdocument/saxsproperty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct saxs_image {
  char *image_filename;

  saxs_property_list *image_properties;

  /* Callback functions to deal with the current format. */
  saxs_image_format *image_format;

  /* Private data, handled completely by the format callbacks. */
  void *image_private_data;
};

saxs_image*
saxs_image_create() {
  saxs_image *image = malloc(sizeof(saxs_image));

  image->image_properties = saxs_property_list_create();
  image->image_filename = NULL;
  image->image_format = NULL;
  image->image_private_data = NULL;

  return image;
}

int
saxs_image_read(saxs_image *image, const char *filename, const char *format) {
  saxs_image_format* handler = saxs_image_format_find(filename, format);
  if (!handler)
    return -1;

  image->image_filename = strdup(filename);
  image->image_format = handler;

  if (image->image_format->open(&(image->image_private_data)) != 0)
    return -1;

  return image->image_format->read(image->image_private_data, filename);
}

int
saxs_image_write(saxs_image *image, const char *filename, const char *format) {
  return -1;
}

void
saxs_image_free(saxs_image *image) {
  if (image) {
    if (image->image_format)
      image->image_format->close(image->image_private_data);

    if (image->image_filename)
      free(image->image_filename);

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
  return image && image->image_format && image->image_format->width
           ? image->image_format->width(image->image_private_data)
           : 0;
}

size_t
saxs_image_height(saxs_image *image) {
  return image && image->image_format && image->image_format->height
           ? image->image_format->height(image->image_private_data)
           : 0;
}

size_t
saxs_image_value(saxs_image *image, int x, int y) {
  return image && image->image_format && image->image_format->value
           ? image->image_format->value(image->image_private_data, x, y)
           : 0;
}

static size_t
image_value_min(saxs_image *image) {
  size_t i, j, min = INT_MAX;

  const int width = saxs_image_width(image);
  const int height = saxs_image_height(image);

  for (i = 0; i < width; ++i)
    for (j = 0; j < height; ++j) {
      size_t value = saxs_image_value(image, i, j);
      if (min > value)
        min = value;
    }

  return min;
}

size_t
saxs_image_value_min(saxs_image *image) {
  return image && image->image_format && image->image_format->value_min
           ? image->image_format->value_min(image->image_private_data)
           : image_value_min(image);
}

static size_t
image_value_max(saxs_image *image) {
  size_t i, j, max = 0;

  const int width = saxs_image_width(image);
  const int height = saxs_image_height(image);

  for (i = 0; i < width; ++i)
    for (j = 0; j < height; ++j) {
      size_t value = saxs_image_value(image, i, j);
      if (max < value)
        max = value;
    }

  return max;
}

size_t
saxs_image_value_max(saxs_image *image) {
  return image && image->image_format && image->image_format->value_max
           ? image->image_format->value_max(image->image_private_data)
           : image_value_max(image);
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
