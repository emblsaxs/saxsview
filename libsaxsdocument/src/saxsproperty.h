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

#ifndef LIBSAXSDOCUMENT_SAXSPROPERTY_H
#define LIBSAXSDOCUMENT_SAXSPROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

struct saxs_property;
typedef struct saxs_property saxs_property;


saxs_property*
saxs_property_create(const char *name, const char *value);

/* Same as saxs_property_create, but giving the length of the strings as if to strnFUN
 *
 * length == -1 means "read until the NUL terminator"
 */
saxs_property*
saxs_property_create_strn(const char *name, int namelen,
                          const char *value, int valuelen);

void
saxs_property_free(saxs_property *property);


/**
 * @brief Advance to next property.
 *
 * @param property  A pointer to a previously allocated property, may be NULL.
 *
 * @returns The next property, or NULL if there is none.
 *
 * @see @ref saxs_document_property,
 *      @ref saxs_document_property_find
 */
saxs_property*
saxs_property_next(const saxs_property *property);

/**
 * @brief Advance to next named property.
 *
 * @param property  A pointer to a previously allocated property, may be NULL.
 *
 * @returns The next named property, or NULL if there is none.
 *
 * @see @ref saxs_document_property,
 *      @ref saxs_document_property_find
 */
saxs_property*
saxs_property_find_next(const saxs_property *property, const char *name);

/**
 * @brief The name of a property.
 * @param property  A pointer to a previously allocated property, may be NULL.
 * @returns The name of a property, NULL if the property pointer is NULL.
 */
const char*
saxs_property_name(const saxs_property *property);

/**
 * @brief The value of a property.
 * @param property  A pointer to a previously allocated property, may be NULL.
 * @returns The value of a property, NULL if the property pointer is NULL.
 */
const char*
saxs_property_value(const saxs_property *property);



struct saxs_property_list;
typedef struct saxs_property_list saxs_property_list;


saxs_property_list*
saxs_property_list_create();

void
saxs_property_list_insert(saxs_property_list *list, saxs_property *property);

int
saxs_property_list_count(const saxs_property_list *list);

saxs_property*
saxs_property_list_first(const saxs_property_list *list);

saxs_property*
saxs_property_list_find_first(const saxs_property_list *list, const char *name);

void
saxs_property_list_free(saxs_property_list *list);

#ifdef __cplusplus
}
#endif

#endif /* !LIBSAXSDOCUMENT_SAXSPROPERTY_H */
