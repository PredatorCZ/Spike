/*  compile time Jenkins one at time hashing function
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
#include "string_view.hpp"

typedef unsigned long long _SuperJenHash;
typedef unsigned int JenHash;

struct JenHashStrong {
private:
  JenHash _value;

public:
  explicit constexpr JenHashStrong(JenHash _in) : _value(_in) {}
  explicit constexpr operator JenHash() const { return _value; };
  constexpr bool operator==(JenHash _in) const { return _in == _value; }
  constexpr bool operator==(JenHashStrong _in) const {
    return _in._value == _value;
  }

  friend constexpr bool operator==(JenHash _v0, JenHashStrong _v1) {
    return _v1 == _v0;
  }
};

#define _SuperResVal static_cast<_SuperJenHash>(resval)

// clang-format off
constexpr JenHash _JenkinsHash(es::string_view input, JenHash resval = 0U, size_t index = 0)
{
  return index < input.size() ? _JenkinsHash(
    input,
      (
        (_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(input[index]))) + 
        ((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(input[index]))) << 10)
      ) ^
      ((
        (_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(input[index]))) + 
        ((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(input[index]))) << 10)
      ) >> 6), 
    index + 1) :
    ((_SuperResVal + (_SuperResVal << 3)) ^ ((_SuperResVal + (_SuperResVal << 3)) >> 11)) + 
    (((_SuperResVal + (_SuperResVal << 3)) ^ ((_SuperResVal + (_SuperResVal << 3)) >> 11)) << 15);
}
// clang-format on

constexpr JenHash JenkinsHash(es::string_view input) {
  return _JenkinsHash(input);
}

template<typename T, size_t _size>
constexpr JenHash JenkinsHashC(const T (&input)[_size]) {
  return JenkinsHash({input, _size - 1});
}

static_assert(JenkinsHashC("bug") == 0x54908567, "JOAAT Failed");
