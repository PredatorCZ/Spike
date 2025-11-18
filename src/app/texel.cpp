#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO

#include "spike/app/texel.hpp"
#include "bc7decomp.h"
#include "pvr_decompress.hpp"
#include "qoi.h"
#include "spike/app/context.hpp"
#include "spike/crypto/crc32.hpp"
#include "spike/except.hpp"
#include "spike/format/DDS.hpp"
#include "spike/gpu/BlockDecoder.inl"
#include "spike/gpu/addr_ps3.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/uni/format.hpp"
#include "spike/util/endian.hpp"
#include <sstream>
#include <variant>

bool BlockCompression(TexelInputFormatType fmt) {
  using F = TexelInputFormatType;
  switch (fmt) {
  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::BC4:
  case F::BC5:
  case F::BC7:
    return true;
  default:
    return false;
  }
};

TexelContextFormat OutputFormat() {
  return mainSettings.texelSettings.outputFormat;
}

bool IsFormatSupported(TexelContextFormat ofmt, TexelInputFormatType fmt) {
  using F = TexelInputFormatType;

  switch (fmt) {
    // Not supported
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::R4:
  case F::RG4:
    return true;

    // DDS, DDS_Legacy only
  case F::BC1:
  case F::BC2:
  case F::BC3:
    return ofmt == TexelContextFormat::UPNG ||
           ofmt == TexelContextFormat::QOI ||
           ofmt == TexelContextFormat::QOI_BMP;

    // DDS only
  case F::BC4:
  case F::BC5:
  case F::BC7:
  case F::RG8:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return ofmt != TexelContextFormat::DDS;

    // BMP, DDS only
  case F::RGB10A2:
  case F::RGB5A1:
    return ofmt == TexelContextFormat::UPNG ||
           ofmt == TexelContextFormat::QOI ||
           ofmt == TexelContextFormat::DDS_Legacy;

    // BMP only
  case F::P8:
  case F::P4:
    return ofmt != TexelContextFormat::QOI_BMP;

  // DDS, DDS_Legacy, BMP only
  case F::RGBA4:
  case F::R5G6B5:
    return ofmt == TexelContextFormat::UPNG || ofmt == TexelContextFormat::QOI;

  // DDS, DDS_Legacy, BMP, UPNG only
  case F::R8:
    return ofmt == TexelContextFormat::QOI;

    // Supported for all
  case F::RGBA8:
  case F::INVALID:
    return false;

  // QOI, DDS_Legacy, UPNG only
  case F::RGB8:
    return ofmt == TexelContextFormat::DDS;
  }

  return false;
}

bool MustSwap(TexelInputFormatType fmt, bool shouldSwap) {
  using F = TexelInputFormatType;

  switch (fmt) {
  case F::R5G6B5:
  case F::RGBA4:
  case F::RGB10A2:
  case F::RGB5A1:
    return shouldSwap;
  default:
    return false;
    break;
  }
}

bool MustSwizzle(TexelSwizzle swizzle, uint32 numChannels) {
  if (swizzle.a != TexelSwizzleType::Alpha ||
      swizzle.r != TexelSwizzleType::Red ||
      swizzle.g != TexelSwizzleType::Green) {
    return true;
  }

  if (numChannels > 2) {
    return swizzle.b != TexelSwizzleType::Blue &&
           swizzle.b != TexelSwizzleType::DeriveZOrBlue;
  }

  return swizzle.b != TexelSwizzleType::Blue;
};

uint32 GetBPT(TexelInputFormatType fmt) {
  using F = TexelInputFormatType;

  switch (fmt) {
  case F::PVRTC2:
  case F::PVRTC4:
  case F::BC2:
  case F::BC3:
  case F::BC5:
  case F::BC7:
  case F::BC6:
  case F::ETC1A4:
    return 16;

  case F::ETC1:
  case F::BC1:
  case F::BC4:
  case F::RGBA16:
    return 8;

  case F::RG8:
  case F::RGB5A1:
  case F::RGBA4:
  case F::R5G6B5:
  case F::R4:
    return 2;

  case F::RGB10A2:
  case F::RGBA8:
  case F::RGB9E5:
    return 4;

  case F::P8:
  case F::P4:
  case F::R8:
  case F::RG4:
    return 1;
  case F::RGB8:
    return 3;
  case F::INVALID:
    return 0;
  }

  return 0;
}

TexelDataLayout NewTexelContextImpl::ComputeTraditionalDataLayout(
    TexelInputFormatType *typeOverrides) {
  uint32 mipCount = std::max(ctx.numMipmaps, uint8(1));
  uint32 width = ctx.width;
  uint32 height = ctx.height;
  uint32 depth = std::max(ctx.depth, uint16(1));
  uint32 numFaces = std::max(ctx.numFaces, int8(1));

  TexelDataLayout retVal{};

  for (uint32 m = 0; m < mipCount; m++) {
    uint32 _width = width;
    uint32 _height = height;
    TexelInputFormatType type =
        typeOverrides ? typeOverrides[m] : ctx.baseFormat.type;
    uint32 bpt = GetBPT(type);

    if (BlockCompression(type)) {
      _width = (_width + 3) / 4;
      _height = (_height + 3) / 4;
    }

    retVal.mipSizes[m] = depth * _width * _height * bpt;
    retVal.mipOffsets[m] = retVal.mipGroupSize;
    retVal.mipGroupSize += retVal.mipSizes[m];
    width = std::max(1U, width / 2);
    height = std::max(1U, height / 2);
    depth = std::max(1U, depth / 2);
  }

  retVal.groupSize = retVal.mipGroupSize * numFaces;
  return retVal;
}

bool BMPFallback(TexelInputFormatType fmt) {
  using F = TexelInputFormatType;

  switch (fmt) {
  case F::RGB10A2:
  case F::RGB5A1:
  case F::P8:
  case F::P4:
  case F::RGBA4:
  case F::R5G6B5:
  case F::R8:
    return OutputFormat() == TexelContextFormat::QOI_BMP;

  default:
    return false;
  }

  return false;
}

#pragma pack(2)
struct BMPHeader {
  uint16 bfType = 0x4D42; // BM
  uint32 bfSize;
  uint16 bfReserved1 = 0;
  uint16 bfReserved2 = 0;
  uint32 bfOffBits = 54;
};
#pragma pack()

struct BMPInfoHeader {
  uint32 biSize = 40;
  uint32 biWidth;
  uint32 biHeight;
  uint16 biPlanes = 1;
  uint16 biBitCount = 32;
  uint32 biCompression = 0;
  uint32 biSizeImage;
  uint32 biXPelsPerMeter = 0;
  uint32 biYPelsPerMeter = 0;
  uint32 biClrUsed = 0;
  uint32 biClrImportant = 0;
};

union BMPMask {
  struct {
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 alpha;
  };
  uint32 data[4];
};

BMPMask SwizzleMask(BMPMask bits, TexelSwizzle swizzle) {
  uint32 currentOffset = 0;
  BMPMask retVal{};

  for (uint32 c = 0; c < 4; c++) {
    uint32 swizzleIndex = swizzle.types[c] > TexelSwizzleType::Alpha
                              ? c
                              : uint32(swizzle.types[c]);

    uint32 numBits = bits.data[swizzleIndex];
    uint32 mask = (1 << numBits) - 1;

    retVal.data[c] = mask << currentOffset;
    currentOffset += numBits;
  }

  return retVal;
}

BMPMask FillBmpFormat(NewTexelContextCreate ctx, TexelInputFormat fmt,
                      BMPInfoHeader &info) {
  using F = TexelInputFormatType;
  info.biWidth = ctx.width;
  info.biHeight = ctx.height;

  switch (fmt.type) {
  case F::RGB10A2:
    info.biBitCount = 32;
    info.biCompression = 6;
    return SwizzleMask({{.red = 10, .green = 10, .blue = 10, .alpha = 2}},
                       fmt.swizzle);
  case F::RGB5A1:
    info.biBitCount = 16;
    info.biCompression = 6;
    return SwizzleMask({{.red = 4, .green = 4, .blue = 4, .alpha = 1}},
                       fmt.swizzle);
  case F::RGBA4:
    info.biBitCount = 16;
    info.biCompression = 6;
    return SwizzleMask({{.red = 4, .green = 4, .blue = 4, .alpha = 4}},
                       fmt.swizzle);
  case F::R5G6B5:
    info.biBitCount = 16;
    info.biCompression = 3;
    return SwizzleMask({{.red = 5, .green = 6, .blue = 5, .alpha = 0}},
                       fmt.swizzle);
  default:
    return {};
  }

  return {};
}

