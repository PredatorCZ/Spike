/*  traits class for reading data from istream

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
#include "binreader_ref.hpp"
#include <istream>

class BinStreamInTraits : public BinSteamEndian {
public:
  typedef std::istream StreamType;
  typedef std::ios_base::seekdir seekdir;
  static const auto beg = std::ios_base::beg;
  static const auto cur = std::ios_base::cur;
  static const auto end = std::ios_base::end;

protected:
  StreamType *baseStream;
  BinStreamInTraits() noexcept : baseStream(nullptr) {}
  BinStreamInTraits(StreamType &stream) noexcept : baseStream(&stream) {}

public:
  size_t Tell() const { return static_cast<size_t>(baseStream->tellg()); }

  void Seek(size_t position,
            std::ios_base::seekdir vay = std::ios_base::beg) const {
    baseStream->seekg(position, vay);
  }

  void Skip(int length) const {
    if (length < 0)
      Seek(Tell() + length);
    else
      Seek(static_cast<size_t>(length), std::ios_base::cur);
  }

  void Read(char *buffer, size_t size) const { baseStream->read(buffer, size); }

  bool IsEOF() const { return baseStream->eof(); }
};

typedef BinReaderRef_t<BinStreamInTraits> BinReaderRef;
