/*	SettingsManager class, use in source only
	more info in README for PreCore Project

	Copyright 2019 Lukas Cone

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
#include <chrono>
#include <ctime>
#include <codecvt>
#include <locale>
#include <fstream>
#include <tchar.h>
#include "reflector.hpp"
#include "datas/masterprinter.hpp"

struct SettingsManager : private Reflector
{
	using Reflector::FromXML;
	using Reflector::ToXML;

	static std::basic_ofstream<TCHAR, std::char_traits<TCHAR>> logger;

	static void tprintf(const TCHAR *str)
	{
		logger << str;
	}

	void CreateLog(TSTRING logName)
	{
		time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm timeStruct = {};
		localtime_s(&timeStruct, &curTime);
		TSTRING dateBuffer;
		dateBuffer.resize(31);
		_tcsftime(const_cast<TCHAR *>(dateBuffer.data()), 32, _T("_%y_%m_%d-%H.%M.%S"), &timeStruct);
		logName.append(dateBuffer.c_str());
		logName.append(_T(".txt"));
		logger.open(logName, std::ios::out);

		std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<TCHAR>());
		logger.imbue(utf8_locale);

		printer.AddPrinterFunction(tprintf);

		dateBuffer.resize(64);
		_tcsftime(const_cast<TCHAR *>(dateBuffer.data()), 64, _T("%c %Z"), &timeStruct);

		logger << "Current time: " << dateBuffer.c_str() << std::endl;
		logger << "Number of concurrent threads: " << std::thread::hardware_concurrency() << std::endl;
		logger << "Configuration:" << std::endl;

		const int numSettings = GetNumReflectedValues();

		for (int t = 0; t < numSettings; t++)
		{
			KVPair pair = GetReflectedPair(t);
			logger << '\t' << pair.name << ": " << pair.value.c_str() << std::endl;
		}

		logger << std::endl;
	}
};

std::basic_ofstream<TCHAR, std::char_traits<TCHAR>> SettingsManager::logger = {};