/*  set of utilities for string conversions
    Copyright 2020 Lukas Cone

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
#include "supercore.hpp"
#include <codecvt>
#include <locale>
#include <string>

namespace _es {
class cvt168 : public std::codecvt<char16_t, char, std::mbstate_t> {};
class cvt328 : public std::codecvt<char32_t, char, std::mbstate_t> {};

template <typename _Ty, size_t _size> struct _utf8cvt {};

template <typename _Ty> struct _utf8cvt<_Ty, 2> {
  static std::string Cvt(const _Ty *input) {
    std::wstring_convert<cvt168, char16_t> converter;
    const char16_t *data = reinterpret_cast<const char16_t *>(input);
    return converter.to_bytes(data);
  }

  static std::string Cvt(const std::basic_string<_Ty> &input) {
    std::wstring_convert<cvt168, char16_t> converter;
    const char16_t *first = reinterpret_cast<const char16_t *>(&*input.begin());
    const char16_t *last = reinterpret_cast<const char16_t *>(&*input.end());
    return converter.to_bytes(first, last);
  }
};

template <typename _Ty> struct _utf8cvt<_Ty, 4> {
  static std::string Cvt(const _Ty *input) {
    std::wstring_convert<cvt328, char32_t> converter;
    const char32_t *data = reinterpret_cast<const char32_t *>(input);
    return converter.to_bytes(data);
  }

  static std::string Cvt(const std::basic_string<_Ty> &input) {
    std::wstring_convert<cvt328, char32_t> converter;
    const char32_t *first = reinterpret_cast<const char32_t *>(&*input.begin());
    const char32_t *last = reinterpret_cast<const char32_t *>(&*input.end());
    return converter.to_bytes(first, last);
  }
};

template <typename _Ty> using utf8cvt = _utf8cvt<_Ty, sizeof(_Ty)>;

} // namespace _es

namespace es {
template <class C> std::string ToUTF8(const C *input) {
  return _es::utf8cvt<C>::Cvt(input);
}

template <class C> std::string ToUTF8(const std::basic_string<C> &input) {
  return _es::utf8cvt<C>::Cvt(input);
}

inline std::wstring ToUTF1632(const char *input) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(input);
}

inline std::wstring ToUTF1632(const std::string &input) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(input);
}

}; // namespace es