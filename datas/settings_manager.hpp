/*  SettingsManager class with file logger

    Copyright 2019-2020 Lukas Cone

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
#include "master_printer.hpp"
#include "reflector.hpp"
#include "tchar.hpp"

#include <chrono>
#include <codecvt>
#include <ctime>
#include <fstream>
#include <locale>
#include <thread>

template<class Base>
struct SettingsManager : private ReflectorBase<Base> {
  static std::ofstream &GetLogger() {
    static std::ofstream logger;
    return logger;
  }

  static void printf(const char *str) { GetLogger() << str; }

  void CreateLog(std::string logName) {
    time_t curTime =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm timeStruct = *localtime(&curTime);
    TCHAR dateBuffer[128]{};
    const size_t dateBufferSize = sizeof(dateBuffer) / sizeof(TCHAR);

    _tcsftime(dateBuffer, dateBufferSize, _T("_%y_%m_%d-%H.%M.%S"),
              &timeStruct);
    logName.append(es::ToUTF8(dateBuffer));
    logName.append(".txt");
    GetLogger().open(ToTSTRING(logName), std::ios::out);
    printer.AddPrinterFunction(SettingsManager::printf, false);

    _tcsftime(dateBuffer, dateBufferSize, _T("%c %Z"), &timeStruct);

    GetLogger() << "Current time: " << es::ToUTF8(dateBuffer) << std::endl;
    GetLogger() << "Number of concurrent threads: "
                << std::thread::hardware_concurrency() << std::endl;
    GetLogger() << "Configuration:" << std::endl;

    const size_t numSettings = GetNumReflectedValues();
	KVPairFormat prSettings;
	prSettings.aliasName = true;

    for (int t = 0; t < numSettings; t++) {
      KVPair pair = GetReflectedPair(t, prSettings);
      GetLogger() << '\t' << pair.name << ": " << pair.value.c_str()
                  << std::endl;
    }

    GetLogger() << std::endl;
  }
};
