/*	esString class allows to store and convert between string/wstring
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
#include <cstring>
#include <locale>
#include <codecvt>
#include <ostream>
#include <vector>
#include "allocator_hybrid.hpp"
#include "supercore.hpp"

class esString
{
	typedef std::allocator_hybrid<char> Alloc_Type;
	typedef std::vector<char, Alloc_Type> Container_Type;
	Container_Type masterBuffer;
	bool UseWideChar;
	
public:
	esString() : UseWideChar(true) {}
	template<class C> esString(const UniString<C> &input) : esString() { *this = input; }
	esString(const char *input) : esString() { *this = input; };
	esString(const wchar_t *input) : esString() { *this = input; };

	void Link(const char *input);
	void Link(const wchar_t *input);
	void Set(const char *input);
	void Set(const wchar_t *input);
	ES_FORCEINLINE std::wstring ToWString() const { return static_cast<std::wstring>(*this); }
	ES_FORCEINLINE std::string ToString() const { return static_cast<std::string>(*this); }

	template<class C>
	ES_FORCEINLINE UniString<C> ToTString() const { return static_cast<UniString<C>>(*this); }

	operator std::wstring() const;
	operator std::string() const;

	template<class C> void operator = (const UniString<C> &input) { Set(input.c_str()); }
	void operator = (const char *input) { Set(input); }
	void operator = (const wchar_t *input) { Set(input); }
	
	friend ES_INLINE std::ostream& operator<<(std::ostream &strm, const esString &v) { return strm << static_cast<std::string>(v).c_str(); }
	friend ES_INLINE std::wostream& operator<<(std::wostream &strm, const esString &v) { return strm << static_cast<std::wstring>(v).c_str(); }
};

template<class C>
static UniString<C> esStringConvert(const wchar_t *input)
{
	return esString(input).ToTString<C>();
}

template<class C>
static UniString<C> esStringConvert(const char *input)
{
	return esString(input).ToTString<C>();
}

template<class R, class C>
static UniString<R> esToString(C input, R) 
{
	//Shall be never called
	return esString(std::to_string(input).c_str()).ToTString<R>();
}

template<class C>
static UniString<wchar_t> esToString(C input, wchar_t)
{
	return std::to_wstring(input);
}

template<class C>
static UniString<char> esToString(C input, char)
{
	return std::to_string(input);
}

ES_INLINE esString::operator std::string() const
{
	if (UseWideChar)
	{
		const wchar_t *buffa = reinterpret_cast<const wchar_t *>(masterBuffer.data());
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(buffa);
	}
	else
		return masterBuffer.data();
}

ES_INLINE esString::operator std::wstring() const
{
	if (UseWideChar)
		return reinterpret_cast<const wchar_t *>(masterBuffer.data());
	else
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(masterBuffer.data());
	}
}

ES_INLINE void esString::Set(const char *input)
{
	size_t sLen = strlen(input) + 1;
	UseWideChar = false;
	masterBuffer = Container_Type(input, input + sLen);
}

ES_INLINE void esString::Set(const wchar_t *input)
{
	size_t sLen = (wcslen(input) + 1) * sizeof(wchar_t);
	UseWideChar = true;
	const char *rcInput = reinterpret_cast<const char *>(input);
	masterBuffer = Container_Type(rcInput, rcInput + sLen);
}

ES_INLINE void esString::Link(const char *input)
{
	size_t sLen = strlen(input) + 1;
	UseWideChar = false;
	masterBuffer = Container_Type(input, input + sLen, Alloc_Type(const_cast<char*>(input)));
}

ES_INLINE void esString::Link(const wchar_t *input)
{
	size_t sLen = (wcslen(input) + 1) * sizeof(wchar_t);
	UseWideChar = true;
	const char *rcInput = reinterpret_cast<const char *>(input);
	masterBuffer = Container_Type(rcInput, rcInput + sLen, Alloc_Type(const_cast<char *>(rcInput)));
}
