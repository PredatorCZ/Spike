/*  FWSE (MT Framework modified IMA-ADPCM) format header

    Copyright 2019-2021 Lukas Cone

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

struct FWSE {
  static constexpr uint32 ID = CompileFourCC("FWSE");

  uint32 id;
  uint32 version;
  uint32 fileSize;
  uint32 bufferOffset;
  uint32 numChannels;
  uint32 numSamples;
  uint32 sampleRate;
  uint32 sampleDepth;
  uint8 unk[992]; // 4bit?
};

static_assert(sizeof(FWSE) == 0x400);