DDS MakeDDS(NewTexelContextCreate ctx) {
  DDS dds;
  dds.width = ctx.width;
  dds.height = ctx.height;
  if (ctx.baseFormat.premultAlpha) {
    dds.alphaMode = dds.AlphaMode_Premultiplied;
  }

  dds.arraySize = ctx.arraySize;

  if (ctx.depth > 1) {
    dds.caps01 += dds.Caps01Flags_Volume;
    dds.flags += dds.Flags_Depth;
    dds.dimension = dds.Dimension_3D;
    dds.depth = ctx.depth;
  } else if (ctx.numFaces > 0) {
    dds.caps01 += dds.Caps01Flags_CubeMap;
    dds.miscFlag += dds.MiscFlag_CubeTexture;

    for (int32 i = 0; i < ctx.numFaces; i++) {
      dds.caps01 += static_cast<DDS_HeaderEnd::Caps01Flags>(i + 10);
    }
  }

  if (mainSettings.texelSettings.processMipMaps) {
    dds.NumMipmaps(ctx.numMipmaps);
  }

  return dds;
}

void SetDDSFormat(DDS &dds, TexelInputFormat fmt) {
  dds = DDSFormat_DX10;
  switch (fmt.type) {
    using F = TexelInputFormatType;
  case F::BC1:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC1_UNORM + fmt.srgb);
    break;

  case F::BC2:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC2_UNORM + fmt.srgb);
    break;

  case F::BC3:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC3_UNORM + fmt.srgb);
    break;

  case F::BC4:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC4_UNORM + fmt.snorm);
    break;

  case F::BC5:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC5_UNORM + fmt.snorm);
    break;

  case F::BC7:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC7_UNORM + fmt.srgb);
    break;

  case F::RGBA4:
    dds.dxgiFormat = DXGI_FORMAT_B4G4R4A4_UNORM;
    break;

  case F::R5G6B5:
    dds.dxgiFormat = DXGI_FORMAT_B5G6R5_UNORM;
    break;

  case F::RGB5A1:
    dds.dxgiFormat = DXGI_FORMAT_B5G5R5A1_UNORM;
    break;

  case F::RGB10A2:
    dds.dxgiFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
    break;

  case F::RG8:
  case F::RG4:
    if (fmt.snorm) {
      dds.dxgiFormat = DXGI_FORMAT_R8G8_SNORM;
    } else if (fmt.swizzle.a == TexelSwizzleType::Green &&
               fmt.swizzle.r == TexelSwizzleType::Red) {
      dds.dxgiFormat = DXGI_FORMAT_A8P8;
    } else {
      dds.dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
    }
    break;

  case F::R8:
  case F::R4:
    dds.dxgiFormat = fmt.snorm ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_UNORM;
    break;

  case F::RGBA8:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::RGB8:
  case F::ETC1A4:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_R8G8B8A8_UNORM + fmt.srgb);
    break;

  case F::RGBA16:
    dds.dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
    break;

  case F::BC6:
    dds.dxgiFormat = DXGI_FORMAT(DXGI_FORMAT_BC6H_UF16 + fmt.snorm);
    break;

  case F::RGB9E5:
    dds.dxgiFormat = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    break;

  case F::INVALID:
    dds.dxgiFormat = DXGI_FORMAT_UNKNOWN;
    break;
  }
}

void SetDDSLegacyFormat(DDS &dds, TexelInputFormat fmt) {
  switch (fmt.type) {
    using F = TexelInputFormatType;
  case F::BC1:
    dds = DDSFormat_DXT1;
    break;

  case F::BC2:
    dds = DDSFormat_DXT3;
    break;

  case F::BC3:
    dds = DDSFormat_DXT5;
    break;

  case F::BC4:
  case F::R8:
  case F::R4:
    dds = DDSFormat_L8;
    break;

  case F::RGBA4:
    dds = DDSFormat_A4R4G4B4;
    break;

  case F::R5G6B5:
    dds = DDSFormat_R5G6B5;
    break;

  case F::RGB8:
    dds = DDSFormat_R8G8B8;
    break;

  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
  case F::ETC1A4:
    dds = DDSFormat_A8R8G8B8;
    break;

  case F::BC5:
  case F::RG8:
  case F::RG4:
    if (fmt.swizzle.a == TexelSwizzleType::Green &&
        fmt.swizzle.r == TexelSwizzleType::Red) {
      dds = DDSFormat_A8L8;
    } else {
      dds = DDSFormat_R8G8B8;
    }
    break;

  case F::INVALID:
    break;
  }
}

uint8 DesiredQOIChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
  case F::R5G6B5:
  case F::BC5:
  case F::RG8:
  case F::RGB8:
  case F::R4:
  case F::RG4:
    return 3;

  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::RGBA4:
  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::INVALID:
    return 0;
  }

  return 4;
}

enum class PngColorType : uint8 {
  Gray = 0,
  RGB = 2,
  Palette = 3,
  GrayAlpha = 4,
  RGBA = 6,
};

uint32 GetPngChannels(PngColorType fmt) {
  switch (fmt) {
  case PngColorType::Gray:
    return 1;
  case PngColorType::GrayAlpha:
    return 2;
  case PngColorType::RGB:
    return 3;
  case PngColorType::RGBA:
    return 4;
  }

  return 0;
}

PngColorType DesiredPngColorType(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::R8:
  case F::BC4:
  case F::R4:
    return PngColorType::Gray;

  case F::BC5:
  case F::RG8:
  case F::RG4:
    // return PngColorType::GrayAlpha;

  case F::R5G6B5:
  case F::RGB8:
    return PngColorType::RGB;

  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::RGBA4:
  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return PngColorType::RGBA;

  case F::INVALID:
    return PngColorType::RGBA;
  }

  return PngColorType::RGBA;
}

PngColorType DesiredPngColorType(PngColorType type, TexelSwizzle &swizzle) {
  switch (type) {
  case PngColorType::RGBA: {
    if (swizzle.r == swizzle.g && swizzle.g == swizzle.b) {
      if (swizzle.a == TexelSwizzleType::White) {
        return PngColorType::Gray;
      }

      swizzle.g = swizzle.a;
      return PngColorType::GrayAlpha;
    }
    if (swizzle.a == TexelSwizzleType::White) {
      return PngColorType::RGB;
    }

    return type;
  }

  case PngColorType::RGB: {
    if (swizzle.r == swizzle.g && swizzle.g == swizzle.b) {
      if (swizzle.a == TexelSwizzleType::White) {
        return PngColorType::Gray;
      }

      swizzle.g = swizzle.a;
      return PngColorType::GrayAlpha;
    }

    if (swizzle.a != TexelSwizzleType::White) {
      return PngColorType::RGBA;
    }

    return type;
  }
  }

  return type;
}

/*
PngColorType DesiredPngColorType(PngColorType type, TexelSwizzle swizzle) {
  switch (type) {
  case PngColorType::Gray: {
    int8 factors[4]{0, 0, 0, 0};

    for (uint32 f = 0; f < 4; f++) {
      switch (swizzle.types[f]) {
      case TexelSwizzleType::Red:
        factors[f] = 1;
        break;
      case TexelSwizzleType::RedInverted:
        factors[f] = -1;
        break;

      default:
        break;
      }
    }

    if ((factors[0] == factors[1] &&
         factors[1] == factors[2]) ||           // rgb == red || redinverted
        (factors[0] == 0 && factors[1] == 0) || // r == red || redinverted
        (factors[0] == 0 && factors[2] == 0) || // g == red || redinverted
        (factors[1] == 0 && factors[2] == 0)) { // // b == red || redinverted
      if (factors[3]) {
        return PngColorType::GrayAlpha;
      }
      return type;
    } else {
      if (factors[3]) {
        return PngColorType::RGBA;
      }
      return PngColorType::RGB;
    }
  }

  case PngColorType::GrayAlpha: {
    int8 factors[4]{0, 0, 0, 0};

    for (uint32 f = 0; f < 4; f++) {
      switch (swizzle.types[f]) {
      case TexelSwizzleType::Red:
      case TexelSwizzleType::Alpha:
        factors[f] = 1;
        break;
      case TexelSwizzleType::RedInverted:
      case TexelSwizzleType::AlphaInverted:
        factors[f] = -1;
        break;

      default:
        break;
      }
    }

    if ((factors[0] == factors[1] &&
         factors[1] == factors[2]) ||           // rgb == red || redinverted
        (factors[0] == 0 && factors[1] == 0) || // r == red || redinverted
        (factors[0] == 0 && factors[2] == 0) || // g == red || redinverted
        (factors[1] == 0 && factors[2] == 0)) { // // b == red || redinverted
      if (factors[3]) {
        return PngColorType::GrayAlpha;
      }
      return type;
    } else {
      if (factors[3]) {
        return PngColorType::RGBA;
      }
      return PngColorType::RGB;
    }
  }
  }
}*/

uint8 DesiredDDSChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
  case F::R4:
    return 1;

  case F::R5G6B5:
    return 3;

  case F::BC5:
  case F::RG8:
  case F::RG4:
    return 2;

  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::RGBA4:
  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::RGB8:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::INVALID:
    return 0;
  }

  return 4;
}

