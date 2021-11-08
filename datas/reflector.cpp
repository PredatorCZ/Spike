/*  A source for Reflector class
    more info in README for PreCore Project

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

#include "reflector.hpp"
#include "bitfield.hpp"
#include "float.hpp"
#include "master_printer.hpp"
#include "string_view.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ostream>

static bool IsArray(REFType type) {
  return type == REFType::Array || type == REFType::ArrayClass;
}

static bool IsArrayVec(REFType type) {
  return type == REFType::Array || type == REFType::ArrayClass ||
         type == REFType::Vector;
}

static Reflector::ErrorType
SetReflectedMember(ReflType reflValue, es::string_view value, char *objAddr);

const ReflType *Reflector::GetReflectedType(const JenHash hash) const {
  const reflectorStatic *inst = GetReflectedInstance().rfStatic;
  const size_t _ntypes = GetNumReflectedValues();

  for (size_t t = 0; t < _ntypes; t++) {
    if (inst->types[t].valueNameHash == hash) {
      return GetReflectedType(t);
    }
  }

  if (inst->typeAliasHashes)
    for (size_t t = 0; t < _ntypes; t++) {
      if (inst->typeAliasHashes[t] == hash) {
        return GetReflectedType(t);
      }
    }

  return nullptr;
}

template <class T> struct LimitProxy {
  static const size_t numBits = sizeof(T) * 8;
  static const uint64 uMax = std::numeric_limits<T>::max();
  static const int64 iMin = std::numeric_limits<T>::min();
  static const int64 iMax = static_cast<int64>(uMax);
};

template <class type> struct BFTag {
  type value;
  BFTag(const type &value_) : value(value_) {}
  operator type() const { return value; }
};

template <class type> struct LimitProxy<BFTag<type>> {
  size_t numBits;
  const uint64 uMax;
  const int64 iMax;
  const int64 iMin;

  LimitProxy(size_t numBits_)
      : numBits(numBits_), uMax((1 << numBits) - 1), iMax(uMax >> 1),
        iMin(~iMax) {}
};

template <typename T, class ProxyType>
static Reflector::ErrorType SetNumber(es::string_view input_, T &output,
                                      ProxyType proxy) {
  std::string input(input_);
  Reflector::ErrorType errType = Reflector::ErrorType::None;
  const int base =
      !input.compare(0, 2, "0x") || !input.compare(0, 3, "-0x") ? 16 : 10;
  if constexpr (std::is_signed_v<T>) {
    int64 value = 0;
    const int64 iMin = proxy.iMin;
    const int64 iMax = proxy.iMax;
    bool OOR = false;

    try {
      value = std::stoll(input, nullptr, base);
    } catch (const std::invalid_argument &) {
      printerror("[Reflector] Invalid value: " << input);
      return Reflector::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      OOR = true;
    }

    if (OOR || value > iMax || value < iMin) {
      printwarning("[Reflector] Integer out of range, got: "
                   << value << " for a signed " << proxy.numBits
                   << "bit number!");
      output = static_cast<T>(input[0] == '-' ? iMin : iMax);
      return Reflector::ErrorType::OutOfRange;
    }

    output = static_cast<T>(value);
  } else {
    uint64 value = 0;
    const uint64 iMax = proxy.uMax;
    bool OOR = false;

    try {
      value = std::stoull(input, nullptr, base);
    } catch (const std::invalid_argument &) {
      printerror("[Reflector] Invalid value: " << input);
      return Reflector::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      OOR = true;
    }

    if (input[0] == '-') {
      value = static_cast<T>(value);
      printwarning("[Reflector] Applying "
                   << input
                   << " to an unsigned integer, casting to: " << value);
      errType = Reflector::ErrorType::SignMismatch;
    }

    if (OOR || value > iMax) {
      printwarning("[Reflector] Integer out of range, got: "
                   << value << " for an unsigned " << proxy.numBits
                   << "bit number!");
      output = static_cast<T>(iMax);
      return Reflector::ErrorType::OutOfRange;
    }

    output = static_cast<T>(value);
  }

  return errType;
}

template <typename T>
static Reflector::ErrorType SetNumber(es::string_view input_, T &output) {
  if constexpr (std::is_floating_point_v<T>) {
    std::string input(input_);
    constexpr T fMax = std::numeric_limits<T>::max();
    constexpr T fMin = std::numeric_limits<T>::min();

    try {
      output = [&input] {
        if constexpr (std::is_same_v<T, float>) {
          return std::stof(input);
        } else {
          return std::stod(input);
        }
      }();

      // MSVC Underflow fix
      if (std::fpclassify(output) == FP_SUBNORMAL) {
        throw std::out_of_range("");
      }
    } catch (const std::invalid_argument &) {
      printerror("[Reflector] Invalid value: " << input);
      return Reflector::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      printwarning("[Reflector] Float out of range, got: " << input);

      double fVal = atof(input.c_str());

      if (fVal > 0)
        output = fVal > 1.0 ? fMax : fMin;
      else
        output = fVal < -1.0 ? -fMax : -fMin;

      return Reflector::ErrorType::OutOfRange;
    }

    return Reflector::ErrorType::None;
  } else {
    return SetNumber(input_, output, LimitProxy<T>{});
  }
}

static Reflector::ErrorType SetBoolean(std::string input, bool &output) {
  std::for_each(input.begin(), input.end(),
                [](char &c) { c = static_cast<char>(std::tolower(c)); });

  output = !memcmp(input.c_str(), "true", 4);

  if (!output && memcmp(input.c_str(), "false", 5)) {
    printwarning("[Reflector] Expected true/false, got: " << input);
    return Reflector::ErrorType::InvalidFormat;
  }

  return Reflector::ErrorType::None;
}

static uint64 GetEnumValue(es::string_view input, JenHash hash,
                           const ReflectedEnum **rEnumFallback = nullptr) {
  const ReflectedEnum *rEnum = rEnumFallback && *rEnumFallback
                                   ? *rEnumFallback
                                   : REFEnumStorage.at(hash);

  if (rEnumFallback) {
    *rEnumFallback = rEnum;
  }

  auto namesEnd = rEnum->names + rEnum->numMembers;
  auto foundItem =
      std::find_if(rEnum->names, namesEnd, [input](es::string_view item) {
        return !item.compare(input);
      });

  if (namesEnd == foundItem) {
    throw std::range_error("[Reflector] Enum value not found: " +
                           static_cast<std::string>(input));
  }

  return rEnum->values[std::distance(rEnum->names, foundItem)];
}

static Reflector::ErrorType SetEnum(es::string_view input, char *objAddr,
                                    JenHash hash, uint16 size) {
  input = es::SkipEndWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum " << hash.raw());
    return Reflector::ErrorType::EmptyInput;
  }

  uint64 eValue = 0;

  try {
    eValue = GetEnumValue(input, hash);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: "
               << hash.raw() << " for value: " << input);
    return Reflector::ErrorType::InvalidDestination;
  } catch (const std::range_error &e) {
    printerror(e.what());
    return Reflector::ErrorType::InvalidFormat;
  }

  memcpy(objAddr, &eValue, size);

  return Reflector::ErrorType::None;
}

static Reflector::ErrorType FlagFromEnum(es::string_view input, JenHash hash,
                                         uint64 &fallbackValue,
                                         const ReflectedEnum *&fallback) {
  input = es::TrimWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum flag " << hash.raw());
    return Reflector::ErrorType::EmptyInput;
  }

  uint64 cValue = 0;

  try {
    cValue = GetEnumValue(input, hash, &fallback);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: "
               << hash.raw() << " for value: " << input);
    return Reflector::ErrorType::InvalidDestination;
  } catch (const std::range_error &e) {
    if (input != "NULL") {
      printerror(e.what());
      return Reflector::ErrorType::InvalidFormat;
    }
    return Reflector::ErrorType::None;
  }

  fallbackValue |= 1ULL << cValue;

  return Reflector::ErrorType::None;
}

static Reflector::ErrorType SetEnumFlags(es::string_view input, char *objAddr,
                                         JenHash hash, uint16 size) {
  const char *lastIterator = input.begin();
  Reflector::ErrorType errType = Reflector::ErrorType::None;
  uint64 eValue = 0;
  const ReflectedEnum *rEnumFallback = nullptr;

  for (auto &c : input) {
    if (c == '|') {
      auto subErrType = FlagFromEnum({lastIterator, size_t(&c - lastIterator)},
                                     hash, eValue, rEnumFallback);
      if (subErrType == Reflector::ErrorType::InvalidDestination) {
        return subErrType;
      } else if (subErrType != Reflector::ErrorType::None) {
        errType = subErrType;
      }

      lastIterator = &c + 1;
    }
  }

  if (lastIterator < input.end()) {
    auto subErrType =
        FlagFromEnum({lastIterator, size_t(input.end() - lastIterator)}, hash,
                     eValue, rEnumFallback);
    if (subErrType == Reflector::ErrorType::InvalidDestination) {
      return subErrType;
    } else if (subErrType != Reflector::ErrorType::None) {
      errType = subErrType;
    }
  }

  memcpy(objAddr, &eValue, size);

  return errType;
}

static Reflector::ErrorType SetReflectedArray(char startBrace, char endBrace,
                                              char *objAddr,
                                              es::string_view value,
                                              ReflType reflValue) {
  const size_t arrBegin = value.find(startBrace);

  if (arrBegin == value.npos) {
    printerror("[Reflector] Expected " << startBrace << " not found.");
    return Reflector::ErrorType::InvalidFormat;
  }

  const size_t arrEnd = value.find_last_of(endBrace);

  if (arrEnd == value.npos) {
    printerror("[Reflector] Expected " << endBrace << " not found.");
    return Reflector::ErrorType::InvalidFormat;
  } else if (arrEnd < arrBegin) {
    printerror("[Reflector] " << endBrace << " was found before " << startBrace
                              << '.');
    return Reflector::ErrorType::InvalidFormat;
  }

  value = {value.begin() + arrBegin + 1, arrEnd - arrBegin - 1};

  if (value.empty()) {
    printerror("[Reflector] Empty array input.");
    return Reflector::ErrorType::EmptyInput;
  }

  size_t curElement = 0;
  auto curIter = value.begin();
  bool localScope = false;
  const auto &arr = reflValue.asArray;

  for (auto it = value.begin(); it != value.end(); it++) {
    const bool isEnding = std::next(it) == value.end();

    if ((*it == ']' || *it == '"' || *it == ')' || *it == '}') && localScope) {
      localScope = false;
    }

    if (localScope || *it == '[' || *it == '"' || *it == '(' || *it == '{') {
      localScope = true;
      continue;
    }

    if (*it == ',' || isEnding) {
      es::string_view cValue(curIter, (isEnding ? value.end() : it) - curIter);
      cValue = es::SkipStartWhitespace(cValue);

      if (cValue.empty() && curElement < arr.numItems) {
        printerror("[Reflector] Array expected " << arr.numItems << " but got "
                                                 << curElement << '.');
        return Reflector::ErrorType::ShortInput;
      }

      if (!cValue.empty() && curElement >= arr.numItems) {
        printerror("[Reflector] Too many array elements, " << arr.numItems
                                                           << " expected.");
        return Reflector::ErrorType::OutOfRange;
      }

      SetReflectedMember(arr, cValue, objAddr + (arr.stride * curElement));
      curIter = it + 1;
      curElement++;
    }
  }

  return Reflector::ErrorType::None;
}

static Reflector::ErrorType
SetReflectedMember(ReflType reflValue, es::string_view value, char *objAddr) {
  Reflector::ErrorType errType = Reflector::ErrorType::None;
  char startBrace = 0;
  char endBrace = 0;
  value = es::SkipStartWhitespace(value);

  switch (reflValue.type) {
  case REFType::Array:
    startBrace = '{';
    endBrace = '}';
    break;
  case REFType::Vector:
    startBrace = '[';
    endBrace = ']';
    break;
  case REFType::ArrayClass:
    startBrace = '(';
    endBrace = ')';
  default:
    break;
  }

  switch (reflValue.type) {
  case REFType::Bool:
    return SetBoolean(std::string(value), *reinterpret_cast<bool *>(objAddr));
  case REFType::Integer: {
    switch (reflValue.size) {
    case 1:
      return SetNumber(value, *objAddr);
    case 2:
      return SetNumber(value, *reinterpret_cast<short *>(objAddr));
    case 4:
      return SetNumber(value, *reinterpret_cast<int *>(objAddr));
    case 8:
      return SetNumber(value, *reinterpret_cast<int64 *>(objAddr));
    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::UnsignedInteger: {
    switch (reflValue.size) {
    case 1:
      return SetNumber(value, *reinterpret_cast<unsigned char *>(objAddr));
    case 2:
      return SetNumber(value, *reinterpret_cast<unsigned short *>(objAddr));
    case 4:
      return SetNumber(value, *reinterpret_cast<unsigned int *>(objAddr));
    case 8:
      return SetNumber(value, *reinterpret_cast<uint64 *>(objAddr));
    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::FloatingPoint: {
    if (reflValue.asFloat.customFormat) {
      const auto &flt = reflValue.asFloat;
      double outValue;
      auto err = SetNumber(value, outValue);
      if (!flt.sign && outValue < 0) {
        printwarning("[Reflector] Applying " << outValue
                                             << " to an unsigned float");
        err = Reflector::ErrorType::SignMismatch;
      }
      size_t convertedValue = esFloatDetail::FromFloat(outValue, flt.mantissa,
                                                       flt.exponent, flt.sign);
      memcpy(objAddr, &convertedValue, reflValue.size);
      return err;
    } else {
      switch (reflValue.size) {
      case 4:
        return SetNumber(value, *reinterpret_cast<float *>(objAddr));
      case 8:
        return SetNumber(value, *reinterpret_cast<double *>(objAddr));
      default:
        return Reflector::ErrorType::InvalidDestination;
      }
    }
  }
  case REFType::BitFieldMember: {
    uint64 &output = *reinterpret_cast<uint64 *>(objAddr);
    auto doStuff = [&](auto &&insertVal) {
      LimitProxy<std::decay_t<decltype(insertVal)>>
          proxy{reflValue.bit.size};
      auto err = SetNumber(value, insertVal, proxy);
      BitMember bfMember;
      bfMember.size = reflValue.bit.size;
      bfMember.position = reflValue.bit.position;
      auto mask = bfMember.GetMask<uint64>();
      output &= ~mask;
      output |= insertVal.value << reflValue.bit.position;
      return err;
    };

    switch (reflValue.asBitfield.type) {
    case REFType::UnsignedInteger:
      return doStuff(BFTag<uint64>{0});
      break;
    case REFType::Integer:
      return doStuff(BFTag<int64>{0});
      break;
    case REFType::FloatingPoint: {
      const auto &flt = reflValue.asBitfield.asFloat;

      if (flt.customFormat) {
        double outValue;
        auto err = SetNumber(value, outValue);
        if (!flt.sign && outValue < 0) {
          printwarning("[Reflector] Applying " << outValue
                                               << " to an unsigned float");
          err = Reflector::ErrorType::SignMismatch;
        }
        size_t convertedValue = esFloatDetail::FromFloat(
            outValue, flt.mantissa, flt.exponent, flt.sign);
        BitMember bfMember;
        bfMember.size = reflValue.bit.size;
        bfMember.position = reflValue.bit.position;
        auto mask = bfMember.GetMask<uint64>();
        output &= ~mask;
        output |= convertedValue << reflValue.bit.position;
        return err;
      }

      return Reflector::ErrorType::InvalidDestination;
    }
    case REFType::Bool: {
      bool result;
      auto err = SetBoolean(value, result);
      auto mask = (1ULL << reflValue.bit.position);

      if (result) {
        output |= mask;
      } else {
        output &= ~mask;
      }
      return err;
    }
    case REFType::Enum: {
      uint64 bValue;
      auto err =
          SetEnum(value, reinterpret_cast<char *>(&bValue),
                  JenHash(reflValue.asBitfield.typeHash), sizeof(bValue));
      BitMember bfMember;
      bfMember.size = reflValue.bit.size;
      bfMember.position = reflValue.bit.position;
      auto mask = bfMember.GetMask<uint64>();
      output &= ~mask;
      output |= bValue << reflValue.bit.position;
      return err;
    }

    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::Enum:
    return SetEnum(value, objAddr, JenHash(reflValue.asClass.typeHash),
                   reflValue.size);
  case REFType::EnumFlags:
    return SetEnumFlags(value, objAddr, JenHash(reflValue.asClass.typeHash),
                        reflValue.size);
  case REFType::String:
    *reinterpret_cast<std::string *>(objAddr) = value;
    break;
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass:
    return SetReflectedArray(startBrace, endBrace, objAddr, value, reflValue);
  default:
    break;
  }

  return errType;
}

Reflector::ErrorType Reflector::SetReflectedValue(ReflType type,
                                                  es::string_view value) {
  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr =
      thisAddr + (type.type == REFType::BitFieldMember ? 0 : type.offset);

  return SetReflectedMember(type, value, thisAddr);
}

Reflector::ErrorType Reflector::SetReflectedValue(ReflType type,
                                                  es::string_view value,
                                                  size_t subID) {
  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr += type.offset;

  if (IsArrayVec(type.type)) {
    thisAddr += type.asArray.stride * subID;
  }

  if (type.type == REFType::Vector) {
    return SetReflectedMember(type.asVector, value, thisAddr);
  }

  return SetReflectedMember(type.asArray, value, thisAddr);
}

Reflector::ErrorType Reflector::SetReflectedValue(ReflType type,
                                                  es::string_view value,
                                                  size_t subID,
                                                  size_t element) {
  if (!::IsArray(type.type)) {
    return ErrorType::InvalidDestination;
  }
  bool enumFlags = false;
  const auto &arr = type.asArray;

  switch (arr.type) {
  /*case REFType::EnumFlags:
    enumFlags = true;*/
  case REFType::Vector:
    break;
  default:
    return ErrorType::InvalidDestination;
  }

  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr += type.offset + arr.stride * subID;

  if (!enumFlags) {
    const auto &vec = arr.asVector;

    if (element >= vec.numItems) {
      printerror("[Reflector] Too many vector elements, " << (int)vec.numItems
                                                          << " expected.");
      return ErrorType::OutOfRange;
    }

    thisAddr += vec.stride * element;
    return SetReflectedMember(arr.asVector, value, thisAddr);
  }

  return ErrorType::InvalidDestination;
}

