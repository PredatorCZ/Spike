/*  XVAG (PS3) format header

    Copyright 2017-2021 Lukas Cone

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
#include "datas/supercore.hpp"

static constexpr uint32 XVAGID = CompileFourCC("XVAG");
static constexpr uint32 XVAGFMAT = CompileFourCC("fmat");

struct XVAGChunk {
  uint32 id;
  uint32 size;
};

struct XVAGHeader : XVAGChunk {
  uint8 numBlocks;
  uint8 flags; // 0x1 LE?
  uint8 unk[2];
  uint32 null[5];
};

enum class XVAGFormat : uint32 {
  PS_ADPCM = 6,
  PS_ADPCM_EX,
  MPEG = 8,
  AT9,
};

struct XVAGfmat : XVAGChunk {
  uint32 numChannels;
  XVAGFormat format;
  uint32 numSamples;
  uint32 numSamples2;
  uint32 interleave;
  uint32 sampleRate;
  uint32 bufferSize;
};
