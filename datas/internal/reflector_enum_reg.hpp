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
#include "../reflector_fwd.hpp"
#include "../string_view.hpp"
#include "../supercore.hpp"
#include <cstring>
#include <vector>

template <class E> static void _REnumInitializer() {
  const size_t rEnumSize = _EnumWrap<E>::NUM_ITEMS;
  const auto rEnumNames = _EnumWrap<E>::GetNames();
  auto rEnumValues = _EnumWrap<E>::GetValues();

  uint64 lastValue = 0;

  for (size_t t = 0; t < rEnumSize; t++) {
    es::string_view cName = rEnumNames[t].end();

    while (cName[0] != '=' && !cName.empty())
      cName.remove_prefix(1);

    if (cName.empty() || (cName[0] == '=' && cName.size() < 2)) {
      rEnumValues[t] = lastValue++;
      continue;
    }

    cName.remove_prefix(1);
    cName = es::SkipStartWhitespace(cName);

    const int _base = !cName.compare(0, 2, "0x") ? 16 : 10;

    uint64 resVal = std::strtoull(cName.data(), nullptr, _base);
    lastValue = resVal + 1;
    rEnumValues[t] = resVal;
  }
}

template <class C> struct _ETag {};

class ReflectedEnum : protected std::vector<es::string_view> {
protected:
  typedef std::vector<es::string_view> parent;

public:
  const uint64 *values;
  es::string_view name;
  JenHash hash;

  ReflectedEnum() = default;

  template <class E>
  ReflectedEnum(_ETag<E>)
      : parent(_EnumWrap<E>::GetNames(),
               _EnumWrap<E>::GetNames() + _EnumWrap<E>::NUM_ITEMS),
        values(_EnumWrap<E>::GetValues()), name(_EnumWrap<E>::GetClassName()),
        hash(_EnumWrap<E>::GetHash()) {
    if (!_EnumWrap<E>::Initialized(false)) {
      _REnumInitializer<E>();
      _EnumWrap<E>::Initialized(true);
    }
  }

  using parent::size;
  using parent::operator[];
  using parent::at;
  using parent::begin;
  using parent::const_iterator;
  using parent::end;
  using parent::iterator;
};

template <class E> static ReflectedEnum GetReflectedEnum() {
  return {_ETag<E>{}};
}