Reflector::ErrorType
Reflector::SetReflectedValueInt(ReflType reflValue, int64 value, size_t subID) {
  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr = thisAddr + reflValue.offset;
  bool isArrVec = IsArrayVec(reflValue.type);
  REFType cType = reflValue.type;
  size_t typeSize = reflValue.size;

  if (isArrVec) {
    const auto &arr = reflValue.asArray;
    if (subID >= arr.numItems) {
      return Reflector::ErrorType::OutOfRange;
    }

    thisAddr += subID * arr.stride;
    cType = arr.type;
    typeSize = arr.stride;
  }

  if (cType != REFType::Integer) {
    return Reflector::ErrorType::InvalidDestination;
  }

  memcpy(thisAddr, &value, typeSize);

  return Reflector::ErrorType::None;
}

Reflector::ErrorType Reflector::SetReflectedValueUInt(ReflType reflValue,
                                                      uint64 value,
                                                      size_t subID) {
  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr = thisAddr + reflValue.offset;
  bool isArrVec = IsArrayVec(reflValue.type);
  REFType cType = reflValue.type;
  size_t typeSize = reflValue.size;

  if (isArrVec) {
    const auto &arr = reflValue.asArray;
    if (subID >= arr.numItems) {
      return Reflector::ErrorType::OutOfRange;
    }

    thisAddr += subID * arr.stride;
    cType = arr.type;
    typeSize = arr.stride;
  }

  if (cType != REFType::UnsignedInteger) {
    return Reflector::ErrorType::InvalidDestination;
  }

  memcpy(thisAddr, &value, typeSize);

  return Reflector::ErrorType::None;
}

