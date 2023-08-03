/*  a Win source for stat

    Copyright 2021-2023 Lukas Cone

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

#include "spike/except.hpp"
#include <direct.h>
#include <windows.h>

#undef max

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

MappedFile::MappedFile(const std::string &path, size_t allocSize) {
  auto cvted = es::ToUTF1632(path);
  auto accessFlags = GENERIC_READ | (allocSize ? GENERIC_WRITE : 0);
  auto openFlags = allocSize ? OPEN_ALWAYS : OPEN_EXISTING;

  hdl = CreateFileW(cvted.c_str(), accessFlags, FILE_SHARE_READ, NULL,
                    openFlags, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hdl == INVALID_HANDLE_VALUE) {
    throw es::FileNotFoundError(path);
  }

  fileSize = GetFileSize(hdl, NULL);
  mappedSize = std::max(fileSize, allocSize);

  if (!allocSize) {
    ReserveFileSize(0);
  } else {
    ReserveFileSize(0x1000);
  }
}

void MappedFile::ReserveFileSize(size_t newSize) {
  if (newSize && fileSize >= newSize) {
    return;
  }

  fileSize = newSize;

  if (data) {
    UnmapViewOfFile(data);
  }

  auto mappingFlags = newSize ? PAGE_READWRITE : PAGE_READONLY;

  HANDLE mapping =
      CreateFileMapping(hdl, NULL, mappingFlags, 0, newSize, NULL);

  if (!mapping) {
    auto errCode = GetLastError();
    throw std::runtime_error("WinApi call error " + std::to_string(errCode));
  }

  auto mappingViewFlags = newSize ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;

  data = MapViewOfFileEx(mapping, mappingViewFlags, 0, 0, newSize, NULL);
  CloseHandle(mapping);

  if (!data) {
    auto errCode = GetLastError();
    throw std::runtime_error("WinApi call error " + std::to_string(errCode));
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
