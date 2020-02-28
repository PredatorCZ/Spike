/*  FByteswapper function, for endianness swapping, swaps any datatype,
    calls void SwapEndian() on class instead of direct swap, if available,
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

#ifndef ES_ENDIAN_DEFINED
#define ES_ENDIAN_DEFINED
#include "supercore.hpp"

namespace {

template <size_t size> struct _TypeFromSize { typedef char type; };
template <> struct _TypeFromSize<1> { typedef uint8 type; };
template <> struct _TypeFromSize<2> { typedef uint16 type; };
template <> struct _TypeFromSize<4> { typedef uint32 type; };
template <> struct _TypeFromSize<8> { typedef uint64 type; };

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

ES_STATIC_ASSERT(_fbswap<uint16>(0xabcd) == 0xcdab);
ES_STATIC_ASSERT(_fbswap<uint32>(0x89abcdef) == 0xefcdab89);
ES_STATIC_ASSERT(_fbswap<uint64>(0x0123456789abcdef) == 0xefcdab8967452301);

template <class C, class D>
auto fbswap(D &input, int) -> decltype(std::declval<C>().SwapEndian(), void()){
  input.SwapEndian();
};

template <class C, class D> void fbswap(D &input, ...) {
  auto rType = _fbswap(
      reinterpret_cast<typename _TypeFromSize<sizeof(C)>::type &>(input));
  input = reinterpret_cast<C &>(rType);
}
} // namespace

template <class C> void FByteswapper(C &input) { fbswap<C>(input, 0); }

template <class E, class C> void FArraySwapper(C &input) {
  const size_t numItems = sizeof(C) / sizeof(E);
  E *inputPtr = reinterpret_cast<E *>(&input);

  for (size_t t = 0; t < numItems; t++)
    FByteswapper(*(inputPtr + t));
}
#endif
