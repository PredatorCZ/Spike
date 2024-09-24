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
#include <functional>

#define printerror(...)                                                        \
  {                                                                            \
    es::print::PrintFn(es::print::MPType::ERR, [&](std::ostream &str) {        \
      str << __VA_ARGS__ << std::endl;                                         \
    });                                                                        \
  }
#define printwarning(...)                                                      \
  {                                                                            \
    es::print::PrintFn(es::print::MPType::WRN, [&](std::ostream &str) {        \
      str << __VA_ARGS__ << std::endl;                                         \
    });                                                                        \
  }
#define printline(...)                                                         \
  {                                                                            \
    es::print::PrintFn(es::print::MPType::MSG, [&](std::ostream &str) {        \
      str << __VA_ARGS__ << std::endl;                                         \
    });                                                                        \
  }

#define printinfo(...)                                                         \
  {                                                                            \
    es::print::PrintFn(es::print::MPType::INF, [&](std::ostream &str) {        \
      str << __VA_ARGS__ << std::endl;                                         \
    });                                                                        \
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
using stream_func = std::function<void(std::ostream &)>;

void PC_EXTERN PrintFn(MPType type, stream_func fn);
void PC_EXTERN AddPrinterFunction(print_func func, bool useColor = true);
void PC_EXTERN AddQueuer(queue_func func);
void PC_EXTERN PrintThreadID(bool yn);

template <class... C> void Print(es::print::MPType type, C... args) {
  // todo?, add detectors and to_string converters
  PrintFn(type, [&](std::ostream &str) {
    ((str << std::forward<C>(args)), ...) << '\n';
  });
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
