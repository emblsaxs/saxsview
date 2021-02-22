
#include <Python.h>

#include "saxsdocument.h"

#if PY_VERSION_HEX < 0x03030000
const char* PyUnicode_AsUTF8(PyObject *string) {
  if (PyString_Check(string)) {
    return PyString_AsString(string);

  } else if (PyUnicode_Check(string)) {
    PyObject *utf8 = PyUnicode_AsUTF8String(string);
    if (!utf8)
      return NULL;

    return PyString_AsString(utf8);

  } else {
    PyErr_BadArgument();
    return NULL;
  }
}
#endif


/**
 * This module implements two types and two module
 * functions:
 *  - type: saxsdocument.api.document
 *    wrapper for saxs_dcoument
 *  - type: saxsdocument.api.curve
 *    wrapper for saxs_curve
 *
 *  - procedure: read
 *    reads a file, returns a 'filled in' document object
 *  - procedure: create
 *    returns an empty document object
 */

typedef struct {
  PyObject_HEAD

  struct saxs_curve *curve;

} PySaxsCurveObject;

static PyObject*
PySaxsCurveObject_data(PyObject *self, PyObject *args) {
  PySaxsCurveObject *obj = (PySaxsCurveObject*)self;

  Py_ssize_t k = 0, n = saxs_curve_data_count(obj->curve);
  PyObject *x    = PyList_New(n);
  PyObject *y    = PyList_New(n);
  PyObject *yerr = PyList_New(n);

  saxs_data *data = saxs_curve_data(obj->curve);
  while (data) {
    PyList_SET_ITEM(x, k, PyFloat_FromDouble(saxs_data_x(data)));
    PyList_SET_ITEM(y, k, PyFloat_FromDouble(saxs_data_y(data)));
    PyList_SET_ITEM(yerr, k, PyFloat_FromDouble(saxs_data_y_err(data)));
    k += 1;

    data = saxs_data_next(data);
  }

  return PyTuple_Pack(3, x, y, yerr);
}

static PyObject*
PySaxsCurveObject_add_data(PyObject *self, PyObject *args) {
  PyObject *x, *y, *yerr;

  if (!PyArg_ParseTuple(args, "OOO", &x, &y, &yerr))
    return NULL;

  if (!PyList_Check(x))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'x'"); 

  if (!PyList_Check(y))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'y'"); 

  if (!PyList_Check(yerr))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'yerr'"); 

  Py_ssize_t k, n = PyList_Size(x);
  if (n != PyList_Size(y) || n != PyList_Size(yerr))
    return PyErr_Format(PyExc_RuntimeError, "list sizes differ (x: %ld, y: %ld, yerr: %ld)", 
                        n, PyList_Size(y), PyList_Size(yerr));

  for (k = 0; k < n; ++k) {
    PyObject *value_x = PyList_GetItem(x, k);
    if (!PyFloat_Check(value_x))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    PyObject *value_y = PyList_GetItem(y, k);
    if (!PyFloat_Check(value_y))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    PyObject *value_yerr = PyList_GetItem(yerr, k);
    if (!PyFloat_Check(value_yerr))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    PySaxsCurveObject *obj = (PySaxsCurveObject*)self;
    saxs_curve_add_data(obj->curve, PyFloat_AsDouble(value_x), 0.0,
                                    PyFloat_AsDouble(value_y), PyFloat_AsDouble(value_yerr));
  }

  Py_RETURN_NONE;
}

static PyMethodDef PySaxsCurveObject_methods[] = {
    { "data", PySaxsCurveObject_data, METH_NOARGS,
      "Returns a list of data objects" },
    { "add_data", PySaxsCurveObject_add_data, METH_VARARGS,
      "" },

    {NULL}  /* Sentinel */
};

static PyTypeObject PySaxsCurveObject_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "saxsdocument.api.curve",
  .tp_doc = "Curve Object",
  .tp_basicsize = sizeof(PySaxsCurveObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_methods = PySaxsCurveObject_methods,
};



typedef struct {
  PyObject_HEAD

  struct saxs_document *doc;

} PySaxsDocumentObject;

static PyObject *
PySaxsDocumentObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PySaxsDocumentObject *obj;
  obj = (PySaxsDocumentObject *)type->tp_alloc(type, 0);
  obj->doc = saxs_document_create();

  return (PyObject*)obj;
}

static int
PySaxsDocumentObject_init(PyObject *self, PyObject *args, PyObject *kwargs) {
  static char *kwlist[] = {};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "", kwlist))
    return -1;

  return 0;
}

