/*  std::array with reflection declaration

    Copyright 2018-2020 Lukas Cone

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
#include <array>
#include "internal/reflector_type.hpp"

template <class C, size_t _Size> struct _getType<std::array<C, _Size>> {
  static const REFType TYPE = REFType::Array;
  static const JenHash HASH = _getType<C>::HASH;
  static const uint8 SUBSIZE = sizeof(C);
  static const REFType SUBTYPE = _getType<C>::TYPE;
  static const uint16 NUMITEMS = _Size;
};