#include "spike/app/context.hpp"
#include "spike/app/texel.hpp"
#include "spike/format/DDS.hpp"
#include "spike/io/binreader.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/io/fileinfo.hpp"

TexelConf &TexelSettings() { return mainSettings.texelSettings; }

struct DDSBuffer {
  DDS dds;
  std::string buffer;

  void Read(BinReaderRef rd) {
    rd.Read(dds);
    const size_t headerSize =
        dds.fourCC == DDSFormat_DX10.fourCC ? dds.DDS_SIZE : dds.LEGACY_SIZE;
    rd.Seek(headerSize);

    rd.ReadContainer(buffer, rd.GetSize() - headerSize);
  }
};

void Convert(std::string_view path, TexelInputFormatType fmt) {
  auto appContext = MakeIOContext(std::string(path));
  BinReaderRef rd(appContext->GetStream());
  DDSBuffer buf;
  rd.Read(buf);

  NewTexelContextCreate nctx{
      .width = uint16(buf.dds.width),
      .height = uint16(buf.dds.height),
      .baseFormat =
          {
              .type = fmt,
          },
  };

  {
    TexelSettings().outputFormat = TexelContextFormat::DDS_Legacy;
    std::string npath("out_");
    npath.append(appContext->workingFile.GetFullPathNoExt());
    npath.append("_legacy.dds");
    auto ctx = appContext->NewImage(nctx, &npath);
    ctx->SendRasterData(buf.buffer.data(), {}, {});
  }

  {
    TexelSettings().outputFormat = TexelContextFormat::QOI;
    auto ctx = appContext->NewImage(nctx);
    ctx->SendRasterData(buf.buffer.data(), {}, {});
  }
}

void ConvertArray(std::string_view path, TexelInputFormatType fmt) {
  auto appContext = MakeIOContext(std::string(path));
  BinReaderRef rd(appContext->GetStream());
  DDSBuffer buf;
  rd.Read(buf);
  DDS::Mips mips;
  buf.dds.ComputeBPP();
  buf.dds.ComputeBufferSize(mips);

  NewTexelContextCreate nctx{
      .width = uint16(buf.dds.width),
      .height = uint16(buf.dds.height),
      .baseFormat =
          {
              .type = fmt,
          },
      .numMipmaps = uint8(std::max(1U, buf.dds.mipMapCount)),
      .arraySize = uint16(buf.dds.arraySize),
  };

  {
    TexelSettings().outputFormat = TexelContextFormat::DDS_Legacy;
    std::string npath("out_");
    npath.append(appContext->workingFile.GetFullPathNoExt());
    npath.append("_legacy.dds");
    auto ctx = appContext->NewImage(nctx, &npath);

    for (uint16 i = 0; i < buf.dds.arraySize; i++) {
      for (uint8 m = 0; m < buf.dds.mipMapCount; m++) {
        ctx->SendRasterData(buf.buffer.data() + mips.frameStride * i +
                                mips.offsets[m],
                            {
                                .mipMap = m,
                                .layer = i,
                            });
      }
    }
  }

  {
    TexelSettings().outputFormat = TexelContextFormat::QOI;
    auto ctx = appContext->NewImage(nctx);
    for (uint16 i = 0; i < buf.dds.arraySize; i++) {
      ctx->SendRasterData(buf.buffer.data() + mips.frameStride * i,
                          {
                              .layer = i,
                          });
    }
  }
}

void ConvertVolume(std::string_view path, TexelInputFormatType fmt) {
  auto appContext = MakeIOContext(std::string(path));
  BinReaderRef rd(appContext->GetStream());
  DDSBuffer buf;
  rd.Read(buf);
  DDS::Mips mips;
  buf.dds.ComputeBPP();
  buf.dds.ComputeBufferSize(mips);

  NewTexelContextCreate nctx{
      .width = uint16(buf.dds.width),
      .height = uint16(buf.dds.height),
      .baseFormat =
          {
              .type = fmt,
          },
      .depth = uint16(buf.dds.depth),
      .numMipmaps = uint8(std::max(1U, buf.dds.mipMapCount)),
  };

  {
    TexelSettings().outputFormat = TexelContextFormat::DDS_Legacy;
    std::string npath("out_");
    npath.append(appContext->workingFile.GetFullPathNoExt());
    npath.append("_legacy.dds");
    auto ctx = appContext->NewImage(nctx, &npath);

    for (uint8 m = 0; m < buf.dds.mipMapCount; m++) {
      for (uint16 i = 0; i < mips.numSlices[m]; i++) {
        ctx->SendRasterData(buf.buffer.data() + mips.offsets[m] +
                                mips.sizes[m] * i,
                            {
                                .mipMap = m,
                            });
      }
    }
  }

  {
    TexelSettings().outputFormat = TexelContextFormat::QOI;
    auto ctx = appContext->NewImage(nctx);
    ctx->SendRasterData(buf.buffer.data());
  }
}

