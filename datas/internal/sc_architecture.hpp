/*  Supercore architecture detectors/macros

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
#include "sc_type.hpp"
#include <type_traits>

#ifdef __MINGW64__
#define ES_PRAGMA(command) _Pragma(#command)
#define ES_EXPORT __attribute__((dllexport))
#define ES_IMPORT
#elif defined(__GNUC__) || defined(__GNUG__)
#define ES_PRAGMA(command) _Pragma(#command)
#define ES_EXPORT __attribute__((visibility("default")))
#define ES_IMPORT
#elif defined(_MSC_VER)
#define ES_PRAGMA(command) __pragma(command)
#define ES_EXPORT __declspec(dllexport)
#define ES_IMPORT __declspec(dllimport)
#endif

#if defined(__cplusplus) || defined(c_plusplus)

static inline bool LittleEndian() {
  static const union {
    const uint32 NUM;
    const uint8 ACT[4];
  } end_dec{1};

  return end_dec.ACT[0] == 1;
}

constexpr bool ReusePadding() {
  struct Base_ {};
  struct Derived_ : Base_ {
    uint32 v1;
    uint16 v20;
  };
  struct Final_ : Derived_ {
    uint8 v21;
  };

  return sizeof(Final_) == 8;
}

static constexpr bool ES_REUSE_PADDING = ReusePadding();
#endif
