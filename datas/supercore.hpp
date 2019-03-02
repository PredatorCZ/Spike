/*	Supercore, mostly detectors/macros
	more info in README for PreCore Project

	Copyright 2018-2019 Lukas Cone

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
#define uint					unsigned int
#define ushort					unsigned short
#define uchar					unsigned char
#define ulong					unsigned long long

#if defined(__GNUC__) || defined(__GNUG__)
#define ES_FORCEINLINE __attribute__((always_inline))
#define ES_INLINE
#elif defined(_MSC_VER)
#define ES_FORCEINLINE __forceinline
#define ES_INLINE inline
#endif

typedef union { const char ACT[4]; const int NUM; } __es_endian_detector_u;

static constexpr bool ES_LITTLE_ENDIAN = __es_endian_detector_u{ 1 }.ACT[0] == 1;
static constexpr bool ES_X64 = sizeof(void*) == 8;

#ifndef _TCHAR_DEFINED
#ifdef _UNICODE
typedef wchar_t     TCHAR;
#else
typedef char     TCHAR;
#endif
#endif
#include <string>
template<class T>using UniString = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;

typedef UniString<TCHAR> TSTRING;

#ifdef _UNICODE
#define ToTSTRING std::to_wstring
#else
#define ToTSTRING std::to_string
#endif