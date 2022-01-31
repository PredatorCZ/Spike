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
#include "settings.hpp"
#include <iosfwd>
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
} // namespace es::print