Reflector::ErrorType Reflector::SetReflectedValueFloat(ReflType reflValue,
                                                       double value,
                                                       size_t subID) {
  auto inst = GetReflectedInstance();
  char *thisAddr = static_cast<char *>(inst.instance);
  thisAddr = thisAddr + reflValue.offset;
  bool isArrVec = IsArrayVec(reflValue.type);
  REFType cType = reflValue.type;
  size_t typeSize = reflValue.size;

  if (isArrVec) {
    const auto &arr = reflValue.asArray;
    if (subID >= arr.numItems) {
      return Reflector::ErrorType::OutOfRange;
    }

    thisAddr += subID * arr.stride;
    cType = arr.type;
    typeSize = arr.stride;
  }

  if (cType != REFType::FloatingPoint) {
    return Reflector::ErrorType::InvalidDestination;
  }
  // todo esfloat
  switch (typeSize) {
  case 4:
    reinterpret_cast<float &>(*thisAddr) = static_cast<float>(value);
    return Reflector::ErrorType::None;
  case 8:
    reinterpret_cast<double &>(*thisAddr) = value;
    return Reflector::ErrorType::None;
  default:
    return Reflector::ErrorType::InvalidDestination;
  }
}

static es::string_view
PrintEnumValue(JenHash hash, uint64 value,
               const ReflectedEnum **rEnumFallback = nullptr) {
  const ReflectedEnum *rEnum = rEnumFallback && *rEnumFallback
                                   ? *rEnumFallback
                                   : REFEnumStorage.at(hash);

  if (rEnumFallback)
    *rEnumFallback = rEnum;

  const uint64 *valuesEnd = rEnum->values + rEnum->numMembers;
  const uint64 *foundItem = std::find_if(
      rEnum->values, valuesEnd, [value](uint64 item) { return item == value; });

  if (foundItem == valuesEnd) {
    throw std::range_error("[Reflector] Enum value not found: " +
                           std::to_string(value));
  }

  return rEnum->names[std::distance(rEnum->values, foundItem)];
}

