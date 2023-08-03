/*  ostream for Vector, Vector2, Vector4

    Copyright 2018-2023 Lukas Cone

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
#include "spike/type/vectors.hpp"
#include <ostream>

template <class T>
static std::ostream &operator<<(std::ostream &strm, const t_Vector2<T> &v) {
  return strm << v.X << " " << v.Y;
}

template <class T>
static std::ostream &operator<<(std::ostream &strm, const t_Vector<T> &v) {
  return strm << v.X << " " << v.Y << " " << v.Z;
}

template <class T>
static std::ostream &operator<<(std::ostream &strm, const t_Vector4_<T> &v) {
  return strm << v.X << " " << v.Y << " " << v.Z << " " << v.W;
}
