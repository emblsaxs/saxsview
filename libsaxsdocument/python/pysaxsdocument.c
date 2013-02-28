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

#include <Python.h>
#include <structmember.h>
#include <errno.h>

#include "saxsdocument.h"
#include "saxsproperty.h"

#ifndef Py_TYPE
#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
#endif


/*
 * class saxsdocument.saxsdocument(object):
 */
typedef struct {
  PyObject_HEAD

  /* A list of curves; a curve is a list of tuples (s, I, err). */
  PyObject *curves;

  /* Dictionary of name-value property pairs. */
  PyObject *properties;

} PySaxsDocumentObject;

PyTypeObject PySaxsDocument_Type  = {
  PyObject_HEAD_INIT(NULL)
};

static void
saxsdocument_dealloc(PySaxsDocumentObject *self) {
  Py_XDECREF(self->curves);
  Py_XDECREF(self->properties);

  Py_TYPE(self)->tp_free((PyObject*)self);
}

static int
saxsdocument_init(PySaxsDocumentObject *self, PyObject *args, PyObject *kwargs) {
  self->curves     = PyList_New(0);
  self->properties = PyDict_New();
  return 0;
}

static PyObject *
saxsdocument_repr(PySaxsDocumentObject *self) {
  return PyString_FromFormat("saxsdocument: %u properties, %u curve(s)",
                             PyDict_Size(self->properties), PyList_Size(self->curves));
}

static PyMethodDef saxsdocument_methods[] = {
  { NULL, NULL, 0, NULL}
};


PyDoc_STRVAR(saxsdocument_curve_doc, "");
PyDoc_STRVAR(saxsdocument_property_doc, "");

static PyMemberDef saxsdocument_members[] = {
  { "curve",    T_OBJECT_EX, offsetof(PySaxsDocumentObject, curves), 0,
                saxsdocument_curve_doc },
  { "property", T_OBJECT_EX, offsetof(PySaxsDocumentObject, properties), 0,
                saxsdocument_property_doc },
  { NULL, 0, 0, 0, NULL }
};


PyObject*
PySaxsDocument_New() {
  return PyObject_CallFunction((PyObject*)&PySaxsDocument_Type, NULL);
}

/*
 * Module function(s).
 */
PyObject*
PySaxsDocument_Read(const char *filename, const char *format,
                    PyObject *curves, PyObject *properties) {
  saxs_document *doc;
  saxs_curve *curve;
  saxs_data *data;
  saxs_property *property;

  doc = saxs_document_create();
  int res = saxs_document_read(doc, filename, format);
  if (res != 0)
    return PyErr_Format(PyExc_IOError, "%s: %s", filename, strerror(res));

  curve = saxs_document_curve(doc);
  while (curve) {
    PyObject *pycurve = PyList_New(0);

    data = saxs_curve_data(curve);
    while (data) {
      PyObject *pt = Py_BuildValue("(ddd)", saxs_data_x(data),
                                            saxs_data_y(data),
                                            saxs_data_y_err(data));
      PyList_Append(pycurve, pt);
      Py_DECREF(pt);

      data = saxs_data_next(data);
    }
    PyList_Append(curves, pycurve);
    Py_DECREF(pycurve);

    curve = saxs_curve_next(curve);
  }

  property = saxs_document_property_first(doc);
  while (property) {
    PyObject *name = PyString_FromString(saxs_property_name(property));
    PyObject *value = PyString_FromString(saxs_property_value(property));

    PyDict_SetItem(properties, name, value);

    Py_DECREF(value);
    Py_DECREF(name);

    property = saxs_property_next(property);
  }

  saxs_document_free(doc);
  Py_RETURN_NONE;
}

PyObject*
saxsdocument_read(PyObject *self, PyObject *args) {
  char *filename, *format = NULL;

  PySaxsDocumentObject *doc;
  PyObject *res;

  if (!PyArg_ParseTuple(args, "s|s", &filename, &format))
    return NULL;

  doc = (PySaxsDocumentObject*)PySaxsDocument_New();

  res = PySaxsDocument_Read(filename, format, doc->curves, doc->properties);
  if (!res) {
    Py_XDECREF(doc);
    doc = NULL;
  }
  Py_XDECREF(res);

  return (PyObject*) doc;
}

PyDoc_STRVAR(saxsdocument_read_doc, "");

static PyMethodDef saxsdocument_functions[] = {
  { "read", saxsdocument_read, METH_VARARGS, saxsdocument_read_doc },
  { NULL, NULL, 0, NULL }
};


/*
 * Module Init function.
 */
PyDoc_STRVAR(saxsdocument_module_doc, "saxsdocument module");

PyMODINIT_FUNC initsaxsdocument(void) {
  PySaxsDocument_Type.tp_name      = "saxsdocument.saxsdocument";
  PySaxsDocument_Type.tp_basicsize = sizeof(PySaxsDocumentObject);
  PySaxsDocument_Type.tp_dealloc   = (destructor)saxsdocument_dealloc;
  PySaxsDocument_Type.tp_flags     = Py_TPFLAGS_DEFAULT;
  PySaxsDocument_Type.tp_methods   = saxsdocument_methods;
  PySaxsDocument_Type.tp_members   = saxsdocument_members;
  PySaxsDocument_Type.tp_repr      = (reprfunc)saxsdocument_repr;
  PySaxsDocument_Type.tp_new       = PyType_GenericNew;
  PySaxsDocument_Type.tp_init      = (initproc)saxsdocument_init;
  PySaxsDocument_Type.tp_base      = &PyBaseObject_Type;
  if (PyType_Ready(&PySaxsDocument_Type) < 0)
    return;

  PyObject *saxsdocument_module = Py_InitModule3("saxsdocument",
                                                 saxsdocument_functions,
                                                 saxsdocument_module_doc);
  if (saxsdocument_module == NULL)
    return;

  Py_INCREF(&PySaxsDocument_Type);
  PyModule_AddObject(saxsdocument_module, "saxsdocument",
                     (PyObject*)&PySaxsDocument_Type);
}