uint8 DesiredDDSLegacyChannels(TexelInputFormat fmt) {
  switch (fmt.type) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
  case F::R4:
    return 1;

  case F::R5G6B5:
  case F::BC5:
  case F::RGB8:
    return 3;

  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::RGBA4:
  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::RG8:
  case F::RG4:
    if (fmt.swizzle.a == TexelSwizzleType::Green &&
        fmt.swizzle.r == TexelSwizzleType::Red) {
      return 2;
    } else {
      return 3;
    }
    break;
  case F::INVALID:
    return 0;
  }

  return 4;
}

uint8 FormatChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
  case F::R4:
    return 1;

  case F::BC5:
  case F::RG8:
  case F::RG4:
    return 2;

  case F::R5G6B5:
  case F::RGB8:
    return 3;

  case F::BC1:
  case F::BC2:
  case F::BC3:
  case F::RGBA4:
  case F::RGBA8:
  case F::BC7:
  case F::RGB5A1:
  case F::RGB10A2:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::ETC1A4:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::INVALID:
    return 0;
  }

  return 0;
}

struct LinearTile : TileBase {
  void reset(uint32, uint32, uint32) override {}
  uint32 get(uint32 inTexel) const override { return inTexel; }
};

struct MortonTile : TileBase {
  MortonSettings settings;

  MortonTile(uint32 width, uint32 height) : settings(width, height) {}

  void reset(uint32, uint32, uint32) override {}

  uint32 get(uint32 inTexel) const override {
    return MortonAddr(inTexel % settings.width, inTexel / settings.height,
                      settings);
  }
};

uint32 RoundToPow2(uint32 number) {
  number--;
  number |= number >> 1;
  number |= number >> 2;
  number |= number >> 4;
  number |= number >> 8;
  number |= number >> 16;
  number++;
  return number;
}

struct PS4Tile : TileBase {
  size_t width;
  size_t height;
  size_t widthp2;

  PS4Tile(size_t width_, size_t height_)
      : width(width_), height(height_),
        widthp2(RoundToPow2(std::max(width_, size_t(8)))) {}

  void reset(uint32, uint32, uint32) override {}

  static size_t MortonAddr(size_t x, size_t y, size_t width) {
    const size_t x0 = x & 1;
    const size_t x1 = (x & 2) << 1;
    const size_t x2 = (x & 4) << 2;

    const size_t y0 = (y & 1) << 1;
    const size_t y1 = (y & 2) << 2;
    const size_t y2 = (y & 4) << 3;

    size_t retval = x0 | x1 | x2 | y0 | y1 | y2;

    const size_t macroX = x / 8;
    const size_t macroY = y / 8;
    const size_t macroWidth = width / 8;

    const size_t macroAddr = (macroWidth * macroY) + macroX;

    return retval | (macroAddr << 6);
  }

  uint32 get(uint32 inTexel) const override {
    return MortonAddr(inTexel % width, inTexel / height, widthp2);
  }
};

struct N3DSTile : TileBase {
  uint32 width;
  uint32 height;
  uint32 (*getter)(uint32, const N3DSTile &);

  N3DSTile(size_t width_, size_t height_, TexelInputFormatType fmt)
      : width(width_), height(height_) {
    if (fmt == TexelInputFormatType::ETC1 ||
        fmt == TexelInputFormatType::ETC1A4) {
      width = (width + 3) / 4;
      height = (height + 3) / 4;
      getter = GetCompressed;
    } else {
      getter = GetRaw;
    }
  }

  void reset(uint32, uint32, uint32) override {}

  static uint32 GetRaw(uint32 inTexel, const N3DSTile &self) {
    uint32 x = inTexel % self.width;
    uint32 y = inTexel / self.width;
    // y = self.height - y - 1;

    const size_t x0 = x & 1;
    const size_t x1 = (x & 2) << 1;
    const size_t x2 = (x & 4) << 2;

    const size_t y0 = (y & 1) << 1;
    const size_t y1 = (y & 2) << 2;
    const size_t y2 = (y & 4) << 3;

    size_t retval = x0 | x1 | x2 | y0 | y1 | y2;

    const size_t macroX = x / 8;
    const size_t macroY = y / 8;
    const size_t macroWidth = self.width / 8;

    const size_t macroAddr = (macroWidth * macroY) + macroX;

    return retval | (macroAddr << 6);
  }

  static uint32 GetCompressed(uint32 inTexel, const N3DSTile &self) {
    uint32 x = inTexel % self.width;
    uint32 y = inTexel / self.width;
    // y = self.height - y - 1;

    const size_t x0 = x & 1;

    const size_t y0 = (y & 1) << 1;

    size_t retval = x0 | y0;

    const size_t macroX = x / 2;
    const size_t macroY = y / 2;
    const size_t macroWidth = self.width / 2;

    const size_t macroAddr = (macroWidth * macroY) + macroX;

    return retval | (macroAddr << 2);
  }

  uint32 get(uint32 inTexel) const override { return getter(inTexel, *this); }
};

struct NXTile : TileBase {
  uint32 width;
  uint32 height;
  uint32 numUsedYBlockBits;
  uint32 yBlockMask;
  uint32 yTailMask;
  uint32 macroTileWidth;
  uint32 BPT;
  uint32 yTailOffset;
  uint32 upperBound;
  uint32 numUsedMicroYBits;
  uint32 midYShift;

  NXTile(size_t width_, size_t height_, TexelInputFormatType fmt)
      : width(width_), height(height_), BPT(GetBPT(fmt)),
        upperBound(width * height) {
    numUsedMicroYBits = std::min(1 << uint32(std::round(log2(height))), 3);
    midYShift = numUsedMicroYBits + 3;

    const uint32 macroTileHeight =
        std::min(uint32(std::round(log2((height + 7) / 8))), 4U);

    // addr bits 9 - 13
    // y bits 3 - 7
    yBlockMask = ((1 << macroTileHeight) - 1) << numUsedMicroYBits;
    numUsedYBlockBits = numUsedMicroYBits + macroTileHeight;

    // y bits 8 - end
    yTailMask = ~((1 << numUsedYBlockBits) - 1);

    macroTileWidth = ((width * BPT) + 63) / 64;

    yTailOffset = macroTileHeight + 2;
  }

  void reset(uint32, uint32, uint32) override {}

  uint32 get(uint32 inTexel) const override {
    // 12 11 10 9  8  7  6  5  4  3  2  1  0
    // y  y  y  y  x  y  y  x  y  x  x  x  x
    // y*x*y{0,4}xyyxyx[x|0]{4}

    uint32 x = (inTexel % width) * BPT;
    uint32 y = inTexel / width;

    // Volumetrics not implemented
    // Some small rasters use alignment

    // x bits 0 - 5
    // y bits 0 - 2
    uint32 microTile = (x & 0xf) | ((y & 1) << 4) | ((x & 0x10) << 1) |
                       ((y & 0x6) << 5) | ((x & 0x20) << numUsedMicroYBits);

    // addr tail after yBlockMask bits
    // x bits 6 - end
    constexpr uint32 xTailMask = ~0x3f;

    uint32 macroTile = (((y & yBlockMask) << midYShift) | //
                        ((x & xTailMask) << numUsedYBlockBits)) +
                       (((y & yTailMask) << yTailOffset) * macroTileWidth);

    uint32 wholeTile = (microTile | macroTile) / BPT;

    /*if (wholeTile >= upperBound) [[unlikely]] {
      throw es::RuntimeError("NX tile error, accessing block out of range");
    }*/

    return wholeTile;
  }
};

using TileVariant =
    std::variant<LinearTile, MortonTile, PS4Tile, NXTile, N3DSTile>;

TileVariant TileVariantFromCtx(NewTexelContextCreate ctx) {
  uint32 width = ctx.width;
  uint32 height = ctx.height;

  switch (ctx.baseFormat.tile) {
  case TexelTile::Linear:
    return LinearTile{};

  case TexelTile::Morton:
    return MortonTile(width, height);

  case TexelTile::PS4:
    return PS4Tile(width, height);

  case TexelTile::NX:
    return NXTile(width, height, ctx.baseFormat.type);

  case TexelTile::N3DS:
    return N3DSTile(width, height, ctx.baseFormat.type);

  case TexelTile::Custom:
    break;
  }

  return LinearTile{};
}

