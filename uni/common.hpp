/*  Hybrid mode copyable unique pointer
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
#include "datas/deleter_hybrid.hpp"
#include "datas/vectors_simd.hpp"
#include <memory>

namespace uni {
template <class C> using Element = std::unique_ptr<C, es::deleter_hybrid>;

struct RTSValue {
  Vector4A16 translation;
  Vector4A16 rotation;
  Vector4A16 scale;

  RTSValue()
      : rotation(0, 0, 0, 1.f), translation(0), scale(1.f, 1.f, 1.f, 0) {}
  RTSValue(const Vector4A16 &pos, const Vector4A16 &rot, const Vector4A16 &scl)
      : translation(pos), rotation(rot), scale(scl) {}

  bool operator==(const RTSValue &o) const {
    return translation == o.translation && rotation == o.rotation &&
           scale == o.scale;
  }

  bool operator!=(const RTSValue &o) const { return !(*this == o); }
};
} // namespace uni
