/*  A source for Reflector class

    Copyright 2018-2024 Lukas Cone

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

#include "spike/reflect/reflector.hpp"
#include "spike/master_printer.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/type/float.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <ostream>
#include <string_view>

static const char *TYPE_NAMES[]{"x", "y", "z", "w"};

ReflType Make(REFType type, int index, int size) {
  ReflType retVal;
  retVal.type = type;
  retVal.index = index;
  retVal.offset = size * index;
  retVal.valueNameHash = JenHash(TYPE_NAMES[index]);
  retVal.size = size;

  return retVal;
};

union VectorKey {
  uint32 raw = 0;
  struct {
    REFType type;
    uint8 numElements;
    uint8 size;
  };

  bool operator<(VectorKey o) const { return raw < o.raw; }
};

template <REFType type, int size>
static const ReflType VECTOR_TYPES[4]{
    Make(type, 0, size),
    Make(type, 1, size),
    Make(type, 2, size),
    Make(type, 3, size),
};

template <REFType type, int numElements, int size>
consteval std::pair<VectorKey, reflectorStatic> MakeVectorRefl() {
  VectorKey key;
  key.type = type;
  key.numElements = numElements;
  key.size = size;

  return {key, reflectorStatic(numElements, VECTOR_TYPES<type, size>,
                               TYPE_NAMES, numElements * size)};
}

static const std::map<VectorKey, reflectorStatic> VECTORS{
    MakeVectorRefl<REFType::FloatingPoint, 2, 4>(),
    MakeVectorRefl<REFType::FloatingPoint, 3, 4>(),
    MakeVectorRefl<REFType::FloatingPoint, 4, 4>(),
    MakeVectorRefl<REFType::Integer, 2, 1>(),
    MakeVectorRefl<REFType::Integer, 3, 1>(),
    MakeVectorRefl<REFType::Integer, 4, 1>(),
    MakeVectorRefl<REFType::Integer, 2, 2>(),
    MakeVectorRefl<REFType::Integer, 3, 2>(),
    MakeVectorRefl<REFType::Integer, 4, 2>(),
    MakeVectorRefl<REFType::Integer, 2, 4>(),
    MakeVectorRefl<REFType::Integer, 3, 4>(),
    MakeVectorRefl<REFType::Integer, 4, 4>(),
    MakeVectorRefl<REFType::UnsignedInteger, 2, 1>(),
    MakeVectorRefl<REFType::UnsignedInteger, 3, 1>(),
    MakeVectorRefl<REFType::UnsignedInteger, 4, 1>(),
    MakeVectorRefl<REFType::UnsignedInteger, 2, 2>(),
    MakeVectorRefl<REFType::UnsignedInteger, 3, 2>(),
    MakeVectorRefl<REFType::UnsignedInteger, 4, 2>(),
    MakeVectorRefl<REFType::UnsignedInteger, 2, 4>(),
    MakeVectorRefl<REFType::UnsignedInteger, 3, 4>(),
    MakeVectorRefl<REFType::UnsignedInteger, 4, 4>(),
};

static ReflectorMember::ErrorType
SetReflectedMember(ReflType reflValue, std::string_view value, char *objAddr,
                   const reflectorStatic *refl);

static const ReflType *GetReflectedType(const reflectorStatic *inst,
                                        const JenHash hash) {
  const size_t _ntypes = inst->nTypes;

  for (size_t t = 0; t < _ntypes; t++) {
    if (inst->types[t].valueNameHash == hash) {
      return inst->types + t;
    }
  }

  if (inst->typeAliasHashes) {
    for (size_t t = 0; t < _ntypes; t++) {
      if (inst->typeAliasHashes[t] == hash) {
        return inst->types + t;
      }
    }
  }

  return nullptr;
}

ReflectorMember Reflector::operator[](JenHash hash) const {
  ReflectedInstance inst = GetReflectedInstance();
  if (!inst) {
    return {{}, 0};
  }
  const ReflType *retVal = ::GetReflectedType(inst.rfStatic, hash);

  if (retVal) {
    return {inst, retVal->index};
  }

  while (inst.rfStatic->baseClass.raw()) {
    inst.rfStatic = reflectorStatic::Registry().at(inst.rfStatic->baseClass);
    retVal = ::GetReflectedType(inst.rfStatic, hash);
    if (retVal) {
      return {inst, retVal->index};
    }
  }

  return {{}, 0};
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
static ReflectorMember::ErrorType SetNumber(std::string_view input_, T &output,
                                            ProxyType proxy) {
  std::string input(input_);
  ReflectorMember::ErrorType errType = ReflectorMember::ErrorType::None;
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
      return ReflectorMember::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      OOR = true;
    }

    if (OOR || value > iMax || value < iMin) {
      printwarning("[Reflector] Integer out of range, got: "
                   << value << " for a signed " << proxy.numBits
                   << "bit number!");
      output = static_cast<T>(input[0] == '-' ? iMin : iMax);
      return ReflectorMember::ErrorType::OutOfRange;
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
      return ReflectorMember::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      OOR = true;
    }

    if (input[0] == '-') {
      value = static_cast<T>(value);
      printwarning("[Reflector] Applying "
                   << input
                   << " to an unsigned integer, casting to: " << value);
      errType = ReflectorMember::ErrorType::SignMismatch;
    }

    if (OOR || value > iMax) {
      printwarning("[Reflector] Integer out of range, got: "
                   << value << " for an unsigned " << proxy.numBits
                   << "bit number!");
      output = static_cast<T>(iMax);
      return ReflectorMember::ErrorType::OutOfRange;
    }

    output = static_cast<T>(value);
  }

  return errType;
}

template <typename T>
static ReflectorMember::ErrorType SetNumber(std::string_view input_,
                                            T &output) {
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
      return ReflectorMember::ErrorType::InvalidFormat;
    } catch (const std::out_of_range &) {
      printwarning("[Reflector] Float out of range, got: " << input);

      double fVal = atof(input.c_str());

      if (fVal > 0)
        output = fVal > 1.0 ? fMax : fMin;
      else
        output = fVal < -1.0 ? -fMax : -fMin;

      return ReflectorMember::ErrorType::OutOfRange;
    }

    return ReflectorMember::ErrorType::None;
  } else {
    return SetNumber(input_, output, LimitProxy<T>{});
  }
}

static ReflectorMember::ErrorType SetBoolean(std::string input, bool &output) {
  std::for_each(input.begin(), input.end(),
                [](char &c) { c = static_cast<char>(std::tolower(c)); });

  output = !memcmp(input.c_str(), "true", 4);

  if (!output && memcmp(input.c_str(), "false", 5)) {
    printwarning("[Reflector] Expected true/false, got: " << input);
    return ReflectorMember::ErrorType::InvalidFormat;
  }

  return ReflectorMember::ErrorType::None;
}

static uint64 GetEnumValue(std::string_view input, JenHash hash,
                           const ReflectedEnum **rEnumFallback = nullptr) {
  const ReflectedEnum *rEnum = rEnumFallback && *rEnumFallback
                                   ? *rEnumFallback
                                   : ReflectedEnum::Registry().at(hash);

  if (rEnumFallback) {
    *rEnumFallback = rEnum;
  }

  auto namesEnd = rEnum->names + rEnum->numMembers;
  auto foundItem =
      std::find_if(rEnum->names, namesEnd, [input](std::string_view item) {
        return !item.compare(input);
      });

  if (namesEnd == foundItem) {
    throw std::range_error("[Reflector] Enum value not found: " +
                           static_cast<std::string>(input));
  }

  return rEnum->values[std::distance(rEnum->names, foundItem)];
}

static ReflectorMember::ErrorType SetEnum(std::string_view input, char *objAddr,
                                          JenHash hash, uint16 size) {
  input = es::SkipEndWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum " << hash.raw());
    return ReflectorMember::ErrorType::EmptyInput;
  }

  uint64 eValue = 0;

  try {
    eValue = GetEnumValue(input, hash);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: "
               << hash.raw() << " for value: " << input);
    return ReflectorMember::ErrorType::InvalidDestination;
  } catch (const std::range_error &e) {
    printerror(e.what());
    return ReflectorMember::ErrorType::InvalidFormat;
  }

  memcpy(objAddr, &eValue, size);

  return ReflectorMember::ErrorType::None;
}

static ReflectorMember::ErrorType FlagFromEnum(std::string_view input,
                                               JenHash hash,
                                               uint64 &fallbackValue,
                                               const ReflectedEnum *&fallback) {
  input = es::TrimWhitespace(input);

  if (input.empty()) {
    printerror("[Reflector] Empty input for enum flag " << hash.raw());
    return ReflectorMember::ErrorType::EmptyInput;
  }

  uint64 cValue = 0;

  try {
    cValue = GetEnumValue(input, hash, &fallback);
  } catch (const std::out_of_range &) {
    printerror("[Reflector] Unregistered enum hash: "
               << hash.raw() << " for value: " << input);
    return ReflectorMember::ErrorType::InvalidDestination;
  } catch (const std::range_error &e) {
    if (input != "NULL") {
      printerror(e.what());
      return ReflectorMember::ErrorType::InvalidFormat;
    }
    return ReflectorMember::ErrorType::None;
  }

  fallbackValue |= 1ULL << cValue;

  return ReflectorMember::ErrorType::None;
}

static ReflectorMember::ErrorType
SetEnumFlags(std::string_view input, char *objAddr, JenHash hash, uint16 size) {
  auto lastIterator = input.data();
  auto inputEnd = input.data() + input.size();
  ReflectorMember::ErrorType errType = ReflectorMember::ErrorType::None;
  uint64 eValue = 0;
  const ReflectedEnum *rEnumFallback = nullptr;

  for (auto &c : input) {
    if (c == '|') {
      auto subErrType = FlagFromEnum({lastIterator, size_t(&c - lastIterator)},
                                     hash, eValue, rEnumFallback);
      if (subErrType == ReflectorMember::ErrorType::InvalidDestination) {
        return subErrType;
      } else if (subErrType != ReflectorMember::ErrorType::None) {
        errType = subErrType;
      }

      lastIterator = &c + 1;
    }
  }

  if (lastIterator < inputEnd) {
    auto subErrType =
        FlagFromEnum({lastIterator, size_t(inputEnd - lastIterator)}, hash,
                     eValue, rEnumFallback);
    if (subErrType == ReflectorMember::ErrorType::InvalidDestination) {
      return subErrType;
    } else if (subErrType != ReflectorMember::ErrorType::None) {
      errType = subErrType;
    }
  }

  memcpy(objAddr, &eValue, size);

  return errType;
}

static size_t ReflectedArraySize(char startBrace, char endBrace,
                                 std::string_view value) {
  if (value.empty()) {
    return 0;
  }

  const size_t arrBegin = value.find(startBrace);

  if (arrBegin == value.npos) {
    return -1;
  }

  const size_t arrEnd = value.find_last_of(endBrace);

  if (arrEnd == value.npos) {
    return -1;
  } else if (arrEnd < arrBegin) {
    return -1;
  }

  value = {value.data() + arrBegin + 1, arrEnd - arrBegin - 1};

  if (value.empty()) {
    return 0;
  }

  size_t curElement = 0;
  bool localScope = false;

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
      curElement++;
    }
  }

  return curElement;
}

static ReflectorMember::ErrorType
SetReflectedArray(char startBrace, char endBrace, char *objAddr,
                  std::string_view value, ReflType reflValue,
                  const reflectorStatic *refl) {
  const size_t arrBegin = value.find(startBrace);

  if (arrBegin == value.npos) {
    printerror("[Reflector] Expected " << startBrace << " not found.");
    return ReflectorMember::ErrorType::InvalidFormat;
  }

  const size_t arrEnd = value.find_last_of(endBrace);

  if (arrEnd == value.npos) {
    printerror("[Reflector] Expected " << endBrace << " not found.");
    return ReflectorMember::ErrorType::InvalidFormat;
  } else if (arrEnd < arrBegin) {
    printerror("[Reflector] " << endBrace << " was found before " << startBrace
                              << '.');
    return ReflectorMember::ErrorType::InvalidFormat;
  }

  value = {value.data() + arrBegin + 1, arrEnd - arrBegin - 1};

  if (value.empty()) {
    printerror("[Reflector] Empty array input.");
    return ReflectorMember::ErrorType::EmptyInput;
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
      std::string_view cValue(
          &*curIter, std::distance(curIter, (isEnding ? value.end() : it)));
      cValue = es::SkipStartWhitespace(cValue);

      if (cValue.empty() && curElement < arr.numItems) {
        printerror("[Reflector] Array expected "
                   << int(arr.numItems) << " but got " << curElement << '.');
        return ReflectorMember::ErrorType::ShortInput;
      }

      if (!cValue.empty() && curElement >= arr.numItems) {
        printerror("[Reflector] Too many array elements, " << int(arr.numItems)
                                                           << " expected.");
        return ReflectorMember::ErrorType::OutOfRange;
      }

      SetReflectedMember(arr, cValue, objAddr + (arr.stride * curElement),
                         refl);
      curIter = it + 1;
      curElement++;
    }
  }

  return ReflectorMember::ErrorType::None;
}

static ReflectorMember::ErrorType
SetReflectedMember(ReflType reflValue, std::string_view value, char *objAddr,
                   const reflectorStatic *refl) {
  ReflectorMember::ErrorType errType = ReflectorMember::ErrorType::None;
  char startBrace = 0;
  char endBrace = 0;
  value = es::SkipStartWhitespace(value);

  if (!(reflValue.container == REFContainer::None ||
        reflValue.container == REFContainer::Pointer)) {
    startBrace = '{';
    endBrace = '}';
  } else if (reflValue.type == REFType::Vector) {
    startBrace = '[';
    endBrace = ']';
  }

  if (reflValue.container == REFContainer::InlineArray) {
    return SetReflectedArray(startBrace, endBrace, objAddr, value, reflValue,
                             refl);
  } else if (reflValue.container == REFContainer::ContainerVector ||
             reflValue.container == REFContainer::ContainerVectorMap) {
    const VectorMethods methods = refl->vectorMethods[reflValue.index];
    const size_t numItems = ReflectedArraySize(startBrace, endBrace, value);
    methods.signal(objAddr, VectorMethods::Resize, numItems, 0);
    if (numItems > 0) {
      char *vData = static_cast<char *>(methods.at(objAddr, 0));
      reflValue.asArray.type = reflValue.type;
      reflValue.asArray.numItems = numItems;
      reflValue.asArray.stride = reflValue.size;
      return SetReflectedArray(startBrace, endBrace, vData, value, reflValue,
                               refl);
    }
    return ReflectorMember::ErrorType::EmptyInput;
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
      return ReflectorMember::ErrorType::InvalidDestination;
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
      return ReflectorMember::ErrorType::InvalidDestination;
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
        err = ReflectorMember::ErrorType::SignMismatch;
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
        return ReflectorMember::ErrorType::InvalidDestination;
      }
    }
  }
  case REFType::BitFieldMember: {
    uint64 &output = *reinterpret_cast<uint64 *>(objAddr);
    auto doStuff = [&](auto &&insertVal) {
      LimitProxy<std::decay_t<decltype(insertVal)>> proxy{reflValue.bit.size};
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
          err = ReflectorMember::ErrorType::SignMismatch;
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

      return ReflectorMember::ErrorType::InvalidDestination;
    }
    case REFType::Bool: {
      bool result;
      auto err = SetBoolean(std::string(value), result);
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
      return ReflectorMember::ErrorType::InvalidDestination;
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
  case REFType::Vector:
    return SetReflectedArray(startBrace, endBrace, objAddr, value, reflValue,
                             refl);
  default:
    break;
  }

  return errType;
}

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

ReflectorMember::ErrorType
ReflectorMember::ReflectValue(ReflectorInputValue value, size_t index) {
  if (!data) {
    return ErrorType::InvalidDestination;
  }
  char *thisAddr = static_cast<char *>(data.instance);
  ReflType type = data.rfStatic->types[id];
  thisAddr =
      thisAddr + (type.type == REFType::BitFieldMember ? 0 : type.offset);

  if (index != size_t(-1)) {
    if (type.container == REFContainer::InlineArray) {
      ReflTypeArray arrType = type.asArray;
      thisAddr += index * arrType.stride;
      type = arrType;

      if (index >= arrType.numItems) {
        return ReflectorMember::ErrorType::OutOfRange;
      }
    } else if (type.type == REFType::Vector) {
      ReflTypeVector vecType = type.asVector;
      thisAddr += index * vecType.stride;
      type = vecType;

      if (index >= vecType.numItems) {
        return ReflectorMember::ErrorType::OutOfRange;
      }
    } else if (type.container == REFContainer::ContainerVector ||
               type.container == REFContainer::ContainerVectorMap) {
      const VectorMethods methods = data.rfStatic->vectorMethods[type.index];
      type.asArray.type = type.type;
      type.asArray.numItems = methods.size(thisAddr);
      type.asArray.stride = type.size;
      if (type.asArray.numItems <= index) {
        methods.signal(thisAddr, VectorMethods::Resize, index + 1, 0);
        type.asArray.numItems = methods.size(thisAddr);
      }

      thisAddr = static_cast<char *>(methods.at(thisAddr, index));
      type = type.asArray;
    }
  }

  switch (type.type) {
  case REFType::BitFieldClass:
  case REFType::Class:
    throw std::runtime_error("Cannot set reflected value (value is subclass)");

  default:
    break;
  }

  auto SetPodType = [type, thisAddr](auto item) {
    switch (type.type) {
    case REFType::Bool: {
      uint64 cvtItem = bool(item);
      memcpy(thisAddr, &cvtItem, type.size);
      break;
    }
    case REFType::FloatingPoint: {
      if (type.size == 4) {
        float cvtItem = item;
        memcpy(thisAddr, &cvtItem, type.size);
      } else if (type.size == 8) {
        double cvtItem = item;
        memcpy(thisAddr, &cvtItem, type.size);
      } else {
        throw std::logic_error("Invlid floating point size");
      }

      break;
    }
    case REFType::Integer: {
      int64 cvtItem = item;
      memcpy(thisAddr, &cvtItem, type.size);
      break;
    }
    case REFType::UnsignedInteger: {
      uint64 cvtItem = item;
      memcpy(thisAddr, &cvtItem, type.size);
      break;
    }

    default:
      return ErrorType::InvalidDestination;
    }

    return ErrorType::None;
  };

  return std::visit(
      overloaded{
          [&](std::string_view item) {
            return SetReflectedMember(type, item, thisAddr, data.rfStatic);
          },
          SetPodType,
      },
      value);
}

static std::string_view
PrintEnumValue(JenHash hash, uint64 value,
               const ReflectedEnum **rEnumFallback = nullptr) {
  const ReflectedEnum *rEnum = rEnumFallback && *rEnumFallback
                                   ? *rEnumFallback
                                   : ReflectedEnum::Registry().at(hash);

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

static std::string GetReflectedPrimitive(const char *objAddr, ReflType type,
                                         const reflectorStatic *refl) {
  char startBrace = 0;
  char endBrace = 0;

  if (!(type.container == REFContainer::None ||
        type.container == REFContainer::Pointer)) {
    startBrace = '{';
    endBrace = '}';
  } else if (type.type == REFType::Vector) {
    startBrace = '[';
    endBrace = ']';
  }

  if (startBrace && endBrace) {
    ReflTypeArray arr = type.asArray;

    if (type.container == REFContainer::ContainerVector ||
        type.container == REFContainer::ContainerVectorMap) {
      const VectorMethods methods = refl->vectorMethods[type.index];
      arr.type = type.type;
      arr.numItems = methods.size(objAddr);
      arr.stride = type.size;

      if (arr.numItems > 0) {
        objAddr = static_cast<const char *>(
            methods.at(const_cast<char *>(objAddr), 0));
      } else {
        char data[]{startBrace, endBrace, 0};
        return data;
      }
    }

    std::string outVal;
    outVal.push_back(startBrace);

    for (int i = 0; i < arr.numItems; i++) {
      outVal += GetReflectedPrimitive(objAddr + (arr.stride * i), arr, refl);
      outVal += ", ";
    }

    outVal.pop_back();
    outVal.pop_back();
    outVal.push_back(endBrace);

    return outVal;
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
      return std::string(
          PrintEnumValue(JenHash(type.asBitfield.typeHash), output));
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

  return "";
}

std::string ReflectorMember::ReflectedValue(size_t index) const {
  const char *thisAddr = static_cast<const char *>(data.constInstance);
  ReflType type = data.rfStatic->types[id];
  const int valueOffset =
      type.type == REFType::BitFieldMember ? 0 : type.offset;

  const char *objAddr = thisAddr + valueOffset;

  if (index != size_t(-1)) {
    if (type.container == REFContainer::InlineArray) {
      const auto &arr = type.asArray;
      if (arr.numItems <= index) {
        return "";
      }

      objAddr += arr.stride * index;
      type = arr;
    } else if (type.type == REFType::Vector) {
      ReflTypeVector vecType = type.asVector;

      if (index >= vecType.numItems) {
        return "";
      }

      objAddr += vecType.stride * index;
      type = vecType;
    } else if (type.type == REFType::EnumFlags) {
      if (type.size * 8 <= index) {
        return "";
      }

      uint64 eValue;

      memcpy(reinterpret_cast<char *>(&eValue), objAddr, type.size);

      return (eValue & (1ULL << index)) ? "true" : "false";
    } else if (type.container == REFContainer::ContainerVector ||
               type.container == REFContainer::ContainerVectorMap) {
      const VectorMethods methods = data.rfStatic->vectorMethods[type.index];
      type.asArray.type = type.type;
      type.asArray.numItems = methods.size(objAddr);
      type.asArray.stride = type.size;
      if (type.asArray.numItems <= index) {
        return "";
      }

      objAddr = static_cast<const char *>(
          methods.at(const_cast<char *>(objAddr), index));
      type = type.asArray;
    }
  }

  return GetReflectedPrimitive(objAddr, type, data.rfStatic);
}

const reflectorStatic *GetReflectedFlags(uint32 hash) {
  struct FlagProxy {
    std::vector<ReflType> types;
    reflectorStatic main;

    FlagProxy(const ReflectedEnum *refEnum)
        : types(refEnum->numMembers), main(refEnum->numMembers, types.data(),
                                           refEnum->names, refEnum->size) {
      for (size_t i = 0; i < refEnum->numMembers; i++) {
        ReflType &curType = types.at(i);
        curType.type = REFType::BitFieldMember;
        curType.index = i;
        curType.bit.position = refEnum->values[i];
        curType.bit.size = 1;
        curType.valueNameHash = JenHash(refEnum->names[i]);
        curType.size = refEnum->size;
        curType.asBitfield.type = REFType::Bool;
      }
    }
  };
  static std::map<uint32, std::unique_ptr<FlagProxy>> REGISTRY;

  auto foundStored = REGISTRY.find(hash);

  if (foundStored != REGISTRY.end()) {
    return &foundStored->second->main;
  }

  auto found = ReflectedEnum::Registry().find(JenHash(hash));

  if (found == ReflectedEnum::Registry().end()) {
    return nullptr;
  }

  const ReflectedEnum *refEnum = found->second;

  return &REGISTRY.emplace(hash, std::make_unique<FlagProxy>(refEnum))
              .first->second->main;
}

ReflectorPureWrap ReflectorMember::ReflectedSubClass(size_t index) const {
  if (!data) {
    return {{}};
  }
  const char *thisAddr = static_cast<const char *>(data.constInstance);
  ReflType type = data.rfStatic->types[id];
  const char *objAddr = thisAddr + type.offset;
  ReflTypeClass classType = type.asClass;
  ReflTypeVector vectorType = type.asVector;

  if (type.container == REFContainer::InlineArray) {
    const auto &arr = type.asArray;
    if (index >= type.asArray.numItems) {
      return {{}};
    }

    objAddr += index * arr.stride;
    classType = arr.asClass;
    vectorType = arr.asVector;
    type = arr;
  } else if (type.container == REFContainer::ContainerVector ||
             type.container == REFContainer::ContainerVectorMap) {
    const VectorMethods methods = data.rfStatic->vectorMethods[type.index];
    if (methods.size(objAddr) <= index) {
      methods.signal(const_cast<char *>(objAddr), VectorMethods::Resize,
                     index + 1, 0);
    }

    objAddr =
        static_cast<char *>(methods.at(const_cast<char *>(objAddr), index));
    classType = type.asClass;
    vectorType = type.asVector;
  }

  switch (type.type) {
  case REFType::Class:
  case REFType::BitFieldClass: {
    auto found = reflectorStatic::Registry().find(JenHash(classType.typeHash));

    if (found == reflectorStatic::Registry().end()) {
      return {{}};
    }

    return {{found->second, objAddr}};
  }

  case REFType::Vector: {
    VectorKey key;
    key.type = vectorType.type;
    key.numElements = vectorType.numItems;
    key.size = vectorType.stride;
    return {{&VECTORS.at(key), objAddr}};
  }

  case REFType::EnumFlags: {
    const reflectorStatic *asRef = GetReflectedFlags(type.asClass.typeHash);
    return {{asRef, objAddr}};
  }

  default:
    break;
  }

  return {{}};
}

reflectorStatic::RegistryType &reflectorStatic::Registry() {
  static RegistryType registry;
  return registry;
}

ReflectedEnum::RegistryType &ReflectedEnum::Registry() {
  static RegistryType registry;
  return registry;
}
