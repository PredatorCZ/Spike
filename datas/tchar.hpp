/*  TCHAR handles

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
#include "unicode.hpp"

#if defined(_MSC_VER) || defined(__MINGW64__)
#include <tchar.h>
#else
#ifdef UNICODE
typedef wchar_t TCHAR;
#define _tmain wmain
#define __T(x) L##x
#define _tcsftime wcsftime
#else
typedef char TCHAR;
#define __T(x) x
#define _tmain main
#endif
#define _T(x) __T(x)
#define _tcsftime strftime
#endif

template <class T>
using UniString = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;

typedef UniString<TCHAR> TSTRING;

namespace std {
inline string to_string(const wstring &type) { return es::ToUTF8(type); }

inline string to_string(const wchar_t *type) { return es::ToUTF8(type); }

inline string to_string(const string &type) { return type; }

inline string to_string(const char *type) { return type; }

inline wstring to_wstring(const string &type) { return es::ToUTF1632(type); }

inline wstring to_wstring(const char *type) { return es::ToUTF1632(type); }

inline wstring to_wstring(const wstring &type) { return type; }

inline wstring to_wstring(const wchar_t *type) { return type; }
} // namespace std

#ifdef UNICODE
template <typename T> TSTRING ToTSTRING(const T &type) {
  return std::to_wstring(type);
}
#else
template <typename T> TSTRING ToTSTRING(const T &type) {
  return std::to_string(type);
}
#endif

inline bool IsHelp(es::basic_string_view<TCHAR> data) {
  return data == _T("-h") || data == _T("-?") || data == _T("/?") ||
         data == _T("--help");
}
