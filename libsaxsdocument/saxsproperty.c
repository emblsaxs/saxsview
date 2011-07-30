/*
 * API for properties of SAXS documents and images.
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

#include "saxsproperty.h"

#include <stdlib.h>
#include <string.h>

struct saxs_property {
  char *name, *value;
  struct saxs_property *next;
};

struct saxs_property_list {
  size_t count;
  struct saxs_property *head, *tail;
};


saxs_property*
saxs_property_create(const char *name, const char *value) {
  saxs_property *property;

  if (!name)
    return NULL;

  property = malloc(sizeof(saxs_property));
  property->name = strdup(name);
  property->value = strdup(value);
  property->next = NULL;

  return property;
}

saxs_property*
saxs_property_next(saxs_property *property) {
  return property ? property->next : NULL;
}

saxs_property*
saxs_property_find_next(saxs_property *property, const char *name) {
  property = property ? property->next : NULL;
  while (property && (strcmp(property->name, name) != 0))
    property = property->next;

  return property;
}

const char*
saxs_property_name(saxs_property *property) {
  return property ? property->name : NULL;
}

const char*
saxs_property_value(saxs_property *property) {
  return property ? property->value : NULL;
}

void
saxs_property_free(saxs_property *property) {
  if (property) {
    if (property->name)
      free(property->name);
    if (property->value)
      free(property->value);

    free(property);
  }
}


saxs_property_list*
saxs_property_list_create() {
  saxs_property_list *list = malloc(sizeof(saxs_property_list));

  list->count = 0;
  list->head = NULL;
  list->tail = NULL;

  return list;
}

void
saxs_property_list_insert(saxs_property_list *list, saxs_property *property) {
  if (list && property) {
    if (!list->head)
      list->head = property;
    else
      list->tail->next = property;

    list->tail = property;
    list->count += 1;
  }
}

int
saxs_property_list_count(saxs_property_list *list) {
  return list ? list->count : 0;
}

saxs_property*
saxs_property_list_first(saxs_property_list *list) {
  return list ? list->head : NULL;
}

saxs_property*
saxs_property_list_find_first(saxs_property_list *list, const char *name) {
  saxs_property *property = saxs_property_list_first(list);

  while (property && strcmp(saxs_property_name(property), name) != 0)
    property = saxs_property_next(property);

  return property;
}

void
saxs_property_list_free(saxs_property_list *list) {
  while (list && list->head) {
    saxs_property *property = list->head;
    list->head = list->head->next;
    saxs_property_free(property);
  }

  if (list)
    free(list);
}
