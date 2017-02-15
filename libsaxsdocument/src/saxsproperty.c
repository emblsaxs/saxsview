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
#include <assert.h>

struct saxs_property {
  char *name, *value;
  struct saxs_property *next;
};

struct saxs_property_list {
  size_t count;
  struct saxs_property *head, *tail;
};

#ifndef LIBSAXSDOCUMENT_HEAVY_ASSERTS

#define assert_valid_property(p)
#define assert_valid_property_or_null(p)
#define assert_valid_property_list(plist)
#define assert_valid_property_list_or_null(plist)

#else

#define assert_valid_property(p) { \
  assert(p != NULL); \
  assert(p->name != NULL); \
  assert(p->value != NULL); \
}

#define assert_valid_property_or_null(p) { \
  if(p) assert_valid_property(p); \
}

static void assert_valid_property_list(const struct saxs_property_list *plist) {
  assert(plist != NULL);
  assert(plist->count >= 0);
  if (plist->count == 0) {
    assert(plist->head == NULL);
    assert(plist->tail == NULL);
  } else {
    assert_valid_property(plist->head);
    assert_valid_property(plist->tail);
    assert(plist->tail->next == NULL);
    int cnt = 1;
    const saxs_property *p = plist->head;
    for(;p != plist->tail; p = p->next) {
      assert_valid_property(p);
      ++cnt;
    }
    assert(cnt == plist->count);
  }
}

#define assert_valid_property_list_or_null(plist) { \
  if(plist) assert_valid_property_list(plist); \
}

#endif

saxs_property*
saxs_property_create(const char *name, const char *value) {
  return saxs_property_create_strn(name, -1, value, -1);
}

saxs_property*
saxs_property_create_strn(const char *name, int namelen,
                          const char *value, int valuelen) {
  saxs_property *property;

  if (!name || !value)
    return NULL;

  property = malloc(sizeof(saxs_property));
  if (property) {
    if (namelen < 0) {
      property->name = strdup(name);
    } else {
      property->name = strndup(name, namelen);
    }
    if (valuelen < 0) {
      property->value = strdup(value);
    } else {
      property->value = strndup(value, valuelen);
    }
    property->next = NULL;
    if (property->name == NULL || property->value == NULL) {
      free(property->name);
      free(property->value);
      free(property);
      property = NULL;
    }
  }

  assert_valid_property_or_null(property);
  return property;
}

saxs_property*
saxs_property_next(const saxs_property *property) {
  assert_valid_property_or_null(property);
  return property ? property->next : NULL;
}

saxs_property*
saxs_property_find_next(const saxs_property *property, const char *name) {
  assert_valid_property_or_null(property);

  saxs_property* nextproperty = property ? property->next : NULL;
  while (nextproperty && (strcmp(nextproperty->name, name) != 0))
    nextproperty = nextproperty->next;

  return nextproperty;
}

const char*
saxs_property_name(const saxs_property *property) {
  assert_valid_property_or_null(property);
  return property ? property->name : NULL;
}

const char*
saxs_property_value(const saxs_property *property) {
  assert_valid_property_or_null(property);
  return property ? property->value : NULL;
}

void
saxs_property_free(saxs_property *property) {
  assert_valid_property_or_null(property);
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
  if (list) {
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
  }
  assert_valid_property_list_or_null(list);
  return list;
}

void
saxs_property_list_insert(saxs_property_list *list, saxs_property *property) {
  assert_valid_property_list_or_null(list);
  assert_valid_property_or_null(property);
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
saxs_property_list_count(const saxs_property_list *list) {
  assert_valid_property_list_or_null(list);
  return list ? list->count : 0;
}

saxs_property*
saxs_property_list_first(const saxs_property_list *list) {
  assert_valid_property_list_or_null(list);
  return list ? list->head : NULL;
}

saxs_property*
saxs_property_list_find_first(const saxs_property_list *list, const char *name) {
  assert_valid_property_list_or_null(list);
  saxs_property *property = saxs_property_list_first(list);

  while (property && strcmp(saxs_property_name(property), name) != 0)
    property = saxs_property_next(property);

  return property;
}

void
saxs_property_list_free(saxs_property_list *list) {
  assert_valid_property_list_or_null(list);
  while (list && list->head) {
    saxs_property *property = list->head;
    list->head = list->head->next;
    saxs_property_free(property);
  }

  if (list)
    free(list);
}
