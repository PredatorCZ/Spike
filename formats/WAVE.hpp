#include "datas/supercore.hpp"

struct WAVEGenericHeader {
  uint id;
  uint chunkSize;

  WAVEGenericHeader(uint type, uint chSize = 0) : id(type), chunkSize(chSize) {}

  WAVEGenericHeader *Next() {
    return reinterpret_cast<WAVEGenericHeader *>(
        reinterpret_cast<char *>(this) + chunkSize);
  }
};

struct RIFFHeader : WAVEGenericHeader {
  static constexpr int ID = CompileFourCC("RIFF");

  int type;

  RIFFHeader(uint fullSize)
      : WAVEGenericHeader(ID, fullSize - 8), type(CompileFourCC("WAVE")) {}
};

enum class WAVE_FORMAT : ushort {
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
  static constexpr int ID = CompileFourCC("fmt ");

  WAVE_FORMAT format;
  ushort channels = 1;
  uint sampleRate = 0;
  uint sampleCount = 0;
  ushort interleave = 0;
  ushort bitsPerSample;

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
  ushort extendedDataSize;
  char extendedData[2];
};

struct WAVE_data : WAVEGenericHeader {
  static constexpr int ID = CompileFourCC("data");

  char *GetData() { return reinterpret_cast<char *>(this) + sizeof(WAVE_data); }
  WAVE_data(uint dataSize) : WAVEGenericHeader(ID, dataSize) {}
};

struct WAVE_fact : WAVEGenericHeader {
  static constexpr int ID = CompileFourCC("fact");
  int uncompressedSize;
};

struct WAVE_smpl : WAVEGenericHeader {
  struct SampleLoop {
    enum Type {
      TYPE_LOOP_FORWARD,
      TYPE_LOOP_ALTERNATE,
      TYPE_LOOP_BACKWARD
    };

    int id;
    Type type;
    int start,
      end,
      fraction,
      playCount;
  };

  static constexpr int ID = CompileFourCC("smpl");

  int manufacturer,
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

static bool IsValidWaveChunk(const WAVEGenericHeader &hdr) {
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