/*      a source for Reflector class
        more info in README for PreCore Project

        Copyright 2018-2019 Lukas Cone

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
#include "reflectorRegistry.hpp"
#include <algorithm>

const reflType *Reflector::GetReflectedType(const JenHash hash) const {
  const reflectorStatic *inst = _rfRetreive().rfStatic;
  const int _ntypes = GetNumReflectedValues();

  for (int t = 0; t < _ntypes; t++)
    if (inst->types[t].valueNameHash == hash)
      return inst->types + t;

  return nullptr;
}

static ES_INLINE void SetReflectedPrimitive(char *objAddr, uchar type,
                                            const char *value, JenHash hash = 0,
                                            ushort elSize = 0) {
  switch (type) {

  case 1:
    *reinterpret_cast<bool *>(objAddr) = !strcmp(value, "true");
    break;

  case 2:
    *(objAddr) = static_cast<char>(std::atoi(value));
    break;
  case 3:
    *(reinterpret_cast<unsigned char *>(objAddr)) =
        static_cast<unsigned char>(std::atoi(value));
    break;

  case 4:
    *reinterpret_cast<short *>(objAddr) = static_cast<short>(std::atoi(value));
    break;
  case 5:
    *reinterpret_cast<unsigned short *>(objAddr) =
        static_cast<unsigned short>(std::atoi(value));
    break;

  case 6:
    *reinterpret_cast<int *>(objAddr) = std::atoi(value);
    break;
  case 7:
    *reinterpret_cast<unsigned int *>(objAddr) =
        static_cast<unsigned int>(std::atoll(value));
    break;

  case 8:
    *reinterpret_cast<int64 *>(objAddr) = std::atoll(value);
    break;
  case 9:
    *reinterpret_cast<uint64 *>(objAddr) = std::strtoull(value, nullptr, 10);
    break;

  case 10:
    *reinterpret_cast<float *>(objAddr) = static_cast<float>(std::atof(value));
    break;
  case 11:
    *reinterpret_cast<double *>(objAddr) = std::atof(value);
    break;

  case 12: {
    uint64 ival = 0;

    if (REFEnumStorage.count(hash)) {
      ival = REFEnumStorage[hash].MultiConstructor(value);
    }

    memcpy(objAddr, reinterpret_cast<const char *>(&ival), elSize);

    break;
  }

  case 13: {
    uint64 ival = 0;

    if (REFEnumStorage.count(hash)) {
      ival = REFEnumStorage[hash].Constructor(value);
    }

    memcpy(objAddr, reinterpret_cast<const char *>(&ival), elSize);

    break;
  }

  case 19:
    *reinterpret_cast<std::string *>(objAddr) = value;
    break;

  default:
    break;
  }
}

static void SetReflectedMember(const reflType *reflValue, const char *value,
                               char *thisAddr) {
  if (!reflValue)
    return;

  switch (reflValue->type) {
  case 15: // vector2
  case 16: // vector
  case 17: // vector4
  {
    std::string valueCopy = value;
    size_t firstBrace = valueCopy.find_first_of('[');

    if (firstBrace == valueCopy.npos)
      firstBrace = 0;
    else
      firstBrace++;

    valueCopy = valueCopy.substr(firstBrace, valueCopy.find(']', firstBrace));

    std::replace(valueCopy.begin(), valueCopy.end(), '[', ' ');
    std::replace(valueCopy.begin(), valueCopy.end(), ']', ' ');

    size_t currentItem = 0;
    size_t lastItem = 0;

    for (size_t i = 0; i < valueCopy.size() + 1; i++) {
      if (valueCopy[i] == ',' || valueCopy[i] == '\0') {
        SetReflectedPrimitive(thisAddr + reflValue->offset +
                                  (reflValue->subSize * currentItem),
                              reflValue->subType, valueCopy.data() + lastItem,
                              reflValue->typeHash, reflValue->subSize);
        lastItem = i + 1;
        currentItem++;
      }
    }

    break;
  }

  case 20: {
    std::string valueCopy = value;
    size_t firstBrace = valueCopy.find_first_of('{');

    if (firstBrace == valueCopy.npos)
      firstBrace = 0;
    else
      firstBrace++;

    valueCopy = valueCopy.substr(firstBrace, valueCopy.find('}', firstBrace));

    std::replace(valueCopy.begin(), valueCopy.end(), '{', ' ');
    std::replace(valueCopy.begin(), valueCopy.end(), '}', ' ');

    size_t currentItem = 0;
    size_t lastItem = 0;

    for (size_t i = 0; i < valueCopy.size() + 1; i++) {
      if (valueCopy[i] == ',' || valueCopy[i] == '\0') {
        SetReflectedPrimitive(thisAddr + reflValue->offset +
                                  (reflValue->subSize * currentItem),
                              reflValue->subType, valueCopy.data() + lastItem,
                              reflValue->typeHash, reflValue->subSize);
        lastItem = i + 1;
        currentItem++;
      }
    }

    break;
  }
  default:
    SetReflectedPrimitive(thisAddr + reflValue->offset, reflValue->type, value,
                          reflValue->typeHash, reflValue->subSize);
    break;
  }
}

void Reflector::SetReflectedValue(int ID, const char *value) {
  const reflType *reflValue = GetReflectedType(ID);
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);

  SetReflectedMember(reflValue, value, thisAddr);
}

void Reflector::SetReflectedValue(const JenHash hash, const char *value) {
  const reflType *reflValue = GetReflectedType(hash);
  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);

  SetReflectedMember(reflValue, value, thisAddr);
}

static ES_INLINE std::string GetReflectedPrimitive(const char *objAddr,
                                                   uchar type, JenHash hash,
                                                   ushort elSize) {
  switch (type) {

  case 1:
    return *reinterpret_cast<const bool *>(objAddr) ? "true" : "false";

  case 2:
    return std::to_string(
        static_cast<int>(*reinterpret_cast<const char *>(objAddr)));
  case 3:
    return std::to_string(
        static_cast<int>(*reinterpret_cast<const unsigned char *>(objAddr)));

  case 4:
    return std::to_string(*reinterpret_cast<const short *>(objAddr));
  case 5:
    return std::to_string(*reinterpret_cast<const unsigned short *>(objAddr));

  case 6:
    return std::to_string(*reinterpret_cast<const int *>(objAddr));
  case 7:
    return std::to_string(*reinterpret_cast<const unsigned int *>(objAddr));

  case 8:
    return std::to_string(*reinterpret_cast<const int64 *>(objAddr));
  case 9:
    return std::to_string(*reinterpret_cast<const uint64 *>(objAddr));

  case 10:
    return std::to_string(*reinterpret_cast<const float *>(objAddr));
  case 11:
    return std::to_string(*reinterpret_cast<const double *>(objAddr));

  case 12: {
    if (!REFEnumStorage.count(hash))
      return "";

    std::string oval = {};
    uint64 ival = 0;

    memcpy(reinterpret_cast<char *>(&ival), objAddr, elSize);
    REFEnumStorage[hash].MultiDestructor(oval, ival);

    return oval;
  }

  case 13: {
    if (!REFEnumStorage.count(hash))
      return "";

    std::string oval = {};
    uint64 ival = 0;

    memcpy(reinterpret_cast<char *>(&ival), objAddr, elSize);
    REFEnumStorage[hash].Destructor(oval, ival);

    return oval;
  }

  case 18:
		return objAddr;

	case 19:
		return *reinterpret_cast<const std::string *>(objAddr);

  case 14:
		return "SUBCLASS_TYPE";

  default:
    return "";
  }
}

std::string Reflector::GetReflectedValue(int id) const {
  if (id >= GetNumReflectedValues())
    return "";

  const reflectorInstanceConst inst = _rfRetreive();
  const char *thisAddr = static_cast<const char *>(inst.rfInstance);
  const reflType &reflValue = inst.rfStatic->types[id];

  const int valueOffset = reflValue.offset;

  switch (reflValue.type) {
  case 15: // vector2
  case 16: // vector
  case 17: // vector4
  {
    const int numItems = reflValue.numItems;
    std::string outVal = "[";

    for (int i = 0; i < numItems; i++) {
      outVal += GetReflectedPrimitive(
                    thisAddr + valueOffset +
                        (static_cast<intptr_t>(reflValue.subSize) * i),
                    reflValue.subType, reflValue.typeHash, reflValue.subSize) +
                ", ";
    }

    outVal.pop_back();
    outVal.pop_back();

    return outVal + "]";
  }

  case 20: {
    const int numItems = reflValue.numItems;
    std::string outVal = "{";

    for (int i = 0; i < numItems; i++) {
      outVal += GetReflectedPrimitive(
                    thisAddr + valueOffset +
                        (static_cast<intptr_t>(reflValue.subSize) * i),
                    reflValue.subType, reflValue.typeHash, reflValue.subSize) +
                ", ";
    }

    outVal.pop_back();
    outVal.pop_back();

    return outVal + "}";
  }

  default:
    return GetReflectedPrimitive(thisAddr + valueOffset, reflValue.type,
                                 reflValue.typeHash, reflValue.subSize);
  }
}

const Reflector::SubClass Reflector::GetReflectedSubClass(int id) const {
  if (id >= GetNumReflectedValues())
    return {};

  const reflectorInstanceConst inst = _rfRetreive();
  const char *thisAddr = static_cast<const char *>(inst.rfInstance);
  const reflType &reflValue = inst.rfStatic->types[id];

  const int valueOffset = reflValue.offset;

  if (reflValue.typeHash != 14 || !REFSubClassStorage.count(reflValue.typeHash))
    return {};

  return {{},
          {REFSubClassStorage.at(reflValue.typeHash), thisAddr + valueOffset}};
}

const Reflector::SubClass Reflector::GetReflectedSubClass(int id) {
  if (id >= GetNumReflectedValues())
    return {};

  const reflectorInstance inst = _rfRetreive();
  char *thisAddr = static_cast<char *>(inst.rfInstance);
  const reflType &reflValue = inst.rfStatic->types[id];

  const int valueOffset = reflValue.offset;

  if (reflValue.typeHash != 14 || !REFSubClassStorage.count(reflValue.typeHash))
    return {};

  return {{REFSubClassStorage.at(reflValue.typeHash), thisAddr + valueOffset},
          {REFSubClassStorage.at(reflValue.typeHash), thisAddr + valueOffset}};
}

RefEnumMapper REFEnumStorage;
RefSubClassMapper REFSubClassStorage;