/*  a Win source for MasterPrinter class

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

#include <windows.h>

template <class _Func>
void SetConsoleTextColor(_Func, int red, int green, int blue) {
  consoleColorAttrFlags newFlags = MASTER_PRINTER.consoleColorAttr;
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
}

template <class _Func> void RestoreConsoleTextColor(_Func) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                          reinterpret_cast<consoleColorAttrFlags::ValueType &>(
                              MASTER_PRINTER.consoleColorAttr));
}

void SetConsoleTextColor(int red, int green, int blue) {
  SetConsoleTextColor(nullptr, red, green, blue);
}

void RestoreConsoleTextColor() {
  SetConsoleTextColor(nullptr, 0x1f, 0x1f, 0x1f);
}

MasterPrinter::MasterPrinter() {
  CONSOLE_SCREEN_BUFFER_INFO conInfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
  MASTER_PRINTER.consoleColorAttr =
      conInfo.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN |
                             FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}