void DecodeToRGB(const char *data, NewTexelContextCreate ctx,
                 std::span<UCVector> outData) {
  if (BlockCompression(ctx.baseFormat.type)) {
    ctx.width = (ctx.width + 3) / 4;
    ctx.height = (ctx.height + 3) / 4;
  }
  TileVariant tvar(TileVariantFromCtx(ctx));
  const TileBase *tiler =
      ctx.customTile && ctx.baseFormat.tile == TexelTile::Custom
          ? [&] {
              ctx.customTile->reset(ctx.width, ctx.height, ctx.depth);
              return ctx.customTile;
            }()
          : std::visit([](auto &item) -> const TileBase * { return &item; }, tvar);

  const size_t numBlocks = ctx.width * ctx.height;

  switch (ctx.baseFormat.type) {
    using F = TexelInputFormatType;
  case F::R5G6B5: {
    auto &codec = uni::FormatCodec::Get({
        .outType = uni::FormatType::UNORM,
        .compType = uni::DataType::R5G6B5,
    });

    size_t curTexel = 0;
    if (ctx.baseFormat.swapPacked) {
      const uint16 *iData = reinterpret_cast<const uint16 *>(data);
      for (auto &t : outData) {
        Vector4A16 value;
        uint16 col = *(iData + tiler->get(curTexel++));
        FByteswapper(col);

        codec.GetValue(value, reinterpret_cast<const char *>(&col));
        t = Vector(Vector4A16(value * 0xff)).Convert<uint8>();
      }
    } else {
      for (auto &t : outData) {
        Vector4A16 value;
        codec.GetValue(value, data + tiler->get(curTexel++) * 2);
        t = Vector(Vector4A16(value * 0xff)).Convert<uint8>();
      }
    }

    break;
  }

  case F::RG8: {
    size_t curTexel = 0;

    if (ctx.baseFormat.snorm) {
      const CVector2 *iData = reinterpret_cast<const CVector2 *>(data);
      for (auto &t : outData) {
        auto fData = (iData + tiler->get(curTexel++))->Convert<int>() + 0x80;
        t = UCVector(0, fData.y, fData.x);
      }
    } else {
      const UCVector2 *iData = reinterpret_cast<const UCVector2 *>(data);
      for (auto &t : outData) {
        auto tData = iData + tiler->get(curTexel++);
        t = UCVector(0, tData->y, tData->x);
      }
    }
    break;
  }

  case F::BC5:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC5Block(data + tiler->get(p) * 16,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width, 3);
    }
    break;

  case F::BC4:
    for (size_t p = 0; p < numBlocks; p++) {
      _DecodeBC4Block(data + tiler->get(p) * 8,
                      reinterpret_cast<char *>(outData.data()), p % ctx.width,
                      p / ctx.width, ctx.width, 3);
    }

    for (auto &p : outData) {
      p.y = p.z = p.x;
    }

    break;

  case F::RG4: {
    const uint8 *iData = reinterpret_cast<const uint8 *>(data);
    size_t curTexel = 0;

    for (auto &t : outData) {
      uint8 col = *(iData + tiler->get(curTexel++));
      t = UCVector(0, col << 4, col & 0xf0);
    }

    break;
  }

  case F::R8: {
    size_t curTexel = 0;
    for (auto &t : outData) {
      t = UCVector(*(data + tiler->get(curTexel++)));
    }

    break;
  }

  default:
    throw std::logic_error("Implement rgb decode");
  }
}

void DecodeToRGBA(const char *data, NewTexelContextCreate ctx,
                  std::span<UCVector4> outData) {
  if (BlockCompression(ctx.baseFormat.type)) {
    ctx.width = (ctx.width + 3) / 4;
    ctx.height = (ctx.height + 3) / 4;
  }
  TileVariant tvar(TileVariantFromCtx(ctx));
  const TileBase *tiler =
      ctx.customTile && ctx.baseFormat.tile == TexelTile::Custom
          ? [&] {
              ctx.customTile->reset(ctx.width, ctx.height, ctx.depth);
              return ctx.customTile;
            }()
          : std::visit([](auto &item) -> const TileBase * { return &item; }, tvar);

  const size_t numBlocks = ctx.width * ctx.height;

  switch (ctx.baseFormat.type) {
    using F = TexelInputFormatType;
  case F::RGB10A2: {
    auto &codec = uni::FormatCodec::Get({
        .outType = uni::FormatType::UNORM,
        .compType = uni::DataType::R10G10B10A2,
    });

    size_t curTexel = 0;
    if (ctx.baseFormat.swapPacked) {
      const uint32 *iData = reinterpret_cast<const uint32 *>(data);
      for (auto &t : outData) {
        Vector4A16 value;
        uint32 col = *(iData + tiler->get(curTexel++));
        FByteswapper(col);

        codec.GetValue(value, reinterpret_cast<const char *>(&col));
        t = (value * 0xff).Convert<uint8>();
      }
    } else {
      for (auto &t : outData) {
        Vector4A16 value;
        codec.GetValue(value, data + tiler->get(curTexel++) * 4);
        t = (value * 0xff).Convert<uint8>();
      }
    }
    break;
  }

  case F::RGBA4: {
    const uint16 *iData = reinterpret_cast<const uint16 *>(data);
    size_t curTexel = 0;
    if (ctx.baseFormat.swapPacked) {
      for (auto &t : outData) {
        uint16 col = *(iData + tiler->get(curTexel++));
        FByteswapper(col);
        t = UCVector4(col << 4, col & 0xf0, (col >> 4) & 0xf0,
                      (col >> 8) & 0xf0);
      }
    } else {
      for (auto &t : outData) {
        uint16 col = *(iData + tiler->get(curTexel++));
        t = UCVector4(col << 4, col & 0xf0, (col >> 4) & 0xf0,
                      (col >> 8) & 0xf0);
      }
    }

    break;
  }

  case F::RGB5A1: {
    const uint16 *iData = reinterpret_cast<const uint16 *>(data);
    size_t curTexel = 0;

    if (ctx.baseFormat.swapPacked) {
      for (auto &t : outData) {
        uint16 col = *(iData + tiler->get(curTexel++));
        FByteswapper(col);
        t = UCVector4(col << 3, (col >> 2) & 0xf8, (col >> 7) & 0xf8,
                      int16(col) >> 15);
      }
    } else {
      for (auto &t : outData) {
        uint16 col = *(iData + tiler->get(curTexel++));
        t = UCVector4(col << 3, (col >> 2) & 0xf8, (col >> 7) & 0xf8,
                      int16(col) >> 15);
      }
    }

    break;
  }

  case F::PVRTC2:
    pvr::PVRTDecompressPVRTC(data, 1, ctx.width, ctx.height,
                             reinterpret_cast<uint8_t *>(outData.data()));
    break;

  case F::PVRTC4:
    pvr::PVRTDecompressPVRTC(data, 0, ctx.width, ctx.height,
                             reinterpret_cast<uint8_t *>(outData.data()));
    break;

  case F::ETC1:
    if (ctx.baseFormat.tile != TexelTile::Linear) {
      uint32 bwidth = (ctx.width + 3) / 4;
      uint32 bheight = (ctx.height + 3) / 4;
      uint32 bnumBlocks = bwidth * bheight;
      std::vector<char> tmpBuffer(bwidth * bheight * 8);
      for (size_t p = 0; p < bnumBlocks; p++) {
        memcpy(tmpBuffer.data() + p * 8, data + tiler->get(p) * 8, 8);
      }

      if (ctx.baseFormat.tile == TexelTile::N3DS) {
        for (uint32 b = 0; b < bnumBlocks; b++) {
          FByteswapper(reinterpret_cast<uint64 *>(tmpBuffer.data())[b]);
        }
      }

      pvr::PVRTDecompressETC(tmpBuffer.data(), ctx.width, ctx.height,
                             reinterpret_cast<uint8_t *>(outData.data()),
                             0x100);
    } else {
      pvr::PVRTDecompressETC(data, ctx.width, ctx.height,
                             reinterpret_cast<uint8_t *>(outData.data()),
                             0x100);
    }
    break;

  case F::ETC1A4:
    if (ctx.baseFormat.tile == TexelTile::N3DS) {
      uint32 bwidth = (ctx.width + 3) / 4;
      uint32 bheight = (ctx.height + 3) / 4;
      uint32 bnumBlocks = bwidth * bheight;
      std::vector<char> tmpBuffer(bwidth * bheight * 16);
      for (size_t p = 0; p < bnumBlocks; p++) {
        memcpy(tmpBuffer.data() + p * 16, data + tiler->get(p) * 16, 16);
      }

      for (uint32 b = 0; b < bnumBlocks * 2; b++) {
        b++;
        FByteswapper(reinterpret_cast<uint64 *>(tmpBuffer.data())[b]);
      }

      pvr::PVRTDecompressETC(tmpBuffer.data(), ctx.width, ctx.height,
                             reinterpret_cast<uint8_t *>(outData.data()),
                             0x102);

      for (uint32 p = 0; p < bnumBlocks; p++) {
        uint8 *iData = reinterpret_cast<uint8 *>(tmpBuffer.data() + p * 16);

        uint32 x = p % bwidth;
        uint32 y = p / bwidth;
        uint32 blockOffset = bwidth * y * 16 + x * 4;

        for (size_t h = 0; h < 4; h++) {
          UCVector4 *addr = outData.data() + blockOffset + h * ctx.width;
          for (size_t w = 0; w < 4; w++) {
            uint32 tile = w * 4 + h;
            uint8 col = *(iData + (tile >> 1));
            addr[w].w = col << (4 * !(tile & 1)) & 0xf0;
          }
        }
      }
    } else {
      throw es::RuntimeError("ETC1A4 is only supported for TexelTile::N3DS");
    }
    break;

  case F::BC7: {
    uint32 localBlock[16];
    for (size_t p = 0; p < numBlocks; p++) {
      if (!detexDecompressBlockBPTC(
              reinterpret_cast<const uint8_t *>(data) + tiler->get(p) * 16, -1,
              0, reinterpret_cast<uint8_t *>(localBlock))) [[unlikely]] {
        throw es::RuntimeError("Failed to decompress BC7 block");
      }
      uint32 x = p % ctx.width;
      uint32 y = p / ctx.width;
      uint32 blockOffset = ctx.width * y * 16 + x * 4;

      for (size_t r = 0; r < 4; r++) {
        UCVector4 *addr = outData.data() + blockOffset + r * ctx.width * 4;
        memcpy(static_cast<void *>(addr), localBlock + r * 4, 16);
      }
    }
    break;
  }

  case F::BC1:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC1BlockA(data + tiler->get(p) * 8,
                      reinterpret_cast<char *>(outData.data()), p % ctx.width,
                      p / ctx.width, ctx.width);
    }

    break;

  case F::BC2:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC2Block(data + tiler->get(p) * 16,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width);
    }

    break;

  case F::BC3:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC3Block(data + tiler->get(p) * 16,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width);
    }

    break;

  default:
    throw std::logic_error("Implement rgba decode");
  }
}