static std::string PrintEnum(const char *objAddr, JenHash hash, uint16 elSize) {
  uint64 eValue = 0;

  memcpy(reinterpret_cast<char *>(&eValue), objAddr, elSize);

  return std::string{PrintEnumValue(hash, eValue)};
}

static std::string PrintEnumFlags(const char *objAddr, JenHash hash,
                                  uint16 elSize) {
  uint64 eValue;
  const ReflectedEnum *rEnumFallback = nullptr;
  std::string result;
  const size_t numBits = elSize * 8;

  memcpy(reinterpret_cast<char *>(&eValue), objAddr, elSize);

  for (size_t t = 0; t < numBits; t++) {
    if (eValue & (1ULL << t)) {
      if (result.size())
        result.append(" | ");

      try {
        result.append(PrintEnumValue(hash, t, &rEnumFallback));
      } catch (const std::out_of_range &) {
        printerror("[Reflector] Unregistered enum hash: " << hash.raw());
        break;
      } catch (const std::range_error &e) {
        printerror(e.what());
      } catch (...) {
        printerror("[Reflector] Unhandled exception: PrintEnumValue");
        break;
      }
    }
  }

  if (result.empty())
    return "NULL";

  return result;
}

static std::string GetReflectedPrimitive(const char *objAddr, ReflType type) {
  char startBrace = 0;
  char endBrace = 0;

  switch (type.type) {
  case REFType::Array:
    startBrace = '{';
    endBrace = '}';
    break;
  case REFType::Vector:
    startBrace = '[';
    endBrace = ']';
    break;
  case REFType::ArrayClass:
    startBrace = '(';
    endBrace = ')';
  default:
    break;
  }

  switch (type.type) {
  case REFType::Bool:
    return *reinterpret_cast<const bool *>(objAddr) ? "true" : "false";

  case REFType::Integer: {
    switch (type.size) {
    case 1:
      return std::to_string(
          static_cast<int32>(*reinterpret_cast<const int8 *>(objAddr)));
    case 2:
      return std::to_string(*reinterpret_cast<const int16 *>(objAddr));
    case 4:
      return std::to_string(*reinterpret_cast<const int32 *>(objAddr));
    case 8:
      return std::to_string(*reinterpret_cast<const int64 *>(objAddr));

    default:
      return "";
    }
  }
  case REFType::UnsignedInteger: {
    switch (type.size) {
    case 1:
      return std::to_string(
          static_cast<int32>(*reinterpret_cast<const uint8 *>(objAddr)));
    case 2:
      return std::to_string(*reinterpret_cast<const uint16 *>(objAddr));
    case 4:
      return std::to_string(*reinterpret_cast<const uint32 *>(objAddr));
    case 8:
      return std::to_string(*reinterpret_cast<const uint64 *>(objAddr));

    default:
      return "";
    }
  }
  case REFType::FloatingPoint: {
    char _tmpBuffer[0x20]{};
    if (type.asFloat.customFormat) {
      size_t encValue = 0;
      const auto &flt = type.asFloat;
      memcpy(&encValue, objAddr, type.size);

      auto retVal = esFloatDetail::ToFloat(encValue, flt.mantissa, flt.exponent,
                                           flt.sign);
      snprintf(_tmpBuffer, sizeof(_tmpBuffer), "%.6g", retVal);
    } else {

      switch (type.size) {
      case 4:
        snprintf(_tmpBuffer, sizeof(_tmpBuffer), "%.6g",
                 *reinterpret_cast<const float *>(objAddr));
        break;
      case 8:
        snprintf(_tmpBuffer, sizeof(_tmpBuffer), "%.13g",
                 *reinterpret_cast<const double *>(objAddr));
        break;

      default:
        break;
      }
    }
    return _tmpBuffer;
  }
  case REFType::BitFieldMember: {
    uint64 output = *reinterpret_cast<const uint64 *>(objAddr);
    BitMember bfMember;
    bfMember.size = type.bit.size;
    bfMember.position = type.bit.position;
    auto mask = bfMember.GetMask<uint64>();
    output = (output & mask) >> bfMember.position;

    switch (type.asBitfield.type) {
    case REFType::UnsignedInteger:
      return std::to_string(output);
    case REFType::Bool:
      return output ? "true" : "false";
    case REFType::Enum:
      return PrintEnumValue(JenHash(type.asBitfield.typeHash), output);
    case REFType::FloatingPoint: {
      const auto &flt = type.asBitfield.asFloat;

      if (flt.customFormat) {
        char _tmpBuffer[0x20]{};
        auto retVal = esFloatDetail::ToFloat(output, flt.mantissa, flt.exponent,
                                             flt.sign);
        snprintf(_tmpBuffer, sizeof(_tmpBuffer), "%.6g", retVal);
        return _tmpBuffer;
      }

      return "NaN";
    }
    default:
      break;
    }

    int64 signedOutput = output;
    LimitProxy<BFTag<int64>> limit{type.size};

    if (signedOutput & limit.iMin) {
      signedOutput |= ~limit.uMax;
    }

    return std::to_string(signedOutput);
  }

  case REFType::EnumFlags:
    return PrintEnumFlags(objAddr, JenHash(type.asClass.typeHash), type.size);

  case REFType::Enum: {
    try {
      return PrintEnum(objAddr, JenHash(type.asClass.typeHash), type.size);
    } catch (const std::out_of_range &) {
      printerror(
          "[Reflector] Unregistered enum hash: " << type.asClass.typeHash);
    } catch (const std::range_error &e) {
      printerror(e.what());
    } catch (...) {
      printerror("[Reflector] Unhandled exception: PrintEnum");
    }

    break;
  }

  case REFType::CString:
    return *reinterpret_cast<const char *const *>(objAddr);

  case REFType::String:
    return *reinterpret_cast<const std::string *>(objAddr);

  case REFType::Class:
  case REFType::BitFieldClass:
    return "SUBCLASS_TYPE";
  default:
    break;
  }

  if (startBrace && endBrace) {
    const auto &arr = type.asArray;
    const auto numItems = arr.numItems;
    std::string outVal;
    outVal.push_back(startBrace);

    for (int i = 0; i < numItems; i++) {
      outVal += GetReflectedPrimitive(objAddr + (arr.stride * i), arr);
      outVal += ", ";
    }

    outVal.pop_back();
    outVal.pop_back();
    outVal.push_back(endBrace);

    return outVal;
  }

  return "";
}

