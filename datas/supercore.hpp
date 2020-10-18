/*  Supercore, mostly detectors/macros
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

#pragma once

#include "internal/sc_architecture.hpp"
#include <iterator>

#if defined(__cplusplus) || defined(c_plusplus)

static constexpr uint8 atohLUT[] = {
    "................................................"
    "\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9.......\xa\xb\xc\xd\xe\xf..................."
    ".......\xa\xb\xc\xd\xe\xf"};

ES_STATIC_ASSERT(atohLUT[uint8('0')] == 0);
ES_STATIC_ASSERT(atohLUT[uint8('9')] == 9);
ES_STATIC_ASSERT(atohLUT[uint8('a')] == 10);
ES_STATIC_ASSERT(atohLUT[uint8('f')] == 15);
ES_STATIC_ASSERT(atohLUT[uint8('A')] == 10);
ES_STATIC_ASSERT(atohLUT[uint8('F')] == 15);

constexpr uint32 CompileFourCC(const char *input, const uint32 hash = 0,
                               const uint32 indexOffset = 0,
                               const uint32 currentIndex = 0) {
  return currentIndex > 3
             ? hash
             : CompileFourCC(
                   input,
                   hash |
                       (static_cast<uint32>(input[indexOffset + currentIndex])
                        << (currentIndex * 8)),
                   indexOffset, currentIndex + 1);
}

static inline size_t GetPadding(size_t value, size_t alignment) {
  const size_t mask = alignment - 1;
  const size_t result = value & mask;

  return !result ? 0 : (alignment - result);
}

// Build a compile time fraction for number quantization
template <size_t numBits> class GetFraction_t {
  static_assert(numBits < 127, "Fraction overflow!");
  static constexpr size_t EXPONENT = (0x7f ^ numBits) << 23;
  constexpr static size_t _CMan(size_t shiftVal) {
    return shiftVal > 24 ? 0 : (1ULL << (24 - shiftVal));
  }
  constexpr static size_t _MantissaBuilder(size_t lastValue,
                                           size_t shiftOffset) {
    return shiftOffset > 23
               ? lastValue >> 1
               : _MantissaBuilder(lastValue | _CMan(numBits + shiftOffset),
                                  shiftOffset + numBits);
  }

public:
  static constexpr size_t VALUE =
      numBits < 2 ? 0x3f800000 : _MantissaBuilder(0, 0) | EXPONENT;
};

constexpr float GetFraction(size_t numBits) {
  return 1.f / static_cast<float>((1ULL << numBits) - 1);
}

namespace es {

template <class cnt, class iterType>
bool IsEnd(const cnt &container, const iterType &iter) {
  using std::end;
  return end(container) == iter;
}

template <class sview>
sview SkipStartWhitespace(sview input, bool inclNewLine = false) noexcept {
  while ((input[0] == ' ' || input[0] == '\t' ||
          (inclNewLine && input[0] == '\n')) &&
         !input.empty())
    input.remove_prefix(1);

  return input;
}

template <class sview>
sview SkipEndWhitespace(sview input, bool inclNewLine = false) noexcept {
  while ((*std::prev(input.end()) == ' ' || *std::prev(input.end()) == '\t' ||
          (inclNewLine && *std::prev(input.end()) == '\n')) &&
         !input.empty())
    input.remove_suffix(1);

  return input;
}

template <class sview> sview TrimWhitespace(sview input) noexcept {
  input = SkipStartWhitespace(input);
  return SkipEndWhitespace(input);
}

} // namespace es

#endif