void DecodeToRG(const char *data, NewTexelContextCreate ctx,
                std::span<UCVector2> outData) {
  if (BlockCompression(ctx.baseFormat.type)) {
    ctx.width = (ctx.width + 3) / 4;
    ctx.height = (ctx.height + 3) / 4;
  }
  TileVariant tvar(TileVariantFromCtx(ctx));
  const TileBase *tiler =
      ctx.customTile && ctx.baseFormat.tile == TexelTile::Custom
          ? [&] {
              ctx.customTile->reset(ctx.width, ctx.height, ctx.depth);
              return ctx.customTile;
            }()
          : std::visit([](auto &item) -> const TileBase * { return &item; }, tvar);
  const size_t numBlocks = ctx.width * ctx.height;

  switch (ctx.baseFormat.type) {
    using F = TexelInputFormatType;
  case F::BC5:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC5Block(data + tiler->get(p) * 16,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width, 2);
    }
    break;

  case F::RG4: {
    const uint8 *iData = reinterpret_cast<const uint8 *>(data);
    size_t curTexel = 0;

    for (auto &t : outData) {
      uint8 col = *(iData + tiler->get(curTexel++));
      t = UCVector2(col << 4, col & 0xf0);
    }

    break;
  }

  default:
    break;
  }
}

void DecodeToGray(const char *data, NewTexelContextCreate ctx,
                  std::span<char> outData) {
  if (BlockCompression(ctx.baseFormat.type)) {
    ctx.width = (ctx.width + 3) / 4;
    ctx.height = (ctx.height + 3) / 4;
  }
  TileVariant tvar(TileVariantFromCtx(ctx));
  const TileBase *tiler =
      ctx.customTile && ctx.baseFormat.tile == TexelTile::Custom
          ? [&] {
              ctx.customTile->reset(ctx.width, ctx.height, ctx.depth);
              return ctx.customTile;
            }()
          : std::visit([](auto &item) -> const TileBase * { return &item; }, tvar);

  switch (ctx.baseFormat.type) {
    using F = TexelInputFormatType;
  case F::BC4: {
    const size_t numBlocks = ctx.width * ctx.height;
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC4Block(data + tiler->get(p) * 8,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width);
    }
    break;
  }

  case F::R4: {
    const uint8 *iData = reinterpret_cast<const uint8 *>(data);
    size_t curTexel = 0;

    for (auto &t : outData) {
      uint32 tile = tiler->get(curTexel++);
      uint8 col = *(iData + (tile >> 1));
      t = col << (4 * !(tile & 1)) & 0xf0;
    }

    break;
  }

  default:
    break;
  }
}

void RetileData(const char *data, NewTexelContextCreate ctx, char *outData) {
  const size_t BPT = GetBPT(ctx.baseFormat.type);

  if (BlockCompression(ctx.baseFormat.type)) {
    ctx.width = (ctx.width + 3) / 4;
    ctx.height = (ctx.height + 3) / 4;
  }
  TileVariant tvar(TileVariantFromCtx(ctx));
  const TileBase *tiler =
      ctx.customTile && ctx.baseFormat.tile == TexelTile::Custom
          ? [&] {
              ctx.customTile->reset(ctx.width, ctx.height, ctx.depth);
              return ctx.customTile;
            }()
          : std::visit([](auto &item) -> const TileBase * { return &item; }, tvar);

  const size_t numBlocks = ctx.width * ctx.height;

  if (ctx.baseFormat.swapPacked) {
    if (ctx.baseFormat.type == TexelInputFormatType::R5G6B5 ||
        ctx.baseFormat.type == TexelInputFormatType::RGBA4) {
      uint16 *oData = reinterpret_cast<uint16 *>(outData);
      for (size_t p = 0; p < numBlocks; p++, oData++) {
        memcpy(oData, data + tiler->get(p) * BPT, BPT);
        FByteswapper(*oData);
      }

      return;
    } else if (ctx.baseFormat.type == TexelInputFormatType::RGB10A2 ||
               ctx.baseFormat.type == TexelInputFormatType::RGBA8) {
      uint32 *oData = reinterpret_cast<uint32 *>(outData);
      for (size_t p = 0; p < numBlocks; p++, oData++) {
        memcpy(oData, data + tiler->get(p) * BPT, BPT);
        FByteswapper(*oData);
      }

      return;
    }
  }

  for (size_t p = 0; p < numBlocks; p++) {
    memcpy(outData + p * BPT, data + tiler->get(p) * BPT, BPT);
  }
}

void Reencode(NewTexelContextCreate ctx, uint32 numDesiredChannels,
              const char *data, std::span<char> outData_) {
  const uint32 numInputChannels = FormatChannels(ctx.baseFormat.type);
  const uint32 numTexels = ctx.width * ctx.height;
  std::string tempBuffer;
  std::span<char> outData = outData_;
  uint32 outDataOffset = numTexels * (numDesiredChannels - numInputChannels);

  if (numDesiredChannels < numInputChannels) {
    tempBuffer.resize(numTexels * numInputChannels);
    outData = tempBuffer;
    outDataOffset = 0;
  }

  char *outDataBegin = outData.data() + outDataOffset;

  char white = 0xff;
  char black = 0;

  const char *swizzleData[4];
  uint8 factors[4]{1, 1, 1, 1};
  uint8 invert[4]{};

  for (uint8 index = 0; TexelSwizzleType t : ctx.baseFormat.swizzle.types) {
    switch (t) {
    case TexelSwizzleType::RedInverted:
      invert[index] = 0xff;
      [[fallthrough]];
    case TexelSwizzleType::Red:
      swizzleData[index] = outDataBegin + 2;
      break;
    case TexelSwizzleType::GreenInverted:
      invert[index] = 0xff;
      [[fallthrough]];
    case TexelSwizzleType::Green:
      swizzleData[index] = outDataBegin + 1;
      break;
    case TexelSwizzleType::BlueInverted:
    case TexelSwizzleType::DeriveZOrBlueInverted:
      invert[index] = 0xff;
      [[fallthrough]];
    case TexelSwizzleType::Blue:
    case TexelSwizzleType::DeriveZOrBlue:
      swizzleData[index] = outDataBegin;
      break;
    case TexelSwizzleType::AlphaInverted:
      invert[index] = 0xff;
      [[fallthrough]];
    case TexelSwizzleType::Alpha:
      swizzleData[index] = outDataBegin + 3;
      break;
    case TexelSwizzleType::Black:
    case TexelSwizzleType::DeriveZ:
      swizzleData[index] = &black;
      factors[index] = 0;
      break;
    case TexelSwizzleType::White:
      swizzleData[index] = &white;
      factors[index] = 0;
      break;

    default:
      break;
    }

    index++;
  }

  if (numInputChannels == 1) {
    for (uint32 i = 1; i < numDesiredChannels; i++) {
      if (swizzleData[i] != &white && swizzleData[i] != &black) {
        invert[i] = invert[0];
        swizzleData[i] = swizzleData[0];
        factors[i] = factors[0];
      }
    }

    if (ctx.baseFormat.type == TexelInputFormatType::R8) {
      RetileData(data, ctx, outDataBegin);
    } else {
      DecodeToGray(
          data, ctx,
          outData.subspan(numTexels * (numDesiredChannels - 1), numTexels));
    }
  } else if (numInputChannels == 2) {
    if (ctx.baseFormat.type == TexelInputFormatType::RG8) {
      RetileData(data, ctx, outDataBegin);
    } else {
      DecodeToRG(data, ctx,
                 {reinterpret_cast<UCVector2 *>(outDataBegin), numTexels});
    }
  } else if (numInputChannels == 3) {
    if (ctx.baseFormat.type == TexelInputFormatType::RGB8) {
      RetileData(data, ctx, outDataBegin);
    } else {
      DecodeToRGB(data, ctx,
                  {reinterpret_cast<UCVector *>(outDataBegin), numTexels});
    }
  } else if (numInputChannels == 4) {
    if (ctx.baseFormat.type == TexelInputFormatType::RGBA8) {
      RetileData(data, ctx, outDataBegin);
    } else {
      DecodeToRGBA(data, ctx,
                   {reinterpret_cast<UCVector4 *>(outDataBegin), numTexels});
    }
  }

  for (uint32 t = 0; t < numTexels; t++) {
    char texel[4];

    for (uint32 s = 0; s < numDesiredChannels; s++) {
      texel[s] = (invert[s] - uint8(*swizzleData[s])) * int8(~invert[s] | 1);

      swizzleData[s] += factors[s] * numInputChannels;
    }

    memcpy(outData_.data() + t * numDesiredChannels, texel, numDesiredChannels);
  }

  [&] {
    if (numDesiredChannels < 3) {
      return;
    }

    if (ctx.baseFormat.swizzle.b == TexelSwizzleType::DeriveZ) {
      ComputeBC5Blue(outData_.data(), numTexels * numDesiredChannels,
                     numDesiredChannels);
      return;
    }

    if (numInputChannels == 2) {
      if (ctx.baseFormat.swizzle.b != TexelSwizzleType::DeriveZOrBlue &&
          ctx.baseFormat.swizzle.b != TexelSwizzleType::DeriveZOrBlueInverted) {
        return;
      }

      ComputeBC5Blue(outData_.data(), numTexels * numDesiredChannels,
                     numDesiredChannels);
    }
  }();

  if ((numDesiredChannels > 2 &&
       ctx.baseFormat.swizzle.b == TexelSwizzleType::DeriveZ) ||
      (numDesiredChannels == 2 &&
       (ctx.baseFormat.swizzle.b == TexelSwizzleType::DeriveZOrBlue ||
        ctx.baseFormat.swizzle.b == TexelSwizzleType::DeriveZOrBlueInverted))) {
    ComputeBC5Blue(outData_.data(), numTexels * numDesiredChannels,
                   numDesiredChannels);
  }

  if (ctx.postProcess) {
    ctx.postProcess(outData_.data(), numDesiredChannels, numTexels);
  }
}

