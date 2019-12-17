#include "datas/supercore.hpp"

struct FWSE {
  static constexpr int ID = CompileFourCC("FWSE");

  uint id;
  uint version;
  uint fileSize;
  uint bufferOffset;
  uint numChannels;
  uint numSamples;
  uint sampleRate;
  uint sampleDepth;
  char unk[992]; // 4bit?
};

static_assert(sizeof(FWSE) == 0x400, "Check assumptions!");