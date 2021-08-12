/*  SettingsManager class with file logger

    Copyright 2019-2021 Lukas Cone

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
#include "datas/binwritter.hpp"

#include <chrono>
#include <ctime>
#include <locale>
#include <thread>

template <class Base> struct SettingsManager : public ReflectorBase<Base> {
  using stream_type =  BinWritter<BinCoreOpenMode::Text>;
  static stream_type &GetStream() {
    static stream_type outStream;
    return outStream;
  }
  static std::ostream &GetLogger() {
    return GetStream().BaseStream();
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
    logName.append(std::to_string(dateBuffer));
    logName.append(".txt");
    GetStream().Open(logName);
    es::print::AddPrinterFunction(SettingsManager::printf, false);

    _tcsftime(dateBuffer, dateBufferSize, _T("%c %Z"), &timeStruct);

    GetLogger() << "Current time: " << std::to_string(dateBuffer) << std::endl;
    GetLogger() << "Number of concurrent threads: "
                << std::thread::hardware_concurrency() << std::endl;
    GetLogger() << "Configuration:" << std::endl;

    const size_t numSettings = this->GetNumReflectedValues();
    Reflector::KVPairFormat prSettings;
    prSettings.aliasName = true;

    for (size_t t = 0; t < numSettings; t++) {
      Reflector::KVPair pair = this->GetReflectedPair(t, prSettings);
      GetLogger() << '\t' << pair.name << ": " << pair.value.c_str()
                  << std::endl;
    }

    GetLogger() << std::endl;
  }

  void GetHelp(std::ostream &str, const reflectorStatic *ref,
               size_t level = 1) {
    auto fillIndent = [&](size_t mod = 0) -> std::ostream & {
      for (size_t i = 0; i < level + mod; i++) {
        str << '\t';
      }

      return str;
    };

    for (size_t i = 0; i < ref->nTypes; i++) {
      es::string_view elName = ref->typeNames[i];
      auto elDesc = ref->typeDescs[i];
      fillIndent() << elName << std::endl;

      if (!elDesc.part1.empty()) {
        fillIndent(1) << elDesc.part1 << std::endl;
      }

      auto fl = ref->types[i];

      if (fl.type == REFType::Class || fl.type == REFType::BitFieldClass ||
          ((fl.type == REFType::Array || fl.type == REFType::ArrayClass) &&
           (fl.subType == REFType::Class ||
            fl.subType == REFType::BitFieldClass))) {
        GetHelp(str, REFSubClassStorage.at(fl.typeHash), level + 1);
      } else if (fl.type == REFType::Enum) {
        auto refEnum = REFEnumStorage.at(fl.typeHash);
        fillIndent(1) << "Values: ";

        for (auto r : refEnum) {
          str << r << ", ";
        }

        str << std::endl;
      }
    }
  }

  void GetHelp(std::ostream &str) {
    auto ref = GetReflectedClass<Base>();
    str << ref->className << " settings." << std::endl;
    GetHelp(str, ref);
  }
};
