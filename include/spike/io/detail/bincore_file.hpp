/*  Binary writter/reader file handle

    Copyright 2018-2023 Lukas Cone

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
#include "../bincore_fwd.hpp"
#include "spike/util/unicode.hpp"

constexpr std::ios_base::openmode MakeOpenMode(BinCoreOpenMode mode) {
  std::ios_base::openmode retVal{};

  if (!(mode & BinCoreOpenMode::Text)) {
    retVal = retVal | std::ios_base::binary;
  }

  if (mode & BinCoreOpenMode::Append) {
    retVal = retVal | std::ios_base::app;
  }

  if (mode & BinCoreOpenMode::Ate) {
    retVal = retVal | std::ios_base::ate;
  }

  if (mode & BinCoreOpenMode::Truncate) {
    retVal = retVal | std::ios_base::trunc;
  }

  if (mode & BinCoreOpenMode::Out) {
    retVal = retVal | std::ios_base::out;
  } else {
    retVal = retVal | std::ios_base::in;
  }

  return retVal;
}

#ifdef __MINGW64__
#include <ext/stdio_filebuf.h>
#include <iostream>

struct MGWFileStreamBase {
  using filebuf = __gnu_cxx::stdio_filebuf<char>;
  filebuf underLying;

  MGWFileStreamBase(filebuf &&file) : underLying(std::move(file)) {}
  MGWFileStreamBase() = default;
  MGWFileStreamBase(MGWFileStreamBase&&) = default;

  MGWFileStreamBase &operator=(MGWFileStreamBase &&other) = default;
};

struct MGWFileStream : MGWFileStreamBase, std::iostream {
  using parent = std::iostream;

  MGWFileStream(filebuf &&file)
      : MGWFileStreamBase(std::move(file)), parent(&underLying) {}

  MGWFileStream() : parent(&underLying) {}

  MGWFileStream(MGWFileStream &&other)
      : MGWFileStreamBase(std::move(other)), parent(&underLying) {}

  MGWFileStream &operator=(MGWFileStream &&other) = default;

  void close() {
    fclose(underLying.file());
    underLying.close();
    clear();
  }
};

template <BinCoreOpenMode MODE> class BinStreamFile {
  static constexpr std::ios::openmode OMODE = MakeOpenMode(MODE);

protected:
  MGWFileStream fileStream;
  void Close_() {
    if (!fileStream.fail()) {
      fileStream.close();
    }
  }

  bool WOpen(const std::wstring &fileName) {
    constexpr wchar_t mode[]{(OMODE & std::ios_base::in) != 0 ? 'r' : 'w',
                             (OMODE & std::ios_base::binary) != 0 ? 'b' : 't',
                             0};

    FILE *cFile = _wfopen(fileName.data(), mode);

    if (!cFile) {
      fileStream.setstate(std::ios_base::badbit);
      return false;
    }

    const size_t bufSize =
        MODE & BinCoreOpenMode::NoBuffer ? 0 : static_cast<size_t>(BUFSIZ);

    fileStream = std::move(MGWFileStreamBase::filebuf(cFile, OMODE, bufSize));

    return !fileStream.fail();
  }

  bool Open_(const char *_fileName) { return WOpen(es::ToUTF1632(_fileName)); }

  bool Open_(const std::string &_fileName) {
    return WOpen(es::ToUTF1632(_fileName));
  }

public:
  bool IsValid() const { return !fileStream.fail(); }
};
#else
#include <fstream>
template <BinCoreOpenMode MODE> class BinStreamFile {
  static constexpr std::ios::openmode OMODE = MakeOpenMode(MODE);

protected:
  std::fstream fileStream;

  void Close_() {
    if (IsValid())
      fileStream.close();
  }

  bool Open_(const char *fileName) {
    if (MODE & BinCoreOpenMode::NoBuffer) {
      fileStream.rdbuf()->pubsetbuf(0, 0);
    }
#if defined(UNICODE) && defined(_MSC_VER)
    fileStream.open(es::ToUTF1632(fileName), OMODE);
#else
    fileStream.open(fileName, OMODE);
#endif

    return IsValid();
  }

  bool Open_(const std::string &fileName) {
    if (MODE & BinCoreOpenMode::NoBuffer) {
      fileStream.rdbuf()->pubsetbuf(0, 0);
    }
#if defined(UNICODE) && defined(_MSC_VER)
    fileStream.open(es::ToUTF1632(fileName), OMODE);
#else
    fileStream.open(fileName, OMODE);
#endif

    return IsValid();
  }

public:
  bool IsValid() const { return fileStream.rdbuf()->is_open(); }
};
#endif
