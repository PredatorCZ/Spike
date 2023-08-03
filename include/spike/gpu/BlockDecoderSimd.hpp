/*  Block Compression SIMD Decoder

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

#include "spike/type/vectors_simd.hpp"
#include <cstring>

static inline void DecodeRGB565Blocks(const char *data, IVector4A16 &color0,
                                      IVector4A16 &color1) {
  const uint32 rawdata = reinterpret_cast<const uint32 &>(*data);
  const auto cl00 = IVector4A16(rawdata);
  const auto cl0 = cl00 * IVector4A16(0x8000000, 0x200000, 0x10000, 0);
  const auto cl1 = cl00 * IVector4A16(0x800, 0x20, 0x1, 0);

  color0 = cl0 >> 24;
  color1 = cl1 >> 24;
}

inline void _SDecodeBC1Block(const char *data, char *obuffer, uint32 w,
                             uint32 h, uint32 width,
                             const uint32 pixeloffset = 3) {
  IVector4A16 color0, color1;
  DecodeRGB565Blocks(data, color0, color1);
  data += 4;
  h *= 4;
  w *= 4;
  width *= 4;

  // Precompute colors
  IVector4A16 color2(((color0 * 0xaa) + (color1 * 0x55)) >> 8);
  IVector4A16 color3(((color0 * 0x55) + (color1 * 0xaa)) >> 8);

  static const IVector4A16 shiftMask(1, 0x100, 0x10000, 1);

  // left shift y << 8, z << 16 but with mul
  color0 = color0 * shiftMask;
  color1 = color1 * shiftMask;
  color2 = color2 * shiftMask;
  color3 = color3 * shiftMask;

  // collect shifted values, x + y + z
  const __m128i cl0 =
      _mm_hadd_epi32(_mm_hadd_epi32(color0._data, color0._data), color0._data);
  const __m128i cl1 =
      _mm_hadd_epi32(_mm_hadd_epi32(color1._data, color1._data), color1._data);
  const __m128i cl2 =
      _mm_hadd_epi32(_mm_hadd_epi32(color2._data, color2._data), color2._data);
  const __m128i cl3 =
      _mm_hadd_epi32(_mm_hadd_epi32(color3._data, color3._data), color3._data);

  // final collection, x = c10, y = c11, z = c12, w = c13
  // now we have all precomputed colors coverted to a 16x8bit matrix
  __m128i finalMix = _mm_unpacklo_epi64(_mm_unpacklo_epi32(cl0, cl1),
                                        _mm_unpacklo_epi32(cl2, cl3));

  for (uint32 row = 0; row < 4; row++) {
    const IVector4A16 cMask0(*data++);
    IVector4A16 cMask = cMask0 & IVector4A16(0x3, 0xc, 0x30, 0xc0);
    // shift, (x << 8, y << 6, z << 4, w << 2) * 4
    static const IVector4A16 shiftMask1(0x400, 0x100, 0x40, 0x10);
    cMask = cMask * shiftMask1;

    const auto sfMask = _mm_shuffle_epi8(
        cMask._data,
        IVector4A16(0x01010101, 0x05050505, 0x09090909, 0x0d0d0d0d)._data);
    const auto finalIndices = _mm_add_epi8(
        sfMask,
        IVector4A16(0x03020100, 0x03020100, 0x03020100, 0x03020100)._data);
    const IVector4A16 indexedMix(_mm_shuffle_epi8(finalMix, finalIndices));

    const auto finalBuffer = _mm_shuffle_epi8(
        indexedMix._data,
        IVector4A16(0x04020100, 0x09080605, 0x0e0d0c0a, 0x10101010)._data);

    memcpy((obuffer + ((h + row) * width + w) * pixeloffset),
           reinterpret_cast<const char *>(&finalBuffer), 12);
  }
}
