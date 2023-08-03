/*  Python binding class for uni::List
    part of uni module
    Copyright 2020-2023 Lukas Cone

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
#include "spike/uni/list.hpp"
#include <Python.h>
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

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

    static PyMappingMethods mappingMethods = {
        (lenfunc)List::Len,
        (binaryfunc)List::Subscript,
        0,
    };

    static PyTypeObject typeObject{
        .tp_name = Info::GetName(),
        .tp_basicsize = sizeof(List),
        .tp_dealloc = (destructor)List::Dealloc,
        .tp_as_mapping = &mappingMethods,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IS_ABSTRACT,
        .tp_doc = Info::GetDoc(),
        .tp_iter = (getiterfunc)List::Iter,
        .tp_iternext = (iternextfunc)List::IterNext,
    };

    return &typeObject;
  }

  static void Dealloc(List *self) {
    auto t0 = std::move(self->cList);
    Py_TYPE(self)->tp_free(self);
  }

  static size_t Len(List *self) { return self->cList->Size(); }

  static PyObject *Subscript(List *self, PyObject *index) {
    const auto id = PyLong_AsSize_t(index);
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

#pragma GCC diagnostic pop
