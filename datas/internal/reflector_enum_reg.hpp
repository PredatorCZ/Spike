/*  Registering enum reflectors
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

#pragma once
#include "../string_view.hpp"
#include "../supercore.hpp"
#include <cstring>
#include <unordered_map>
#include <vector>

class ReflectedEnum : protected std::vector<es::string_view> {
protected:
  typedef std::vector<es::string_view> parent;

public:
  uint64 *values;
  const char *name;
  JenHash hash;

  ReflectedEnum() : values(nullptr), name(nullptr) {}
  ReflectedEnum(int32 _numValues, const es::string_view *_names,
                uint64 *_values, const char *_name, JenHash _hash)
      : parent(_names, _names + _numValues), values(_values), name(_name),
        hash(_hash) {
  }

  using parent::size;
  using parent::operator[];
  using parent::at;
  using parent::begin;
  using parent::const_iterator;
  using parent::end;
  using parent::iterator;
};

template <class E>
static ReflectedEnum GetReflectedEnum() {
  _EnumWrap<E> enumInstance = {};

  return {
      enumInstance._reflectedSize,
      enumInstance._reflected,
      enumInstance._reflectedValues,
      enumInstance._name,
      enumInstance.HASH,
  };
}

static inline ReflectedEnum _REnumInitializer(ReflectedEnum rEnum) {
  uint64 lastValue = 0;

  for (size_t t = 0; t < rEnum.size(); t++) {
    es::string_view cName = rEnum[t].end();

    while (cName[0] != '=' && !cName.empty())
      cName.remove_prefix(1);

    if (cName.empty() || (cName[0] == '=' && cName.size() < 2)) {
      rEnum.values[t] = lastValue++;
      continue;
    }

    cName.remove_prefix(1);
    cName = es::SkipStartWhitespace(cName);

    const int _base = !cName.compare(0, 2, "0x") ? 16 : 10;

    uint64 resVal = std::strtoull(cName.data(), nullptr, _base);
    lastValue = resVal + 1;
    rEnum.values[t] = resVal;
  }

  return rEnum;
}

typedef std::unordered_map<unsigned int, ReflectedEnum> RefEnumMapper;
extern RefEnumMapper REFEnumStorage;

#define _REFLECTOR_REGISTER_ENUM(classname)                                    \
  REFEnumStorage[static_cast<const JenHash>(_EnumWrap<classname>::HASH)] =     \
      _REnumInitializer(GetReflectedEnum<classname>());

#define _REFLECTOR_REGISTER_ENUM_EXTERN(classname)                             \
  uint64 _EnumWrap<                                                            \
      classname>::_reflectedValues[_EnumWrap<classname>::_reflectedSize] = {};

#define REGISTER_ENUMS(...)                                                    \
  static bool _localEnumInit = false;                                          \
  static void RegisterLocalEnums() {                                           \
    if (_localEnumInit)                                                        \
      return;                                                                  \
    StaticFor(_REFLECTOR_REGISTER_ENUM, __VA_ARGS__);                          \
    _localEnumInit = true;                                                     \
  };                                                                           \
  StaticFor(_REFLECTOR_REGISTER_ENUM_EXTERN, __VA_ARGS__)
