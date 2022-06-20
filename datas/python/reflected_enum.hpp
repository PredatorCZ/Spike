/*  Reflected Enum Python binding
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
#include "../internal/reflector_enum.hpp"
#include <Python.h>
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

template <class E, const char *const doc = nullptr> struct ReflectedEnumPy {
  PyObject_HEAD;
  static inline const ReflectedEnum *ENUM = GetReflectedEnum<E>();
  static PyTypeObject *GetType() {
    static PyMappingMethods mappingMethods[] = {
        Len,
        (binaryfunc)Subscript,
        0,
    };

    static PyTypeObject typeType{
      tp_name : GetName(),
      tp_basicsize : sizeof(ReflectedEnumPy),
      tp_getattr : GetAttribute,
      tp_as_mapping : mappingMethods,
      tp_flags : Py_TPFLAGS_DEFAULT,
      tp_doc : doc,
      tp_new : New,
    };

    return &typeType;
  }

  static Py_ssize_t Len(PyObject *) { return ENUM->numMembers; }

  static constexpr const char *GetName() { return ENUM->enumName; }

  static PyObject *New(PyTypeObject *type, PyObject *, PyObject *) {
    return type->tp_alloc(type, 0);
  }

  static PyObject *Subscript(ReflectedEnumPy *, PyObject *index) {
    return SubscriptRaw(PyLong_AsSize_t(index));
  }

  static PyObject *SubscriptRaw(size_t index) {
    return PyUnicode_FromString(ENUM->names[index]);
  }

  static PyObject *GetAttribute(PyObject *, char *attrName) {
    const auto foundEnum = std::find(
        ENUM->names, ENUM->names + ENUM->numMembers, es::string_view(attrName));

    if (ENUM->names + ENUM->numMembers == foundEnum) {
      return nullptr;
    }

    const size_t fndID = std::distance(ENUM->names, foundEnum);

    return PyLong_FromSize_t(ENUM->values[fndID]);
  }
};
#pragma GCC diagnostic pop
