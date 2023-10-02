/*  constexpr Jenkins lookup3 class

    Copyright 2020-2023 Lukas Cone
    Algorithm by Bob Jenkins, May 2006

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
#include <string_view>

struct JenTriplet_ {
  uint32_t a, b, c;

  constexpr static uint32_t Setup_(uint32_t len, uint32_t seed) {
    return 0xdeadbeef + len + seed;
  }

  constexpr static uint32_t JenRotate(uint32_t num, uint32_t by) {
    return (num << by) | (num >> (32 - by));
  }

  constexpr JenTriplet_(uint32_t len, uint32_t seed)
      : a(Setup_(len, seed)), b(Setup_(len, seed)), c(Setup_(len, seed)) {}

  constexpr JenTriplet_(uint32_t a_, uint32_t b_, uint32_t c_)
      : a(a_), b(b_), c(c_) {}

  constexpr JenTriplet_() : a(), b(), c() {}

  constexpr JenTriplet_ &operator+=(JenTriplet_ key) {
    a += key.a;
    b += key.b;
    c += key.c;
    return *this;
  }

  constexpr void Mix() {
    a -= c;
    a ^= JenRotate(c, 4);
    c += b;
    b -= a;
    b ^= JenRotate(a, 6);
    a += c;
    c -= b;
    c ^= JenRotate(b, 8);
    b += a;
    a -= c;
    a ^= JenRotate(c, 16);
    c += b;
    b -= a;
    b ^= JenRotate(a, 19);
    a += c;
    c -= b;
    c ^= JenRotate(b, 4);
    b += a;
  }

  constexpr void Finalize() {
    c ^= b;
    c -= JenRotate(b, 14);
    a ^= c;
    a -= JenRotate(c, 11);
    b ^= a;
    b -= JenRotate(a, 25);
    c ^= b;
    c -= JenRotate(b, 16);
    a ^= c;
    a -= JenRotate(c, 4);
    b ^= a;
    b -= JenRotate(a, 14);
    c ^= b;
    c -= JenRotate(b, 24);
  }
};

constexpr uint32_t JenMutateKey_(std::string_view input, size_t offset) {
  uint32_t ck0 = offset >= input.size() ? 0 : static_cast<uint8_t>(input[offset++]);
  uint32_t ck1 = offset >= input.size() ? 0 : static_cast<uint8_t>(input[offset++]);
  uint32_t ck2 = offset >= input.size() ? 0 : static_cast<uint8_t>(input[offset++]);
  uint32_t ck3 = offset >= input.size() ? 0 : static_cast<uint8_t>(input[offset++]);
  return ck0 | (ck1 << 8) | (ck2 << 16) | (ck3 << 24);
}

constexpr uint32_t JenkinsHash3_(std::string_view input, uint32_t seed = 0) {
  JenTriplet_ cTrip(input.size(), seed);

  for (size_t k = 0; k < std::max(input.size(), size_t(12)) - 12; k += 12) {
    uint32_t k0 = JenMutateKey_(input, k);
    uint32_t k1 = JenMutateKey_(input, k + 4);
    uint32_t k2 = JenMutateKey_(input, k + 8);
    JenTriplet_ trip(k0, k1, k2);
    cTrip += trip;
    cTrip.Mix();
  }

  size_t lastKeys = input.size() % 12;

  if (input.size() > 0 && lastKeys == 0) {
    lastKeys = 12;
  }

  JenTriplet_ lastTrip;

  if (lastKeys > 8) {
    lastTrip.a = JenMutateKey_(input, input.size() - lastKeys);
    lastKeys -= 4;
    lastTrip.b = JenMutateKey_(input, input.size() - lastKeys);
    lastKeys -= 4;
    lastTrip.c = JenMutateKey_(input, input.size() - lastKeys);
  } else if (lastKeys > 4) {
    lastTrip.a = JenMutateKey_(input, input.size() - lastKeys);
    lastKeys -= 4;
    lastTrip.b = JenMutateKey_(input, input.size() - lastKeys);
  } else if (lastKeys > 0) {
    lastTrip.a = JenMutateKey_(input, input.size() - lastKeys);
  }

  if (lastKeys > 0) {
    cTrip += lastTrip;
    cTrip.Finalize();
  }

  return cTrip.c;
}

struct JenHash3 {
  constexpr JenHash3() : value_() {}
  constexpr JenHash3(JenHash3 &&) = default;
  constexpr JenHash3(const JenHash3 &) = default;
  constexpr explicit JenHash3(uint32_t in) : value_(in) {}
  template <size_t n>
  constexpr JenHash3(const char (&input)[n])
      : value_(JenkinsHash3_({input, n - 1})) {}
  constexpr JenHash3(std::string_view input) : value_(JenkinsHash3_(input)) {}

  constexpr JenHash3 &operator=(const JenHash3 &) = default;
  constexpr JenHash3 &operator=(JenHash3 &&) = default;

  constexpr operator uint32_t() const { return value_; }

private:
  uint32_t value_;
};

static_assert(JenHash3("ahoj") == 0xE915A979, "JenHash3 failed.");
static_assert(JenHash3("nazdar") == 0xF5FDF04A, "JenHash3 failed.");
static_assert(JenHash3("seeyalater") == 0xE886430B, "JenHash3 failed.");
static_assert(JenHash3("A very big thingy") == 0x8C19469A, "JenHash3 failed.");

namespace es::jenhash_literals {
constexpr JenHash3 operator""_jh3(const char *str, size_t len) noexcept {
  return JenHash3{{str, len}};
}
} // namespace es::jenhash_literals
