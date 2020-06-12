/*  A source for Reflector class
    more info in README for PreCore Project

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

#include "reflector.hpp"
#include "masterprinter.hpp"
#include "string_view.hpp"
#include <algorithm>
#include <cmath>
#include <cctype>

static Reflector::ErrorType
SetReflectedMember(reflType reflValue, es::string_view value, char *objAddr);

const reflType *Reflector::GetReflectedType(const JenHashStrong hash) const {
  const reflectorStatic *inst = _rfRetreive().rfStatic;
  const size_t _ntypes = GetNumReflectedValues();

  for (size_t t = 0; t < _ntypes; t++)
    if (inst->types[t].valueNameHash == hash)
      return GetReflectedType(t);

  if (inst->typeAliasHashes)
    for (size_t t = 0; t < _ntypes; t++)
      if (inst->typeAliasHashes[t] == hash)
        return GetReflectedType(t);

  return nullptr;
}

template <typename T> T SetNumber(const std::string &input);

template <> float SetNumber(const std::string &input) {
  return std::stof(input);
}

template <> double SetNumber(const std::string &input) {
  return std::stod(input);
}

template <typename T>
static Reflector::ErrorType SetNumber(const std::string &input, T &output,
                                      std::false_type) {
  constexpr T fMax = std::numeric_limits<T>::max();
  constexpr T fMin = std::numeric_limits<T>::min();

  try {
    output = SetNumber<T>(input);

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
}

template <typename T>
static Reflector::ErrorType SetNumber(const std::string &input, T &output,
                                      std::true_type) {
  constexpr int numBits = sizeof(T) * 8;
  Reflector::ErrorType errType = Reflector::ErrorType::None;
  const int base =
      !input.compare(0, 2, "0x") || !input.compare(0, 3, "-0x") ? 16 : 10;

  if (std::is_signed<T>::value) {
    int64 value = 0;
    constexpr int64 iMin = std::numeric_limits<T>::min();
    constexpr int64 iMax = std::numeric_limits<T>::max();
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
                   << value << " for a signed " << numBits << "bit number!");
      output = static_cast<T>(input[0] == '-' ? iMin : iMax);
      return Reflector::ErrorType::OutOfRange;
    }

    output = static_cast<T>(value);
  } else {
    uint64 value = 0;
    constexpr uint64 iMax = std::numeric_limits<T>::max();
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
                   << value << " for an unsigned " << numBits << "bit number!");
      output = static_cast<T>(iMax);
      return Reflector::ErrorType::OutOfRange;
    }

    output = static_cast<T>(value);
  }

  return errType;
}

template <typename T>
static Reflector::ErrorType SetValue(const std::string &input, T &output) {
  return SetNumber(input, output, typename std::is_integral<T>::type{});
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
                           ReflectedEnum **rEnumFallback = nullptr) {
  ReflectedEnum &rEnum = rEnumFallback && *rEnumFallback
                             ? **rEnumFallback
                             : REFEnumStorage.at(hash);

  if (rEnumFallback)
    *rEnumFallback = &rEnum;

  ReflectedEnum::iterator foundItem =
      std::find_if(rEnum.begin(), rEnum.end(), [input](es::string_view item) {
        return !item.compare(input);
      });

  if (es::IsEnd(rEnum, foundItem)) {
    throw std::range_error("[Reflector] Enum value not found: " +
                           static_cast<std::string>(input));
  }

  return rEnum.values[std::distance(rEnum.begin(), foundItem)];
}

static Reflector::ErrorType SetEnum(es::string_view input, char *objAddr,
                                    JenHash hash, uint16 size) {
  input = es::SkipEndWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum " << hash);
    return Reflector::ErrorType::EmptyInput;
  }

  uint64 eValue = 0;

  try {
    eValue = GetEnumValue(input, hash);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: " << hash << " for value: "
                                                      << input);
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
                                         ReflectedEnum *&fallback) {
  input = es::TrimWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum flag " << hash);
    return Reflector::ErrorType::EmptyInput;
  }

  uint64 cValue = 0;

  try {
    cValue = GetEnumValue(input, hash, &fallback);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: " << hash << " for value: "
                                                      << input);
    return Reflector::ErrorType::InvalidDestination;
  } catch (const std::range_error &e) {
    printerror(e.what());
    return Reflector::ErrorType::InvalidFormat;
  }

  fallbackValue |= 1ULL << cValue;

  return Reflector::ErrorType::None;
}

static Reflector::ErrorType SetEnumFlags(es::string_view input, char *objAddr,
                                         JenHash hash, uint16 size) {
  const char *lastIterator = input.begin();
  Reflector::ErrorType errType = Reflector::ErrorType::None;
  uint64 eValue = 0;
  ReflectedEnum *rEnumFallback = nullptr;

  for (auto &c : input) {
    if (c == '|') {
      auto subErrType =
          FlagFromEnum({lastIterator, &c}, hash, eValue, rEnumFallback);
      if (subErrType == Reflector::ErrorType::InvalidDestination)
        return subErrType;
      else if (subErrType != Reflector::ErrorType::None)
        errType = subErrType;

      lastIterator = &c + 1;
    }
  }

  if (lastIterator < input.end()) {
    auto subErrType =
        FlagFromEnum({lastIterator, input.end()}, hash, eValue, rEnumFallback);
    if (subErrType == Reflector::ErrorType::InvalidDestination)
      return subErrType;
    else if (subErrType != Reflector::ErrorType::None)
      errType = subErrType;
  }

  memcpy(objAddr, &eValue, size);

  return errType;
}

static Reflector::ErrorType SetReflectedArray(char startBrace, char endBrace,
                                              char *objAddr,
                                              es::string_view value,
                                              reflType reflValue) {
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

  value = {value.begin() + arrBegin + 1, value.begin() + arrEnd};

  if (value.empty()) {
    printerror("[Reflector] Empty array input.");
    return Reflector::ErrorType::EmptyInput;
  }

  size_t curElement = 0;
  auto curIter = value.begin();
  reflType subVal = reflValue;
  subVal.type = subVal.subType;

  if (subVal.type == REFType::Vector) {
    _DecomposedVectorHash dec = {reflValue.typeHash};
    subVal.subType = dec.type;
    subVal.numItems = dec.numItems;
    subVal.subSize = dec.size;
  }
  bool localScope = false;

  for (auto it = value.begin(); it != value.end(); it++) {
    const bool isEnding = std::next(it) == value.end();

    if ((*it == ']' || *it == '"' || *it == ')' || *it == '}') && localScope)
      localScope = false;

    if (localScope || *it == '[' || *it == '"' || *it == '(' || *it == '{') {
      localScope = true;
      continue;
    }

    if (*it == ',' || isEnding) {
      es::string_view cValue(curIter, isEnding ? value.end() : it);
      cValue = es::SkipStartWhitespace(cValue);

      if (cValue.empty() && curElement < reflValue.numItems) {
        printerror("[Reflector] Array expected "
                   << reflValue.numItems << " but got " << curElement << '.');
        return Reflector::ErrorType::ShortInput;
      }

      if (!cValue.empty() && curElement >= reflValue.numItems) {
        printerror("[Reflector] Too many array elements, " << reflValue.numItems
                                                           << " expected.");
        return Reflector::ErrorType::OutOfRange;
      }

      SetReflectedMember(subVal, cValue,
                         objAddr + (reflValue.subSize * curElement));
      curIter = it + 1;
      curElement++;
    }
  }

  return Reflector::ErrorType::None;
}

static Reflector::ErrorType
SetReflectedMember(reflType reflValue, es::string_view value, char *objAddr) {
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
    return SetBoolean(value, *reinterpret_cast<bool *>(objAddr));
  case REFType::Integer: {
    switch (reflValue.subSize) {
    case 1:
      return SetValue(value, *objAddr);
    case 2:
      return SetValue(value, *reinterpret_cast<short *>(objAddr));
    case 4:
      return SetValue(value, *reinterpret_cast<int *>(objAddr));
    case 8:
      return SetValue(value, *reinterpret_cast<int64 *>(objAddr));
    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::UnsignedInteger: {
    switch (reflValue.subSize) {
    case 1:
      return SetValue(value, *reinterpret_cast<unsigned char *>(objAddr));
    case 2:
      return SetValue(value, *reinterpret_cast<unsigned short *>(objAddr));
    case 4:
      return SetValue(value, *reinterpret_cast<unsigned int *>(objAddr));
    case 8:
      return SetValue(value, *reinterpret_cast<uint64 *>(objAddr));
    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::FloatingPoint: {
    switch (reflValue.subSize) {
    case 4:
      return SetValue(value, *reinterpret_cast<float *>(objAddr));
    case 8:
      return SetValue(value, *reinterpret_cast<double *>(objAddr));
    default:
      return Reflector::ErrorType::InvalidDestination;
    }
  }
  case REFType::Enum:
    return SetEnum(value, objAddr, reflValue.typeHash, reflValue.subSize);
  case REFType::EnumFlags:
    return SetEnumFlags(value, objAddr, reflValue.typeHash, reflValue.subSize);
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

Reflector::ErrorType Reflector::SetReflectedValue(reflType type,
                                                  es::string_view value) {
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);
  thisAddr = thisAddr + type.offset;

  return SetReflectedMember(type, value, thisAddr);
}

Reflector::ErrorType
Reflector::SetReflectedValueInt(reflType reflValue, int64 value, size_t subID) {
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);
  thisAddr = thisAddr + reflValue.offset;

  if (subID) {
    switch (reflValue.type) {
    case REFType::Array:
    case REFType::Vector:
    case REFType::ArrayClass:
      if (subID >= reflValue.numItems)
        return Reflector::ErrorType::OutOfRange;
      thisAddr += subID * reflValue.subSize;
      break;
    default:
      return Reflector::ErrorType::OutOfRange;
    }
  }

  REFType cType = subID ? reflValue.subType : reflValue.type;

  if (cType != REFType::Integer)
    return Reflector::ErrorType::InvalidDestination;

  memcpy(thisAddr, &value, reflValue.subSize);

  return Reflector::ErrorType::None;
}

Reflector::ErrorType Reflector::SetReflectedValueUInt(reflType reflValue,
                                                      uint64 value,
                                                      size_t subID) {
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);
  thisAddr = thisAddr + reflValue.offset;

  if (subID) {
    switch (reflValue.type) {
    case REFType::Array:
    case REFType::Vector:
    case REFType::ArrayClass:
      if (subID >= reflValue.numItems)
        return Reflector::ErrorType::OutOfRange;
      thisAddr += subID * reflValue.subSize;
      break;
    default:
      return Reflector::ErrorType::OutOfRange;
    }
  }

  REFType cType = subID ? reflValue.subType : reflValue.type;

  if (cType != REFType::UnsignedInteger)
    return Reflector::ErrorType::InvalidDestination;

  memcpy(thisAddr, &value, reflValue.subSize);

  return Reflector::ErrorType::None;
}

Reflector::ErrorType Reflector::SetReflectedValueFloat(reflType reflValue,
                                                       double value,
                                                       size_t subID) {
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);
  thisAddr = thisAddr + reflValue.offset;

  if (subID) {
    switch (reflValue.type) {
    case REFType::Array:
    case REFType::Vector:
    case REFType::ArrayClass:
      if (subID >= reflValue.numItems)
        return Reflector::ErrorType::OutOfRange;
      thisAddr += subID * reflValue.subSize;
      break;
    default:
      return Reflector::ErrorType::OutOfRange;
    }
  }

  REFType cType = subID ? reflValue.subType : reflValue.type;

  if (cType != REFType::FloatingPoint)
    return Reflector::ErrorType::InvalidDestination;

  switch (reflValue.subSize) {
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

static es::string_view PrintEnumValue(JenHash hash, uint64 value,
                                      ReflectedEnum **rEnumFallback = nullptr) {
  ReflectedEnum &rEnum = rEnumFallback && *rEnumFallback
                             ? **rEnumFallback
                             : REFEnumStorage.at(hash);

  if (rEnumFallback)
    *rEnumFallback = &rEnum;

  const uint64 *valuesEnd = rEnum.values + rEnum.size();
  const uint64 *foundItem = std::find_if(
      rEnum.values, valuesEnd, [value](uint64 item) { return item == value; });

  if (foundItem == valuesEnd) {
    throw std::range_error("[Reflector] Enum value not found: " +
                           std::to_string(value));
  }

  return rEnum.at(std::distance(rEnum.values, foundItem));
}

static std::string PrintEnum(const char *objAddr, JenHash hash, uint16 elSize) {
  uint64 eValue = 0;

  memcpy(reinterpret_cast<char *>(&eValue), objAddr, elSize);

  return PrintEnumValue(hash, eValue);
}

static std::string PrintEnumFlags(const char *objAddr, JenHash hash,
                                  uint16 elSize) {
  uint64 eValue;
  ReflectedEnum *rEnumFallback = nullptr;
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
        printerror("[Reflector] Unregistered enum hash: " << hash);
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

static std::string GetReflectedPrimitive(const char *objAddr, reflType type) {
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
    switch (type.subSize) {
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
    switch (type.subSize) {
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
    char _tmpBuffer[0x20] = {};

    switch (type.subSize) {
    case 4:
      sprintf(_tmpBuffer, "%.6g", *reinterpret_cast<const float *>(objAddr));
      break;
    case 8:
      sprintf(_tmpBuffer, "%.13g", *reinterpret_cast<const double *>(objAddr));
      break;

    default:
      break;
    }

    return _tmpBuffer;
  }

  case REFType::EnumFlags:
    return PrintEnumFlags(objAddr, type.typeHash, type.subSize);

  case REFType::Enum: {
    try {
      return PrintEnum(objAddr, type.typeHash, type.subSize);
    } catch (const std::out_of_range &) {
      printerror("[Reflector] Unregistered enum hash: " << type.typeHash);
    } catch (const std::range_error &e) {
      printerror(e.what());
    } catch (...) {
      printerror("[Reflector] Unhandled exception: PrintEnum");
    }

    break;
  }

  case REFType::CString:
    return objAddr;

  case REFType::String:
    return *reinterpret_cast<const std::string *>(objAddr);

  case REFType::Class:
    return "SUBCLASS_TYPE";
  default:
    break;
  }

  if (startBrace && endBrace) {
    const auto numItems = type.numItems;
    std::string outVal;
    outVal.push_back(startBrace);
    reflType subType = type;
    subType.type = type.subType;

    if (subType.type == REFType::Vector) {
      _DecomposedVectorHash dec = {subType.typeHash};
      subType.subType = dec.type;
      subType.numItems = dec.numItems;
      subType.subSize = dec.size;
    }

    for (int i = 0; i < numItems; i++) {
      outVal += GetReflectedPrimitive(objAddr + (type.subSize * i), subType);
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

  const reflectorInstanceConst inst = _rfRetreive();
  const char *thisAddr = static_cast<const char *>(inst.rfInstance);
  const reflType &reflValue = inst.rfStatic->types[id];
  const int valueOffset = reflValue.offset;

  return GetReflectedPrimitive(thisAddr + valueOffset, reflValue);
}

const Reflector::SubClass Reflector::GetReflectedSubClass(size_t id,
                                                          size_t subID) const {
  if (id >= GetNumReflectedValues())
    return {};

  const reflectorInstanceConst inst = _rfRetreive();
  const reflType &reflValue = inst.rfStatic->types[id];
  const char *thisAddr =
      static_cast<const char *>(inst.rfInstance) + reflValue.offset;
  REFType cType = reflValue.type;

  if (subID && subID >= reflValue.numItems)
    return {};

  switch (reflValue.type) {
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass:
    thisAddr += subID * reflValue.subSize;
    cType = reflValue.subType;
  default:
    break;
  }

  if (cType != REFType::Class || !REFSubClassStorage.count(reflValue.typeHash))
    return {};

  return {{}, {REFSubClassStorage.at(reflValue.typeHash), thisAddr}};
}

const Reflector::SubClass Reflector::GetReflectedSubClass(size_t id,
                                                          size_t subID) {
  if (id >= GetNumReflectedValues())
    return {};

  const reflectorInstance inst = _rfRetreive();
  const reflType &reflValue = inst.rfStatic->types[id];
  char *thisAddr = static_cast<char *>(inst.rfInstance) + reflValue.offset;
  REFType cType = reflValue.type;

  if (subID && subID >= reflValue.numItems)
    return {};

  switch (reflValue.type) {
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass:
    thisAddr += subID * reflValue.subSize;
    cType = reflValue.subType;
  default:
    break;
  }

  if (cType != REFType::Class || !REFSubClassStorage.count(reflValue.typeHash))
    return {};

  return {{REFSubClassStorage.at(reflValue.typeHash), thisAddr},
          {REFSubClassStorage.at(reflValue.typeHash), thisAddr}};
}

RefEnumMapper REFEnumStorage;
RefSubClassMapper REFSubClassStorage;