void ConvertCubemap(std::string_view path, TexelInputFormatType fmt) {
  auto appContext = MakeIOContext(std::string(path));
  BinReaderRef rd(appContext->GetStream());
  DDSBuffer buf;
  rd.Read(buf);
  DDS::Mips mips;
  buf.dds.ComputeBPP();
  buf.dds.ComputeBufferSize(mips);

  NewTexelContextCreate nctx{
      .width = uint16(buf.dds.width),
      .height = uint16(buf.dds.height),
      .baseFormat =
          {
              .type = fmt,
          },
      .numMipmaps = uint8(std::max(1U, buf.dds.mipMapCount)),
      .numFaces = 6,
  };

  {
    TexelSettings().outputFormat = TexelContextFormat::DDS_Legacy;
    std::string npath("out_");
    npath.append(appContext->workingFile.GetFullPathNoExt());
    npath.append("_legacy.dds");
    auto ctx = appContext->NewImage(nctx, &npath);

    for (uint16 i = 0; i < 6; i++) {
      for (uint8 m = 0; m < buf.dds.mipMapCount; m++) {
        ctx->SendRasterData(buf.buffer.data() + mips.frameStride * i +
                                mips.offsets[m],
                            {
                                .mipMap = m,
                                .face = static_cast<CubemapFace>(i + 1),
                            });
      }
    }
  }

  {
    TexelSettings().outputFormat = TexelContextFormat::QOI;
    auto ctx = appContext->NewImage(nctx);
    for (uint16 i = 0; i < 6; i++) {
      ctx->SendRasterData(buf.buffer.data() + mips.frameStride * i,
                          {
                              .face = static_cast<CubemapFace>(i + 1),
                          });
    }
  }
}

void ConvertNX(std::string_view path, TexelInputFormatType fmt) {
  auto appContext = MakeIOContext(std::string(path));
  BinReaderRef rd(appContext->GetStream());
  DDSBuffer buf;
  rd.Read(buf);

  NewTexelContextCreate nctx{
      .width = uint16(buf.dds.width),
      .height = uint16(buf.dds.height),
      .baseFormat =
          {
              .type = fmt,
              .tile = TexelTile::NX,
          },
  };

  {
    TexelSettings().outputFormat = TexelContextFormat::DDS_Legacy;
    std::string npath("out_");
    npath.append(appContext->workingFile.GetFullPathNoExt());
    npath.append("_legacy.dds");
    auto ctx = appContext->NewImage(nctx, &npath);
    ctx->SendRasterData(buf.buffer.data(), {}, {});
  }

  {
    TexelSettings().outputFormat = TexelContextFormat::QOI;
    auto ctx = appContext->NewImage(nctx);
    ctx->SendRasterData(buf.buffer.data(), {}, {});
  }
}

int main() {
  Convert("resources/rgba8.dds", TexelInputFormatType::RGBA8);
  Convert("resources/rgba4.dds", TexelInputFormatType::RGBA4);
  Convert("resources/rgb5a1.dds", TexelInputFormatType::RGB5A1);
  Convert("resources/rgb8.dds", TexelInputFormatType::RGB8);
  Convert("resources/rgb10a2.dds", TexelInputFormatType::RGB10A2);
  Convert("resources/rg8.dds", TexelInputFormatType::RG8);
  Convert("resources/r8.dds", TexelInputFormatType::R8);
  Convert("resources/r5g6b5.dds", TexelInputFormatType::R5G6B5);
  Convert("resources/bc1.dds", TexelInputFormatType::BC1);
  Convert("resources/bc2.dds", TexelInputFormatType::BC2);
  Convert("resources/bc3.dds", TexelInputFormatType::BC3);
  Convert("resources/bc4.dds", TexelInputFormatType::BC4);
  Convert("resources/bc5.dds", TexelInputFormatType::BC5);
  Convert("resources/pvrtc2.dds", TexelInputFormatType::PVRTC2);
  Convert("resources/pvrtc4.dds", TexelInputFormatType::PVRTC4);
  Convert("resources/etc1.dds", TexelInputFormatType::ETC1);
  Convert("resources/bc7.dds", TexelInputFormatType::BC7);

  ConvertNX("resources/rgba8_nx.dds", TexelInputFormatType::RGBA8);
  ConvertNX("resources/rgba8_np_nx.dds", TexelInputFormatType::RGBA8);
  ConvertNX("resources/bc3_np_nx.dds", TexelInputFormatType::BC3);
  ConvertNX("resources/r5g6b5_np_nx.dds", TexelInputFormatType::R5G6B5);
  ConvertNX("resources/r8_np_nx.dds", TexelInputFormatType::R8);
  ConvertNX("resources/bc1_np_nx.dds", TexelInputFormatType::BC1);

  TexelSettings().processMipMaps = true;
  ConvertArray("resources/rgba8_array.dds", TexelInputFormatType::RGBA8);
  ConvertArray("resources/rgba8_array_mip.dds", TexelInputFormatType::RGBA8);
  ConvertVolume("resources/rgba8_volume_mip.dds", TexelInputFormatType::RGBA8);

  ConvertCubemap("resources/rgba8_cube.dds", TexelInputFormatType::RGBA8);
  ConvertCubemap("resources/bc3_cube_mip.dds", TexelInputFormatType::BC3);

  return 0;
}
