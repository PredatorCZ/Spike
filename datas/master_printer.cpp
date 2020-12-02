/*  a source for MasterPrinter class

    Copyright 2018-2020 Lukas Cone

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

#include "flags.hpp"
#include "master_printer.hpp"
#include <mutex>
#include <thread>
#include <vector>

enum MSVC_Console_Flags {
  MSC_Text_Blue,
  MSC_Text_Green,
  MSC_Text_Red,
  MSC_Text_Intensify,
};

typedef es::Flags<MSVC_Console_Flags> consoleColorAttrFlags;

static struct MasterPrinter {
  struct FuncType {
    typename MasterPrinterThread::print_func func;
    bool useColor;

    FuncType(typename MasterPrinterThread::print_func fp, bool cl)
        : func(fp), useColor(cl) {}
  };
  std::vector<FuncType> functions;
  std::vector<bool> functionUseColor;
  std::mutex _mutexPrint;
  bool printThreadID = false;
  consoleColorAttrFlags consoleColorAttr;
} __MasterPrinter;

#ifdef _MSC_VER
#include "internal/master_printer_win.inl"
#else
#include "internal/master_printer_ix.inl"
#endif

void MasterPrinterThread::AddPrinterFunction(print_func func, bool useColor) {
  for (auto &c : __MasterPrinter.functions)
    if (c.func == func)
      return;
  __MasterPrinter.functions.emplace_back(func, useColor);
}

void MasterPrinterThread::FlushAll() {
  const size_t buffsize = static_cast<size_t>(_masterstream.tellp()) + 1;

  _masterstream.seekp(0);

  char *tempOut = static_cast<char *>(malloc(buffsize));

  _masterstream.read(tempOut, buffsize);

  if (buffsize > static_cast<size_t>(maximumStreamSize))
    _masterstream.str("");

  _masterstream.clear();
  _masterstream.seekg(0);

  std::lock_guard<std::mutex> guard(__MasterPrinter._mutexPrint);

  for (auto &fc : __MasterPrinter.functions) {
    if (__MasterPrinter.printThreadID) {
      fc.func("Thread[0x");
      std::thread::id threadID = std::this_thread::get_id();
      char buffer[65];
      snprintf(buffer, 65, "%X", reinterpret_cast<uint32 &>(threadID));
      fc.func(buffer);
      fc.func("] ");
    }

    if (fc.useColor) {
      if (cType == MPType::WRN) {
        SetConsoleTextColor(255, 255, 0);
      } else if (cType == MPType::ERR) {
        SetConsoleTextColor(255, 0, 0);
      }
    }

    if (cType == MPType::WRN) {
      fc.func("WARNING: ");
    } else if (cType == MPType::ERR) {
      fc.func("ERROR: ");
    }

    fc.func(tempOut);

    if (fc.useColor && cType != MPType::MSG)
      RestoreConsoleTextColor();
  }

  cType = MPType::MSG;
  free(tempOut);
}

void MasterPrinterThread::operator>>(int endWay) {
  if (endWay)
    _masterstream << std::endl;

  _masterstream << std::ends;
  FlushAll();
}

void MasterPrinterThread::PrintThreadID(bool yn) {
  __MasterPrinter.printThreadID = yn;
}
