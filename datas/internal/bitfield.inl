/*  BitField internals

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

#include "../endian.hpp"

static_assert(BitMember{0, 16}.GetMask<uint32>() == 0xffff, "Invalid mask!");
static_assert(BitMember{16, 16}.GetMask<uint32>() == 0xffff0000,
              "Invalid mask!");
static_assert(BitMember{16, 4}.GetMask<uint32>() == 0xf0000, "Invalid mask!");

static_assert(BitMember{0, 16}.GetMirrorMask<uint32>() == 0xffff0000,
              "Invalid mask!");
static_assert(BitMember{16, 16}.GetMirrorMask<uint32>() == 0xffff,
              "Invalid mask!");
static_assert(BitMember{16, 4}.GetMirrorMask<uint32>() == 0xf000,
              "Invalid mask!");

template <class type, class fn>
void SwapBitField(size_t numItems, bool outWay, type &input, fn GetMember) {
  type newVal = 0;

  if (!outWay) {
    if (sizeof(type) > 1) {
      FByteswapper(input);
    }
    for (size_t i = 0; i < numItems; i++) {
      BitMember item = GetMember(i);
      type mask = item.GetMirrorMask<type>();
      type masked = input & mask;
      newVal |= (masked >> ((sizeof(type) * 8) - item.position - item.size))
                << item.position;
    }

    input = newVal;
  } else {
    for (size_t i = 0; i < numItems; i++) {
      BitMember item = GetMember(i);
      type mask = item.GetMask<type>();
      type masked = input & mask;
      newVal |= (masked >> item.position)
                << ((sizeof(type) * 8) - item.position - item.size);
    }

    input = newVal;

    if constexpr (sizeof(type) > 1) {
      FByteswapper(input);
    }
  }
}

template <class type, class... members>
struct _getType<BitFieldType<type, members...>> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::BitFieldClass;
  static constexpr JenHash Hash() {
    return ReflectorType<BitFieldType<type, members...>>::Hash();
  }
  static constexpr uint8 SUBSIZE = sizeof(type);
};