std::string Reflector::GetReflectedValue(size_t id) const {
  if (id >= GetNumReflectedValues())
    return "";

  auto inst = GetReflectedInstance();
  const char *thisAddr = static_cast<const char *>(inst.constInstance);
  const ReflType &reflValue = inst.rfStatic->types[id];
  const int valueOffset =
      reflValue.type == REFType::BitFieldMember ? 0 : reflValue.offset;

  return GetReflectedPrimitive(thisAddr + valueOffset, reflValue);
}

std::string Reflector::GetReflectedValue(size_t id, size_t subID) const {
  if (id >= GetNumReflectedValues())
    return "";

  auto inst = GetReflectedInstance();
  const char *thisAddr = static_cast<const char *>(inst.constInstance);
  const ReflType &reflValue = inst.rfStatic->types[id];
  const char *objAddr = thisAddr + reflValue.offset;

  switch (reflValue.type) {
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass: {
    const auto &arr = reflValue.asArray;
    if (arr.numItems <= subID) {
      return "";
    }

    return GetReflectedPrimitive(objAddr + arr.stride * subID, arr);
  }
  case REFType::EnumFlags: {
    if (reflValue.size * 8 <= subID) {
      return "";
    }

    uint64 eValue;

    memcpy(reinterpret_cast<char *>(&eValue), objAddr, reflValue.size);

    return (eValue & (1ULL << subID)) ? "true" : "false";
  }

  default:
    return "";
  }
}

