/*  Python extension header

    Copyright 2020-2021 Lukas Cone

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

#include <Python.h>

inline void PyAddType(PyObject *module, PyTypeObject *type) {
  if (PyType_Ready(type) < 0)
    return;
  Py_INCREF(type);
  PyModule_AddObject(module, type->tp_name, (PyObject *)type);
}

template <class C> void PyAddType(PyObject *module) {
  auto type = C::GetType();
  if (PyType_Ready(type) < 0)
    return;
  Py_INCREF(type);
  PyModule_AddObject(module, type->tp_name, (PyObject *)type);
}

#if (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION < 7)
struct PyGetSet {
  const char *name;
  getter get;
  setter set;
  const char *doc;
  void *closure;
};
struct PyMember {
  const char *name;
  int type;
  Py_ssize_t offset;
  int flags;
  const char *doc;
};
#else
using PyMember = PyMemberDef;
using PyGetSet = PyGetSetDef;
#endif
