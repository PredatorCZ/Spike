/*  class for writing data to a file

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
#include "binwritter_stream.hpp"
#include "except.hpp"
#include "internal/bincore_file.hpp"

template<BinCoreOpenMode MODE>
class BinWritter : public BinWritterRef,
                   public BinStreamFile<MakeOpenMode(MODE) | std::ios::out> {
  template <class C> void OpenFile(const C filePath) {
    if (!this->Open_(filePath)) {
      throw es::FileInvalidAccessError(filePath);
    }

    this->baseStream = &this->FileStream;
  }

public:
  BinWritter() = default;
  BinWritter(const std::string &filePath) {
    OpenFile<decltype(filePath)>(filePath);
  }
  BinWritter(const char *filePath) { OpenFile(filePath); }

  BinWritter(const BinWritter &rd) = delete;
  BinWritter(BinWritter &&) = default;
  BinWritter &operator=(const BinWritter &other) = delete;

  void Open(const std::string &filePath) { OpenFile(filePath); }
  void Open(const char *filePath) { OpenFile(filePath); }
};