std::string Reflector::GetReflectedValue(size_t id, size_t subID,
                                         size_t element) const {
  if (id >= GetNumReflectedValues() || !IsArray(id))
    return "";

  auto inst = GetReflectedInstance();
  const char *thisAddr = static_cast<const char *>(inst.constInstance);
  const ReflType &reflValue = inst.rfStatic->types[id];
  const char *objAddr = thisAddr + reflValue.offset;
  const auto &arr = reflValue.asArray;

  switch (arr.type) {
  case REFType::Vector: {
    if (arr.numItems <= subID || arr.asVector.numItems <= element) {
      return "";
    }

    ReflType subType = arr.asVector;

    return GetReflectedPrimitive(
        objAddr + arr.stride * subID + subType.size * element, subType);
  }
  case REFType::EnumFlags: {
    if (arr.stride * 8 <= element || arr.numItems <= subID) {
      return "";
    }

    uint64 eValue;

    memcpy(reinterpret_cast<char *>(&eValue), objAddr + arr.stride * subID,
           arr.stride);

    return (eValue & (1ULL << element)) ? "true" : "false";
  }

  default:
    return "";
  }
}

ReflectedInstance Reflector::GetReflectedSubClass(size_t id,
                                                  size_t subID) const {
  if (id >= GetNumReflectedValues())
    return {};

  auto inst = GetReflectedInstance();
  const ReflType &reflValue = inst.rfStatic->types[id];
  const char *thisAddr =
      static_cast<const char *>(inst.constInstance) + reflValue.offset;
  REFType cType = reflValue.type;
  const bool isArray = IsArray(id);
  ReflTypeClass classType = reflValue.asClass;

  if (isArray) {
    const auto &arr = reflValue.asArray;
    if (subID >= reflValue.asArray.numItems) {
      return {};
    }

    thisAddr += subID * arr.stride;
    cType = arr.type;
    classType = arr.asClass;
  }

  if ((cType != REFType::Class && cType != REFType::BitFieldClass) ||
      !REFSubClassStorage.count(JenHash(classType.typeHash)))
    return {};

  return {REFSubClassStorage.at(JenHash(classType.typeHash)), thisAddr};
}

ReflectedInstance Reflector::GetReflectedSubClass(size_t id, size_t subID) {
  return const_cast<const Reflector *>(this)->GetReflectedSubClass(id, subID);
}

RefEnumMapper REFEnumStorage;
RefSubClassMapper REFSubClassStorage;
