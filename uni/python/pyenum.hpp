/*  Python binding class for enumerations
    part of uni module
    Copyright 2020 Lukas Cone

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include "datas/string_view.hpp"
#include <Python.h>
#include <algorithm>

namespace UniPy {
template <class Info> struct Enum {
  PyObject_HEAD;

  static PyTypeObject *GetType() {
    static PyMappingMethods mappingMethods = {
        (lenfunc)Info::Len,
        Subscript,
        0,
    };

    static PyTypeObject typeType = {
        PyVarObject_HEAD_INIT(NULL, 0) /* init macro */
        Info::GetName(),               /* tp_name */
        sizeof(Enum),                  /* tp_basicsize */
        0,                             /* tp_itemsize */
        0,                             /* tp_dealloc */
        0,                             /* tp_print */
        GetAttribute,                  /* tp_getattr */
        0,                             /* tp_setattr */
        0,                             /* tp_compare */
        0,                             /* tp_repr */
        0,                             /* tp_as_number */
        0,                             /* tp_as_sequence */
        &mappingMethods,               /* tp_as_mapping */
        0,                             /* tp_hash */
        0,                             /* tp_call */
        0,                             /* tp_str */
        0,                             /* tp_getattro */
        0,                             /* tp_setattro */
        0,                             /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,            /* tp_flags */
        Info::GetDoc(),                /* tp_doc */
        0,                             /* tp_traverse */
        0,                             /* tp_clear */
        0,                             /* tp_richcompare */
        0,                             /* tp_weaklistoffset */
        0,                             /* tp_iter */
        0,                             /* tp_iternext */
        0,                             /* tp_methods */
        0,                             /* tp_members */
        0,                             /* tp_getset */
        0,                             /* tp_base */
        0,                             /* tp_dict */
        0,                             /* tp_descr_get */
        0,                             /* tp_descr_set */
        0,                             /* tp_dictoffset */
        0,                             /* tp_init */
        0,                             /* tp_alloc */
        New,                           /* tp_new */
    };

    return &typeType;
  }

  static PyObject *New(PyTypeObject *type, PyObject *, PyObject *) {
    return type->tp_alloc(type, 0);
  }

  static PyObject *Subscript(PyObject *, PyObject *index) {
    return SubscriptRaw(PyLong_AsSize_t(index));
  }

  static PyObject *SubscriptRaw(size_t index) {
    if (index >= Info::Len()) {
      PyErr_SetString(PyExc_IndexError, "index out of range");
      return nullptr;
    }

    const auto cName = std::next(Info::begin(), index)->name;

    return PyUnicode_FromStringAndSize(cName.data(), cName.size());
  }

  static PyObject *GetAttribute(PyObject *, char *attrName) {
    es::string_view attrNameRef(attrName);
    const auto foundEnum = std::find_if(Info::begin(), Info::end(),
                                        [&](typename Info::value_type &typ) {
                                          return typ.name == attrNameRef;
                                        });

    if (foundEnum == Info::end()) {
      return nullptr;
    }

    return PyLong_FromSize_t(foundEnum->id);
  }
};
} // namespace UniPy
