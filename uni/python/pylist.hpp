/*  Python binding class for uni::List
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
#include "uni/list.hpp"
#include <Python.h>

namespace UniPy {

template <class Info> struct List {
  using item_type = typename Info::item_type;
  using wrap_type = typename Info::wrap_type;
  using value_type = uni::Element<const uni::List<item_type>>;
  PyObject_HEAD;
  union {
    value_type cList;
    uni::Element<uni::List<item_type>> mList;
  };
  size_t iterPos;

  static PyTypeObject *GetType() {

    static PyMappingMethods mappingMethods[] = {
        (lenfunc)List::Len,
        (binaryfunc)List::Subscript,
        0,
    };

    static PyTypeObject typeObject = {
        PyVarObject_HEAD_INIT(NULL, 0)               /* init macro */
        Info::GetName(),                             /* tp_name */
        sizeof(List),                                /* tp_basicsize */
        0,                                           /* tp_itemsize */
        (destructor)List::Dealloc,                   /* tp_dealloc */
        0,                                           /* tp_print */
        0,                                           /* tp_getattr */
        0,                                           /* tp_setattr */
        0,                                           /* tp_compare */
        0,                                           /* tp_repr */
        0,                                           /* tp_as_number */
        0,                                           /* tp_as_sequence */
        mappingMethods,                              /* tp_as_mapping */
        0,                                           /* tp_hash */
        0,                                           /* tp_call */
        0,                                           /* tp_str */
        0,                                           /* tp_getattro */
        0,                                           /* tp_setattro */
        0,                                           /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT, /* tp_flags */
        Info::GetDoc(),                              /* tp_doc */
        0,                                           /* tp_traverse */
        0,                                           /* tp_clear */
        0,                                           /* tp_richcompare */
        0,                                           /* tp_weaklistoffset */
        (getiterfunc)List::Iter,                     /* tp_iter */
        (iternextfunc)List::IterNext,                /* tp_iternext */
        0,                                           /* tp_methods */
        0,                                           /* tp_members */
        0,                                           /* tp_getset */
        0,                                           /* tp_base */
        0,                                           /* tp_dict */
        0,                                           /* tp_descr_get */
        0,                                           /* tp_descr_set */
        0,                                           /* tp_dictoffset */
        0,                                           /* tp_init */
        0,                                           /* tp_alloc */
        0,                                           /* tp_new */
    };

    return &typeObject;
  }

  static void Dealloc(List *self) { auto t0 = std::move(self->cList); }

  static size_t Len(List *self) { return self->cList->Size(); }

  static PyObject *Subscript(List *self, PyObject *index) {
    const auto id = PyInt_AsSsize_t(index);
    return self->SubscriptRaw(id);
  }

  PyObject *SubscriptRaw(size_t index) {
    wrap_type *rtVal = reinterpret_cast<wrap_type *>(
        PyType_GenericAlloc(wrap_type::GetType(), 0));
    rtVal->item = cList->At(index);
    return reinterpret_cast<PyObject *>(rtVal);
  }

  static PyObject *Iter(List *self) {
    self->iterPos = 0;
    return reinterpret_cast<PyObject *>(self);
  }

  static PyObject *IterNext(List *self) {
    if (self->iterPos < Len(self)) {
      return self->SubscriptRaw(self->iterPos++);
    } else {
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
  }

  static PyObject *Create(value_type &&tp) {
    auto obj = reinterpret_cast<List *>(
        PyType_GenericNew(GetType(), nullptr, nullptr));
    obj->cList = std::move(tp);
    return reinterpret_cast<PyObject *>(obj);
  }
};
} // namespace UniPy