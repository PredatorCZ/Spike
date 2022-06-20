/*  FByteswapper function, for endianness swapping, swaps any datatype,
    calls void SwapEndian() on class instead of direct swap, if available,
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

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
#include "endian_fwd.hpp"
#include "supercore.hpp"
#include <cstring>
#include <stdexcept>

namespace {

template <class C> C _fbswap(C) {
  throw std::logic_error("Invalid swap type!");
}

template <> constexpr uint16 _fbswap(uint16 input) {
  return ((input & 0xFF) << 8) | ((input & 0xFF00) >> 8);
}

template <> constexpr uint32 _fbswap(uint32 input) {
  return ((input & 0xFFU) << 24) | ((input & 0xFF00U) << 8) |
         ((input & 0xFF0000U) >> 8) | ((input & 0xFF000000U) >> 24);
}

template <> constexpr uint64 _fbswap(uint64 input) {
  return ((input & 0xFFULL) << 56) | ((input & 0xFF00ULL) << 40) |
         ((input & 0xFF0000ULL) << 24) | ((input & 0xFF000000ULL) << 8) |
         ((input & 0xFF00000000ULL) >> 8) |
         ((input & 0xFF0000000000ULL) >> 24) |
         ((input & 0xFF000000000000ULL) >> 40) |
         ((input & 0xFF00000000000000ULL) >> 56);
}

static_assert(_fbswap<uint16>(0xabcd) == 0xcdab);
static_assert(_fbswap<uint32>(0x89abcdef) == 0xefcdab89);
static_assert(_fbswap<uint64>(0x0123456789abcdef) == 0xefcdab8967452301);

template <class T>
using use_basic_swap = decltype(std::declval<T>().SwapEndian());
template <class C>
constexpr bool use_basic_swap_v = es::is_detected_v<use_basic_swap, C>;

template <class T>
using use_new_swap = decltype(std::declval<T>().SwapEndian(true));
template <class C>
constexpr bool use_new_swap_v = es::is_detected_v<use_new_swap, C>;
} // namespace

template <class C> void FByteswapper(C &input, bool outWay) {
  (void)(outWay);
  if constexpr (use_basic_swap_v<C>) {
    input.SwapEndian();
  } else if constexpr (use_new_swap_v<C>) {
    input.SwapEndian(outWay);
  } else {
    auto rType = _fbswap(
        reinterpret_cast<typename es::TypeFromSize<sizeof(C)>::type &>(input));
    memcpy(&input, &rType, sizeof(input));
  }
}

template <class C, size_t _size>
void FByteswapper(C (&input)[_size], bool outWay) {
  for (auto &a : input) {
    FByteswapper(a, outWay);
  }
}

template <class E, class C> void FArraySwapper(C &input) {
  const size_t numItems = sizeof(C) / sizeof(E);
  E *inputPtr = reinterpret_cast<E *>(&input);

  for (size_t t = 0; t < numItems; t++) {
    FByteswapper(*(inputPtr + t));
  }
}

struct Endian_ {
  bool Defined();
};
