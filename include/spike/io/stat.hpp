/*  make directories and stuff

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

#pragma once
#include "fileinfo.hpp"
#include "spike/util/settings.hpp"
#include "spike/util/unicode.hpp"
#include <set>

enum FileType_e {
  Error,
  Invalid,
  Directory,
  File,
};

#if defined(_MSC_VER) || defined(__MINGW64__)
#define MKDIR_EXTERN_ PC_EXTERN
#define USEWIN
#else
#define MKDIR_EXTERN_
#endif

FileType_e PC_EXTERN FileType(const std::string &path);

namespace es {
int MKDIR_EXTERN_ mkdir(const char *path, uint32 mode = 0777);
int MKDIR_EXTERN_ mkdir(const std::string &path, uint32 mode = 0777);
std::string GetTempFilename();
void RemoveFile(const std::string &path);

// Setup stdout handle to support utf8 and virtual env
// Win only
void MKDIR_EXTERN_ SetupWinApiConsole();
void MKDIR_EXTERN_ SetDllRunPath(const std::string &folder);

struct MappedFile {
  void *data = nullptr;
  size_t mappedSize = 0;
  size_t fileSize = 0;
  union {
    int64 fd = -1;
    void *hdl;
  };

  // When allocSize = 0, opened in read only state
  PC_EXTERN
  MappedFile(const std::string &path, size_t allocSize = 0);
  MappedFile() = default;
  MappedFile(const MappedFile &) = delete;
  MappedFile(MappedFile &&other)
      : data(other.data), fileSize(other.fileSize), fd(other.fd) {
    other.data = nullptr;
    other.fd = -1;
  }

  // File size cannot be decreased
  // Win only: data member can be a new address
  void PC_EXTERN ReserveFileSize(size_t newSize);

  MappedFile &operator=(MappedFile &&other) {
    data = other.data;
    fileSize = other.fileSize;
    fd = other.fd;
    other.data = nullptr;
    other.fd = -1;
    return *this;
  }
  PC_EXTERN ~MappedFile();
};

} // namespace es

#undef MKDIR_EXTERN_

#ifndef USEWIN
#include <sys/stat.h>

namespace es {
inline int mkdir(const char *path, uint32 mode) { return ::mkdir(path, mode); }

inline int mkdir(const std::string &path, uint32 mode) {
  return ::mkdir(path.data(), mode);
}

inline std::string GetTempFilename() { return std::tmpnam(nullptr); }
inline void RemoveFile(const std::string &path) {
  if (std::remove(path.data())) {
    throw std::runtime_error("Cannot remove file: " + path);
  }
}

inline void SetupWinApiConsole() {}
inline void SetDllRunPath(const std::string &) {}
} // namespace es
#else
namespace es {
inline std::string GetTempFilename() { return es::ToUTF8(_wtmpnam(nullptr)); }

inline void RemoveFile(const std::string &path) {
  auto wpath = es::ToUTF1632(path);
  if (_wremove(wpath.data())) {
    throw std::runtime_error("Cannot remove file: " + path);
  }
}
} // namespace es
#endif

template <class storage, class getfile>
void mkdirs(const std::string &inFilepath, const storage &files, getfile &&fc) {
  std::set<std::string> tree;

  for (auto &f : files) {
    AFileInfo cfleWrap(fc(f));
    auto cfle = cfleWrap.GetFullPath();

    for (auto it = cfle.begin(); it != cfle.end(); it++) {
      if (*it == '/') {
        tree.emplace(cfle.begin(), it);
      }
    }
  }

  for (auto &f : tree) {
    auto genFolder = inFilepath + f;
    es::mkdir(genFolder);
  }
}

inline void mkdirs(const std::string &inFilepath) {
  AFileInfo cfleWrap(inFilepath);
  auto cfle = cfleWrap.GetFullPath();

  for (auto it = cfle.begin(); it != cfle.end(); it++) {
    if (*it == '/') {
      auto genFolder = std::string(cfle.begin(), it);
      es::mkdir(genFolder);
    }
  }
}
