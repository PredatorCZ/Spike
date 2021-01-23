/*  MasterPrinter class is multithreaded logging service
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

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

#define printerror(...) printer << MPType::ERR << __VA_ARGS__ >> 1;
#define printwarning(...) printer << MPType::WRN << __VA_ARGS__ >> 1;
#define printline(...) printer << __VA_ARGS__ >> 1;

enum class MPType { MSG, WRN, ERR, INF };

thread_local static class MasterPrinterThread {
  std::stringstream _masterstream;
  int maximumStreamSize = 2048;
  MPType cType = MPType::MSG;

public:
  typedef void (*print_func)(const char *c);

  template <class C> MasterPrinterThread &operator<<(const C input) {
    _masterstream << input;
    return *this;
  }

  void AddPrinterFunction(print_func func, bool useColor = true);
  void FlushAll();
  void operator>>(int endWay);
  void PrintThreadID(bool yn);
  void Locale(const char *localeName) {
    _masterstream.imbue(std::locale(localeName));
  }
  void Locale(const std::locale &loc) { _masterstream.imbue(loc); }
  MasterPrinterThread();
  ~MasterPrinterThread() = default;
} printer;

template <>
inline MasterPrinterThread &
MasterPrinterThread::operator<<(const MPType input) {
  cType = input;
  return *this;
}

void SetConsoleTextColor(int red, int green, int blue);
void RestoreConsoleTextColor();
