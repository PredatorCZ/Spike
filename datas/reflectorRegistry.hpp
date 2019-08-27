/*      Registering enum reflection IO
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

#pragma once
#include <unordered_map>

struct RefEnumFunctions {
  uint64 (*MultiConstructor)(const char *);
  void (*MultiDestructor)(std::string &, uint64 value);
  uint64 (*Constructor)(const char *);
  void (*Destructor)(std::string &, uint64 value);

  template <class E> RefEnumFunctions &C() {
    E enumInstance = {};
    uint64 lastValue = 0;

    for (int t = 0; t < enumInstance._reflectedSize; t++) {
      const char *cName = enumInstance._reflected[t];

      while (*cName != '=' && *cName != 0)
        cName++;

      if (!*cName) {
        enumInstance._reflectedValues[t] = lastValue++;
        continue;
      }
      
      cName++;

      while (*cName == ' ')
        cName++;

      uint64 resVal = 0;

      if (reinterpret_cast<const short &>(*cName) == 0x7830) {
        cName += 2;

        size_t cLen = strlen(cName) - 1;

        for (int t = cLen; t > -1; t--)
          resVal |= atohLUT[cName[t]] << 4 * (cLen - t);

        lastValue = resVal;
      } else {
        resVal = std::atoll(cName);

        lastValue = resVal;
      }

      enumInstance._reflectedValues[t] = resVal;
    }

    return *this;
  }
};

typedef std::unordered_map<unsigned int, RefEnumFunctions> RefEnumMapper;
extern RefEnumMapper REFEnumStorage;

template <class E> uint64 EnumConstructor_t(const char *value) {
  E enumInstance = {};
  uint64 resval = 0;

  while (*value == ' ')
    value++;

  for (int t = 0; t < enumInstance._reflectedSize; t++)
    if (!memcmp(value, enumInstance._reflected[t],
                enumInstance._reflectedSizes[t])) {
      resval = enumInstance._reflectedValues[t];
      break;
    }

  return resval;
}

template <class E> uint64 EnumMultiConstructor_t(const char *value) {
  E enumInstance = {};
  uint64 resval = 0;
  const char *c = value;
  std::string currentValue = {};
  currentValue.reserve(64);

  while (true) {
    if (*c == ' ' || *c == '|' || !*c || *c == ',') {
      if (currentValue.size()) {
        for (int t = 0; t < enumInstance._reflectedSize; t++)
          if (!memcmp(enumInstance._reflected[t], currentValue.data(),
                      enumInstance._reflectedSizes[t])) {
            resval |= 1 << enumInstance._reflectedValues[t];
            break;
          }
        currentValue.clear();
      }

      if (!*c || *c == ',') {
        break;
      } else {
        c++;
        continue;
      }
    } else
      currentValue.push_back(*c);

    c++;
  }

  return resval;
}

template <class E>
void EnumMultiDestructor_t(std::string &output, uint64 value) {
  E enumInstance = {};

  for (int i = 0; i < enumInstance._reflectedSize; i++)
    if (value & (1 << enumInstance._reflectedValues[i])) {
      if (output.size())
        output.append(" | ");

      output.append(enumInstance._reflected[i], enumInstance._reflectedSizes[i]);
    }

  if (!output.size())
    output = "NULL";
}

template <class E> void EnumDestructor_t(std::string &output, uint64 value) {
  E enumInstance = {};

  for (int t = 0; t < enumInstance._reflectedSize; t++)
    if (enumInstance._reflectedValues[t] == value)
      output = std::string(enumInstance._reflected[t], enumInstance._reflectedSizes[t]);
}

#define REGISTER_ENUM(classname)                                               \
  REFEnumStorage[static_cast<const JenHash>(_EnumWrap<classname>::HASH)] =     \
      RefEnumFunctions{EnumMultiConstructor_t<_EnumWrap<classname>>,           \
                       EnumMultiDestructor_t<_EnumWrap<classname>>,            \
                       EnumConstructor_t<_EnumWrap<classname>>,                \
                       EnumDestructor_t<_EnumWrap<classname>>}                 \
          .C<_EnumWrap<classname>>();

struct reflectorStatic;

typedef std::unordered_map<unsigned int, const reflectorStatic *>
    RefSubClassMapper;
extern RefSubClassMapper REFSubClassStorage;

#define REGISTER_SUBCLASS(classname)                                           \
  REFSubClassStorage[_SubReflClassWrap<classname>::HASH] =                     \
      classname::__rfPtrStatic;
