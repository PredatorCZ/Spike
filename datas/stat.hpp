/*  make directories and stuff

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

#pragma once
#include "fileinfo.hpp"
#include "unicode.hpp"
#include <set>

enum FileType_e {
  Error,
  Invalid,
  Directory,
  File,
};

FileType_e FileType(const std::string &path);
namespace es {
int mkdir(const char *path, uint32 mode = 0777);
int mkdir(const std::string &path, uint32 mode = 0777);
} // namespace es

#ifndef _MSC_VER
#include <sys/stat.h>

namespace es {
inline int mkdir(const char *path, uint32 mode) {
  return ::mkdir(path, mode);
}

inline int mkdir(const std::string &path, uint32 mode) {
  return ::mkdir(path.data(), mode);
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
