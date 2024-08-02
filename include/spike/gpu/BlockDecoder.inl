/*  Block Compression Decoder

    Copyright 2018-2023 Lukas Cone

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

#include "BlockDecoder.h"

inline void _DecodeBC1Block(const char *data, char *obuffer, uint32 w, uint32 h,
                            uint32 width, const uint32 pixeloffset = 3) {
  Vector color1, color2;
  DecodeRGB565Block(data, color1);
  data += 2;
  DecodeRGB565Block(data, color2);
  data += 2;
  h *= 4;
  w *= 4;
  width *= 4;

  UCVector colors[4] = {
      color1.Convert<uint8>(),
      color2.Convert<uint8>(),
      ((color1 * (2.0f / 3.0f)) + (color2 * (1.0f / 3.0f))).Convert<uint8>(),
      ((color1 * (1.0f / 3.0f)) + (color2 * (2.0f / 3.0f))).Convert<uint8>(),
  };

  for (int row = 0; row < 4; row++) {
    const uint8 udata = *data++;
    const uint8 d1 = udata & 3;
    const uint8 d2 = (udata >> 2) & 3;
    const uint8 d3 = (udata >> 4) & 3;
    const uint8 d4 = udata >> 6;

    reinterpret_cast<UCVector &>(
        *(obuffer + ((h + row) * width + w) * pixeloffset)) = colors[d1];
    reinterpret_cast<UCVector &>(
        *(obuffer + ((h + row) * width + w + 1) * pixeloffset)) = colors[d2];
    reinterpret_cast<UCVector &>(
        *(obuffer + ((h + row) * width + w + 2) * pixeloffset)) = colors[d3];
    reinterpret_cast<UCVector &>(
        *(obuffer + ((h + row) * width + w + 3) * pixeloffset)) = colors[d4];
  }
}

inline void _DecodeBC1BlockA(const char *data, char *obuffer, uint32 w,
                             uint32 h, uint32 width,
                             const uint32 pixeloffset = 4) {
  Vector color0;
  Vector color1;
  DecodeRGB565Block(data, color0);
  uint16 cl0 = reinterpret_cast<const uint16 &>(*data);
  data += 2;
  DecodeRGB565Block(data, color1);
  uint16 cl1 = reinterpret_cast<const uint16 &>(*data);
  data += 2;
  h *= 4;
  w *= 4;
  width *= 4;

  bool usealpha = false;
  UCVector colors[4]{
      color0.Convert<uint8>(),
      color1.Convert<uint8>(),
  };

  if (cl0 > cl1) {
    colors[2] =
        ((color0 * (2.0f / 3.0f)) + (color1 * (1.0f / 3.0f))).Convert<uint8>();
    colors[3] =
        ((color0 * (1.0f / 3.0f)) + (color1 * (2.0f / 3.0f))).Convert<uint8>();
  } else {
    colors[2] = ((color0 + color1) * 0.5f).Convert<uint8>();
    colors[3] = colors[2];
    usealpha = true;
  }

  for (int row = 0; row < 4; row++) {
    const uint8 udata = *data++;
    const uint8 d1 = udata & 3;
    const uint8 d2 = (udata >> 2) & 3;
    const uint8 d3 = (udata >> 4) & 3;
    const uint8 d4 = udata >> 6;

    auto &v0 = reinterpret_cast<UCVector4 &>(
        *(obuffer + ((h + row) * width + w) * pixeloffset)) = colors[d1];
    v0.w = usealpha && d1 == 3 ? 0 : 0xff;
    auto &v1 = reinterpret_cast<UCVector4 &>(
        *(obuffer + ((h + row) * width + w + 1) * pixeloffset)) = colors[d2];
    v1.w = usealpha && d2 == 3 ? 0 : 0xff;
    auto &v2 = reinterpret_cast<UCVector4 &>(
        *(obuffer + ((h + row) * width + w + 2) * pixeloffset)) = colors[d3];
    v2.w = usealpha && d3 == 3 ? 0 : 0xff;
    auto &v3 = reinterpret_cast<UCVector4 &>(
        *(obuffer + ((h + row) * width + w + 3) * pixeloffset)) = colors[d4];
    v3.w = usealpha && d4 == 3 ? 0 : 0xff;
  }
}

inline void _DecodeBC2Block(const char *data, char *obuffer, uint32 w, uint32 h,
                            uint32 width, const uint32 pixeloffset = 4) {
  h *= 4;
  w *= 4;
  width *= 4;

  for (int row = 0; row < 4; row++, data++) {
    reinterpret_cast<uint8 &>(*(
        obuffer + ((h + row) * width + w) * pixeloffset)) = (*data & 0xf) * 17;
    reinterpret_cast<uint8 &>(
        *(obuffer + ((h + row) * width + w + 1) * pixeloffset)) =
        ((*data & 0xf0) >> 4) * 17;
    data++;
    reinterpret_cast<uint8 &>(
        *(obuffer + ((h + row) * width + w + 2) * pixeloffset)) =
        (*data & 0xf) * 17;
    reinterpret_cast<uint8 &>(
        *(obuffer + ((h + row) * width + w + 3) * pixeloffset)) =
        ((*data & 0xf0) >> 4) * 17;
  }
}

inline void DecodeBC1BlockA(const char *data, char *obuffer, uint32 w, uint32 h,
                            uint32 width) {
  _DecodeBC1BlockA(data, obuffer, w, h, width);
}

inline void DecodeBC2Block(const char *data, char *obuffer, uint32 w, uint32 h,
                           uint32 width) {
  _DecodeBC2Block(data, obuffer + 3, w, h, width);
  _DecodeBC1Block(data + 8, obuffer, w, h, width, 4);
}

inline void DecodeBC1Block(const char *data, char *obuffer, uint32 w, uint32 h,
                           uint32 width) {
  _DecodeBC1Block(data, obuffer, w, h, width);
}

inline void _DecodeBC4Block(const char *data, char *obuffer, uint32 w, uint32 h,
                            uint32 width, const uint32 pixeloffset = 1) {
  h *= 4;
  w *= 4;
  width *= 4;

  uint16 alpha[8]{*reinterpret_cast<const uint8 *>(data++),
                  *reinterpret_cast<const uint8 *>(data++)};

  if (*alpha > alpha[1]) {
    alpha[2] = (6 * (*alpha) + 1 * alpha[1]) / 7;
    alpha[3] = (5 * (*alpha) + 2 * alpha[1]) / 7;
    alpha[4] = (4 * (*alpha) + 3 * alpha[1]) / 7;
    alpha[5] = (3 * (*alpha) + 4 * alpha[1]) / 7;
    alpha[6] = (2 * (*alpha) + 5 * alpha[1]) / 7;
    alpha[7] = (1 * (*alpha) + 6 * alpha[1]) / 7;
  } else {
    alpha[2] = (4 * (*alpha) + 1 * alpha[1]) / 5;
    alpha[3] = (3 * (*alpha) + 2 * alpha[1]) / 5;
    alpha[4] = (2 * (*alpha) + 3 * alpha[1]) / 5;
    alpha[5] = (1 * (*alpha) + 4 * alpha[1]) / 5;
    alpha[6] = 0;
    alpha[7] = 255;
  }

  for (int b = 0; b < 3; b += 2, data++) {

    *(obuffer + ((h + b) * width + w) * pixeloffset) =
        reinterpret_cast<char &>(alpha[*data & 7]);
    *(obuffer + ((h + b) * width + w + 1) * pixeloffset) =
        reinterpret_cast<char &>(alpha[(*data & 0x38) >> 3]);
    *(obuffer + ((h + b) * width + w + 2) * pixeloffset) =
        reinterpret_cast<char &>(
            alpha[((*data & 0xc0) >> 6) | ((*(data + 1) & 1) << 2)]);
    data++;
    *(obuffer + ((h + b) * width + w + 3) * pixeloffset) =
        reinterpret_cast<char &>(alpha[(*data & 0xE) >> 1]);
    *(obuffer + ((h + b + 1) * width + w) * pixeloffset) =
        reinterpret_cast<char &>(alpha[(*data & 0x70) >> 4]);
    *(obuffer + ((h + b + 1) * width + w + 1) * pixeloffset) =
        reinterpret_cast<char &>(
            alpha[((*data & 0x80) >> 7) | ((*(data + 1) & 3) << 1)]);
    data++;
    *(obuffer + ((h + b + 1) * width + w + 2) * pixeloffset) =
        reinterpret_cast<char &>(alpha[(*data & 0x1c) >> 2]);
    *(obuffer + ((h + b + 1) * width + w + 3) * pixeloffset) =
        reinterpret_cast<char &>(alpha[(*data & 0xE0) >> 5]);
  }
}

inline void DecodeBC4Block(const char *data, char *obuffer, uint32 w, uint32 h,
                           uint32 width) {
  _DecodeBC4Block(data, obuffer, w, h, width);
}

inline void DecodeBC5Block(const char *data, char *obuffer, uint32 w, uint32 h,
                           uint32 width, uint32 stride) {
  _DecodeBC4Block(data, obuffer + 1, w, h, width, stride);
  _DecodeBC4Block(data + 8, obuffer + 2, w, h, width, stride);
}

inline void DecodeBC5BlockGA(const char *data, char *obuffer, uint32 w,
                             uint32 h, uint32 width) {
  _DecodeBC4Block(data, obuffer, w, h, width, 2);
  _DecodeBC4Block(data + 8, obuffer + 1, w, h, width, 2);
}

inline void DecodeBC3Block(const char *data, char *obuffer, uint32 w, uint32 h,
                           uint32 width) {
  _DecodeBC4Block(data, obuffer + 3, w, h, width, 4);
  _DecodeBC1Block(data + 8, obuffer, w, h, width, 4);
}
