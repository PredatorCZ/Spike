/*  class for writing data to a file

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
#include "binwritter_stream.hpp"
#include "internal/bincore_file.hpp"

class BinWritter : public BinWritterRef,
                   public BinStreamFile<std::ios::binary | std::ios::out> {
public:
  BinWritter() = default;
  template <class T> BinWritter(const UniString<T> &filePath) {
    this->Open(filePath);
    this->baseStream = &this->FileStream;
  }
  BinWritter(const char *filePath) {
    this->Open(filePath);
    this->baseStream = &this->FileStream;
  }
  BinWritter(const wchar_t *filePath) {
    this->Open(filePath);
    this->baseStream = &this->FileStream;
  }

  BinWritter(const BinWritter &rd) = delete;
  BinWritter &operator=(const BinWritter &other) = delete;
};
