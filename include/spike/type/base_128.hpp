/*  Base 128 encoder (based on LEB128)

    Copyright 2020-2023 Lukas Cone

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
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"

struct bint128 {
  int64 value;

  bint128(int64 in) : value(in) {}
  bint128() : value(0) {}

  operator int64() const { return value; }

  int64 Read(BinReaderRef rd) {
    value = 0;
    for (size_t id = 0; id < 9; id++) {
      uint8 cNum;
      rd.Read(cNum);

      if (id < 8) {
        if (!(cNum & 0x80)) {
          value |= static_cast<int64>(cNum & 0x3f) << (7 * id);

          if (cNum & 0x40) {
            value = ~value;
          }
          break;
        } else {
          value |= static_cast<int64>(cNum & 0x7f) << (7 * id);
        }
      } else {
        value |= static_cast<int64>(cNum) << (7 * id);
      }
    }

    return value;
  }

  void Write(BinWritterRef wr) const {
    // there shouldn't be a sign flag for values lower than
    // -72'057'594'037'927'936
    bool sign = value < 0 && !(~(value >> 56));
    uint64 valueCopy = sign ? ~value : value;
    uint8 lastValue = 0;
    size_t lastIndex = 0;

    while (true) {
      lastValue = static_cast<uint8>(valueCopy);
      valueCopy >>= 7;

      if (!valueCopy) {
        break;
      }

      wr.Write<uint8>(lastValue | 0x80);
      lastIndex++;
    }

    if (lastIndex < 8) {
      auto signMask = sign ? 0x40 : 0;
      if (lastValue & 0x40) {
        wr.Write<uint8>(lastValue | 0x80);
        wr.Write<uint8>(signMask);
      } else {
        wr.Write<uint8>(lastValue | signMask);
      }
    } else {
      wr.Write(lastValue);
    }
  }
};

struct buint128 {
  uint64 value;

  buint128(uint64 in) : value(in) {}
  buint128() : value(0) {}

  operator uint64() const { return value; }

  uint64 Read(BinReaderRef rd) {
    value = 0;
    for (size_t id = 0; id < 9; id++) {
      uint8 cNum;
      rd.Read(cNum);

      if (id == 8) {
        value |= static_cast<uint64>(cNum) << 56;
      } else {
        value |= static_cast<uint64>(cNum & 0x7f) << (7 * id);
      }

      if (!(cNum & 0x80)) {
        break;
      }
    }

    return value;
  }

  void Write(BinWritterRef wr) const {
    uint64 valueCopy = value;
    size_t index = 0;

    while (true) {
      const auto lastValue = static_cast<uint8>(valueCopy);
      valueCopy >>= 7;

      if (!valueCopy || index == 8) {
        wr.Write(lastValue);
        break;
      } else {
        wr.Write<uint8>(lastValue | 0x80);
      }

      index++;
    }
  }
};