struct NewTexelContextQOI : NewTexelContextImpl {
  qoi_desc qoiDesc{};
  std::string yasBuffer;

  NewTexelContextQOI(NewTexelContextCreate ctx_) : NewTexelContextImpl(ctx_) {
    qoiDesc.width = ctx.width;
    qoiDesc.height = ctx.height * std::max(uint16(1), ctx.depth);
    qoiDesc.colorspace = !ctx.baseFormat.srgb;
    qoiDesc.channels = DesiredQOIChannels(ctx.baseFormat.type);
  }

  void InitBuffer() {
    if (BlockCompression(ctx.baseFormat.type)) {
      uint32 widthPadding = qoiDesc.width % 4;
      widthPadding = widthPadding ? 4 - widthPadding : 0;
      uint32 heightPadding = qoiDesc.height % 4;
      heightPadding = heightPadding ? 4 - heightPadding : 0;

      const uint32 rasterDataSize = (qoiDesc.width + widthPadding) *
                                    (qoiDesc.height + heightPadding) *
                                    qoiDesc.channels;
      yasBuffer.resize(rasterDataSize);
      return;
    }

    const uint32 rasterDataSize =
        qoiDesc.width * qoiDesc.height * qoiDesc.channels;
    yasBuffer.resize(rasterDataSize);
  }

  void SendRasterData(const void *data, TexelInputLayout layout,
                      TexelInputFormat *) override {
    if (layout.mipMap > 0) {
      return;
    }

    auto mctx = ctx;
    mctx.height *= std::max(mctx.depth, uint16(1));

    auto Write = [&](const void *buffer) {
      int encodedSize = 0;
      void *buffa = qoi_encode(buffer, &qoiDesc, &encodedSize);
      std::string suffix;

      if (ctx.arraySize > 1) {
        suffix.push_back('_');
        suffix.append(std::to_string(layout.layer));
      }

      if (layout.face != CubemapFace::NONE) {
        suffix.push_back('_');
        static const ReflectedEnum *refl = GetReflectedEnum<CubemapFace>();
        suffix.append(refl->names[uint32(layout.face)]);
      }

      suffix.append(".qoi");

      outCtx->NewFile(std::string(pathOverride.ChangeExtension(suffix)));
      outCtx->SendData({static_cast<char *>(buffa), size_t(encodedSize)});

      free(buffa);
    };

    bool mustDecode =
        IsFormatSupported(ctx.formatOverride, ctx.baseFormat.type) ||
        MustSwap(ctx.baseFormat.type, ctx.baseFormat.swapPacked) ||
        ctx.baseFormat.tile != TexelTile::Linear ||
        MustSwizzle(ctx.baseFormat.swizzle, qoiDesc.channels);

    if (mustDecode) {
      InitBuffer();
      Reencode(mctx, qoiDesc.channels, static_cast<const char *>(data),
               yasBuffer);
      Write(yasBuffer.data());
    } else {
      Write(data);
    }
  }

  bool ShouldDoMipmaps() override { return false; }

  void Finish() override {}
};

struct NewTexelContextQOIBMP : NewTexelContextQOI {
  BMPHeader bmpHdr;
  BMPInfoHeader bmpInfo;

  NewTexelContextQOIBMP(NewTexelContextCreate ctx_) : NewTexelContextQOI(ctx_) {
    if (BMPFallback(ctx.baseFormat.type)) {
      FillBmpFormat(ctx, ctx.baseFormat, bmpInfo);
    }
  }

  bool ShouldDoMipmaps() override { return false; }

  void Finish() override {}
};

struct NewTexelContextDDS : NewTexelContextImpl {
  DDS dds;
  DDS::Mips ddsMips{};
  std::vector<std::vector<bool>> mipmaps;
  std::string yasBuffer;
  bool mustDecode;
  uint8 numChannels;

  NewTexelContextDDS(NewTexelContextCreate ctx_, bool isBase = false)
      : NewTexelContextImpl(ctx_), dds(MakeDDS(ctx)),
        mustDecode(IsFormatSupported(ctx.formatOverride, ctx.baseFormat.type)),
        numChannels(DesiredDDSChannels(ctx.baseFormat.type)) {
    mipmaps.resize(std::max(1U, dds.mipMapCount));
    const int8 numFaces = std::max(ctx.numFaces, int8(1));
    const uint32 arraySize = dds.arraySize * numFaces;

    if (arraySize > 1) {
      for (auto &a : mipmaps) {
        a.resize(arraySize);
      }
    } else {
      size_t curSlice = 0;
      for (auto &a : mipmaps) {
        a.resize(std::max(uint16(1), ddsMips.numSlices[curSlice++]));
      }
    }

    if (!isBase) {
      TexelInputFormat baseFmt = ctx.baseFormat;
      mustDecode |= MustSwizzle(baseFmt.swizzle, numChannels);

      if (mustDecode) {
        switch (numChannels) {
        case 1:
          baseFmt.type = TexelInputFormatType::R8;
          break;
        case 2:
          baseFmt.type = TexelInputFormatType::RG8;
          break;
        case 3:
          baseFmt.type = TexelInputFormatType::RGB8;
          break;
        case 4:
          baseFmt.type = TexelInputFormatType::RGBA8;
          break;
        default:
          break;
        }
      }

      SetDDSFormat(dds, baseFmt);
      dds.ComputeBPP();
      yasBuffer.resize(dds.ComputeBufferSize(ddsMips));
      for (int32 i = 0; i < ctx.numFaces; i++) {
        dds.caps01 -= static_cast<DDS_HeaderEnd::Caps01Flags>(i + 10);
      }
    }
  }

  bool ShouldWrite() const {
    for (auto &l : mipmaps) {
      for (bool m : l) {
        if (!m) {
          return false;
        }
      }
    }

    return true;
  }

