/*  Python binding class for enumerations
    part of uni module
    Copyright 2020-2022 Lukas Cone

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
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace UniPy {
template <class Info> struct Enum {
  PyObject_HEAD;

  static PyTypeObject *GetType() {
    static PyMappingMethods mappingMethods = {
        (lenfunc)Info::Len,
        Subscript,
        0,
    };

    static PyTypeObject typeType{
      tp_name : Info::GetName(),
      tp_basicsize : sizeof(Enum),
      tp_getattr : GetAttribute,
      tp_as_mapping : &mappingMethods,
      tp_flags : Py_TPFLAGS_DEFAULT,
      tp_doc : Info::GetDoc(),
      tp_new : New,
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
    if (index >= Info::Len(nullptr)) {
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

#pragma GCC diagnostic pop
