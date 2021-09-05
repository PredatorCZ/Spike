/*  a source for MasterPrinter class

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

#include "master_printer.hpp"
#include "flags.hpp"
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace es::print;

enum MSVC_Console_Flags {
  MSC_Text_Blue,
  MSC_Text_Green,
  MSC_Text_Red,
  MSC_Text_Intensify,
};

typedef es::Flags<MSVC_Console_Flags> consoleColorAttrFlags;

static struct MasterPrinter {
  struct FuncType {
    print_func func;
    bool useColor;

    FuncType(print_func fp, bool cl) : func(fp), useColor(cl) {}
  };

  MasterPrinter();

  std::vector<FuncType> functions;
  std::stringstream buffer;
  std::mutex mutex;
  std::thread::id lockedThread;
  bool printThreadID = false;
  MPType cType = MPType::MSG;
  consoleColorAttrFlags consoleColorAttr;
} MASTER_PRINTER;

#if defined(_MSC_VER) or defined(__MINGW64__)
#include "internal/master_printer_win.inl"
#else
#include "internal/master_printer_ix.inl"
#endif

namespace es::print {

void AddPrinterFunction(print_func func, bool useColor) {
  for (auto &c : MASTER_PRINTER.functions)
    if (c.func == func)
      return;
  MASTER_PRINTER.functions.emplace_back(func, useColor);
}

std::ostream &Get(MPType type) {
  if (auto threadID = std::this_thread::get_id();
      !(MASTER_PRINTER.lockedThread == threadID)) {
    MASTER_PRINTER.mutex.lock();
    MASTER_PRINTER.lockedThread = threadID;
  }

  if (type != MPType::PREV) {
    MASTER_PRINTER.cType = type;
  }
  return MASTER_PRINTER.buffer;
}

void FlushAll() {
  for (auto &fc : MASTER_PRINTER.functions) {
    if (fc.useColor) {
      if (MASTER_PRINTER.cType == MPType::WRN) {
        SetConsoleTextColor(255, 255, 0);
      } else if (MASTER_PRINTER.cType == MPType::ERR) {
        SetConsoleTextColor(255, 0, 0);
      } else if (MASTER_PRINTER.cType == MPType::INF) {
        SetConsoleTextColor(0, 180, 255);
      }
    }

    if (MASTER_PRINTER.printThreadID) {
      fc.func("Thread[0x");
      std::thread::id threadID = std::this_thread::get_id();
      char buffer[65];
      snprintf(buffer, 65, "%X", reinterpret_cast<uint32 &>(threadID));
      fc.func(buffer);
      fc.func("] ");
    }

    if (MASTER_PRINTER.cType == MPType::WRN) {
      fc.func("WARNING: ");
    } else if (MASTER_PRINTER.cType == MPType::ERR) {
      fc.func("ERROR: ");
    }

    auto &&tmpData = MASTER_PRINTER.buffer.str();

    fc.func(tmpData.data());

    if (fc.useColor && MASTER_PRINTER.cType != MPType::MSG) {
      RestoreConsoleTextColor();
    }
  }

  MASTER_PRINTER.buffer.str("");
  MASTER_PRINTER.cType = MPType::MSG;
  MASTER_PRINTER.mutex.unlock();
  MASTER_PRINTER.lockedThread = {};
}

void PrintThreadID(bool yn) { MASTER_PRINTER.printThreadID = yn; }
} // namespace es::print
