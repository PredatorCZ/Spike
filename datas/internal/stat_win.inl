/*  a Win source for stat

    Copyright 2021 Lukas Cone

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

#include "datas/except.hpp"
#include <direct.h>
#include <windows.h>

namespace es {
int mkdir(const char *path, uint32) {
#ifdef UNICODE
  auto cvted = es::ToUTF1632(path);
  return ::_wmkdir(cvted.data());
#else
  return ::_mkdir(path);
#endif
}

int mkdir(const std::string &path, uint32) {
#ifdef UNICODE
  auto cvted = es::ToUTF1632(path);
  return ::_wmkdir(cvted.data());
#else
  return ::_mkdir(path.data());
#endif
}

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
#endif

#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN 8
#endif

void SetupWinApiConsole() {
  auto checkApi = [](auto what) {
    if (!what) {
      auto errCode = GetLastError();
      throw std::runtime_error("WinApi call error " + std::to_string(errCode));
    }
  };

  auto consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

  if (!consoleHandle || consoleHandle == INVALID_HANDLE_VALUE) {
    return;
  }

  // Force utf8 codepage
  SetConsoleOutputCP(CP_UTF8);
  // Enable virtual terminal
  DWORD mode{};
  GetConsoleMode(consoleHandle, &mode);
  SetConsoleMode(consoleHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                                    DISABLE_NEWLINE_AUTO_RETURN);

  // Setup only capable font for advanced unicode characters
  CONSOLE_FONT_INFOEX infoEx{};
  infoEx.cbSize = sizeof(infoEx);
  GetCurrentConsoleFontEx(consoleHandle, false, &infoEx);
  static const wchar_t buffer[] = L"NSimSun";
  wcsncpy(infoEx.FaceName, buffer, sizeof(buffer) / sizeof(wchar_t));
  SetCurrentConsoleFontEx(consoleHandle, false, &infoEx);
}

MappedFile::MappedFile(const std::string &path) {
  auto cvted = es::ToUTF1632(path);
  hdl = CreateFileW(cvted.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hdl == INVALID_HANDLE_VALUE) {
    throw es::FileNotFoundError(path);
  }

  auto fileSize = GetFileSize(hdl, NULL);
  HANDLE mapping = CreateFileMapping(hdl, NULL, PAGE_READONLY, 0, 0, NULL);

  if (!mapping) {
    throw std::runtime_error("Cannot map file " + path);
  }

  data = MapViewOfFileEx(mapping, FILE_MAP_READ, 0, 0, fileSize, NULL);
  CloseHandle(mapping);

  if (!data) {
    throw std::runtime_error("Cannot map file " + path);
  }
}

MappedFile::~MappedFile() {
  if (data) {
    UnmapViewOfFile(data);
  }

  if (hdl != INVALID_HANDLE_VALUE) {
    CloseHandle(hdl);
  }
}

}; // namespace es

FileType_e FileType(const std::string &path) {
#ifdef UNICODE
  auto cvted = es::ToUTF1632(path);
  auto flags = GetFileAttributesW(cvted.data());
#else
  auto flags = GetFileAttributesA(path.data());
#endif

  if (flags == INVALID_FILE_ATTRIBUTES) {
    return FileType_e::Invalid;
  }

  if (flags & FILE_ATTRIBUTE_DIRECTORY) {
    return FileType_e::Directory;
  }

  return FileType_e::File;
}
