/*  traits class for writing data to ostream

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
#include "binwritter_ref.hpp"
#include <ostream>

class BinStreamOutTraits : public BinSteamEndian {
public:
  typedef std::ostream StreamType;
  typedef std::ios_base::seekdir seekdir;
  static const auto beg = std::ios_base::beg;
  static const auto cur = std::ios_base::cur;
  static const auto end = std::ios_base::end;

protected:
  StreamType *baseStream;
  BinStreamOutTraits() noexcept : baseStream(nullptr) {}
  BinStreamOutTraits(StreamType &stream) noexcept : baseStream(&stream) {}

public:
  size_t Tell() const { return static_cast<size_t>(baseStream->tellp()); }

  void Seek(size_t position,
            std::ios_base::seekdir vay = std::ios_base::beg) const {
    baseStream->seekp(position, vay);
  }

  void Skip(int64 length) const {
    if (length > 0) {
      static constexpr char FILLBUFFER[32] = {};
      const size_t numLoops = length / 32;

      for (size_t t = 0; t < numLoops; t++) {
        Write(FILLBUFFER, 32);
      }

      Write(FILLBUFFER, length % 32);
    } else {
      Seek(Tell() + length);
    }
  }

  void Write(const char *buffer, size_t size) const {
    baseStream->write(buffer, size);
  }

  bool IsEOF() const { return baseStream->eof(); }

  StreamType &BaseStream() { return *baseStream; }
};

typedef BinWritterRef_t<BinStreamOutTraits> BinWritterRef;
