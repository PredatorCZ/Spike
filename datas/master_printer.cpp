/*  a source for MasterPrinter class
    more info in README for PreCore Project

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

#include "masterprinter.hpp"
#include "flags.hpp"
#include "tchar.hpp"
#include <mutex>
#include <thread>
#include <vector>

#ifndef _TCHAR_DEFINED
#ifdef _UNICODE
#define tprintf swprintf
#else
#define tprintf snprintf
#endif
#endif

#ifdef _MSC_VER
#include <Windows.h>
#endif



enum MSVC_Console_Flags {
  MSC_Text_Blue,
  MSC_Text_Green,
  MSC_Text_Red,
  MSC_Text_Intensify,
};

typedef esFlags<short, MSVC_Console_Flags> consoleColorAttrFlags;

static struct MasterPrinter {
  struct FuncType {
    void *func;
    bool useColor;

    FuncType(void *fp, bool cl) : func(fp), useColor(cl) {}
  };
  std::vector<FuncType> functions;
  std::vector<bool> functionUseColor;
  std::mutex _mutexPrint;
  bool printThreadID = false;
  consoleColorAttrFlags consoleColorAttr;
} __MasterPrinter;

template <class _Func>
void SetConsoleTextColor(_Func fc, int red, int green, int blue) {
#ifdef _MSC_VER
  consoleColorAttrFlags newFlags = __MasterPrinter.consoleColorAttr;
  const bool intesify = (red | green | blue) > 128;

  if (intesify) {
    red -= 127;
    green -= 127;
    blue -= 127;
  }

  newFlags.Set(MSC_Text_Intensify, intesify);
  newFlags.Set(MSC_Text_Red, red > 0);
  newFlags.Set(MSC_Text_Green, green > 0);
  newFlags.Set(MSC_Text_Blue, blue > 0);
  

  SetConsoleTextAttribute(
      GetStdHandle(STD_OUTPUT_HANDLE),
      reinterpret_cast<consoleColorAttrFlags::ValueType &>(newFlags));
#else
  char buffer[32] = {};
  sprintf(buffer, "\033[38;2;%i;%i;%im", red, green, blue);
  fc(buffer);
#endif
}

template <class _Func> void RestoreConsoleTextColor(_Func fc) {
#ifdef _MSC_VER
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                          reinterpret_cast<consoleColorAttrFlags::ValueType &>(
                              __MasterPrinter.consoleColorAttr));
#else
  fc("\033[0m");
#endif
}
void MasterPrinterThread::AddPrinterFunction(void *funcPtr, bool useColor) {
  for (auto &c : __MasterPrinter.functions)
    if (c.func == funcPtr)
      return;
  __MasterPrinter.functions.emplace_back(funcPtr, useColor);
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

  for (auto &f : __MasterPrinter.functions) {
    int (*Func)(const TCHAR *) =
        reinterpret_cast<int (*)(const TCHAR *)>(f.func);

    if (__MasterPrinter.printThreadID) {
      Func(_T("Thread[0x"));
      std::thread::id threadID = std::this_thread::get_id();
      char buffer[65];
      printf(buffer, 65, "%X",
              reinterpret_cast<uint32 &>(threadID));
      Func(ToTSTRING(buffer).c_str());
      Func(_T("] "));
    }

    if (f.useColor) {
      if (cType == MPType::WRN) {
        SetConsoleTextColor(255, 255, 0);
      } else if (cType == MPType::ERR) {
        SetConsoleTextColor(255, 0, 0);
      }
    }

    if (cType == MPType::WRN) {
      Func(_T("WARNING: "));
    } else if (cType == MPType::ERR) {
      Func(_T("ERROR: "));
    }

    Func(ToTSTRING(tempOut).c_str());

    if (f.useColor && cType != MPType::MSG)
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

MasterPrinterThread::MasterPrinterThread() {
#ifdef _MSC_VER
  CONSOLE_SCREEN_BUFFER_INFO conInfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
  __MasterPrinter.consoleColorAttr =
      conInfo.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN |
                             FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
}
MasterPrinterThread::~MasterPrinterThread() {
}

void SetConsoleTextColor(int red, int green, int blue) {
#ifdef _MSC_VER
  SetConsoleTextColor(nullptr, red, green, blue);
#else
  for (auto &f : __MasterPrinter.functions) {
    if (!f.useColor)
      continue;

    int (*Func)(const TCHAR *) =
        reinterpret_cast<int (*)(const TCHAR *)>(f.func);

    SetConsoleTextColor(Func, red, green, blue);
  }
#endif
}

void RestoreConsoleTextColor() {
#ifdef _MSC_VER
  SetConsoleTextColor(nullptr, 0x1f, 0x1f, 0x1f);
#else
  for (auto &f : __MasterPrinter.functions) {
    if (!f.useColor)
      continue;

    int (*Func)(const TCHAR *) =
        reinterpret_cast<int (*)(const TCHAR *)>(f.func);

    RestoreConsoleTextColor(Func);
  }
#endif
}