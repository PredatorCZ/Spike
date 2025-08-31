/*  Supercore, mostly detectors/macros

    Copyright 2018-2023 Lukas Cone

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

#include "detail/sc_architecture.hpp"
#include <iterator>

#if defined(__cplusplus) || defined(c_plusplus)

static constexpr uint8 atohLUT[] = {
    "................................................"
    "\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9.......\xa\xb\xc\xd\xe\xf..................."
    ".......\xa\xb\xc\xd\xe\xf"};

static_assert(atohLUT[uint8('0')] == 0);
static_assert(atohLUT[uint8('9')] == 9);
static_assert(atohLUT[uint8('a')] == 10);
static_assert(atohLUT[uint8('f')] == 15);
static_assert(atohLUT[uint8('A')] == 10);
static_assert(atohLUT[uint8('F')] == 15);

template <size_t n> constexpr uint32 CompileFourCC(const char (&input)[n]) {
  uint32 retVal = 0;
  for (size_t i = 0; i < sizeof(retVal); i++) {
    const uint32 tmp = input[i];
    retVal |= tmp << (8 * i);
  }

  return retVal;
}

static_assert(CompileFourCC("ABCD") == 0x44434241);

constexpr size_t GetPadding(size_t value, size_t alignment) {
  const size_t mask = alignment - 1;
  const size_t result = value & mask;

  return !result ? 0 : (alignment - result);
}

static_assert(GetPadding(7, 8) == 1);
static_assert(GetPadding(7, 16) == 9);

constexpr float GetFraction(size_t numBits) {
  return 1.f / static_cast<float>((1ULL << numBits) - 1);
}

template <typename type> constexpr bool IsPow2(type input) {
  return !(input & (input - 1));
}

namespace es {
static inline void Print(const char *data) { printf("%s", data); }
static inline void Print(const char8_t *data) {
  printf("%s", reinterpret_cast<const char *>(data));
}

template <class cnt, class iterType>
bool IsEnd(const cnt &container, const iterType &iter) {
  using std::end;
  return end(container) == iter;
}

template <class sview>
sview SkipStartWhitespace(sview input, bool inclNewLine = false) noexcept {
  while (!input.empty() &&
         (input[0] == ' ' || input[0] == '\t' ||
          (inclNewLine && (input[0] == '\n' || input[0] == '\r'))))
    input.remove_prefix(1);

  return input;
}

template <class sview>
sview SkipEndWhitespace(sview input, bool inclNewLine = false) noexcept {
  while (!input.empty() &&
         (input.back() == ' ' || input.back() == '\t' ||
          (inclNewLine && (input.back() == '\n' || input.back() == '\r'))))
    input.remove_suffix(1);

  return input;
}

template <class sview>
sview TrimWhitespace(sview input, bool inclNewLine = false) noexcept {
  input = SkipStartWhitespace(input, inclNewLine);
  return SkipEndWhitespace(input, inclNewLine);
}

template <class C> void Dispose(C &item) { auto removed = std::move(item); }

template <class, template <class...> class Op, class... Args>
struct detector_ : std::false_type {};

template <template <class...> class Op, class... Args>
struct detector_<std::void_t<Op<Args...>>, Op, Args...> : std::true_type {};

template <template <class...> class Op, class... Args>
using is_detected = detector_<void, Op, Args...>;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

} // namespace es

#endif
