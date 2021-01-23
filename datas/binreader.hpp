/*  class for reading data from a file

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
#include "binreader_stream.hpp"
#include "except.hpp"
#include "internal/bincore_file.hpp"

class BinReader : public BinReaderRef,
                  public BinStreamFile<std::ios::binary | std::ios::in> {
  template <class C> void OpenFile(const C fileName) {
    if (!this->Open_(fileName)) {
      throw es::FileNotFoundError(fileName);
    }

    this->baseStream = &this->FileStream;
  }

public:
  BinReader() = default;
  BinReader(const std::string &filePath) {
    OpenFile<decltype(filePath)>(filePath);
  }
  BinReader(const char *filePath) { OpenFile(filePath); }
  BinReader(const BinReader &) = delete;
  BinReader(BinReader &&) = default;
  BinReader &operator=(const BinReader &other) = delete;

  void Open(const std::string &filePath) { OpenFile(filePath); }
  void Open(const char *filePath) { OpenFile(filePath); }
};
