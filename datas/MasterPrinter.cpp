/*	a source for MasterPrinter class
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

#include "MasterPrinter.hpp"
#include <mutex>
#include <vector>
#include <tchar.h>
static struct MasterPrinter
{
	std::vector<void*> functions;
	std::mutex _mutexPrint;
	bool printThreadID = false;
}__MasterPrinter;

void MasterPrinterThread::AddPrinterFunction(void *funcPtr)
{
	for (auto &c : __MasterPrinter.functions)
		if (c == funcPtr)
			return;
	__MasterPrinter.functions.push_back(funcPtr);
}

void MasterPrinterThread::FlushAll()
{
	const size_t buffsize = static_cast<size_t>(_masterstream->tellp()) + 1;
	_masterstream->seekp(0);
	TCHAR *tempOut = static_cast<TCHAR*>(malloc(buffsize * sizeof(TCHAR)));
	_masterstream->read(tempOut, buffsize);

	if (buffsize > static_cast<size_t>(maximumStreamSize))
		_masterstream->str(StringType(_T("")));

	_masterstream->clear();
	_masterstream->seekg(0);

	std::lock_guard<std::mutex> guard(__MasterPrinter._mutexPrint);
	for (auto &f : __MasterPrinter.functions)
	{
		int(*Func)(TCHAR*) = reinterpret_cast<int(*)(TCHAR*)>(f);
		if (__MasterPrinter.printThreadID)
		{
			Func(_T("Thread[0x"));
			std::thread::id threadID = std::this_thread::get_id();
			TCHAR buffer[65];
			_itot_s(reinterpret_cast<_Thrd_id_t&>(threadID), buffer, 65, 16);
			Func(buffer);
			Func(_T("] "));
		}
		Func(tempOut);
	}
	free(tempOut);
}

void MasterPrinterThread::operator >> (int endWay)
{
	if (endWay)
		(*_masterstream) << std::endl;
	(*_masterstream) << std::ends;
	FlushAll();
}

void MasterPrinterThread::PrintThreadID(bool yn)
{
	__MasterPrinter.printThreadID = yn;
}

MasterPrinterThread::MasterPrinterThread() :_masterstream(new MasterStreamType()) {};
MasterPrinterThread::~MasterPrinterThread()
{
	if (_masterstream)
		delete _masterstream;
}
