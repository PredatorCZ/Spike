/*  a posix source for MasterPrinter class

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

template <class _Func>
void SetConsoleTextColor(_Func fc, int red, int green, int blue) {
  char buffer[32] = {};
  sprintf(buffer, "\033[38;2;%i;%i;%im", red, green, blue);
  fc(buffer);
}

template <class _Func> void RestoreConsoleTextColor(_Func fc) { fc("\033[0m"); }

void SetConsoleTextColor(int red, int green, int blue) {
  for (auto &[func, useColor] : MASTER_PRINTER.functions) {
    if (!useColor)
      continue;

    SetConsoleTextColor(func, red, green, blue);
  }
}

void RestoreConsoleTextColor() {
  for (auto &[func, useColor] : MASTER_PRINTER.functions) {
    if (!useColor)
      continue;

    RestoreConsoleTextColor(func);
  }
}

MasterPrinter::MasterPrinter() = default;
