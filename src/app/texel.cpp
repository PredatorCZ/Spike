#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO

#include "spike/app/texel.hpp"
#include "bc7decomp.h"
#include "pvr_decompress.hpp"
#include "qoi.h"
#include "spike/app/context.hpp"
#include "spike/format/DDS.hpp"
#include "spike/gpu/BlockDecoder.inl"
#include "spike/gpu/addr_ps3.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/uni/format.hpp"
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

bool MustDecode(TexelInputFormatType fmt) {
  using F = TexelInputFormatType;

  switch (fmt) {
    // Not supported
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
    return true;

    // DDS, DDS_Legacy only
  case F::BC1:
  case F::BC2:
  case F::BC3:
    return OutputFormat() == TexelContextFormat::QOI ||
           OutputFormat() == TexelContextFormat::QOI_BMP;

    // DDS only
  case F::BC4:
  case F::BC5:
  case F::BC7:
  case F::RG8:
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return OutputFormat() != TexelContextFormat::DDS;

    // BMP, DDS only
  case F::RGB10A2:
  case F::RGB5A1:
    return OutputFormat() == TexelContextFormat::QOI ||
           OutputFormat() == TexelContextFormat::DDS_Legacy;

    // BMP only
  case F::P8:
  case F::P4:
    return OutputFormat() != TexelContextFormat::QOI_BMP;

  // DDS, DDS_Legacy, BMP only
  case F::RGBA4:
  case F::R5G6B5:
  case F::R8:
    return OutputFormat() == TexelContextFormat::QOI;

    // Supported for all
  case F::RGBA8:
  case F::INVALID:
    return false;

  // QOI, DDS_Legacy only
  case F::RGB8:
    return OutputFormat() == TexelContextFormat::DDS;
  }

  return false;
}

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
    return 2;

  case F::RGB10A2:
  case F::RGBA8:
  case F::RGB9E5:
    return 4;

  case F::P8:
  case F::P4:
  case F::R8:
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

BMPMask SwizzleMask(BMPMask bits, TexelSwizzle swizzle[4]) {
  uint32 currentOffset = 0;
  BMPMask retVal{};

  for (uint32 c = 0; c < 4; c++) {
    uint32 swizzleIndex =
        swizzle[c] > TexelSwizzle::Alpha ? c : uint32(swizzle[c]);

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
    dds.dxgiFormat =
        fmt.snorm ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_UNORM;
    break;

  case F::R8:
    dds.dxgiFormat = fmt.snorm ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_UNORM;
    break;

  case F::RGBA8:
  case F::P8:
  case F::P4:
  case F::PVRTC2:
  case F::PVRTC4:
  case F::ETC1:
  case F::RGB8:
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
    dds = DDSFormat_A8R8G8B8;
    break;

  case F::BC5:
  case F::RG8:
    dds = DDSFormat_R8G8B8;
    break;

  case F::INVALID:
    break;
  }
}

uint8 GetQOIChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
  case F::R5G6B5:
  case F::BC5:
  case F::RG8:
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
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::INVALID:
    return 0;
  }

  return 4;
}

uint8 GetDDSChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
    return 1;

  case F::R5G6B5:
    return 3;

  case F::BC5:
  case F::RG8:
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

uint8 GetDDSLegacyChannels(TexelInputFormatType fmt) {
  switch (fmt) {
    using F = TexelInputFormatType;
  case F::BC4:
  case F::R8:
    return 1;

  case F::R5G6B5:
  case F::BC5:
  case F::RG8:
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
  case F::RGBA16:
  case F::BC6:
  case F::RGB9E5:
    return 4;

  case F::INVALID:
    return 0;
  }

  return 4;
}

struct LinearTile : TileBase {
  void reset(uint32, uint32, uint32) {}
  uint32 get(uint32 inTexel) const override { return inTexel; }
};

struct MortonTile : TileBase {
  MortonSettings settings;

  MortonTile(uint32 width, uint32 height) : settings(width, height) {}

  void reset(uint32, uint32, uint32) {}

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

struct MortonPow2Tile : TileBase {
  size_t width;
  size_t height;
  size_t widthp2;

  MortonPow2Tile(size_t width_, size_t height_)
      : width(width_), height(height_),
        widthp2(RoundToPow2(std::max(width_, size_t(8)))) {}

  void reset(uint32, uint32, uint32) {}

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

  void reset(uint32, uint32, uint32) {}

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
      throw std::runtime_error("NX tile error, accessing block out of range");
    }*/

