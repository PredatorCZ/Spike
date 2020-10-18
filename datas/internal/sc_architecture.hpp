/*  Supercore architecture detectors/macros

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
#include <type_traits>
#include "sc_type.hpp"

#if defined(__GNUC__) || defined(__GNUG__)
#define ES_PRAGMA(command) _Pragma(#command)
#if __GNUC__ < 7
#undef offsetof
#define offsetof(T, M) reinterpret_cast<size_t>(&(((T *)0)->M))
#endif

#elif defined(_MSC_VER)
#define ES_PRAGMA(command) __pragma(command)
#endif

#define ES_STATIC_ASSERT(eval) static_assert(eval, #eval)

#if defined(__cplusplus) || defined(c_plusplus)

static inline bool LittleEndian() {
  static const union {
    const int NUM;
    const char ACT[4];
  } end_dec{1};

  return end_dec.ACT[0] == 1;
}

static constexpr bool ES_X64 = sizeof(void *) == 8;

struct __es_reusePadding_detector_superbase_s {};
struct __es_reusePadding_detector_base_s
    : __es_reusePadding_detector_superbase_s {
  int v1;
  short v20;
};
struct __es_reusePadding_detector_s : __es_reusePadding_detector_base_s {
  char v21;
};

static constexpr bool ES_REUSE_PADDING =
    sizeof(__es_reusePadding_detector_s) == 8;

typedef typename std::conditional<ES_X64, uint64, uint32>::type esIntPtr;

#else
static const int ES_X64 = sizeof(void *) == 8;
#endif
