/*  MSF (PS3) format header

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

struct MSF {
  enum CodecType {
    PCM16BE,
    PCM16LE,
    PCM32F,
    PSX_ADPCM,
    AT3L,
    AT3M,
    AT3H,
    LAME,
  };

  uint32 id;
  CodecType codec;
  uint32 numChannels, dataSize, sampleRate, flags, loopStart, loopDuration, unk[8];
};

ES_STATIC_ASSERT(sizeof(MSF) == 0x40);