    return wholeTile;
  }
};

using TileVariant =
    std::variant<LinearTile, MortonTile, MortonPow2Tile, NXTile>;

TileVariant TileVariantFromCtx(NewTexelContextCreate ctx) {
  uint32 width = ctx.width;
  uint32 height = ctx.height;

  switch (ctx.baseFormat.tile) {
  case TexelTile::Linear:
    return LinearTile{};

  case TexelTile::Morton:
    return MortonTile(width, height);

  case TexelTile::MortonForcePow2:
    return MortonPow2Tile(width, height);

  case TexelTile::NX:
    return NXTile(width, height, ctx.baseFormat.type);

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
    for (auto &t : outData) {
      Vector4A16 value;
      codec.GetValue(value, data + tiler->get(curTexel++) * 2);
      t = Vector(Vector4A16(value * 0xff)).Convert<uint8>();
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

    if (ctx.baseFormat.deriveZNormal) {
      ComputeBC5Blue(reinterpret_cast<char *>(outData.data()),
                     outData.size() * 3);
    }
    break;
  }

  case F::BC5:
    for (size_t p = 0; p < numBlocks; p++) {
      DecodeBC5Block(data + tiler->get(p) * 16,
                     reinterpret_cast<char *>(outData.data()), p % ctx.width,
                     p / ctx.width, ctx.width);
    }

    if (ctx.baseFormat.deriveZNormal) {
      ComputeBC5Blue(reinterpret_cast<char *>(outData.data()),
                     outData.size() * 3);
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
    for (auto &t : outData) {
      Vector4A16 value;
      codec.GetValue(value, data + tiler->get(curTexel++) * 4);
      t = (value * 0xff).Convert<uint8>();
    }
    break;
  }

  case F::RGBA4: {
    const uint16 *iData = reinterpret_cast<const uint16 *>(data);
    size_t curTexel = 0;
    for (auto &t : outData) {
      uint16 col = *(iData + tiler->get(curTexel++));
      t = UCVector4(col << 4, col & 0xf0, (col >> 4) & 0xf0, (col >> 8) & 0xf0);
    }

    break;
  }

  case F::RGB5A1: {
    const uint16 *iData = reinterpret_cast<const uint16 *>(data);
    size_t curTexel = 0;
    for (auto &t : outData) {
      uint16 col = *(iData + tiler->get(curTexel++));
      t = UCVector4(col << 3, (col >> 2) & 0xf8, (col >> 7) & 0xf8,
                    int16(col) >> 15);
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
    pvr::PVRTDecompressETC(data, ctx.width, ctx.height,
                           reinterpret_cast<uint8_t *>(outData.data()), 0);
    break;

  case F::BC7: {
    uint32 localBlock[16];
    for (size_t p = 0; p < numBlocks; p++) {
      if (!detexDecompressBlockBPTC(
              reinterpret_cast<const uint8_t *>(data) + tiler->get(p) * 16, -1,
              0, reinterpret_cast<uint8_t *>(localBlock))) [[unlikely]] {
        throw std::runtime_error("Failed to decompress BC7 block");
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
  for (size_t p = 0; p < numBlocks; p++) {
    memcpy(outData + p * BPT, data + tiler->get(p) * BPT, BPT);
  }
}

struct NewTexelContextQOI : NewTexelContextImpl {
  qoi_desc qoiDesc{};
  std::string yasBuffer;

  NewTexelContextQOI(NewTexelContextCreate ctx_) : NewTexelContextImpl(ctx_) {
    qoiDesc.width = ctx.width;
    qoiDesc.height = ctx.height * std::max(uint16(1), ctx.depth);
    qoiDesc.colorspace = !ctx.baseFormat.srgb;
    qoiDesc.channels = GetQOIChannels(ctx.baseFormat.type);
  }

  void InitBuffer() {
    uint32 widthPadding = qoiDesc.width % 4;
    widthPadding = widthPadding ? 4 - widthPadding : 0;
    uint32 heightPadding = qoiDesc.height % 4;
    heightPadding = heightPadding ? 4 - heightPadding : 0;

    const uint32 rasterDataSize = (qoiDesc.width + widthPadding) *
                                  (qoiDesc.height + heightPadding) *
                                  qoiDesc.channels;
    yasBuffer.resize(rasterDataSize);
  }

  void SendRasterData(const void *data, TexelInputLayout layout,
                      TexelInputFormat *) override {
    if (layout.mipMap > 0) {
      return;
    }

    auto mctx = ctx;
    mctx.height *= std::max(mctx.depth, uint16(1));

    auto DecodeStream = [&] {
      InitBuffer();

      if (qoiDesc.channels == 4) {
        UCVector4 *bgn = reinterpret_cast<UCVector4 *>(yasBuffer.data());
        UCVector4 *edn =
            reinterpret_cast<UCVector4 *>(yasBuffer.data() + yasBuffer.size());

        DecodeToRGBA(static_cast<const char *>(data), mctx, {bgn, edn});
      } else if (qoiDesc.channels == 3) {
        UCVector *bgn = reinterpret_cast<UCVector *>(yasBuffer.data());
        UCVector *edn =
            reinterpret_cast<UCVector *>(yasBuffer.data() + yasBuffer.size());

        DecodeToRGB(static_cast<const char *>(data), mctx, {bgn, edn});

      } else if (qoiDesc.channels == 1) {
        DecodeToGray(static_cast<const char *>(data), mctx,
                     {yasBuffer.data(), yasBuffer.size()});

      } else {
        throw std::logic_error("Implement channel");
      }
    };

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

      if (ectx) {
        if (pathOverride.GetFullPath().empty()) {
          throw std::logic_error("Expected path");
        }

        ectx->NewFile(std::string(pathOverride.ChangeExtension(suffix)));
        ectx->SendData({static_cast<char *>(buffa), size_t(encodedSize)});
      } else {
        AFileInfo &workingPath = pathOverride.GetFullPath().empty()
                                     ? actx->workingFile
                                     : pathOverride;

        BinWritterRef wr(
            actx->NewFile(workingPath.ChangeExtension(suffix)).str);
        wr.WriteBuffer(static_cast<char *>(buffa), encodedSize);
      }

      free(buffa);
    };

    if (MustDecode(ctx.baseFormat.type)) {
      DecodeStream();
      Write(yasBuffer.data());
    } else if (ctx.baseFormat.tile == TexelTile::Linear) {
      Write(data);
    } else {
      InitBuffer();
      RetileData(static_cast<const char *>(data), mctx, yasBuffer.data());
      Write(yasBuffer.data());
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

  NewTexelContextDDS(NewTexelContextCreate ctx_, bool isBase = false)
      : NewTexelContextImpl(ctx_), dds(MakeDDS(ctx)) {
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
      SetDDSFormat(dds, ctx.baseFormat);
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
      throw std::runtime_error("Writing image data beyond buffer's bounds");
    }

    auto mctx = ctx;

    for (uint32 m = 0; m < layout.mipMap; m++) {
      mctx.width = std::max(1, mctx.width / 2);
      mctx.depth = std::max(1, mctx.depth / 2);
      mctx.height = std::max(1, mctx.height / 2);
    }

    mctx.height *= mctx.depth;

    auto DecodeStream = [&] {
      uint8 numChannels = GetDDSChannels(ctx.baseFormat.type);
      if (numChannels == 4) {
        UCVector4 *bgn = reinterpret_cast<UCVector4 *>(yasBuffer.data());
        UCVector4 *edn =
            reinterpret_cast<UCVector4 *>(yasBuffer.data() + rDataSize);

        DecodeToRGBA(static_cast<const char *>(data), mctx, {bgn, edn});
      } else if (numChannels == 3) {
        UCVector *bgn = reinterpret_cast<UCVector *>(yasBuffer.data());
        UCVector *edn =
            reinterpret_cast<UCVector *>(yasBuffer.data() + rDataSize);

        DecodeToRGB(static_cast<const char *>(data), mctx, {bgn, edn});

      } else if (numChannels == 1) {
        DecodeToGray(static_cast<const char *>(data), mctx,
                     {yasBuffer.data(), rDataSize});
      } else {
        throw std::logic_error("Implement channel");
      }
    };

    const bool mustDecode = MustDecode(ctx.baseFormat.type);

    if (mustDecode) {
      DecodeStream();
    } else if (ctx.baseFormat.tile == TexelTile::Linear) {
      memcpy(rData, data, rDataSize);
    } else {
      RetileData(static_cast<const char *>(data), mctx, rData);
    }

    if (ShouldWrite()) {
      if (ectx) {
        if (pathOverride.GetFullPath().empty()) {
          throw std::logic_error("Expected path");
        }

        ectx->NewFile(std::string(pathOverride.GetFullPath()));
        ectx->SendData({reinterpret_cast<const char *>(&dds), size_t(dds.DDS_SIZE)});
        ectx->SendData(yasBuffer);
      } else {
        AFileInfo &workingPath = pathOverride.GetFullPath().empty()
                                     ? actx->workingFile
                                     : pathOverride;
        BinWritterRef wr(
            actx->NewFile(workingPath.ChangeExtension2("dds")).str);
        wr.WriteBuffer(reinterpret_cast<const char *>(&dds), dds.DDS_SIZE);
        wr.WriteContainer(yasBuffer);
      }
      es::Dispose(yasBuffer);
    }
  }

  bool ShouldDoMipmaps() override {
    return mainSettings.texelSettings.processMipMaps;
  }

  void Finish() override {
    if (!ShouldWrite()) {
      throw std::runtime_error("Incomplete dds file");
    }
  }
};

struct NewTexelContextDDSLegacy : NewTexelContextDDS {
  std::vector<std::string> arrayMipmapBuffers;

  NewTexelContextDDSLegacy(NewTexelContextCreate ctx_)
      : NewTexelContextDDS(ctx_, true) {
    arrayMipmapBuffers.resize(dds.arraySize);
    dds.arraySize = 1;

    SetDDSLegacyFormat(dds, ctx.baseFormat);
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
      throw std::runtime_error("Writing image data beyond buffer's bounds");
    }

    auto mctx = ctx;

    for (uint32 m = 0; m < layout.mipMap; m++) {
      mctx.width = std::max(1, mctx.width / 2);
      mctx.depth = std::max(1, mctx.depth / 2);
      mctx.height = std::max(1, mctx.height / 2);
    }

    mctx.height *= mctx.depth;

    auto DecodeStream = [&] {
      uint8 numChannels = GetDDSLegacyChannels(ctx.baseFormat.type);
      if (numChannels == 4) {
        UCVector4 *bgn = reinterpret_cast<UCVector4 *>(buffar.data());
        UCVector4 *edn =
            reinterpret_cast<UCVector4 *>(buffar.data() + rDataSize);

        DecodeToRGBA(static_cast<const char *>(data), mctx, {bgn, edn});
      } else if (numChannels == 3) {
        UCVector *bgn = reinterpret_cast<UCVector *>(buffar.data());
        UCVector *edn = reinterpret_cast<UCVector *>(buffar.data() + rDataSize);

        DecodeToRGB(static_cast<const char *>(data), mctx, {bgn, edn});

      } else if (numChannels == 1) {
        DecodeToGray(static_cast<const char *>(data), mctx,
                     {buffar.data(), rDataSize});
      } else {
        throw std::logic_error("Implement channel");
      }
    };

    const bool mustDecode = MustDecode(ctx.baseFormat.type);

    if (mustDecode) {
      DecodeStream();
    } else if (ctx.baseFormat.tile == TexelTile::Linear) {
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

      if (ectx) {
        if (pathOverride.GetFullPath().empty()) {
          throw std::logic_error("Expected path");
        }

        ectx->NewFile(std::string(pathOverride.GetFullPath()));
        ectx->SendData({reinterpret_cast<const char *>(&dds), size_t(dds.LEGACY_SIZE)});
        ectx->SendData(buffar);
      } else {
        AFileInfo &workingPath = pathOverride.GetFullPath().empty()
                                     ? actx->workingFile
                                     : pathOverride;
        BinWritterRef wr(
            actx->NewFile(workingPath.ChangeExtension(suffix)).str);
        wr.WriteBuffer(reinterpret_cast<const char *>(&dds), dds.LEGACY_SIZE);
        wr.WriteContainer(buffar);
      }
      es::Dispose(buffar);
    }
  }

  void Finish() override {
    if (!ShouldWrite(-1)) {
      throw std::runtime_error("Incomplete dds file");
    }
  }
};

std::unique_ptr<NewTexelContextImpl>
CreateTexelContext(NewTexelContextCreate ctx) {
  switch (OutputFormat()) {
  case TexelContextFormat::DDS:
    return std::make_unique<NewTexelContextDDS>(ctx);
  case TexelContextFormat::DDS_Legacy:
    return std::make_unique<NewTexelContextDDSLegacy>(ctx);
  case TexelContextFormat::QOI_BMP:
    return std::make_unique<NewTexelContextQOIBMP>(ctx);
  case TexelContextFormat::QOI:
    return std::make_unique<NewTexelContextQOI>(ctx);
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
  retVal->actx = actx;
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
  retVal->ectx = ectx;
  retVal->pathOverride.Load(path);
  if (ctx.data) {
    retVal->ProcessContextData();
    return {};
  }
  return retVal;
}
