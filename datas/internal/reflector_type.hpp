/*  Interfaces for reflected types
    internal file

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
#include "../jenkinshash.hpp"
#include "../reflector_fwd.hpp"
#include "../supercore.hpp"

struct reflType {
  REFType type;          // type of main element
  REFType subType;       // type of sub elements (array item type)
  uint8 subSize;         // size if sub element
  uint8 ID;              // index of main element within master table
  uint16 numItems;       // number of sub elements
  uint16 offset;         // offset of main element
  JenHash valueNameHash; // hash of main element's name
  JenHash typeHash;      // lookup hash of main/sub element (enum, sublass)
};

const int __sizeof_RelfType = sizeof(reflType);

static_assert(__sizeof_RelfType == 16);

struct reflectorInstance;

template <class T>
using refl_is_reflected = decltype(std::declval<T>().ReflectorTag());
template <class C>
constexpr bool refl_is_reflected_v = es::is_detected_v<refl_is_reflected, C>;

template <class C> constexpr REFType RefGetType() {
  if (refl_is_reflected_v<C>) {
    return REFType::Class;
  } else if (std::is_enum_v<C>) {
    return REFType::Enum;
  } else if (std::is_arithmetic_v<C>) {
    if (std::is_floating_point_v<C>) {
      return REFType::FloatingPoint;
    } else if (std::is_unsigned_v<C>) {
      return REFType::UnsignedInteger;
    } else {
      return REFType::Integer;
    }
  } else {
    return REFType::None;
  }
}

static_assert(RefGetType<int32>() == REFType::Integer);
static_assert(RefGetType<float>() == REFType::FloatingPoint);
static_assert(RefGetType<uint32>() == REFType::UnsignedInteger);
static_assert(RefGetType<int64>() == REFType::Integer);
static_assert(RefGetType<double>() == REFType::FloatingPoint);
static_assert(RefGetType<uint64>() == REFType::UnsignedInteger);
static_assert(RefGetType<int8>() == REFType::Integer);
static_assert(RefGetType<uint8>() == REFType::UnsignedInteger);
static_assert(RefGetType<int16>() == REFType::Integer);
static_assert(RefGetType<uint16>() == REFType::UnsignedInteger);

template <typename _Ty> struct _getType : reflTypeDefault_ {
  static constexpr REFType TYPE = RefGetType<_Ty>();
  static constexpr JenHash Hash() {
    if constexpr (std::is_enum_v<_Ty>) {
      return EnumHash<_Ty>();
    } else {
      return ClassHash<_Ty>();
    }
  }
  static constexpr uint8 SUBSIZE = sizeof(_Ty);
};
template <> struct _getType<bool> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::Bool;
  static constexpr uint8 SUBSIZE = 1;
};

template <> struct _getType<const char *> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::CString;
  static constexpr uint8 SUBSIZE = 0;
};

template <> struct _getType<std::string> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::String;
  static constexpr uint8 SUBSIZE = 0;
};

template <class C, size_t _Size> struct _getType<C[_Size]> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::Array;
  static constexpr JenHash Hash() { return _getType<C>::Hash(); }
  static constexpr uint8 SUBSIZE = sizeof(C);
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr uint16 NUMITEMS = _Size;
};

union _DecomposedVectorHash {
  JenHash hash;
  struct {
    REFType type;
    uint8 size;
    uint16 numItems;
  };
};

template <class type>
reflType BuildReflType(JenHash classHash, uint8 index, size_t offset) {
  typedef typename std::remove_reference<type>::type unref_type;
  typedef _getType<unref_type> type_class;
  static_assert(type_class::TYPE != REFType::None,
                "Undefined type to reflect. Did you forget void "
                "ReflectorTag(); tag method for subclass member?");

  return reflType{type_class::TYPE,
                  type_class::SUBTYPE,
                  type_class::SUBSIZE,
                  index,
                  type_class::NUMITEMS,
                  static_cast<decltype(reflType::offset)>(offset),
                  classHash,
                  type_class::Hash()};
}

template <class main_class, class declmem>
reflType BuildBFReflType(JenHash hash) {
  constexpr auto item = main_class::parent::Get(declmem::index);

  return reflType{REFType::BitFieldMember,
                  RefGetType<typename main_class::value_type>(),
                  static_cast<decltype(reflType::subSize)>(item.size),
                  declmem::index,
                  0,
                  static_cast<decltype(reflType::offset)>(item.position),
                  hash,
                  {}};
}
