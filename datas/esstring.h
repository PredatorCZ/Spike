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
#include "supercore.hpp"
#include <ostream>

class BinWritter;
class BinReader;

struct esString
{
private:
	char *_Data;
	struct { unsigned int UseWideChar : 1, IsLinked : 1, IOWideChar : 1; }Flags;
	uint size = 0;
	uint capacity = 0;
	void _refresh(unsigned int newsize);
	void Set(const char *input, const uint size);
	void Set(const wchar_t *input, const uint size);
public:
	ES_FORCEINLINE esString() : Flags{ 0, 0, OutputAsUTF16 }, size(0), capacity(0), _Data(nullptr) {}
	template<class C> ES_FORCEINLINE esString(const UniString<C> &input) { *this = input; }
	ES_FORCEINLINE esString(const char *input) : esString() { *this = input; };
	ES_FORCEINLINE esString(const wchar_t *input) : esString() { *this = input; };
	ES_FORCEINLINE esString(const esString &input) { ReceiveLinkCopy(input); };

	void Save(BinWritter *wr, bool pureText = false);
	void Load(BinReader *wr, bool pureText = false);
	void Link(char *ptr);


	operator std::wstring() const;
	operator std::string() const;

	template<class C> ES_FORCEINLINE void operator = (const UniString<C> &input) { Set(input.c_str(), static_cast<uint>(input.size())); }
	ES_FORCEINLINE void operator = (const char *input) { Set(input, static_cast<uint>(strlen(input))); }
	ES_FORCEINLINE void operator = (const wchar_t *input) { Set(input, static_cast<uint>(wcslen(input))); }
	ES_FORCEINLINE void operator = (const esString &input) { ReceiveLinkCopy(input); }
		
	bool compare(const esString &input)const;
	ES_FORCEINLINE bool Unicode()const { return Flags.IOWideChar; }
	ES_FORCEINLINE void Unicode(bool setval) { Flags.IOWideChar = setval; }
	void SwapEndian();
	void LinkCopyInto(esString &input)const;
	void ReceiveLinkCopy(const esString &input);
	
	~esString();
	friend ES_INLINE std::ostream& operator<<(std::ostream &strm, const esString &v) { return strm << static_cast<std::string>(v).c_str(); }
	friend ES_INLINE std::wostream& operator<<(std::wostream &strm, const esString &v) { return strm << static_cast<std::wstring>(v).c_str(); }

	thread_local static bool OutputAsUTF16;
};