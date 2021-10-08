/*  BitField class

    Copyright 2020-2021 Lukas Cone

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

template <size_t index_, size_t size_,
          class type = std::conditional_t<size_ == 1, bool, void>>
struct BitMemberDecl {
  static constexpr size_t index = index_;
  static constexpr size_t size = size_;
  using value_type = type;
};

struct BitMember {
  size_t position;
  size_t size;

  template <class type> constexpr type GetMask() const {
    static_assert(std::is_unsigned<type>::value, "Type must be unsigned!");
    const type init = type(-1);
    constexpr size_t typeSize = sizeof(type) * 8;
    return (init >> (typeSize - size)) << position;
  }

  template <class type> constexpr type GetMirrorMask() const {
    static_assert(std::is_unsigned<type>::value, "Type must be unsigned!");
    const type init = type(-1);
    constexpr size_t typeSize = sizeof(type) * 8;
    return (init >> (typeSize - size)) << (typeSize - position - size);
  }
};

template <class head, class... tail> struct BitField : BitField<tail...> {
  static constexpr size_t numItems = sizeof...(tail) + 1;
  using type = head;
  using base = BitField<tail...>;
  static constexpr size_t totalSize = type::size + base::totalSize;

  static constexpr BitMember Get(size_t index) {
    if (!index) {
      return {0, head::size};
    }

    return base::Get_(index - 1, totalSize);
  }

  template <class memdecl> static constexpr BitMember Get() {
    return Get(memdecl::index);
  }

protected:
  static constexpr BitMember Get_(size_t index, size_t size) {
    if (!index) {
      return {size - totalSize, head::size};
    }

    return base::Get_(index - 1, size);
  }
};

template <class head> struct BitField<head> {
  using type = head;
  static constexpr size_t totalSize = type::size;

  static constexpr BitMember Get_(size_t, size_t size) {
    return {size - totalSize, head::size};
  }
};

template <class type, class fn>
void SwapBitField(size_t numItems, bool outWay, type &input, fn GetMember);

template <class type_, class... members>
struct BitFieldType : BitField<members...> {
  using parent = BitField<members...>;
  using value_type = type_;
  type_ value;

  template <class bitmem> constexpr type_ Get() const {
    BitMember item = parent::Get(bitmem::index);
    type_ retval = value & item.GetMask<type_>();
    return retval >> item.position;
  }

  template <class bitmem> constexpr void Set(type_ input) {
    BitMember item = parent::Get(bitmem::index);
    type_ mask = item.GetMask<type_>();
    value &= ~mask;
    value |= (input << item.position) & mask;
  }

  void SwapEndian(bool outWay) {
    SwapBitField(parent::numItems, outWay, value,
                 [](size_t id) { return parent::Get(id); });
  }
};

#include "internal/bitfield.inl"
