/*  Supercore data types

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
#include <stdint.h>

using muint64 = uint64_t;
using muint32 = uint32_t;
using muint16 = uint16_t;
using muint8 = uint8_t;

using mint64 = int64_t;
using mint32 = int32_t;
using mint16 = int16_t;
using mint8 = int8_t;

using mreal32 = float;
using mreal64 = double;

using mintmax = intmax_t;
using msize = uintmax_t;

using uint64 = const uint64_t;
using uint32 = const uint32_t;
using uint16 = const uint16_t;
using uint8 = const uint8_t;

using int64 = const int64_t;
using int32 = const int32_t;
using int16 = const int16_t;
using int8 = const int8_t;

using real32 = const float;
using real64 = const double;

using intmax = const intmax_t;
using size = const uintmax_t;

using bytes = const char *;
using mbytes = char *;

template <class C> struct rm_const_ {
  using type = C;
};

template <class C> struct rm_const_<const C> {
  using type = C;
};

template <class C> using mut = rm_const_<C>::type;

static_assert(sizeof(uint64) == 8, "Unexpected integer size!");
static_assert(sizeof(uint32) == 4, "Unexpected integer size!");
static_assert(sizeof(uint16) == 2, "Unexpected integer size!");
static_assert(sizeof(uint8) == 1, "Unexpected integer size!");
static_assert(sizeof(int64) == 8, "Unexpected integer size!");
static_assert(sizeof(int32) == 4, "Unexpected integer size!");
static_assert(sizeof(int16) == 2, "Unexpected integer size!");
static_assert(sizeof(int8) == 1, "Unexpected integer size!");
