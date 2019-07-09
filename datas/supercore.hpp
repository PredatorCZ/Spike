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

typedef unsigned int			uint;
typedef unsigned short			ushort;					
typedef unsigned char			uchar;
typedef unsigned long long		uint64;
typedef long long				int64;

#if defined(__GNUC__) || defined(__GNUG__)
#define ES_FORCEINLINE __attribute__((always_inline))
#define ES_INLINE inline

#if __GNUC__ < 7
#undef offsetof
#define offsetof(T, M) reinterpret_cast<size_t>(&(((T*)0)->M))
#endif

#elif defined(_MSC_VER)
#define ES_FORCEINLINE __forceinline
#define ES_INLINE inline
#endif

#if defined(__cplusplus) || defined(c_plusplus)

typedef union { const char ACT[4]; const int NUM; } __es_endian_detector_u;

static constexpr bool ES_LITTLE_ENDIAN = __es_endian_detector_u{ 1 }.ACT[0] == 1;
static constexpr bool ES_X64 = sizeof(void*) == 8;

struct __es_reusePadding_detector_superbase_s {};
struct __es_reusePadding_detector_base_s : __es_reusePadding_detector_superbase_s
{
	int v1;
	short v20;
};
struct __es_reusePadding_detector_s : __es_reusePadding_detector_base_s
{
	char v21;
};

static constexpr bool ES_REUSE_PADDING = sizeof(__es_reusePadding_detector_s) == 8;

#else
static const int ES_X64 = sizeof(void*) == 8;
#endif

#ifndef _TCHAR_DEFINED
#ifdef _UNICODE
typedef wchar_t     TCHAR;
#else
typedef char     TCHAR;
#endif
#endif

#if defined(__cplusplus) || defined(c_plusplus)
#include <string>
template<class T>using UniString = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;

typedef UniString<TCHAR> TSTRING;

#ifdef _UNICODE
#define ToTSTRING std::to_wstring
#else
#define ToTSTRING std::to_string
#endif

typedef std::conditional<ES_X64, uint64, uint>::type esIntPtr;

constexpr int CompileFourCC(const char *input, const int hash = 0, const int indexOffset = 0, const int currentIndex = 0)
{
	return currentIndex > 3 ? hash : CompileFourCC(input, hash | (static_cast<int>(input[indexOffset + currentIndex]) << (currentIndex * 8)), indexOffset, currentIndex + 1);
}

#endif