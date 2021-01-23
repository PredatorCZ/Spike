/*  constexpr Jenkins one at time class
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone
    Algorithm by Bob Jenkins

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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4307)
#endif
#include "internal/sc_type.hpp"
#include "string_view.hpp"

template <typename T = uint64>
constexpr uint32 JenkinsHash_(es::string_view input) {
  T result = 0;

  for (const auto c : input) {
    const T cChar = static_cast<uint8>(c);
    result += cChar;
    result += result << 10;
    result ^= result >> 6;
    result = static_cast<uint32>(result);
  }

  result += result << 3;
  result ^= result >> 11;
  result += result << 15;

  return static_cast<uint32>(result);
}

template <typename I> struct JenHash_t {
  constexpr JenHash_t() : value_() {}
  constexpr JenHash_t(JenHash_t &&) = default;
  constexpr JenHash_t(const JenHash_t &) = default;
  constexpr explicit JenHash_t(uint32 in) : value_(in) {}
  template <size_t n>
  constexpr JenHash_t(const char (&input)[n])
      : value_(JenkinsHash_<I>({input, n - 1})) {}
  constexpr JenHash_t(es::string_view input) : value_(JenkinsHash_<I>(input)) {}

  constexpr JenHash_t &operator=(const JenHash_t &) = default;
  constexpr JenHash_t &operator=(JenHash_t &&) = default;

  constexpr auto raw() const { return value_; }

  constexpr bool operator==(JenHash_t o) const { return o.value_ == value_; }
  constexpr bool operator==(uint32 o) const { return o == value_; }
  constexpr friend bool operator==(uint32 o, JenHash_t h) { return h == o; }

  constexpr bool operator!=(JenHash_t o) const { return o.value_ != value_; }
  constexpr bool operator!=(uint32 o) const { return o != value_; }
  constexpr friend bool operator!=(uint32 o, JenHash_t h) { return h != o; }

  constexpr bool operator<(JenHash_t o) const { return o.value_ < value_; }
  constexpr bool operator>(JenHash_t o) const { return o.value_ > value_; }

private:
  uint32 value_;
};

using JenHash = JenHash_t<uint64>;
using JenHashCannon = JenHash_t<uint32>;

static_assert(JenHash("bug") == 0x54908567, "JenkinsHash Failed");
static_assert(JenHashCannon("bug") == 0xF37C8567, "JOAAT Failed");

#ifdef _MSC_VER
#pragma warning(pop)
#endif
