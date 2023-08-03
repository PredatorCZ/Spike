/*  std::array with reflection declaration

    Copyright 2018-2021 Lukas Cone

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
#include "reflector_fwd.hpp"
#include <array>

template <class C, size_t _Size>
struct _getType<std::array<C, _Size>> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::Array;
  static constexpr JenHash Hash() { return _getType<C>::Hash(); }
  static constexpr size_t SIZE = sizeof(std::array<C, _Size>);
  static constexpr size_t SUBSIZE = sizeof(C);
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr uint16 NUMITEMS = _Size;
  using child_type = C;
};
