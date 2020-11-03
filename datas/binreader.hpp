/*  class for reading data from a file

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
#include "binreader_stream.hpp"
#include "internal/bincore_file.hpp"

class BinReader : public BinReaderRef,
                  public BinStreamFile<std::ios::binary | std::ios::in> {
public:
  BinReader() = default;
  BinReader(const std::string &filePath) {
    this->Open(filePath);
    this->baseStream = &this->FileStream;
  }
  BinReader(const char *filePath) {
    this->Open(filePath);
    this->baseStream = &this->FileStream;
  }
  BinReader(const BinReader &) = delete;
  BinReader(BinReader &&) = default;
  BinReader &operator=(const BinReader &other) = delete;
};
