/*  Vectors Python binding
    Copyright 2021-2023 Lukas Cone

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
#include "spike/type/vectors_simd.hpp"
#include <Python.h>

inline PyObject *Py_BuildValue(rreal32x2 vec) {
  return Py_BuildValue("(ff)", vec.X, vec.Y);
}
inline PyObject *Py_BuildValue(rreal32x3 vec) {
  return Py_BuildValue("(fff)", vec.X, vec.Y, vec.Z);
}
inline PyObject *Py_BuildValue(rreal32x4 vec) {
  return Py_BuildValue("(ffff)", vec.X, vec.Y, vec.Z, vec.W);
}
inline PyObject *Py_BuildValue(real32x4a16 &vec) {
  return Py_BuildValue("(ffff)", vec.X, vec.Y, vec.Z, vec.W);
}
