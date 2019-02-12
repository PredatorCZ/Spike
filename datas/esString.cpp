/*	esString class source
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

#pragma warning(disable: 4267)
#include "binreader.hpp"
#include "BinWritter.hpp"
#include "esString.h"

thread_local bool esString::OutputAsUTF16 = false;

esString::operator std::string() const
{
	if (Flags.UseWideChar)
	{
		std::wstring tmp = reinterpret_cast<wchar_t*>(_Data);
		return std::string(tmp.begin(), tmp.end());
	}
	else
		return _Data;
}

esString::operator std::wstring() const
{
	if (Flags.UseWideChar)
		return reinterpret_cast<wchar_t*>(_Data);
	else
	{
		std::string tmp = reinterpret_cast<char*>(_Data);
		return std::wstring(tmp.begin(), tmp.end());
	}
}

void esString::_refresh(unsigned int numchars)
{
	Flags.IsLinked = false;
	size = numchars;
	const unsigned int newcapacity = Flags.UseWideChar ? (numchars + 1) * 2 : (numchars + 1);
	if (newcapacity <= capacity)return;
	if (!capacity)
		_Data = static_cast<char*>(malloc(newcapacity));
	else if (newcapacity > capacity)
		_Data = static_cast<char*>(realloc(_Data, newcapacity));
	capacity = newcapacity;
}

void esString::Set(const char *input, const uint size)
{
	Flags.UseWideChar = false;
	_refresh(size);
	memcpy(_Data, input, size + 1);
}

void esString::Set(const wchar_t *input, const uint size)
{
	Flags.UseWideChar = true;
	_refresh(size);
	memcpy(_Data, input, (size + 1) * 2);
}
	
void esString::Save(BinWritter *bw, bool pureText)
{
	if (pureText)
	{
		if (Flags.IOWideChar)
			bw->WriteContainer(operator std::wstring());
		else
			bw->WriteContainer(operator std::string());
	}
	else
	{
		if (Flags.IOWideChar)
			bw->WriteContainerWCount<uchar>(operator std::wstring());
		else
			bw->WriteContainerWCount<uchar>(operator std::string());
	}
}

void esString::Load(BinReader *rd, bool pureText)
{
	if (pureText)
	{
		if (Flags.IOWideChar)
		{
			std::wstring tmp;
			rd->ReadString(tmp);
			*this = tmp;
		}
		else
		{
			std::string tmp;
			rd->ReadString(tmp);
			*this = tmp;
		}
	}
	else
	{
		if (Flags.IOWideChar)
		{
			std::wstring tmp;
			rd->ReadContainer<uchar>(tmp);
			*this = tmp;
		}
		else
		{
			std::string tmp;
			rd->ReadContainer<uchar>(tmp);
			*this = tmp;
		}
	}
}

void esString::Link(char *ptr)
{
	size = *ptr;
	capacity = 0;
	Flags.UseWideChar = Flags.IOWideChar;
	Flags.IsLinked = true;
	_Data = ptr + 1;
}

esString::~esString()
{
	if (!Flags.IsLinked&&_Data)
	{
		free(_Data);
		_Data = nullptr;
	}
}

bool esString::compare(const esString &input) const
{
	bool pass = (input.size == size) && input.Flags.UseWideChar == Flags.UseWideChar;
	if (pass)
		pass = Flags.UseWideChar ? !wcscmp(reinterpret_cast<wchar_t*>(_Data), reinterpret_cast<wchar_t*>(input._Data)) : !strcmp(reinterpret_cast<char*>(_Data), reinterpret_cast<char*>(input._Data));
	return pass;
}

void esString::SwapEndian()
{
	if (Flags.UseWideChar)
		for (unsigned int i = 0; i < size; i++)
			reinterpret_cast<wchar_t&>(_Data[i * 2]) = (_Data[i * 2] << 8) | _Data[(i * 2) + 1];
}

void esString::LinkCopyInto(esString &input)const
{
	input.size = size;
	input.capacity = capacity;
	input.Flags = Flags;
	input.Flags.IsLinked = true;
	input._Data = _Data;
}

void esString::ReceiveLinkCopy(const esString &input)
{
	size = input.size;
	capacity = input.capacity;
	Flags = input.Flags;
	Flags.IsLinked = true;
	_Data = input._Data;
}