  void SendRasterData(const void *data, TexelInputLayout layout,
                      TexelInputFormat *) override {
    if (!ShouldDoMipmaps() && layout.mipMap > 0) {
      return;
    }

    auto &mipLayers = mipmaps.at(layout.mipMap);

    const uint32 layer =
        layout.layer * std::max(int8(1), ctx.numFaces) + uint8(layout.face);

    if (mipLayers.empty() || mipLayers.at(layer)) {
      // mipmap already filled
      return;
    }

    mipLayers.at(layer).flip();

    auto rData = yasBuffer.data() + ddsMips.offsets[layout.mipMap];

    const size_t rDataSize =
        ddsMips.sizes[layout.mipMap] *
        std::max(ddsMips.numSlices[layout.mipMap], uint16(1));

    if (layout.face != CubemapFace::NONE) {
      dds.caps01 +=
          static_cast<DDS_HeaderEnd::Caps01Flags>(int(layout.face) + 9);
      rData += ddsMips.frameStride * (int(layout.face) - 1);
    }

    ptrdiff_t boundDiff = (&yasBuffer.back() + 1) - (rData + rDataSize);
    if (boundDiff < 0) {
      throw es::RuntimeError("Writing image data beyond buffer's bounds");
    }

    auto mctx = ctx;

    for (uint32 m = 0; m < layout.mipMap; m++) {
      mctx.width = std::max(1, mctx.width / 2);
      mctx.depth = std::max(1, mctx.depth / 2);
      mctx.height = std::max(1, mctx.height / 2);
    }

    mctx.height *= mctx.depth;

    if (mustDecode) {
      Reencode(mctx, numChannels, static_cast<const char *>(data),
               {rData, rDataSize});
    } else if (ctx.baseFormat.tile == TexelTile::Linear &&
               !ctx.baseFormat.swapPacked) {
      memcpy(rData, data, rDataSize);
    } else {
      RetileData(static_cast<const char *>(data), mctx, rData);
    }

    if (ShouldWrite()) {
      outCtx->NewFile(std::string(pathOverride.ChangeExtension2("dds")));
      outCtx->SendData(
          {reinterpret_cast<const char *>(&dds), size_t(dds.DDS_SIZE)});
      outCtx->SendData(yasBuffer);

      es::Dispose(yasBuffer);
    }
  }

  bool ShouldDoMipmaps() override {
    return mainSettings.texelSettings.processMipMaps;
  }

  void Finish() override {
    if (!ShouldWrite()) {
      throw es::RuntimeError("Incomplete dds file");
    }
  }
};

struct NewTexelContextDDSLegacy : NewTexelContextDDS {
  std::vector<std::string> arrayMipmapBuffers;
  uint8 numChannels;

  NewTexelContextDDSLegacy(NewTexelContextCreate ctx_)
      : NewTexelContextDDS(ctx_, true),
        numChannels(DesiredDDSLegacyChannels(ctx_.baseFormat)) {
    arrayMipmapBuffers.resize(dds.arraySize);
    dds.arraySize = 1;
    TexelInputFormat baseFmt = ctx.baseFormat;
    mustDecode |= MustSwizzle(ctx.baseFormat.swizzle, numChannels);

    if (mustDecode) {
      switch (numChannels) {
      case 1:
        baseFmt.type = TexelInputFormatType::R8;
        break;
      case 2:
        baseFmt.type = TexelInputFormatType::RG8;
        break;
      case 3:
        baseFmt.type = TexelInputFormatType::RGB8;
        break;
      case 4:
        baseFmt.type = TexelInputFormatType::RGBA8;
        break;
      default:
        break;
      }
    }

    SetDDSLegacyFormat(dds, baseFmt);
    dds.ComputeBPP();
    dds.ComputeBufferSize(ddsMips);

    for (int32 i = 0; i < ctx.numFaces; i++) {
      dds.caps01 -= static_cast<DDS_HeaderEnd::Caps01Flags>(i + 10);
    }
  }

  bool ShouldWrite(int32 layer) const {
    if (ctx.arraySize > 1 && layer > -1) {
      const int8 numFaces = std::max(int8(1), ctx.numFaces);
      for (auto &m : mipmaps) {
        for (int8 f = 0; f < numFaces; f++) {
          if (!m.at(layer * numFaces + f)) {
            return false;
          }
        }
      }
    } else {
      for (auto &l : mipmaps) {
        for (bool m : l) {
          if (!m) {
            return false;
          }
        }
      }
    }

    return true;
  }

  void SendRasterData(const void *data, TexelInputLayout layout,
                      TexelInputFormat *) override {
    if (!ShouldDoMipmaps() && layout.mipMap > 0) {
      return;
    }

    auto &buffar = arrayMipmapBuffers.at(layout.layer);
    auto &mipLayers = mipmaps.at(layout.mipMap);

    const uint32 layer = layout.layer * std::max(int8(1), ctx.numFaces) +
                         std::max(int(layout.face) - 1, 0);

    if (mipLayers.empty() || mipLayers.at(layer)) {
      // mipmap already filled
      return;
    }

    mipLayers.at(layer).flip();

    if (buffar.empty()) {
      buffar.resize(ddsMips.frameStride * std::max(int8(1), ctx.numFaces));
    }

    const size_t rDataSize =
        ddsMips.sizes[layout.mipMap] *
        std::max(ddsMips.numSlices[layout.mipMap], uint16(1));

    auto rData = buffar.data() + ddsMips.offsets[layout.mipMap];

    if (layout.face != CubemapFace::NONE) {
      dds.caps01 +=
          static_cast<DDS_HeaderEnd::Caps01Flags>(int(layout.face) + 9);
      rData += ddsMips.frameStride * (int(layout.face) - 1);
    }

    ptrdiff_t boundDiff = (&buffar.back() + 1) - (rData + rDataSize);
    if (boundDiff < 0) {
      throw es::RuntimeError("Writing image data beyond buffer's bounds");
    }

    auto mctx = ctx;

    for (uint32 m = 0; m < layout.mipMap; m++) {
      mctx.width = std::max(1, mctx.width / 2);
      mctx.depth = std::max(1, mctx.depth / 2);
      mctx.height = std::max(1, mctx.height / 2);
    }

    mctx.height *= mctx.depth;

    if (mustDecode) {
      Reencode(mctx, numChannels, static_cast<const char *>(data),
               {rData, rDataSize});
    } else if (ctx.baseFormat.tile == TexelTile::Linear &&
               !ctx.baseFormat.swapPacked) {
      memcpy(rData, data, rDataSize);
    } else {
      RetileData(static_cast<const char *>(data), mctx, rData);
    }

    if (ShouldWrite(layout.layer)) {
      std::string suffix;

      if (ctx.arraySize > 1) {
        suffix.push_back('_');
        suffix.append(std::to_string(layout.layer));
      }
      suffix.append(".dds");

      outCtx->NewFile(std::string(pathOverride.ChangeExtension(suffix)));
      outCtx->SendData(
          {reinterpret_cast<const char *>(&dds), size_t(dds.LEGACY_SIZE)});
      outCtx->SendData(buffar);

      es::Dispose(buffar);
    }
  }

  void Finish() override {
    if (!ShouldWrite(-1)) {
      throw es::RuntimeError("Incomplete dds file");
    }
  }
};

struct PngIHDR {
  uint32 id = CompileFourCC("IHDR");
  uint32 width;
  uint32 height;
  uint8 bitDepth = 8;
  PngColorType colorType = PngColorType::RGBA;
  uint8 compressionMethod = 0;
  uint8 filterMethod = 0;
  uint8 interlaceMethod = 0;
};

void FByteswapper(PngIHDR &item) {
  FByteswapper(item.width);
  FByteswapper(item.height);
}

struct DeflateBlock {
  int16 blockSize;
  int16 blockSizeComplement;

  void BlockSize(int16 size) {
    blockSize = size;
    blockSizeComplement = ~size;
  }

  void SwapEndian() {}
};

struct Png {
  uint64 id = 0x0A1A0A0D474E5089;
  uint32 ihdrSize = 13;
  PngIHDR ihdr;
};

struct PngData {
  uint32 ihdrCRC;
  uint32 idatSize;
  uint32 idat = CompileFourCC("IDAT");
};

void FByteswapper(Png &item) {
  FByteswapper(item.ihdrSize);
  FByteswapper(item.ihdr);
}

void FByteswapper(PngData &item) {
  FByteswapper(item.ihdrCRC);
  FByteswapper(item.idatSize);
}

struct PngEnd {
  uint32 idatCrc;
  uint32 iendSize = 0;
  uint32 iend = CompileFourCC("IEND");
  uint32 iendCrc = crc32b(0, "IEND", 4);
};

void FByteswapper(PngEnd &item) {
  FByteswapper(item.idatCrc);
  FByteswapper(item.iendSize);
  FByteswapper(item.iendCrc);
}

