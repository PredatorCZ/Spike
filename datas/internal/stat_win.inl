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

#include <Windows.h>
#include <direct.h>

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
