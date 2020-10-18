/*  Reflected Enum Python binding
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
#include "../internal/reflector_enum_reg.hpp"
#include <Python.h>

template <class E, const char *const doc = nullptr> struct ReflectedEnumPy {
  PyObject_HEAD;
  using value_type = _EnumWrap<E>;

  static PyTypeObject *GetType() {
    static PyMappingMethods mappingMethods[] = {
        Len,
        (binaryfunc)Subscript,
        0,
    };

    static PyTypeObject typeType = {
        PyVarObject_HEAD_INIT(NULL, 0) /* init macro */
        GetName(),                     /* tp_name */
        sizeof(ReflectedEnumPy),       /* tp_basicsize */
        0,                             /* tp_itemsize */
        0,                             /* tp_dealloc */
        0,                             /* tp_print */
        GetAttribute,                  /* tp_getattr */
        0,                             /* tp_setattr */
        0,                             /* tp_compare */
        0,                             /* tp_repr */
        0,                             /* tp_as_number */
        0,                             /* tp_as_sequence */
        mappingMethods,                /* tp_as_mapping */
        0,                             /* tp_hash */
        0,                             /* tp_call */
        0,                             /* tp_str */
        0,                             /* tp_getattro */
        0,                             /* tp_setattro */
        0,                             /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,            /* tp_flags */
        doc,                           /* tp_doc */
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

  static Py_ssize_t Len(PyObject *) { return value_type::NUM_ITEMS; }

  static constexpr const char *const GetName() {
    return value_type::GetClassName().data();
  }

  static PyObject *New(PyTypeObject *type, PyObject *, PyObject *) {
    return type->tp_alloc(type, 0);
  }

  static PyObject *Subscript(ReflectedEnumPy *self, PyObject *index) {
    return SubscriptRaw(PyInt_AsSsize_t(index));
  }

  static const ReflectedEnum &GetEnum() {
    static const ReflectedEnum refEnum = GetReflectedEnum<E>();
    return refEnum;
  }

  static PyObject *SubscriptRaw(size_t index) {
    auto eName = GetEnum()[index];
    return PyString_FromStringAndSize(eName.data(), eName.size());
  }

  static PyObject *GetAttribute(PyObject *, char *attrName) {
    const auto foundEnum =
        std::find(GetEnum().begin(), GetEnum().end(), attrName);

    if (es::IsEnd(GetEnum(), foundEnum))
      return nullptr;

    const size_t fndID = std::distance(GetEnum().begin(), foundEnum);

    return PyInt_FromSize_t(GetEnum().values[fndID]);
  }
};
