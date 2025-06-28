/*  Contains forward declarations for reflector

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
#include "spike/crypto/jenkinshash.hpp"

enum class REFType : uint8 {
  None,
  Integer,
  UnsignedInteger,
  FloatingPoint,
  Class,
  Enum,
  Bool,
  CString,
  String,
  Array,      // {} braces
  Vector,     // [] braces
  ArrayClass, // () braces
  EnumFlags,
  BitFieldMember,
  BitFieldClass,
};

enum class REFContainer : uint8 {
  None,
  Pointer,
  ContainerVector,
  ContainerVectorMap,
};

struct reflTypeDefault_ {
  static constexpr JenHash Hash() { return {}; }
  static constexpr JenHash SubHash() { return {}; }
  static constexpr REFType SUBTYPE = REFType::None;
  static constexpr uint16 NUMITEMS = 0;
};

template <typename _Ty> struct _getType;
template <class C> constexpr JenHash ClassHash() { return {}; }
template <class C> constexpr JenHash EnumHash() { return {}; }
