/*  Binary writter/reader file handle

    Copyright 2018-2020 Lukas Cone

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
#include <fstream>

template <std::ios_base::openmode MODE> class BinStreamFile {
  template <typename T> bool _Open(const T _fileName) {
    FileStream.open(_fileName, MODE);

    return !FileStream.fail();
  }

protected:
  std::fstream FileStream;

  void _Close() {
    if (!FileStream.fail())
      FileStream.close();
  }

public:
  bool Open(const char *_fileName) {
#ifdef UNICODE
    return _Open(es::ToUTF1632(_fileName));
#else
    return _Open(_fileName);
#endif
  }

  bool Open(const wchar_t *_fileName) {
#ifdef UNICODE
    return _Open(_fileName);
#else
    return _Open(es::ToUTF8(_fileName));
#endif
  }

  bool Open(const std::string &_fileName) {
#ifdef UNICODE
    return _Open(es::ToUTF1632(_fileName));
#else
    return _Open(_fileName);
#endif
  }

  bool Open(const std::wstring &_fileName) {
#ifdef UNICODE
    return _Open(_fileName);
#else
    return _Open(es::ToUTF8(_fileName));
#endif
  }

  bool IsValid() const { return !FileStream.fail(); }
};
