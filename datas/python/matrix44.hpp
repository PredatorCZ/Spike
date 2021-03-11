/*  esMatrix44 Python binding
    Copyright 2021 Lukas Cone

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
#include "datas/matrix44.hpp"
#include <Python.h>

inline PyObject *Py_BuildValue(const esMatrix44 &mtx) {
  return Py_BuildValue("((ffff)(ffff)(ffff)(ffff))", /**********************/
                       mtx.r1.X, mtx.r1.Y, mtx.r1.Z, mtx.r1.W, /************/
                       mtx.r2.X, mtx.r2.Y, mtx.r2.Z, mtx.r2.W, /************/
                       mtx.r3.X, mtx.r3.Y, mtx.r3.Z, mtx.r3.W, /************/
                       mtx.r4.X, mtx.r4.Y, mtx.r4.Z, mtx.r4.W  /************/
  );
}