std::string MakeZlibStream(const char *buffer_, PngIHDR &hdr) {
  std::stringstream str;
  BinWritterRef_e wr(str);
  wr.Write(uint16(0x178));
  wr.SwapEndian(true);
  const uint32 numChannels = GetPngChannels(hdr.colorType);
  const uint32 pitch = hdr.width * numChannels;
  const uint32 scanLine = pitch + 1;
  uint32 adlerA = 1;
  uint32 adlerB = 0;

  auto Adler = [&](uint8 c) {
    adlerA = (c + adlerA) % 65521;
    adlerB = (adlerA + adlerB) % 65521;
  };

  auto AdlerL = [&](const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
      Adler(data[i]);
    }
  };

  DeflateBlock block;
  block.BlockSize(scanLine);

  if (numChannels < 3) {
    for (uint32 h = 0; h < hdr.height; h++) {
      str.put(h == hdr.height - 1);
      wr.Write(block);
      str.put(0); // filter type
      Adler(0);
      AdlerL(buffer_ + pitch * h, pitch);
      str.write(buffer_ + pitch * h, pitch);
    }
  } else if (numChannels == 3) {
    for (uint32 h = 0; h < hdr.height; h++) {
      str.put(h == hdr.height - 1);
      wr.Write(block);
      str.put(0); // filter type
      Adler(0);
      const char *startLine = buffer_ + pitch * h;

      for (uint32 w = 0; w < hdr.width; w++) {
        char rgb[3];
        memcpy(rgb, startLine + w * 3, 3);
        std::swap(rgb[0], rgb[2]);
        AdlerL(rgb, 3);
        str.write(rgb, 3);
      }
    }
  } else {
    for (uint32 h = 0; h < hdr.height; h++) {
      str.put(h == hdr.height - 1);
      wr.Write(block);
      str.put(0); // filter type
      Adler(0);
      const char *startLine = buffer_ + pitch * h;

      for (uint32 w = 0; w < hdr.width; w++) {
        char rgb[4];
        memcpy(rgb, startLine + w * 4, 4);
        std::swap(rgb[0], rgb[2]);
        AdlerL(rgb, 4);
        str.write(rgb, 4);
      }
    }
  }

  adlerA |= adlerB << 16;
  wr.Write(adlerA);
  return std::move(str).str();
}

struct NewTexelContextPNG : NewTexelContextImpl {
  std::string yasBuffer;
  Png hdr;
  uint32 numChannels;

  NewTexelContextPNG(NewTexelContextCreate ctx_) : NewTexelContextImpl(ctx_) {
    hdr.ihdr.width = ctx.width;
    hdr.ihdr.height = ctx.height * std::max(uint16(1), ctx.depth);
    hdr.ihdr.colorType = DesiredPngColorType(
        DesiredPngColorType(ctx.baseFormat.type), ctx.baseFormat.swizzle);
    numChannels = GetPngChannels(hdr.ihdr.colorType);
  }

  void InitBuffer() {
    if (BlockCompression(ctx.baseFormat.type)) {
      uint32 widthPadding = hdr.ihdr.width % 4;
      widthPadding = widthPadding ? 4 - widthPadding : 0;
      uint32 heightPadding = hdr.ihdr.height % 4;
      heightPadding = heightPadding ? 4 - heightPadding : 0;

      const uint32 rasterDataSize = (hdr.ihdr.width + widthPadding) *
                                    (hdr.ihdr.height + heightPadding) *
                                    numChannels;
      yasBuffer.resize(rasterDataSize);
      return;
    }

    const uint32 rasterDataSize =
        hdr.ihdr.width * hdr.ihdr.height * numChannels;
    yasBuffer.resize(rasterDataSize);
  }

  void SendRasterData(const void *data, TexelInputLayout layout,
                      TexelInputFormat *) override {
    if (layout.mipMap > 0) {
      return;
    }

    auto mctx = ctx;
    mctx.height *= std::max(mctx.depth, uint16(1));

    auto Write = [&](const void *buffer) {
      std::string suffix;

      if (ctx.arraySize > 1) {
        suffix.push_back('_');
        suffix.append(std::to_string(layout.layer));
      }

      if (layout.face != CubemapFace::NONE) {
        suffix.push_back('_');
        static const ReflectedEnum *refl = GetReflectedEnum<CubemapFace>();
        suffix.append(refl->names[uint32(layout.face)]);
      }

      suffix.append(".png");

      std::string encodedData =
          MakeZlibStream(static_cast<const char *>(buffer), hdr.ihdr);

      Png hdrCopy = hdr;
      FByteswapper(hdrCopy);
      PngData hdrData{
          .ihdrCRC = crc32b(0, reinterpret_cast<const char *>(&hdrCopy.ihdr),
                            sizeof(hdrCopy.ihdr) - 3),
          .idatSize = uint32(encodedData.size()),

      };
      FByteswapper(hdrData);

      PngEnd tail{
          .idatCrc = crc32b(crc32b(0, "IDAT", 4), encodedData.data(),
                            encodedData.size()),
      };

      FByteswapper(tail);

      outCtx->NewFile(std::string(pathOverride.ChangeExtension(suffix)));
      outCtx->SendData(
          {reinterpret_cast<const char *>(&hdrCopy), sizeof(hdrCopy) - 3});
      outCtx->SendData(
          {reinterpret_cast<const char *>(&hdrData), sizeof(hdrData)});
      outCtx->SendData(encodedData);
      outCtx->SendData({reinterpret_cast<const char *>(&tail), sizeof(tail)});
    };

    bool mustDecode =
        IsFormatSupported(ctx.formatOverride, ctx.baseFormat.type) ||
        MustSwap(ctx.baseFormat.type, ctx.baseFormat.swapPacked) ||
        ctx.baseFormat.tile != TexelTile::Linear ||
        MustSwizzle(ctx.baseFormat.swizzle, numChannels);

    if (mustDecode) {
      InitBuffer();
      Reencode(mctx, numChannels, static_cast<const char *>(data), yasBuffer);
      Write(yasBuffer.data());
    } else {
      Write(data);
    }
  }

  bool ShouldDoMipmaps() override { return false; }

  void Finish() override {}
};

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx) {
  if (ctx.formatOverride == TexelContextFormat::Config) {
    ctx.formatOverride = OutputFormat();
  }

  switch (ctx.formatOverride) {
  case TexelContextFormat::DDS:
    return std::make_unique<NewTexelContextDDS>(ctx);
  case TexelContextFormat::DDS_Legacy:
    return std::make_unique<NewTexelContextDDSLegacy>(ctx);
  case TexelContextFormat::QOI_BMP:
    return std::make_unique<NewTexelContextQOIBMP>(ctx);
  case TexelContextFormat::QOI:
    return std::make_unique<NewTexelContextQOI>(ctx);
  case TexelContextFormat::UPNG:
    return std::make_unique<NewTexelContextPNG>(ctx);
  default:
    throw std::logic_error("Image format not supported");
  }
}

void NewTexelContextImpl::ProcessContextData() {
  auto layout = ComputeTraditionalDataLayout();

  for (uint32 a = 0; a < ctx.arraySize; a++) {
    const char *entryBegin =
        static_cast<const char *>(ctx.data) + a * layout.groupSize;

    if (ctx.numFaces > 0) {
      for (int8 f = 0; f < ctx.numFaces; f++) {
        const char *faceBegin = entryBegin + f * layout.mipGroupSize;

        for (uint32 m = 0; m < ctx.numMipmaps; m++) {
          SendRasterData(faceBegin + layout.mipOffsets[m],
                         {
                             .mipMap = uint8(m),
                             .face = static_cast<CubemapFace>(f + 1),
                             .layer = uint16(a),
                         });
        }
      }
    } else {
      for (uint32 m = 0; m < ctx.numMipmaps; m++) {
        SendRasterData(entryBegin + layout.mipOffsets[m],
                       {
                           .mipMap = uint8(m),
                           .layer = uint16(a),
                       });
      }
    }
  }
}

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppContext *actx) {
  auto retVal = CreateTexelContext(ctx);
  if (ctx.texelOutput) {
    retVal->outCtx = ctx.texelOutput;
  } else {
    TexelOutputContext otx;
    otx.ctx = actx;
    retVal->outVariant = otx;
    retVal->outCtx = &std::get<TexelOutputContext>(retVal->outVariant);
  }

  retVal->pathOverride = actx->workingFile;
  if (ctx.data) {
    retVal->ProcessContextData();
    return {};
  }

  return retVal;
}

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx, AppExtractContext *ectx,
                   const std::string &path) {
  auto retVal = CreateTexelContext(ctx);
  if (ctx.texelOutput) {
    retVal->outCtx = ctx.texelOutput;
  } else {
    TexelOutputExtractContext otx;
    otx.ctx = ectx;
    retVal->outVariant = otx;
    retVal->outCtx = &std::get<TexelOutputExtractContext>(retVal->outVariant);
  }

  retVal->pathOverride.Load(path);

  if (ctx.data) {
    retVal->ProcessContextData();
    return {};
  }
  return retVal;
}

void TexelOutputContext::SendData(std::string_view data) {
  str->write(data.data(), data.size());
}
void TexelOutputContext::NewFile(std::string filePath) {
  str = &ctx->NewFile(filePath).str;
}
