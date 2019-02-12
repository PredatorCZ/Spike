/*	MasterPrinter class is multithreaded logging service
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
#include <sstream>
#ifndef _TCHAR_DEFINED
#ifdef _UNICODE
#define __T(x)      L ## x
typedef wchar_t     TCHAR;
#else
#define __T(x)      x
typedef char     TCHAR;
#endif
#endif
#define _T(x)       __T(x)
#define printerror(x, ...) printer << _T("ERROR: ") << _T(x) __VA_ARGS__>> 1;
#define printwarning(x, ...) printer << _T("WARNING: ") << _T(x) __VA_ARGS__>> 1;
#define printline(x, ...) printer << _T(x) __VA_ARGS__>> 1;

thread_local static class MasterPrinterThread
{
	typedef std::basic_stringstream<TCHAR, std::char_traits<TCHAR>> MasterStreamType;
	MasterStreamType *_masterstream;
	int maximumStreamSize = 2048;
public:
	typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> StringType;
	template<class C> MasterPrinterThread & operator << (const C input)
	{
		(*_masterstream) << input;
		return *this;
	}
	void AddPrinterFunction(void *funcPtr);
	void FlushAll();
	void operator >> (int endWay);
	void PrintThreadID(bool yn);
	MasterPrinterThread();
	~MasterPrinterThread();
}printer;
