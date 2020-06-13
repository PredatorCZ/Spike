/*  Interfaces for reflected types
    internal file

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
#include "../jenkinshash.hpp"
#include "../supercore.hpp"

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
  EnumFlags
};

struct reflType {
  REFType type;          // type of main element
  REFType subType;       // type of sub elements (array item type)
  uint8 subSize;         // size if sub element
  uint8 ID;              // index of main element within master table
  uint16 numItems;       // number of sub elements
  mutable uint16 offset; // offset of main element
  JenHash valueNameHash; // hash of main element's name
  JenHash typeHash;      // lookup hash of main/sub element (enum, sublass)
};

const int __sizeof_RelfType = sizeof(reflType);

ES_STATIC_ASSERT(__sizeof_RelfType == 16);

struct reflectorInstance;

template <typename T> class _REFGetPrimType {
private:
  struct _REFPrimTypeFLP {
    constexpr static REFType value = REFType::FloatingPoint;
  };
  struct _REFPrimTypeUNS {
    constexpr static REFType value = REFType::UnsignedInteger;
  };
  struct _REFPrimTypeSIG {
    constexpr static REFType value = REFType::Integer;
  };
  struct _REFPrimTypeUNK {
    constexpr static REFType value = REFType::None;
  };

  constexpr static REFType _type00 =
      std::conditional<std::is_floating_point<T>::value, _REFPrimTypeFLP,
                       _REFPrimTypeUNK>::type::value;

  constexpr static REFType _type01 =
      std::conditional<std::is_unsigned<T>::value, _REFPrimTypeUNS,
                       _REFPrimTypeSIG>::type::value;

public:
  constexpr static REFType value = _type00 == REFType::None ? _type01 : _type00;
};

ES_STATIC_ASSERT(_REFGetPrimType<float>::value == REFType::FloatingPoint);
ES_STATIC_ASSERT(_REFGetPrimType<int>::value == REFType::Integer);
ES_STATIC_ASSERT(_REFGetPrimType<unsigned>::value == REFType::UnsignedInteger);

template <typename T> constexpr REFType _GetTypeIndex() {
  return _REFGetPrimType<T>::value;
}

ES_STATIC_ASSERT(_GetTypeIndex<int32>() == REFType::Integer);
ES_STATIC_ASSERT(_GetTypeIndex<float>() == REFType::FloatingPoint);
ES_STATIC_ASSERT(_GetTypeIndex<uint32>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(_GetTypeIndex<int64>() == REFType::Integer);
ES_STATIC_ASSERT(_GetTypeIndex<double>() == REFType::FloatingPoint);
ES_STATIC_ASSERT(_GetTypeIndex<uint64>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(_GetTypeIndex<int8>() == REFType::Integer);
ES_STATIC_ASSERT(_GetTypeIndex<uint8>() == REFType::UnsignedInteger);
ES_STATIC_ASSERT(_GetTypeIndex<int16>() == REFType::Integer);
ES_STATIC_ASSERT(_GetTypeIndex<uint16>() == REFType::UnsignedInteger);

template <typename _Ty> struct refl_is_class_reflected {
private:
  template <class C>
  static constexpr auto is_reflected(int)
      -> decltype(C::GetReflector(), bool()) {
    return true;
  }

  template <class C> static constexpr bool is_reflected(...) { return false; }

public:
  static constexpr bool value = is_reflected<_Ty>(0);
};

template <typename _Ty> struct _getType {
  static constexpr bool subReflected = refl_is_class_reflected<_Ty>::value;
  static constexpr bool isEnum = std::is_enum<_Ty>::value;
  static constexpr bool isArithmetic = std::is_arithmetic<_Ty>::value;

  static const REFType TYPE =
      isEnum ? REFType::Enum
             : (subReflected
                    ? REFType::Class
                    : (isArithmetic ? _GetTypeIndex<_Ty>() : REFType::None));
  static const JenHash HASH =
      _EnumWrap<_Ty>::GetHash() + ReflectorType<_Ty>::Hash();
  static const JenHash SUBHASH = 0;
  static const uint8 SUBSIZE = sizeof(_Ty);
  static const REFType SUBTYPE = REFType::None;
  static const uint16 NUMITEMS = 0;
};
template <> struct _getType<bool> {
  static const REFType TYPE = REFType::Bool;
  static const JenHash HASH = 0;
  static const uint8 SUBSIZE = 1;
  static const REFType SUBTYPE = REFType::None;
  static const uint16 NUMITEMS = 0;
};

template <> struct _getType<const char *> {
  static const REFType TYPE = REFType::CString;
  static const JenHash HASH = 0;
  static const uint8 SUBSIZE = 0;
  static const REFType SUBTYPE = REFType::None;
  static const uint16 NUMITEMS = 0;
};

template <> struct _getType<std::string> {
  static const REFType TYPE = REFType::String;
  static const JenHash HASH = 0;
  static const uint8 SUBSIZE = 0;
  static const REFType SUBTYPE = REFType::None;
  static const uint16 NUMITEMS = 0;
};

template <class C, size_t _Size> struct _getType<C[_Size]> {
  static const REFType TYPE = REFType::Array;
  static const JenHash HASH = _getType<C>::HASH;
  static const uint8 SUBSIZE = sizeof(C);
  static const REFType SUBTYPE = _getType<C>::TYPE;
  static const uint16 NUMITEMS = _Size;
};

constexpr JenHash _CompileVectorHash(REFType type, uint8 size,
                                     uint16 numItems) {
  return static_cast<JenHash>(type) | static_cast<JenHash>(size) << 8 |
         static_cast<JenHash>(numItems) << 16;
}

union _DecomposedVectorHash {
  JenHash hash;
  struct {
    REFType type;
    uint8 size;
    uint16 numItems;
  };
};

template <class C>
const reflType BuildReflType(const JenHash classHash, uint8 index,
                             size_t offset = 0xffff) {
  typedef typename std::remove_reference<C>::type unref_type;
  typedef _getType<unref_type> type_class;
  static_assert(type_class::TYPE != REFType::None,
                "Undefined type to reflect!");

  return reflType{type_class::TYPE,
                  type_class::SUBTYPE,
                  type_class::SUBSIZE,
                  index,
                  type_class::NUMITEMS,
                  static_cast<decltype(reflType::offset)>(offset),
                  classHash,
                  type_class::HASH};
}
