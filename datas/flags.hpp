/*  esFlags class to store bit flags

    Copyright 2015-2020 Lukas Cone

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

#ifndef ES_FLAGS_DEFINED
#define ES_FLAGS_DEFINED
#include "supercore.hpp"

template <class T, class E = int> class esFlags {
public:
  typedef E EnumClass;
  typedef T ValueType;

private:
  T value;

  template <typename _Type> const T _eval(const T val, _Type input) {
    return val | (1 << static_cast<T>(input));
  }

  template <typename _Type, typename... _Others>
  const T _eval(const T val, _Type input, _Others... inputs) {
    return _eval(val | (1 << static_cast<T>(input)), inputs...);
  }

public:
  esFlags() noexcept : value() {}
  template <typename... _Type> esFlags(const _Type... inputs) {
    value = _eval(0, inputs...);
  }

  void operator=(T inval) noexcept { value = inval; }

  bool operator[](E pos) const {
    return (value & (1 << static_cast<T>(pos))) != 0;
  }

  void Set(E pos, bool val) {
    val ? value |= (1 << static_cast<T>(pos))
        : value &= ~(1 << static_cast<T>(pos));
  }
  esFlags &operator+=(E input) { Set(input, true); return *this; }
  esFlags &operator-=(E input) { Set(input, false); return *this; }

  bool operator==(const esFlags &input) const noexcept {
    return value == input.value;
  }

  bool operator!=(const esFlags &input) const noexcept {
    return value != input.value;
  }

  bool operator==(E input) { return operator[](input); }
  bool operator!=(E input) { return !operator[](input); }
};

#endif // ES_FLAGS_DEFINED

#ifdef ES_REFLECTOR_DEFINED
#ifndef ES_FLAGS_TEMPLATES_DEFINED
#define ES_FLAGS_TEMPLATES_DEFINED
template <class C, class E> struct _getType<esFlags<C, E>> {
  static const REFType TYPE = REFType::EnumFlags;
  static const JenHash HASH = _EnumWrap<E>::GetHash();
  static const uint8 SUBSIZE = sizeof(C);
  static const uint16 NUMITEMS = 1;
  static const REFType SUBTYPE = REFType::None;
};
#endif
#endif // ES_REFLECTOR_DEFINED
