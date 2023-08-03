/*  std::to_string for Vector, Vector2, Vector4

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
#include <string>

namespace std {
template <class C> string to_string(const t_Vector2<C> &type) {
  return to_string(type.x) + ' ' + to_string(type.y);
}

template <class C> string to_string(const t_Vector<C> &type) {
  return to_string(type.x) + ' ' + to_string(type.y) + ' ' + to_string(type.z);
}

template <class C> string to_string(const t_Vector4_<C> &type) {
  return to_string(type.x) + ' ' + to_string(type.y) + ' ' + to_string(type.z) +
         ' ' + to_string(type.w);
}

template <class C> string to_wstring(const t_Vector2<C> &type) {
  return to_wstring(type.x) + L' ' + to_wstring(type.y);
}

template <class C> string to_wstring(const t_Vector<C> &type) {
  return to_wstring(type.x) + L' ' + to_wstring(type.y) + L' ' +
         to_wstring(type.z);
}

template <class C> string to_wstring(const t_Vector4_<C> &type) {
  return to_wstring(type.x) + L' ' + to_wstring(type.y) + L' ' +
         to_wstring(type.z) + L' ' + to_wstring(type.w);
}
} // namespace std
