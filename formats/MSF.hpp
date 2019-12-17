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

  int id;
  CodecType codec;
  int numChannels, dataSize, sampleRate, flags, loopStart, loopDuration, unk[8];
};

static_assert(sizeof(MSF) == 0x40, "Check assumptions!");