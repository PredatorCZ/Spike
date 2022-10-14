/*  a posix source for stat

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
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

FileType_e FileType(const std::string &path) {
  struct stat s;
  if (!stat(path.data(), &s)) {
    if (s.st_mode & S_IFDIR) {
      return FileType_e::Directory;
    } else if (s.st_mode & S_IFREG) {
      return FileType_e::File;
    } else {
      return FileType_e::Invalid;
    }
  } else {
    return FileType_e::Error;
  }
}

namespace es {
MappedFile::MappedFile(const std::string &path) {
  fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    throw es::FileNotFoundError(path);
  }

  struct stat fileStat;
  if (fstat(fd, &fileStat) == -1) {
    throw std::runtime_error("Cannot stat file " + path);
  }

  dataSize = fileStat.st_size;
  data = mmap(nullptr, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);

  madvise(data, dataSize, MADV_RANDOM);
  madvise(data, dataSize, MADV_WILLNEED);
  madvise(data, dataSize, MADV_DONTDUMP);

  if (data == MAP_FAILED) {
    throw std::runtime_error("Cannot map file " + path);
  }
}

MappedFile::~MappedFile() {
  if (data && data != MAP_FAILED) {
    munmap(data, dataSize);
  }

  if (fd != -1) {
    close(fd);
  }
}
} // namespace es
