/*  class for writing data to a file

    Copyright 2018-2022 Lukas Cone

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

template <BinCoreOpenMode MODE>
class BinWritter_t : public BinStreamFile<MODE | BinCoreOpenMode::Out>,
                   public BinWritterRef {
  using base_file = BinStreamFile<MODE | BinCoreOpenMode::Out>;
  template <class C> void OpenFile(const C filePath) {
    if (!this->Open_(filePath)) {
      throw es::FileInvalidAccessError(filePath);
    }

    this->baseStream = &this->fileStream;
  }

public:
  BinWritter_t() = default;
  BinWritter_t(const std::string &filePath) {
    OpenFile<decltype(filePath)>(filePath);
  }
  BinWritter_t(const char *filePath) { OpenFile(filePath); }
  BinWritter_t(const BinWritter &rd) = delete;
  BinWritter_t(BinWritter_t &&o)
      : base_file(std::move(o)), BinWritterRef(std::move(o)) {
    this->baseStream = &this->fileStream;
  }
  BinWritter_t &operator=(const BinWritter_t &) = delete;
  BinWritter_t &operator=(BinWritter_t &&o) {
    static_cast<base_file &>(*this) = std::move(o);
    static_cast<BinWritterRef &>(*this) = std::move(o);
    this->baseStream = &this->fileStream;
    return *this;
  }

  void Open(const std::string &filePath) { OpenFile(filePath); }
  void Open(const char *filePath) { OpenFile(filePath); }
};
