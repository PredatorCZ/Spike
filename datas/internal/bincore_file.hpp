/*  Binary writter/reader file handle

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

#pragma once
#include "../unicode.hpp"

#ifdef __MINGW64__
#include <ext/stdio_filebuf.h>
#include <iostream>

template <std::ios_base::openmode MODE> class BinStreamFile {
  using filebuf = __gnu_cxx::stdio_filebuf<char>;
  filebuf underLying;

protected:
  std::iostream FileStream{
      static_cast<std::basic_streambuf<char> *>(&underLying)};
  void Close_() {
    if (!FileStream.fail()) {
      fclose(underLying.file());
      underLying.close();
    }
  }

  bool WOpen(const std::wstring &fileName) {
    FILE *cFile = nullptr;
    if constexpr (MODE & std::ios_base::in) {
      cFile = _wfopen(fileName.data(), L"rb");
    } else {
      cFile = _wfopen(fileName.data(), L"wb");
    }

    if (!cFile) {
      FileStream.setstate(std::ios_base::badbit);
      return false;
    }

    // TODO tweak buffer size?
    underLying = filebuf(cFile, MODE);
    new (&FileStream)
        std::iostream{static_cast<std::basic_streambuf<char> *>(&underLying)};

    return !FileStream.fail();
  }

  bool Open_(const char *_fileName) { return WOpen(es::ToUTF1632(_fileName)); }

  bool Open_(const std::string &_fileName) {
    return WOpen(es::ToUTF1632(_fileName));
  }

public:
  bool IsValid() const { return !FileStream.fail(); }
};
#else
#include <fstream>
template <std::ios_base::openmode MODE> class BinStreamFile {
protected:
  std::fstream FileStream;

  void Close_() {
    if (!FileStream.fail())
      FileStream.close();
  }

  bool Open_(const char *_fileName) {
#if defined(UNICODE) && defined(_MSC_VER)
    FileStream.open(es::ToUTF1632(_fileName), MODE);
#else
    FileStream.open(_fileName, MODE);
#endif

    return !FileStream.fail();
  }

  bool Open_(const std::string &_fileName) {
#if defined(UNICODE) && defined(_MSC_VER)
    FileStream.open(es::ToUTF1632(_fileName), MODE);
#else
    FileStream.open(_fileName, MODE);
#endif

    return !FileStream.fail();
  }

public:
  bool IsValid() const { return !FileStream.fail(); }
};
#endif
