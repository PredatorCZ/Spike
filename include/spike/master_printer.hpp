/*  MasterPrinter class is multithreaded logging service

    Copyright 2018-2023 Lukas Cone

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
#include "util/settings.hpp"
#include <ostream>
#include <string>

#define printerror(...)                                                        \
  {                                                                            \
    es::print::Get(es::print::MPType::ERR) << __VA_ARGS__ << std::endl;        \
    es::print::FlushAll();                                                     \
  }
#define printwarning(...)                                                      \
  {                                                                            \
    es::print::Get(es::print::MPType::WRN) << __VA_ARGS__ << std::endl;        \
    es::print::FlushAll();                                                     \
  }
#define printline(...)                                                         \
  {                                                                            \
    es::print::Get(es::print::MPType::MSG) << __VA_ARGS__ << std::endl;        \
    es::print::FlushAll();                                                     \
  }

#define printinfo(...)                                                         \
  {                                                                            \
    es::print::Get(es::print::MPType::INF) << __VA_ARGS__ << std::endl;        \
    es::print::FlushAll();                                                     \
  }

namespace es::print {
using print_func = void (*)(const char *);
enum class MPType { PREV, MSG, WRN, ERR, INF };
struct Queuer {
  std::string payload;
  MPType type;
  uint32 threadId;
};

using queue_func = void (*)(const Queuer &);

// Calling this will lock other threads that will try to access stream until
// FlushAll is called!
std::ostream PC_EXTERN &Get(MPType type = MPType::PREV);
void PC_EXTERN AddPrinterFunction(print_func func, bool useColor = true);
void PC_EXTERN AddQueuer(queue_func func);
// Unlocks other threads access to Get
void PC_EXTERN FlushAll();
void PC_EXTERN PrintThreadID(bool yn);

template <class... C> void Print(es::print::MPType type, C... args) {
  auto &printStream = es::print::Get(type);
  // todo?, add detectors and to_string converters
  ((printStream << std::forward<C>(args)), ...) << '\n';

  es::print::FlushAll();
}

} // namespace es::print

template <class... C> void PrintInfo(C... args) {
  es::print::Print(es::print::MPType::INF, std::forward<C>(args)...);
}

template <class... C> void PrintError(C... args) {
  es::print::Print(es::print::MPType::ERR, std::forward<C>(args)...);
}

template <class... C> void PrintWarning(C... args) {
  es::print::Print(es::print::MPType::WRN, std::forward<C>(args)...);
}

template <class... C> void PrintLine(C... args) {
  es::print::Print(es::print::MPType::MSG, std::forward<C>(args)...);
}
