/*  PS3 memory address calculator

    Copyright 2021 Lukas Cone

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
#include <cmath>
#include <cstddef>

struct MortonSettings {
  size_t width;
  size_t height;
  size_t xmask[10]{};
  size_t ymask[10]{};

  MortonSettings(size_t width_, size_t height_)
      : width(width_), height(height_) {
    const size_t bitOffsetW = log2(width);
    const size_t bitOffsetH = log2(height);
    const size_t minBits = bitOffsetW > bitOffsetH ? bitOffsetH : bitOffsetW;

    for (size_t b = 4; b < minBits; b++) {
      xmask[b - 4] = size_t(1) << b;
      ymask[b - 4] = size_t(1) << b;
    }

    if (bitOffsetW != bitOffsetH) {
      const size_t maskW = width - 1;
      const size_t maskH = height - 1;
      const size_t mask = maskH ^ maskW;

      if (bitOffsetW > bitOffsetH) {
        xmask[bitOffsetH - 4] = mask;
      } else {
        ymask[bitOffsetW - 4] = mask;
      }
    }
  }
};

size_t MortonAddr(size_t x, size_t y, const MortonSettings &settings) {
  const size_t x0 = x & 1;
  const size_t x1 = (x & 2) << 1;
  const size_t x2 = (x & 4) << 2;
  const size_t x3 = (x & 8) << 3;
  const size_t x4 = (x & settings.xmask[0]) << 4;
  const size_t x5 = (x & settings.xmask[1]) << 5;
  const size_t x6 = (x & settings.xmask[2]) << 6;
  const size_t x7 = (x & settings.xmask[3]) << 7;

  const size_t y0 = (y & 1) << 1;
  const size_t y1 = (y & 2) << 2;
  const size_t y2 = (y & 4) << 3;
  const size_t y3 = (y & 8) << 4;
  const size_t y4 = (y & settings.ymask[0]) << 5;
  const size_t y5 = (y & settings.ymask[1]) << 6;
  const size_t y6 = (y & settings.ymask[2]) << 7;
  const size_t y7 = (y & settings.ymask[3]) << 8;

  size_t retval = x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7 | y0 | y1 | y2 | y3 |
                  y4 | y5 | y6 | y7;

  if (settings.xmask[4] || settings.ymask[4]) {
    const size_t x8 = (x & settings.xmask[4]) << 8;
    const size_t x9 = (x & settings.xmask[5]) << 9;
    const size_t x10 = (x & settings.xmask[6]) << 10;

    const size_t y8 = (y & settings.ymask[4]) << 9;
    const size_t y9 = (y & settings.ymask[5]) << 10;
    const size_t y10 = (y & settings.ymask[6]) << 11;
    retval |= x8 | x9 | x10 | y8 | y9 | y10;

    if (settings.xmask[7] || settings.ymask[7]) {
      const size_t x11 = (x & settings.xmask[7]) << 11;
      const size_t x12 = (x & settings.xmask[8]) << 12;
      const size_t x13 = (x & settings.xmask[9]) << 13;

      const size_t y11 = (y & settings.ymask[7]) << 12;
      const size_t y12 = (y & settings.ymask[8]) << 13;
      const size_t y13 = (y & settings.ymask[9]) << 14;
      retval |= x11 | x12 | x13 | y11 | y12 | y13;
    }
  }

  return retval;
}
