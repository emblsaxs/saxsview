/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PYSAXSDOCUMENT_H
#define PYSAXSDOCUMENT_H

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  PyObject_HEAD

  /* A list of curves; a curve is a list of tuples (s, I, err). */
  PyObject *curves;

  /* Dictionary of name-value property pairs. */
  PyObject *properties;

} PySaxsDocumentObject;

PyTypeObject PySaxsDocument_Type;

#define PySaxsDocument_Check(ob) ((ob) && (Py_TYPE(ob) == &PySaxsDocument_Type))


PyObject*
PySaxsDocument_Read(const char *filename, PyObject *curves, PyObject *properties);

#ifdef __cplusplus
}
#endif

#endif /* !PYSAXSDOCUMENT_H */