static void
PySaxsDocumentObject_dealloc(PyObject *self) {
  PySaxsDocumentObject *obj = (PySaxsDocumentObject*)self;
  saxs_document_free(obj->doc);

  Py_TYPE(self)->tp_free(self);
}

static PyObject*
PySaxsDocumentObject_curves(PyObject *self, PyObject *args) {
  PyObject *curves = PyList_New(0);

  PySaxsDocumentObject *obj = (PySaxsDocumentObject*)self;
  saxs_curve *curve = saxs_document_curve(obj->doc);
  while (curve) {
    PySaxsCurveObject *c = (PySaxsCurveObject*)PyType_GenericNew(&PySaxsCurveObject_type, Py_None, Py_None);
    c->curve = curve;
    PyList_Append(curves, (PyObject*)c);
    Py_DECREF(c);

    curve = saxs_curve_next(curve);
  }

  return curves;
}

static PyObject*
PySaxsDocumentObject_curve(PyObject *self, PyObject *args) {
  Py_ssize_t n;

  if (!PyArg_ParseTuple(args, "n", &n))
    return NULL;

  PyObject *curves = PySaxsDocumentObject_curves(self, Py_None);
  PyObject *curve  = PyList_GetItem(curves, n);

  if (!curve) {
    Py_DECREF(curves);
    return NULL;

  } else {
    Py_INCREF(curve);
    Py_DECREF(curves);
    return curve;
  }
}

