/*  Python binding classes for uni::Motion
    part of uni module
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
#include "spike/uni/rts.hpp"
#include <Python.h>

inline PyObject *Py_BuildValue(const uni::RTSValue &rts) {
  return Py_BuildValue("((ffff)(ffff)(ffff))", /****************************/
                       rts.translation.X, rts.translation.Y, /**************/
                       rts.translation.Z, rts.translation.W, /**************/
                       rts.rotation.X, rts.rotation.Y,       /**************/
                       rts.rotation.Z, rts.rotation.W,       /**************/
                       rts.scale.X, rts.scale.Y, rts.scale.Z, rts.scale.W ///
  );
}
