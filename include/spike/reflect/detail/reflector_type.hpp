/*  Interfaces for reflected types
    internal file

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
#include "../reflector_fwd.hpp"
#include "spike/crypto/jenkinshash.hpp"
#include "spike/util/supercore.hpp"
#include <string>

struct ReflType;

struct ReflTypeFloat {
  uint8 mantissa;
  uint8 exponent;
  uint8 sign : 1;
  uint8 customFormat : 1;
};

static_assert(sizeof(ReflTypeFloat) <= 16);

struct ReflTypeClass {
  uint32 typeHash;
};

struct ReflTypeArrayBase {
  uint16 stride;
  REFType type;
  uint8 numItems;
};

struct ReflTypeVector : ReflTypeArrayBase {
  ReflTypeFloat asFloat;
  operator ReflType() const;
};

static_assert(sizeof(ReflTypeVector) <= 16);

struct ReflTypeBitField : ReflTypeClass {
  REFType type;
  ReflTypeFloat asFloat;
};

static_assert(sizeof(ReflTypeBitField) <= 16);

struct ReflTypeArray : ReflTypeArrayBase {
  union {
    uint32 raw[2];
    ReflTypeVector asVector;
    ReflTypeBitField asBitfield;
    ReflTypeFloat asFloat;
    ReflTypeClass asClass;
  };

  operator ReflType() const;
};

static_assert(sizeof(ReflTypeArray) <= 16);

struct ReflType {
  REFType type;
  uint8 index;
  union {
    uint16 offset;
    struct {
      uint8 position;
      uint8 size;
    } bit;
  };
  JenHash valueNameHash;
  uint16 size;

  union {
    uint32 raw[3]{};
    ReflTypeVector asVector;
    ReflTypeBitField asBitfield;
    ReflTypeFloat asFloat;
    ReflTypeClass asClass;
    ReflTypeArray asArray;
  };
};

static_assert(sizeof(ReflType) == 24);

inline ReflTypeArray::operator ReflType() const {
  ReflType subType;
  subType.offset = 0;
  subType.size = stride;
  subType.type = type;
  subType.raw[0] = raw[0];
  subType.raw[1] = raw[1];
  return subType;
}

inline ReflTypeVector::operator ReflType() const {
  ReflType subType;
  subType.offset = 0;
  subType.size = stride;
  subType.type = type;
  subType.asFloat = asFloat;
  return subType;
}
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
  static constexpr uint8 SIZE = sizeof(_Ty);
};
template <> struct _getType<bool> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::Bool;
  static constexpr uint8 SIZE = 1;
};

template <> struct _getType<const char *> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::CString;
  static constexpr uint8 SIZE = 0;
};

template <> struct _getType<std::string> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::String;
  static constexpr uint8 SIZE = 0;
};

template <class T> using refl_has_hash = decltype(std::declval<T>().Hash());

template <class C, size_t _Size> struct _getType<C[_Size]> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::Array;
  static constexpr JenHash Hash() {
    if constexpr (es::is_detected_v<refl_has_hash, _getType<C>>) {
      return _getType<C>::Hash();
    } else {
      return {};
    }
  }
  static constexpr size_t SIZE = sizeof(C[_Size]);
  static constexpr uint8 SUBSIZE = sizeof(C);
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr uint16 NUMITEMS = _Size;
  using child_type = C;
};

template <class type_>
ReflType BuildReflType(JenHash typeHash, uint8 index, size_t offset) {
  using unref_type = std::remove_reference_t<type_>;
  using type_class = _getType<unref_type>;
  static_assert(type_class::TYPE != REFType::None,
                "Undefined type to reflect. Did you forget void "
                "ReflectorTag(); tag method for subclass member?");
  constexpr auto type = type_class::TYPE;

  ReflType mainType{};
  mainType.type = type;
  mainType.index = index;
  mainType.valueNameHash = typeHash;
  mainType.offset = static_cast<decltype(ReflType::offset)>(offset);
  mainType.size = type_class::SIZE;

  auto ParsePrimitive = [](auto &value) {
    constexpr auto type = type_class::TYPE;
    if constexpr (type == REFType::FloatingPoint) {
      constexpr auto vHash = type_class::Hash();
      if constexpr (vHash.raw()) {
        value.asFloat.customFormat = true;
        value.asFloat.mantissa = type_class::MANTISSA;
        value.asFloat.exponent = type_class::EXPONENT;
        value.asFloat.sign = type_class::SIGN;
      }
    } else if constexpr (es::is_detected_v<refl_has_hash, type_class>) {
      constexpr auto vHash = type_class::Hash();

      if constexpr (vHash.raw() > 0) {
        value.asClass.typeHash = vHash.raw();
      }
    }
  };

  if constexpr (type == REFType::Array || type == REFType::ArrayClass ||
                type == REFType::Vector) {
    mainType.asArray.numItems = type_class::NUMITEMS;
    mainType.asArray.type = type_class::SUBTYPE;
    mainType.asArray.stride = type_class::SUBSIZE;

    if constexpr (type_class::SUBTYPE == REFType::Vector) {
      using vec_type = _getType<typename type_class::child_type>;

      mainType.asArray.asVector.numItems = vec_type::NUMITEMS;
      mainType.asArray.asVector.stride = vec_type::SUBSIZE;
      mainType.asArray.asVector.type = vec_type::SUBTYPE;
      ParsePrimitive(mainType.asArray.asVector);

    } else {
      ParsePrimitive(mainType.asArray);
    }
  } else {
    ParsePrimitive(mainType);
  }

  return mainType;
}

template <class main_class, class declmem>
ReflType BuildBFReflType(JenHash hash) {
  constexpr auto item = main_class::parent::Get(declmem::index);
  using member_type = typename declmem::value_type;
  using main_type = typename main_class::value_type;
  using rtype = std::conditional_t<std::is_same_v<member_type, void>, main_type,
                                   member_type>;
  using type_class = _getType<rtype>;
  constexpr auto subType = type_class::TYPE;

  static_assert(subType != REFType::None, "Invalid bit member subtpe!");

  ReflType mainType{};
  mainType.type = REFType::BitFieldMember;
  mainType.index = declmem::index;
  mainType.valueNameHash = hash;
  mainType.bit.position =
      static_cast<decltype(ReflType::bit.position)>(item.position);
  mainType.bit.size = static_cast<decltype(ReflType::bit.size)>(item.size);
  mainType.asBitfield.type = subType;

  if constexpr (subType == REFType::FloatingPoint) {
    constexpr auto flHash = type_class::Hash();

    static_assert(flHash.raw(),
                  "Standard floating point not supported for bitfields.");

    if constexpr (flHash.raw()) {
      mainType.asBitfield.asFloat.customFormat = true;
      mainType.asBitfield.asFloat.mantissa = type_class::MANTISSA;
      mainType.asBitfield.asFloat.exponent = type_class::EXPONENT;
      mainType.asBitfield.asFloat.sign = type_class::SIGN;
    }
  } else if constexpr (subType == REFType::Enum) {
    mainType.asBitfield.typeHash = type_class::Hash().raw();
  }

  static_assert(subType == REFType::Integer ||
                    subType == REFType::UnsignedInteger ||
                    subType == REFType::FloatingPoint ||
                    subType == REFType::Enum || subType == REFType::Bool,
                "Invalid subtype for bit member");

  return mainType;
}