static PyObject*
PySaxsDocumentObject_add_curve(PyObject *self, PyObject *args) {
  PyObject *x, *y, *yerr;

  if (!PyArg_ParseTuple(args, "OOO", &x, &y, &yerr))
    return NULL;

  if (!PyList_Check(x))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'x'");

  if (!PyList_Check(y))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'y'");

  if (!PyList_Check(yerr))
    return PyErr_Format(PyExc_TypeError, "a list of values is required for argument 'yerr'");

  Py_ssize_t k, n = PyList_Size(x);
  if (n != PyList_Size(y) || n != PyList_Size(yerr))
    return PyErr_Format(PyExc_RuntimeError, "list sizes differ (x: %ld, y: %ld, yerr: %ld)",
                        n, PyList_Size(y), PyList_Size(yerr));

  PySaxsDocumentObject *obj = (PySaxsDocumentObject*)self;
  saxs_curve *curve = saxs_document_add_curve(obj->doc, "", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  for (k = 0; k < n; ++k) {
    PyObject *value_x = PyList_GetItem(x, k);
    if (!PyFloat_Check(value_x))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    PyObject *value_y = PyList_GetItem(y, k);
    if (!PyFloat_Check(value_y))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    PyObject *value_yerr = PyList_GetItem(yerr, k);
    if (!PyFloat_Check(value_yerr))
      return PyErr_Format(PyExc_TypeError, "floating point value required");

    saxs_curve_add_data(curve, PyFloat_AsDouble(value_x), 0.0,
                               PyFloat_AsDouble(value_y), PyFloat_AsDouble(value_yerr));
  }

  Py_RETURN_NONE;
}

static PyObject*
PySaxsDocumentObject_properties(PyObject *self, PyObject *args) {
  PyObject *properties = PyDict_New();

  PySaxsDocumentObject *obj = (PySaxsDocumentObject*)self;
  saxs_property *property = saxs_document_property_first(obj->doc);
  while (property) {
    PyObject *key = PyUnicode_FromFormat("%s", saxs_property_name(property));
    PyObject *val = PyUnicode_FromFormat("%s", saxs_property_value(property));
    PyDict_SetItem(properties, key, val);
    Py_DECREF(val);
    Py_DECREF(key);

    property = saxs_property_next(property);
  }

  return properties;
}

static PyObject*
PySaxsDocumentObject_add_properties(PyObject *self, PyObject *args) {
  PyObject *properties;

  if (!PyArg_ParseTuple(args, "O", &properties))
    return NULL;

  if (!PyDict_Check(properties))
    return PyErr_Format(PyExc_TypeError, "dictionary required");

  PyObject *key, *val;
  Py_ssize_t pos = 0;

  while (PyDict_Next(properties, &pos, &key, &val)) {
    PyObject *key_str = PyObject_Str(key);
    PyObject *val_str = PyObject_Str(val);

    PySaxsDocumentObject *obj = (PySaxsDocumentObject*)self;
    saxs_document_add_property(obj->doc, PyUnicode_AsUTF8(key_str),
                                         PyUnicode_AsUTF8(val_str));

    Py_DECREF(val_str);
    Py_DECREF(key_str);
  }

  Py_RETURN_NONE;
}

static PyObject*
PySaxsDocumentObject_add_property(PyObject *self, PyObject *args) {
  PyObject *key, *val;

  if (!PyArg_ParseTuple(args, "OO", &key, &val))
    return NULL;

  PyObject *properties = PyDict_New();
  PyDict_SetItem(properties, key, val);

  return PySaxsDocumentObject_add_properties(self, properties);
}

static PyObject*
PySaxsDocumentObject_write(PyObject *self, PyObject *args) {
  char *filename = NULL, *format = NULL;

  if (!PyArg_ParseTuple(args, "s|s", &filename, &format))
    return NULL;

  PySaxsDocumentObject *obj = (PySaxsDocumentObject*) self;

  int result = saxs_document_write(obj->doc, filename, format);
  if (result) {
    errno = result;
    return PyErr_SetFromErrnoWithFilename(PyExc_IOError, filename);
  }

  Py_RETURN_NONE;
}

static PyMethodDef PySaxsDocumentObject_methods[] = {
    { "curve", PySaxsDocumentObject_curve, METH_VARARGS,
      "Returns the n-th curve." },
    { "curves", PySaxsDocumentObject_curves, METH_NOARGS,
      "Returns a list of curve objects" },
    { "add_curve", PySaxsDocumentObject_add_curve, METH_VARARGS,
      "" },
    { "properties", PySaxsDocumentObject_properties, METH_NOARGS,
      "Returns a dictionary of properties found in the data file" },
    { "add_properties", PySaxsDocumentObject_add_properties, METH_VARARGS,
      "" },
    { "add_property", PySaxsDocumentObject_add_property, METH_VARARGS,
       "" },
    { "write", PySaxsDocumentObject_write, METH_VARARGS,
      "" },

    {NULL}  /* Sentinel */
};

static PyTypeObject PySaxsDocumentObject_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "saxsdocument.api.document",
  .tp_doc = "Document Object",
  .tp_basicsize = sizeof(PySaxsDocumentObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_new = PySaxsDocumentObject_new,
  .tp_init = PySaxsDocumentObject_init,
  .tp_dealloc = PySaxsDocumentObject_dealloc,
  .tp_methods = PySaxsDocumentObject_methods,
};



static PyObject*
saxsdocument_api_create(PyObject *self, PyObject *args) {
  return PySaxsDocumentObject_new(&PySaxsDocumentObject_type, NULL, NULL);
}

static PyObject*
saxsdocument_api_read(PyObject *self, PyObject *args) {
  char *filename = NULL, *format = NULL;

  if (!PyArg_ParseTuple(args, "s|s", &filename, &format))
    return NULL;

  PySaxsDocumentObject *obj = (PySaxsDocumentObject*) PySaxsDocumentObject_new(&PySaxsDocumentObject_type, NULL, NULL);

  int result = saxs_document_read(obj->doc, filename, format);
  if (result) {
    errno = result;
    return PyErr_SetFromErrnoWithFilename(PyExc_IOError, filename);
  }

  return (PyObject*)obj;
}

static PyMethodDef saxsdocument_api_module_methods[] = {
  { "create", saxsdocument_api_create, METH_NOARGS, "" },
  { "read", saxsdocument_api_read, METH_VARARGS, "" },
  { NULL, NULL, 0, NULL }
};


#if PY_MAJOR_VERSION == 2
PyMODINIT_FUNC initapi(void) {
  if (PyType_Ready(&PySaxsCurveObject_type) < 0)
    return;

  if (PyType_Ready(&PySaxsDocumentObject_type) < 0)
    return;

  Py_InitModule3("saxsdocument.api", 
                 saxsdocument_api_module_methods,
                 "Python interface to libsaxsdocument.");
}

#elif PY_MAJOR_VERSION == 3
static PyModuleDef saxsdocument_api_module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "saxsdocument.api",
  .m_doc = "Python interface to libsaxsdocument.",
  .m_size = -1,
  .m_methods = saxsdocument_api_module_methods,
};

PyMODINIT_FUNC PyInit_api(void) {
  if (PyType_Ready(&PySaxsCurveObject_type) < 0)
    return NULL;

  if (PyType_Ready(&PySaxsDocumentObject_type) < 0)
    return NULL;

  return PyModule_Create(&saxsdocument_api_module);
}
#endif

