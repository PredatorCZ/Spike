/*  Block Compression Decoder
    more info in README for PreCore Project

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
#include "datas/vectors.hpp"
#include <climits>
#include <cmath>

void DecodeBC1Block(const char *data, char *obuffer, uint32 w, uint32 h,
                    uint32 width);
void DecodeBC1BlockA(const char *data, char *obuffer, uint32 w, uint32 h,
                     uint32 width);
void DecodeBC2Block(const char *data, char *obuffer, uint32 w, uint32 h,
                    uint32 width);
void DecodeBC4Block(const char *data, char *obuffer, uint32 w, uint32 h,
                    uint32 width);
void DecodeBC5Block(const char *data, char *obuffer, uint32 w, uint32 h,
                    uint32 width);
void DecodeBC5BlockGA(const char *data, char *obuffer, uint32 w, uint32 h,
                      uint32 width);
void DecodeBC3Block(const char *data, char *obuffer, uint32 w, uint32 h,
                    uint32 width);

inline void DecodeRGB565Block(const char *data, Vector &color) {
  const short &col = reinterpret_cast<const short &>(*data);
  color.X = (col & 0x1f) * (1.0f / 31.0f);
  color.Y = ((col & 0x7e0) >> 5) * (1.0f / 63.0f);
  color.Z = ((col & 0xf800) >> 11) * (1.0f / 31.0f);
}

inline void ComputeBC5Blue(char *buffer, uint32 size) {
  for (uint32 p = 0; p < size; p += 3) {
    const Vector2 RG(
        (reinterpret_cast<UCVector2 *>(buffer + p + 1)->Convert<float>() *
         (2.0f / UCHAR_MAX)) -
        1.f);

    float zcomp = RG.Dot(RG);

    if (zcomp > 1.f)
      zcomp = 1.f;
    else if (zcomp < 0.f)
      zcomp = 0.f;

    zcomp = sqrtf(1.0f - zcomp);

    *(buffer + p) = static_cast<char>((zcomp + 1.f) * SCHAR_MAX);
  }
}
