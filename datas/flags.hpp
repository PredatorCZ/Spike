/*  esFlags class to store bit flags

    Copyright 2015-2021 Lukas Cone

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
#include "reflector_fwd.hpp"
#include "supercore.hpp"

namespace es {
template <class enum_type, class store_override_type =
                               typename TypeFromSize<sizeof(enum_type)>::type>
class Flags {
public:
  using EnumClass = enum_type;
  using ValueType = store_override_type;

  constexpr Flags() noexcept : value() {}

  template <typename... args>
  constexpr Flags(args... inputs) noexcept : value((... | MakeMask_(inputs))) {}

  template <typename... args>
  constexpr Flags(const Flags &input, args... inputs) noexcept
      : value(input | (... | MakeMask_(inputs))) {}

  void operator=(ValueType inval) noexcept { value = inval; }

  constexpr bool operator[](EnumClass pos) const noexcept {
    return (value & MakeMask_(pos)) != 0;
  }

  void Set(EnumClass pos, bool val) noexcept {
    val ? value |= MakeMask_(pos) : value &= ~MakeMask_(pos);
  }
  Flags &operator+=(EnumClass input) noexcept {
    Set(input, true);
    return *this;
  }
  Flags &operator-=(EnumClass input) noexcept {
    Set(input, false);
    return *this;
  }

  constexpr bool operator==(const Flags &input) const noexcept {
    return value == input.value;
  }

  constexpr bool operator!=(const Flags &input) const noexcept {
    return value != input.value;
  }

  constexpr bool operator==(EnumClass input) const noexcept {
    return operator[](input);
  }
  constexpr bool operator!=(EnumClass input) const noexcept {
    return !operator[](input);
  }

  constexpr explicit operator ValueType() const noexcept { return value; }

private:
  static constexpr ValueType MakeMask_(EnumClass at) {
    return static_cast<ValueType>(1) << static_cast<ValueType>(at);
  }

  ValueType value;
};
} // namespace es

template <class C, class E>
struct _getType<es::Flags<E, C>> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::EnumFlags;
  static constexpr JenHash Hash() { return _EnumWrap<E>::GetHash(); }
  static constexpr uint8 SUBSIZE = sizeof(C);
  static constexpr uint16 NUMITEMS = 1;
};
