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
#include "string_view.hpp"
#include "supercore.hpp"
#include <codecvt>
#include <locale>
#include <string>

#if _MSC_VER >= 1900 
#define _ES_UTF16 int16
#define _ES_UTF32 int32
#else
#define _ES_UTF16 char16_t
#define _ES_UTF32 char32_t
#endif

namespace _es {
template <typename _Ty, size_t _size> struct _utf8cvt {};

template <typename _Ty> struct _utf8cvt<_Ty, 2> {
  static std::string Cvt(const _Ty *input) {
    const es::basic_string_view<_Ty> sw(input);
    std::wstring_convert<std::codecvt_utf8_utf16<_ES_UTF16>, _ES_UTF16> converter;
    const _ES_UTF16 *first = reinterpret_cast<const _ES_UTF16 *>(&*sw.begin());
    const _ES_UTF16 *last = reinterpret_cast<const _ES_UTF16 *>(&*sw.end());
    return converter.to_bytes(first, last);
  }

  static std::string Cvt(const std::basic_string<_Ty> &input) {
    std::wstring_convert<std::codecvt_utf8_utf16<_ES_UTF16>, _ES_UTF16> converter;
    const _ES_UTF16 *first = reinterpret_cast<const _ES_UTF16 *>(&*input.begin());
    const _ES_UTF16 *last = reinterpret_cast<const _ES_UTF16 *>(&*input.end());
    return converter.to_bytes(first, last);
  }
};

template <typename _Ty> struct _utf8cvt<_Ty, 4> {
  static std::string Cvt(const _Ty *input) {
    const es::basic_string_view<_Ty> sw(input);
    std::wstring_convert<std::codecvt_utf8<_ES_UTF32>, _ES_UTF32> converter;
    const _ES_UTF32 *first = reinterpret_cast<const _ES_UTF32 *>(&*sw.begin());
    const _ES_UTF32 *last = reinterpret_cast<const _ES_UTF32 *>(&*sw.end());
    return converter.to_bytes(first, last);
  }

  static std::string Cvt(const std::basic_string<_Ty> &input) {
    std::wstring_convert<std::codecvt_utf8<_ES_UTF32>, _ES_UTF32> converter;
    const _ES_UTF32 *first = reinterpret_cast<const _ES_UTF32 *>(&*input.begin());
    const _ES_UTF32 *last = reinterpret_cast<const _ES_UTF32 *>(&*input.end());
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
