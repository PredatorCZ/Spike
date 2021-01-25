/*  DDS format header

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
#include "datas/flags.hpp"

// Refers to:
// https://docs.microsoft.com/en-us/windows/desktop/api/dxgiformat/ne-dxgiformat-dxgi_format
enum DXGI_FORMAT : uint32 {
  DXGI_FORMAT_UNKNOWN,
  DXGI_FORMAT_R32G32B32A32_TYPELESS,
  DXGI_FORMAT_R32G32B32A32_FLOAT,
  DXGI_FORMAT_R32G32B32A32_UINT,
  DXGI_FORMAT_R32G32B32A32_SINT,
  DXGI_FORMAT_R32G32B32_TYPELESS,
  DXGI_FORMAT_R32G32B32_FLOAT,
  DXGI_FORMAT_R32G32B32_UINT,
  DXGI_FORMAT_R32G32B32_SINT,
  DXGI_FORMAT_R16G16B16A16_TYPELESS,
  DXGI_FORMAT_R16G16B16A16_FLOAT,
  DXGI_FORMAT_R16G16B16A16_UNORM,
  DXGI_FORMAT_R16G16B16A16_UINT,
  DXGI_FORMAT_R16G16B16A16_SNORM,
  DXGI_FORMAT_R16G16B16A16_SINT,
  DXGI_FORMAT_R32G32_TYPELESS,
  DXGI_FORMAT_R32G32_FLOAT,
  DXGI_FORMAT_R32G32_UINT,
  DXGI_FORMAT_R32G32_SINT,
  DXGI_FORMAT_R32G8X24_TYPELESS,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
  DXGI_FORMAT_R10G10B10A2_TYPELESS,
  DXGI_FORMAT_R10G10B10A2_UNORM,
  DXGI_FORMAT_R10G10B10A2_UINT,
  DXGI_FORMAT_R11G11B10_FLOAT,
  DXGI_FORMAT_R8G8B8A8_TYPELESS,
  DXGI_FORMAT_R8G8B8A8_UNORM,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
  DXGI_FORMAT_R8G8B8A8_UINT,
  DXGI_FORMAT_R8G8B8A8_SNORM,
  DXGI_FORMAT_R8G8B8A8_SINT,
  DXGI_FORMAT_R16G16_TYPELESS,
  DXGI_FORMAT_R16G16_FLOAT,
  DXGI_FORMAT_R16G16_UNORM,
  DXGI_FORMAT_R16G16_UINT,
  DXGI_FORMAT_R16G16_SNORM,
  DXGI_FORMAT_R16G16_SINT,
  DXGI_FORMAT_R32_TYPELESS,
  DXGI_FORMAT_D32_FLOAT,
  DXGI_FORMAT_R32_FLOAT,
  DXGI_FORMAT_R32_UINT,
  DXGI_FORMAT_R32_SINT,
  DXGI_FORMAT_R24G8_TYPELESS,
  DXGI_FORMAT_D24_UNORM_S8_UINT,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT,
  DXGI_FORMAT_R8G8_TYPELESS,
  DXGI_FORMAT_R8G8_UNORM,
  DXGI_FORMAT_R8G8_UINT,
  DXGI_FORMAT_R8G8_SNORM,
  DXGI_FORMAT_R8G8_SINT,
  DXGI_FORMAT_R16_TYPELESS,
  DXGI_FORMAT_R16_FLOAT,
  DXGI_FORMAT_D16_UNORM,
  DXGI_FORMAT_R16_UNORM,
  DXGI_FORMAT_R16_UINT,
  DXGI_FORMAT_R16_SNORM,
  DXGI_FORMAT_R16_SINT,
  DXGI_FORMAT_R8_TYPELESS,
  DXGI_FORMAT_R8_UNORM,
  DXGI_FORMAT_R8_UINT,
  DXGI_FORMAT_R8_SNORM,
  DXGI_FORMAT_R8_SINT,
  DXGI_FORMAT_A8_UNORM,
  DXGI_FORMAT_R1_UNORM,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
  DXGI_FORMAT_R8G8_B8G8_UNORM,
  DXGI_FORMAT_G8R8_G8B8_UNORM,
  DXGI_FORMAT_BC1_TYPELESS,
  DXGI_FORMAT_BC1_UNORM,
  DXGI_FORMAT_BC1_UNORM_SRGB,
  DXGI_FORMAT_BC2_TYPELESS,
  DXGI_FORMAT_BC2_UNORM,
  DXGI_FORMAT_BC2_UNORM_SRGB,
  DXGI_FORMAT_BC3_TYPELESS,
  DXGI_FORMAT_BC3_UNORM,
  DXGI_FORMAT_BC3_UNORM_SRGB,
  DXGI_FORMAT_BC4_TYPELESS,
  DXGI_FORMAT_BC4_UNORM,
  DXGI_FORMAT_BC4_SNORM,
  DXGI_FORMAT_BC5_TYPELESS,
  DXGI_FORMAT_BC5_UNORM,
  DXGI_FORMAT_BC5_SNORM,
  DXGI_FORMAT_B5G6R5_UNORM,
  DXGI_FORMAT_B5G5R5A1_UNORM,
  DXGI_FORMAT_B8G8R8A8_UNORM,
  DXGI_FORMAT_B8G8R8X8_UNORM,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
  DXGI_FORMAT_B8G8R8A8_TYPELESS,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
  DXGI_FORMAT_B8G8R8X8_TYPELESS,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
  DXGI_FORMAT_BC6H_TYPELESS,
  DXGI_FORMAT_BC6H_UF16,
  DXGI_FORMAT_BC6H_SF16,
  DXGI_FORMAT_BC7_TYPELESS,
  DXGI_FORMAT_BC7_UNORM,
  DXGI_FORMAT_BC7_UNORM_SRGB,
  DXGI_FORMAT_AYUV,
  DXGI_FORMAT_Y410,
  DXGI_FORMAT_Y416,
  DXGI_FORMAT_NV12,
  DXGI_FORMAT_P010,
  DXGI_FORMAT_P016,
  DXGI_FORMAT_420_OPAQUE,
  DXGI_FORMAT_YUY2,
  DXGI_FORMAT_Y210,
  DXGI_FORMAT_Y216,
  DXGI_FORMAT_NV11,
  DXGI_FORMAT_AI44,
  DXGI_FORMAT_IA44,
  DXGI_FORMAT_P8,
  DXGI_FORMAT_A8P8,
  DXGI_FORMAT_B4G4R4A4_UNORM,
  DXGI_FORMAT_P208,
  DXGI_FORMAT_V208,
  DXGI_FORMAT_V408,
  DXGI_FORMAT_FORCE_UINT
};

static constexpr bool IsBC(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_BC1_TYPELESS:
  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC1_UNORM_SRGB:
  case DXGI_FORMAT_BC2_TYPELESS:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC4_TYPELESS:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
  case DXGI_FORMAT_BC5_TYPELESS:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC6H_TYPELESS:
  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC7_TYPELESS:
  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
    return true;
  default:
    return false;
  }
}

static constexpr uint32 _bpps[] = {
    0,  128, 128, 128, 128, 96, 96, 96, 96, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64,  64,  64,  64,  32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32,  32,  32,  32,  32, 32, 32, 32, 32, 32, 32, 16, 16, 16, 16, 16, 16,
    16, 16,  16,  16,  16,  16, 8,  8,  8,  8,  8,  8,  1,  32, 32, 32, 4,  4,
    4,  8,   8,   8,   8,   8,  8,  4,  4,  4,  8,  8,  8,  16, 16, 32, 32, 32,
    32, 32,  32,  32,  8,   8,  8,  8,  8,  8,  32, 32, 64, 12, 32, 64, 32, 32,
    32, 64,  0,   0,   0,   0,  0,  16, 0,  0,  0,  0};

ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R32G32B32A32_TYPELESS] == 128);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R32G32B32_TYPELESS] == 96);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R16G16B16A16_TYPELESS] == 64);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT] == 64);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R10G10B10A2_TYPELESS] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_X24_TYPELESS_G8_UINT] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R8G8_TYPELESS] == 16);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R16_SINT] == 16);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R1_UNORM] == 1);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_R9G9B9E5_SHAREDEXP] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_G8R8_G8B8_UNORM] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC1_TYPELESS] == 4);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC1_UNORM_SRGB] == 4);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC2_TYPELESS] == 8);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC3_UNORM_SRGB] == 8);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC4_TYPELESS] == 4);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC5_SNORM] == 8);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_B5G6R5_UNORM] == 16);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_B8G8R8A8_UNORM] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC6H_TYPELESS] == 8);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_BC7_UNORM_SRGB] == 8);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_AYUV] == 32);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_P016] == 64);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_Y216] == 64);
ES_STATIC_ASSERT(_bpps[DXGI_FORMAT_B4G4R4A4_UNORM] == 16);

struct DDS_Header {
  static constexpr uint32 ID = CompileFourCC("DDS ");
  static constexpr uint32 HEADER_SIZE = 124;

  enum Flags : uint32 {
    Flags_Caps,
    Flags_Height,
    Flags_Width,
    Flags_Pitch,
    Flags_PixelFormat = 12,
    Flags_MipMaps = 17,
    Flags_LinearSize = 19,
    Flags_Depth = 23,
  };

  uint32 magic = ID;
  uint32 size = HEADER_SIZE;
  es::Flags<Flags> flags{Flags_Caps, Flags_Height, Flags_Width,
                         Flags_PixelFormat};
  uint32 height, width;
  uint32 pitchOrLinearSize = 0;
  uint32 depth = 1;
  uint32 mipMapCount = 0;
  uint32 reserved00[11]{};
};

struct DDS_HeaderEnd {
  enum Caps00Flags : uint32 {
    Caps00Flags_Complex = 3, // mipmaps, cubemaps
    Caps00Flags_MipMaps = 22,
    Caps00Flags_Texture = 12
  };

  enum Caps01Flags : uint32 {
    Caps01Flags_CubeMap = 9,
    Caps01Flags_CubeMap_PositiveX,
    Caps01Flags_CubeMap_NegativeX,
    Caps01Flags_CubeMap_PositiveY,
    Caps01Flags_CubeMap_NegativeY,
    Caps01Flags_CubeMap_PositiveZ,
    Caps01Flags_CubeMap_NegativeZ,
    Caps01Flags_Volume = 21,
  };

  es::Flags<Caps00Flags> caps00{Caps00Flags_Texture};
  es::Flags<Caps01Flags> caps01;
  uint32 caps02 = 0;
  uint32 caps03 = 0;
  uint32 reserved01 = 0;
};

struct DDS_PixelFormat {
  static constexpr uint32 PIXELFORMAT_SIZE = 32;

  enum PFFlags : uint32 {
    PFFlags_AlphaPixels,
    PFFlags_Alpha, // Alpha Channel Only
    PFFlags_FourCC,
    PFFlags_RGB =
        6, // Uncompressed RGB data, can be used with PFFlags_AlphaPixels
    PFFlags_YUV = 9,
    PFFlags_Luminance =
        17, // Luminance channel, can be used with PFFlags_AlphaPixels
  };

  typedef es::Flags<PFFlags> FlagsType;

  uint32 pfSize = PIXELFORMAT_SIZE;
  FlagsType pfFlags;
  uint32 fourCC = 0;
  uint32 bpp = 0;
  uint32 RBitMask = 0;
  uint32 GBitMask = 0;
  uint32 BBitMask = 0;
  uint32 ABitMask = 0;

  constexpr DDS_PixelFormat() = default;
  constexpr DDS_PixelFormat(const int _fourCC, uint32 _bpp)
      : pfFlags(PFFlags_FourCC), fourCC(_fourCC), bpp(_bpp) {}
  constexpr DDS_PixelFormat(FlagsType _flags, uint32 _bpp, uint32 rMask,
                            uint32 gMask, uint32 bMask, uint32 aMask)
      : pfFlags(_flags), bpp(_bpp), RBitMask(rMask), GBitMask(gMask),
        BBitMask(bMask), ABitMask(aMask) {}

  constexpr bool operator==(const DDS_PixelFormat &input) const {
    return pfFlags == input.pfFlags && fourCC == input.fourCC &&
           bpp == input.bpp && RBitMask == input.RBitMask &&
           GBitMask == input.GBitMask && BBitMask == input.BBitMask &&
           ABitMask == input.ABitMask;
  }
};

static constexpr DDS_PixelFormat DDSFormat_DXT1(CompileFourCC("DXT1"), 4);
static constexpr DDS_PixelFormat DDSFormat_DXT2(CompileFourCC("DXT2"), 8);
static constexpr DDS_PixelFormat DDSFormat_DXT3(CompileFourCC("DXT3"), 8);
static constexpr DDS_PixelFormat DDSFormat_DXT4(CompileFourCC("DXT4"), 8);
static constexpr DDS_PixelFormat DDSFormat_DXT5(CompileFourCC("DXT5"), 8);

static constexpr DDS_PixelFormat DDSFormat_BC4U(CompileFourCC("BC4U"), 4);
static constexpr DDS_PixelFormat DDSFormat_BC4S(CompileFourCC("BC4S"), 4);
static constexpr DDS_PixelFormat DDSFormat_BC5U(CompileFourCC("BC5U"), 8);
static constexpr DDS_PixelFormat DDSFormat_BC5S(CompileFourCC("BC5S"), 8);

static constexpr DDS_PixelFormat DDSFormat_R8G8_B8G8(CompileFourCC("RBGB"), 32);
static constexpr DDS_PixelFormat DDSFormat_G8R8_G8B8(CompileFourCC("GRGB"), 32);
static constexpr DDS_PixelFormat DDSFormat_UYVY(CompileFourCC("UYVY"), 32);
static constexpr DDS_PixelFormat DDSFormat_YUY2(CompileFourCC("YUY2"), 32);

static constexpr DDS_PixelFormat DDSFormat_ATI1(CompileFourCC("ATI1"), 4);
static constexpr DDS_PixelFormat DDSFormat_ATI2(CompileFourCC("ATI2"), 8);

static constexpr DDS_PixelFormat DDSFormat_DX10(CompileFourCC("DX10"), 0);

static constexpr DDS_PixelFormat DDSFormat_A2B10G10R10(
    {DDS_PixelFormat::PFFlags_RGB, DDS_PixelFormat::PFFlags_AlphaPixels}, 32,
    0x000003ff, 0x000ffc00, 0x3ff00000, 0x00000000);
static constexpr DDS_PixelFormat DDSFormat_A8R8G8B8(
    {DDS_PixelFormat::PFFlags_RGB, DDS_PixelFormat::PFFlags_AlphaPixels}, 32,
    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
static constexpr DDS_PixelFormat DDSFormat_A1R5G5B5(
    {DDS_PixelFormat::PFFlags_RGB, DDS_PixelFormat::PFFlags_AlphaPixels}, 16,
    0x00007c00, 0x000003e0, 0x0000001f, 0x00008000);
static constexpr DDS_PixelFormat DDSFormat_A4R4G4B4(
    {DDS_PixelFormat::PFFlags_RGB, DDS_PixelFormat::PFFlags_AlphaPixels}, 16,
    0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000);
static constexpr DDS_PixelFormat DDSFormat_A8L8(
    {DDS_PixelFormat::PFFlags_Luminance, DDS_PixelFormat::PFFlags_AlphaPixels},
    16, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00);

static constexpr DDS_PixelFormat DDSFormat_X8R8G8B8( //
    DDS_PixelFormat::PFFlags_RGB, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_G16R16( //
    DDS_PixelFormat::PFFlags_RGB, 32, 0x0000ffff, 0xffff0000, 0x00000000,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_R8G8B8( //
    DDS_PixelFormat::PFFlags_RGB, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_R5G6B5( //
    DDS_PixelFormat::PFFlags_RGB, 16, 0x0000f800, 0x000007e0, 0x0000001f,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_L16( //
    DDS_PixelFormat::PFFlags_Luminance, 16, 0x0000ffff, 0x00000000, 0x00000000,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_L8( //
    DDS_PixelFormat::PFFlags_Luminance, 8, 0x000000ff, 0x00000000, 0x00000000,
    0x00000000);
static constexpr DDS_PixelFormat DDSFormat_A8( //
    DDS_PixelFormat::PFFlags_Alpha, 8, 0x00000000, 0x00000000, 0x00000000,
    0x000000ff);

struct DDS_HeaderDX10 {
  static constexpr uint32 HEADER_SIZE_DX10 = 20;

  enum Dimension : uint32 {
    Dimension_1D = 2,
    Dimension_2D,
    Dimension_3D,
  };

  enum MiscFlag : uint32 { MiscFlag_CubeTexture = 2 };

  enum AlphaMode : uint32 {
    AlphaMode_Unknown,
    AlphaMode_Straight,
    AlphaMode_Premultiplied,
    AlphaMode_Opaque,
    AlphaMode_Custom
  };

  DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
  Dimension dimension = Dimension_2D;
  es::Flags<MiscFlag> miscFlag;
  uint32 arraySize = 1;
  AlphaMode alphaMode = AlphaMode_Unknown;

  DDS_HeaderDX10() = default;
  DDS_HeaderDX10(DXGI_FORMAT fmt) : dxgiFormat(fmt) {}
};

struct DDS : DDS_Header, DDS_PixelFormat, DDS_HeaderEnd, DDS_HeaderDX10 {
  static const int LEGACY_SIZE =
      sizeof(DDS_Header) + sizeof(DDS_PixelFormat) + sizeof(DDS_HeaderEnd);
  static const int DDS_SIZE = LEGACY_SIZE + sizeof(DDS_HeaderDX10);

  template <class _Base> void operator=(const _Base &input) {
    static_cast<_Base &>(*this) = input;
  }

  void NumMipmaps(int num) {
    mipMapCount = num;
    const bool usesMips = num > 1;

    caps00.Set(Caps00Flags_Complex, usesMips);
    caps00.Set(Caps00Flags_MipMaps, usesMips);
    flags.Set(Flags_MipMaps, usesMips);
  }

  void ComputeBPP() {
    if (bpp)
      return;

    DDS_PixelFormat &tformat = static_cast<DDS_PixelFormat &>(*this);

    if (tformat == DDSFormat_DX10) {
      bpp = _bpps[dxgiFormat];
    } else {
      switch (fourCC) {
      case 36:
      case 110:
      case 113:
      case 115:
        bpp = 64;
        break;
      case 111:
        bpp = 16;
        break;
      case 112:
      case 114:
      case CompileFourCC("RBGB"):
      case CompileFourCC("GRGB"):
      case CompileFourCC("YUY2"):
        bpp = 32;
        break;
      case 116:
        bpp = 128;
        break;
      case CompileFourCC("DXT1"):
      case CompileFourCC("BC4U"):
      case CompileFourCC("ATI1"):
      case CompileFourCC("BC4S"):
        bpp = 4;
        break;
      case CompileFourCC("DXT2"):
      case CompileFourCC("DXT3"):
      case CompileFourCC("DXT4"):
      case CompileFourCC("DXT5"):
      case CompileFourCC("BC5U"):
      case CompileFourCC("ATI2"):
      case CompileFourCC("BC5S"):
        bpp = 8;
        break;
      default:
        break;
      }
    }
  }

  struct Mips {
    static const uint32 maxMips = 15;
    uint32 sizes[maxMips];
    uint32 offsets[maxMips];
  };

  uint32 ComputeBufferSize(Mips &dOut) const {
    if (!bpp)
      return 0;

    uint32 _mipCount = mipMapCount ? mipMapCount : 1;
    uint32 _width = width;
    uint32 _height = height;
    uint32 fullBuffer = 0;
    bool useBlockCompression = false;

    if (dxgiFormat) {
      switch (dxgiFormat) {
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC2_UNORM:
      case DXGI_FORMAT_BC2_UNORM_SRGB:
      case DXGI_FORMAT_BC3_UNORM:
      case DXGI_FORMAT_BC3_UNORM_SRGB:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
      case DXGI_FORMAT_BC5_UNORM:
      case DXGI_FORMAT_BC5_SNORM:
      case DXGI_FORMAT_BC6H_UF16:
      case DXGI_FORMAT_BC6H_SF16:
      case DXGI_FORMAT_BC7_UNORM:
      case DXGI_FORMAT_BC7_UNORM_SRGB:
        useBlockCompression = true;
        break;
      default:
        break;
      }
    } else {
      switch (fourCC) {
      case CompileFourCC("DXT1"):
      case CompileFourCC("DXT2"):
      case CompileFourCC("DXT3"):
      case CompileFourCC("DXT4"):
      case CompileFourCC("DXT5"):
      case CompileFourCC("BC4U"):
      case CompileFourCC("ATI1"):
      case CompileFourCC("BC4S"):
      case CompileFourCC("BC5U"):
      case CompileFourCC("ATI2"):
      case CompileFourCC("BC5S"):
        useBlockCompression = true;
        break;
      default:
        break;
      }
    }

    for (uint32 m = 0; m < _mipCount; m++) {
      uint32 __width = _width, __height = _height;

      if (useBlockCompression && (__width * __height < 16)) {
        __width = 4;
        __height = 4;
      }

      dOut.sizes[m] = ((__width * __height * bpp) + 7) / 8;
      dOut.offsets[m] = fullBuffer;
      fullBuffer += dOut.sizes[m];
      _width /= 2;
      _height /= 2;
    }

    return fullBuffer;
  }

  int FromLegacy() {
    int result = 0;

    switch (fourCC) {
    case 36:
      dxgiFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
      break;
    case 110:
      dxgiFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
      break;
    case 111:
      dxgiFormat = DXGI_FORMAT_R16_FLOAT;
      break;
    case 112:
      dxgiFormat = DXGI_FORMAT_R16G16_FLOAT;
      break;
    case 113:
      dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
      break;
    case 114:
      dxgiFormat = DXGI_FORMAT_R32_FLOAT;
      break;
    case 115:
      dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
      break;
    case 116:
      dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
      break;
    case CompileFourCC("DXT1"):
      dxgiFormat = DXGI_FORMAT_BC1_UNORM;
      break;
    case CompileFourCC("DXT2"):
      dxgiFormat = DXGI_FORMAT_BC2_UNORM;
      alphaMode = AlphaMode_Premultiplied;
      break;
    case CompileFourCC("DXT3"):
      dxgiFormat = DXGI_FORMAT_BC2_UNORM;
      break;
    case CompileFourCC("DXT4"):
      dxgiFormat = DXGI_FORMAT_BC3_UNORM;
      alphaMode = AlphaMode_Premultiplied;
      break;
    case CompileFourCC("DXT5"):
      dxgiFormat = DXGI_FORMAT_BC3_UNORM;
      break;
    case CompileFourCC("BC4U"):
    case CompileFourCC("ATI1"):
      dxgiFormat = DXGI_FORMAT_BC4_UNORM;
      break;
    case CompileFourCC("BC4S"):
      dxgiFormat = DXGI_FORMAT_BC4_SNORM;
      break;
    case CompileFourCC("BC5U"):
    case CompileFourCC("ATI2"):
      dxgiFormat = DXGI_FORMAT_BC5_UNORM;
      break;
    case CompileFourCC("BC5S"):
      dxgiFormat = DXGI_FORMAT_BC5_SNORM;
      break;
    case CompileFourCC("RBGB"):
      dxgiFormat = DXGI_FORMAT_R8G8_B8G8_UNORM;
      break;
    case CompileFourCC("GRGB"):
      dxgiFormat = DXGI_FORMAT_G8R8_G8B8_UNORM;
      break;
    case CompileFourCC("YUY2"):
      dxgiFormat = DXGI_FORMAT_YUY2;
      break;
    default:
      result = 1;
      break;
    }

    if (result) {
      result = 0;
      DDS_PixelFormat &tformat = static_cast<DDS_PixelFormat &>(*this);

      if (tformat == DDSFormat_A2B10G10R10)
        dxgiFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
      else if (tformat == DDSFormat_A8R8G8B8)
        dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
      else if (tformat == DDSFormat_G16R16)
        dxgiFormat = DXGI_FORMAT_R16G16_UNORM;
      else if (tformat == DDSFormat_A8)
        dxgiFormat = DXGI_FORMAT_A8_UNORM;
      else if (tformat == DDSFormat_R5G6B5)
        dxgiFormat = DXGI_FORMAT_B5G6R5_UNORM;
      else if (tformat == DDSFormat_A1R5G5B5)
        dxgiFormat = DXGI_FORMAT_B5G5R5A1_UNORM;
      else if (tformat == DDSFormat_A8L8)
        dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
      else if (tformat == DDSFormat_L16)
        dxgiFormat = DXGI_FORMAT_R16_UNORM;
      else if (tformat == DDSFormat_L8)
        dxgiFormat = DXGI_FORMAT_R8_UNORM;
      else if (tformat ==
               DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                DDS_PixelFormat::PFFlags_AlphaPixels},
                               32, 0x000000ff, 0x0000ff00, 0x00ff0000,
                               0xff000000))
        dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
      else if (tformat ==
               DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                DDS_PixelFormat::PFFlags_AlphaPixels},
                               32, 0x000000ff, 0x0000ff00, 0x00ff0000,
                               0x00000000))
        dxgiFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
      else if (tformat ==
               DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                DDS_PixelFormat::PFFlags_AlphaPixels},
                               16, 0x0000000f, 0x000000f0, 0x00000f00,
                               0x0000f000))
        dxgiFormat = DXGI_FORMAT_B4G4R4A4_UNORM;
      else
        result = 1;
    }

    if (!result)
      operator=(DDSFormat_DX10);

    return result;
  }

  int ToLegacy(bool forceIncompatibilities = false) {
    int result = 0;

    switch (dxgiFormat) {
    case DXGI_FORMAT_R16G16B16A16_UNORM:
      operator=(DDS_PixelFormat(36, 64));
      break;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
      operator=(DDS_PixelFormat(110, 64));
      break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      operator=(DDS_PixelFormat(113, 64));
      break;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
      operator=(DDSFormat_A2B10G10R10);
      break;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
      operator=(DDSFormat_A8R8G8B8);
      break;
    case DXGI_FORMAT_R16G16_UNORM:
      operator=(DDSFormat_G16R16);
      break;
    case DXGI_FORMAT_R16_FLOAT:
      operator=(DDS_PixelFormat(111, 16));
      break;
    case DXGI_FORMAT_R16G16_FLOAT:
      operator=(DDS_PixelFormat(112, 32));
      break;
    case DXGI_FORMAT_R32_FLOAT:
      operator=(DDS_PixelFormat(114, 32));
      break;
    case DXGI_FORMAT_R32G32_FLOAT:
      operator=(DDS_PixelFormat(115, 64));
      break;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      operator=(DDS_PixelFormat(116, 128));
      break;
    case DXGI_FORMAT_A8_UNORM:
      operator=(DDSFormat_A8);
      break;
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
      operator=(DDSFormat_R8G8_B8G8);
      break;
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
      operator=(DDSFormat_G8R8_G8B8);
      break;
    case DXGI_FORMAT_BC1_UNORM:
      operator=(DDSFormat_DXT1);
      break;
    case DXGI_FORMAT_BC2_UNORM: {
      if (alphaMode == AlphaMode_Premultiplied)
        operator=(DDSFormat_DXT2);
      else
        operator=(DDSFormat_DXT3);
      break;
    }
    case DXGI_FORMAT_BC3_UNORM: {
      if (alphaMode == AlphaMode_Premultiplied)
        operator=(DDSFormat_DXT4);
      else
        operator=(DDSFormat_DXT5);
      break;
    }
    case DXGI_FORMAT_BC4_UNORM:
      operator=(DDSFormat_BC4U);
      break;
    case DXGI_FORMAT_BC4_SNORM:
      operator=(DDSFormat_BC4S);
      break;
    case DXGI_FORMAT_BC5_UNORM:
      operator=(DDSFormat_ATI2);
      break;
    case DXGI_FORMAT_BC5_SNORM:
      operator=(DDSFormat_BC5S);
      break;
    case DXGI_FORMAT_B5G6R5_UNORM:
      operator=(DDSFormat_R5G6B5);
      break;
    case DXGI_FORMAT_B5G5R5A1_UNORM:
      operator=(DDSFormat_A1R5G5B5);
      break;
    case DXGI_FORMAT_YUY2:
      operator=(DDSFormat_YUY2);
      break;
    default:
      result = 1;
      break;
    }

    if (forceIncompatibilities && result) {
      result = 0;

      switch (dxgiFormat) {
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        operator=(DDSFormat_A8R8G8B8);
        break;
      case DXGI_FORMAT_R8G8_UNORM:
        operator=(DDSFormat_A8L8);
        break;
      case DXGI_FORMAT_D16_UNORM:
      case DXGI_FORMAT_R16_UNORM:
        operator=(DDSFormat_L16);
        break;
      case DXGI_FORMAT_R8_UNORM:
        operator=(DDSFormat_L8);
        break;
      case DXGI_FORMAT_BC1_UNORM_SRGB:
        operator=(DDSFormat_DXT1);
        break;
      case DXGI_FORMAT_BC2_UNORM_SRGB:
        operator=(DDSFormat_DXT3);
        break;
      case DXGI_FORMAT_BC3_UNORM_SRGB:
        operator=(DDSFormat_DXT5);
        break;
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        operator=(DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                   DDS_PixelFormat::PFFlags_AlphaPixels},
                                  32, 0x000000ff, 0x0000ff00, 0x00ff0000,
                                  0xff000000));
        break;
      case DXGI_FORMAT_B8G8R8X8_UNORM:
      case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        operator=(DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                   DDS_PixelFormat::PFFlags_AlphaPixels},
                                  32, 0x000000ff, 0x0000ff00, 0x00ff0000,
                                  0x00000000));
        break;
      case DXGI_FORMAT_B4G4R4A4_UNORM:
        operator=(DDS_PixelFormat({DDS_PixelFormat::PFFlags_RGB,
                                   DDS_PixelFormat::PFFlags_AlphaPixels},
                                  16, 0x0000000f, 0x000000f0, 0x00000f00,
                                  0x0000f000));
        break;
      default:
        result = 1;
        break;
      }
    }

    return result;
  }
};

ES_STATIC_ASSERT(sizeof(DDS_HeaderDX10) == 20);
ES_STATIC_ASSERT(sizeof(DDS_PixelFormat) == 32);
ES_STATIC_ASSERT(DDS::LEGACY_SIZE == 128);
ES_STATIC_ASSERT(DDS::DDS_SIZE == 148);
