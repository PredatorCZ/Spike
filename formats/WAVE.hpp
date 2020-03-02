/*  RIFF WAVE format header

    Copyright 2019-2020 Lukas Cone

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

struct WAVEGenericHeader {
  uint32 id;
  uint32 chunkSize;

  WAVEGenericHeader(uint32 type, uint32 chSize = 0) : id(type), chunkSize(chSize) {}

  WAVEGenericHeader *Next() {
    return reinterpret_cast<WAVEGenericHeader *>(
        reinterpret_cast<char *>(this) + chunkSize);
  }
};

struct RIFFHeader : WAVEGenericHeader {
  static constexpr uint32 ID = CompileFourCC("RIFF");

  uint32 type;

  RIFFHeader(uint32 fullSize)
      : WAVEGenericHeader(ID, fullSize - 8), type(CompileFourCC("WAVE")) {}
};

enum class WAVE_FORMAT : uint16 {
  UNKNOWN,
  PCM,
  MS_ADPCM,
  ALAW = 6,
  MULAW,
  IMA_ADPCM = 0x11,
  GSM610 = 0x31,
  MPEG = 0x50
};

struct WAVE_fmt : WAVEGenericHeader {
  static constexpr uint32 ID = CompileFourCC("fmt ");

  WAVE_FORMAT format;
  uint16 channels = 1;
  uint32 sampleRate = 0;
  uint32 sampleCount = 0;
  uint16 interleave = 0;
  uint16 bitsPerSample;

  WAVE_fmt(WAVE_FORMAT iFormat)
      : WAVEGenericHeader(ID, sizeof(WAVE_fmt) - 8), format(iFormat) {
    switch (format) {
    case WAVE_FORMAT::PCM:
      bitsPerSample = 16;
      break;
    case WAVE_FORMAT::MS_ADPCM:
    case WAVE_FORMAT::IMA_ADPCM:
      bitsPerSample = 4;
      break;
    case WAVE_FORMAT::ALAW:
    case WAVE_FORMAT::MULAW:
      bitsPerSample = 8;
      break;
    default:
      bitsPerSample = 0;
      break;
    }
  }

  void CalcData() {
    sampleCount = channels * sampleRate * (bitsPerSample / 8);
    interleave = channels * (bitsPerSample / 8);
  }
};

struct WAVE_fmt_MSADPCM : WAVE_fmt {
  uint16 extendedDataSize;
  char extendedData[2];
};

struct WAVE_data : WAVEGenericHeader {
  static constexpr uint32 ID = CompileFourCC("data");

  char *GetData() { return reinterpret_cast<char *>(this) + sizeof(WAVE_data); }
  WAVE_data(uint32 dataSize) : WAVEGenericHeader(ID, dataSize) {}
};

struct WAVE_fact : WAVEGenericHeader {
  static constexpr uint32 ID = CompileFourCC("fact");
  uint32 uncompressedSize;
};

struct WAVE_smpl : WAVEGenericHeader {
  struct SampleLoop {
    enum Type {
      TYPE_LOOP_FORWARD,
      TYPE_LOOP_ALTERNATE,
      TYPE_LOOP_BACKWARD
    };

    uint32 id;
    Type type;
    uint32 start,
      end,
      fraction,
      playCount;
  };

  static constexpr uint32 ID = CompileFourCC("smpl");

  uint32 manufacturer,
    product,
    samplePeriod,
    MIDIUnityNote,
    MIDIPitchFraction,
    SMPTEFormat,
    SMPTEOffset,
    numSampleLoops,
    sampleLoopsSize;

    SampleLoop *GetSampleLoops() {return reinterpret_cast<SampleLoop*>(this + 1);}
};

static inline bool IsValidWaveChunk(const WAVEGenericHeader &hdr) {
  switch (hdr.id) {
  case WAVE_fmt::ID:
  case WAVE_data::ID:
  case WAVE_fact::ID:
  case WAVE_smpl::ID:
    return true;

  default:
    return false;
  }
}