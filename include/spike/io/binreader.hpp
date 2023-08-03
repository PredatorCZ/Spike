/*  class for reading data from a file

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
#include "binreader_stream.hpp"
#include "detail/bincore_file.hpp"
#include "spike/except.hpp"

template <BinCoreOpenMode MODE>
class BinReader_t : public BinStreamFile<MODE>, public BinReaderRef {
  using base_file = BinStreamFile<MODE>;
  template <class C> void OpenFile(const C fileName) {
    if (!this->Open_(fileName)) {
      throw es::FileNotFoundError(fileName);
    }

    this->baseStream = &this->fileStream;
  }

public:
  BinReader_t() = default;
  BinReader_t(const std::string &filePath) {
    OpenFile<decltype(filePath)>(filePath);
  }
  BinReader_t(const char *filePath) { OpenFile(filePath); }
  BinReader_t(const BinReader_t &) = delete;
  BinReader_t(BinReader_t &&o)
      : base_file(std::move(o)), BinReaderRef(std::move(o)) {
    this->baseStream = &this->fileStream;
  }
  BinReader_t &operator=(const BinReader_t &other) = delete;
  BinReader_t &operator=(BinReader_t &&o) {
    static_cast<base_file &>(*this) = std::move(o);
    static_cast<BinReaderRef &>(*this) = std::move(o);
    this->baseStream = &this->fileStream;
    return *this;
  }

  void Open(const std::string &filePath) { OpenFile(filePath); }
  void Open(const char *filePath) { OpenFile(filePath); }
};
