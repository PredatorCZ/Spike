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
#include "reflector_class_reg.hpp"

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

ES_STATIC_ASSERT(__sizeof_RelfType == 16);

struct reflectorInstance;

template <class C>
constexpr auto refl_is_reflected_(int)
    -> decltype(std::declval<C>().ReflectorTag(), bool()) {
  return true;
}

template <class C> constexpr bool refl_is_reflected_(...) { return false; }

template <class C>
constexpr bool refl_is_reflected_v_ = refl_is_reflected_<C>(0);

template <class C> constexpr REFType RefGetType() {
  if (refl_is_reflected_v_<C>) {
    return REFType::Class;
  } else if (std::is_enum<C>::value) {
    return REFType::Enum;
  } else if (std::is_arithmetic<C>::value) {
    if (std::is_floating_point<C>::value) {
      return REFType::FloatingPoint;
    } else if (std::is_unsigned<C>::value) {
      return REFType::UnsignedInteger;
    } else {
      return REFType::Integer;
    }
  } else {
    return REFType::None;
  }
}

ES_STATIC_ASSERT(RefGetType<int32>() == REFType::Integer);
ES_STATIC_ASSERT(RefGetType<float>() == REFType::FloatingPoint);
ES_STATIC_ASSERT(RefGetType<uint32>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(RefGetType<int64>() == REFType::Integer);
ES_STATIC_ASSERT(RefGetType<double>() == REFType::FloatingPoint);
ES_STATIC_ASSERT(RefGetType<uint64>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(RefGetType<int8>() == REFType::Integer);
ES_STATIC_ASSERT(RefGetType<uint8>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(RefGetType<int16>() == REFType::Integer);
ES_STATIC_ASSERT(RefGetType<uint16>() == REFType::UnsignedInteger);

template <typename _Ty> struct _getType : reflTypeDefault_ {
  static constexpr REFType TYPE = RefGetType<_Ty>();
  static constexpr JenHash Hash() {
    return JenHash(_EnumWrap<_Ty>::GetHash().raw() +
                   ReflectorType<_Ty>::Hash().raw());
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

template <class type, class C>
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
