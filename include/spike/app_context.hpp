/*  App context classes for spike modules

    Copyright 2021-2023 Lukas Cone

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
#include "io/fileinfo.hpp"
#include <cstring>
#include <functional>
#include <iosfwd>
#include <span>
#include <string>

#ifdef MAIN_APP
#define AC_EXTERN
#else
#define AC_EXTERN ES_EXPORT
#endif

class ReflectorFriend;
struct AppContextStream;
struct AppContextFoundStream;
struct NewTexelContext;
struct NewTexelContextCreate;

struct ExtractConf {
  bool makeZIP = true;
  bool folderPerArc = true;
  void ReflectorTag();
};

struct CompressConf {
  uint32 ratioThreshold = 90;
  uint32 minFileSize = 0x80;
  void ReflectorTag();
};

struct MainAppConf {
  CompressConf compressSettings;
  uint8 verbosity = 0;

protected:
  bool generateLog = false;
  ExtractConf extractSettings;
};

struct AppInfo_s {
  static constexpr uint32 CONTEXT_VERSION = 10;
  uint32 contextVersion = CONTEXT_VERSION;
  // No RequestFile or FindFile is being called
  bool filteredLoad = false;
  // AppProcessFile is called by thread
  bool multithreaded = true;
  std::string_view header;
  ReflectorFriend *settings = nullptr;
  std::span<std::string_view> filters{};
  const MainAppConf *internalSettings = nullptr;
  std::span<std::string_view> batchControlFilters{};
};

struct AppExtractContext {
  virtual ~AppExtractContext() = default;
  virtual void NewFile(const std::string &path) = 0;

  // Calling this after NewImage or SendRasterData without calling NewFile
  // before, will write data to image instead, thus it's undefined behavior
  virtual void SendData(std::string_view data) = 0;

  virtual bool RequiresFolders() const = 0;
  virtual void AddFolderPath(const std::string &path) = 0;
  virtual void GenerateFolders() = 0;

  // Will invalidate previous NewImage or NewFile calls
  virtual NewTexelContext *NewImage(const std::string &path,
                                    NewTexelContextCreate ctx) = 0;
};

// Every call is multi-threaded
struct AppPackContext {
  virtual ~AppPackContext() = default;
  virtual void SendFile(std::string_view path, std::istream &stream) = 0;
  virtual void Finish() = 0;
};

struct AppHelpContext {
  virtual ~AppHelpContext() = default;
  virtual std::ostream &GetStream(const std::string &tag) = 0;
};

using request_chunk = std::function<std::string(size_t offset, size_t size)>;

struct AppContextLocator {
  virtual ~AppContextLocator() = default;
  virtual AppContextStream RequestFile(const std::string &path) = 0;
  virtual void DisposeFile(std::istream *file) = 0;
  virtual AppContextFoundStream FindFile(const std::string &rootFolder,
                                         const std::string &pattern) = 0;
  virtual const std::vector<std::string> &SupplementalFiles() = 0;
};

struct NewFileContext {
  std::ostream &str;
  std::string fullPath;
  size_t delimiter = 0;
};

enum class TexelInputFormatType : uint8 {
  INVALID = 0,
  RGBA16 = 10,
  RGB10A2 = 24,
  RGBA8 = 28,
  RGB8 = 88,
  RG8 = 49,
  R8 = 61,
  RGB9E5 = 67,
  BC1 = 71,
  BC2 = 74,
  BC3 = 77,
  BC4 = 80,
  BC5 = 83,
  BC6 = 95,
  BC7 = 98,
  RGBA4 = 115,
  R5G6B5 = 85,
  RGB5A1 = 86,
  P8 = 113,
  P4 = 214,
  PVRTC4 = 200,
  PVRTC2 = 201,
  ETC1 = 202,
};

enum class TexelSwizzleType : uint8 {
  Blue,
  Green,
  Red,
  Alpha,
  BlueInverted,
  GreenInverted,
  RedInverted,
  AlphaInverted,
  Black,
  White,
  // Takes R and G channels and derives Z normal
  // Fallback to Blue if blue chanel is used
  // Only applies to blue swizzle component
  DeriveZOrBlue,
  // Fallback to BlueInverted if blue chanel is used
  DeriveZOrBlueInverted,
  // Force derive Z normal, even if blue channel is used
  DeriveZ,
};

union TexelSwizzle {
  TexelSwizzleType types[4]{TexelSwizzleType::DeriveZOrBlue, TexelSwizzleType::Green,
                            TexelSwizzleType::Red, TexelSwizzleType::Alpha};
  struct {
    TexelSwizzleType b;
    TexelSwizzleType g;
    TexelSwizzleType r;
    TexelSwizzleType a;
  };
};

enum class TexelTile : uint8 {
  Linear,
  Morton,
  MortonForcePow2,
  NX,
  PS2,
  Custom,
};

struct TileBase {
  virtual uint32 get(uint32) const = 0;
  virtual void reset(uint32, uint32, uint32) = 0;
};

struct TexelInputFormat {
  TexelInputFormatType type;
  TexelSwizzle swizzle;
  TexelTile tile = TexelTile::Linear;
  // Texel values are in SNORM [-1, 1]
  bool snorm = false;
  bool srgb = false;
  bool premultAlpha = false;
  bool swapPacked = false;
};

enum class CubemapFace : uint8 {
  NONE,
  Right,
  Left,
  Up,
  Down,
  Front,
  Back,
};

// Computing data layout:
//  for each a in array:
//    group_addr = groupSize * a
//
//    if it's a cubemap:
//      for each f in faces:
//        cube_addr = group_addr + mipGroupSize * f
//        for each m in mipmaps:
//          data_addr = cube_addr + mipOffsets[m]
//          data_size = mipSizes[m]
//
//    else:
//      for each m in mipmaps:
//          data_addr = group_addr + mipOffsets[m]
//          data_size = mipSizes[m]
struct TexelDataLayout {
  // SIze of the entire group,
  // this can be cubemap faces and their mipmaps,
  // 2d and volumetric mipmap chain
  // Useful as data stride for texture arrays
  uint32 groupSize;
  // Size of the entire mipmap group
  // Mostly same as groupSize except for cubemaps
  uint32 mipGroupSize;
  static const size_t MAX_MIPS = 15;
  // Offset for each mipmap relative to offset of the first mipmap
  // For volumetrics it also includes depth data
  uint32 mipOffsets[MAX_MIPS];
  // Size of each 2d/3d mipmap
  // For volumetrics data may be slices of 2d planes or not,
  // that's why it includes data size with depth in mind
  uint32 mipSizes[MAX_MIPS];
};

struct TexelInputLayout {
  uint8 mipMap = 0;
  // For cubemaps
  CubemapFace face = CubemapFace::NONE;
  // For texture arrays or volumetrics
  uint16 layer = 0;
};

enum class TexelContextFormat {
  Config,
  DDS_Legacy,
  DDS,
  QOI_BMP,
  QOI,
  UPNG,
};

struct TexelOutput {
  virtual void SendData(std::string_view) = 0;
  virtual void NewFile(std::string filePath) = 0;
};

struct NewTexelContextCreate {
  uint16 width;
  uint16 height;
  TexelInputFormat baseFormat;
  // Depth for volumetrics or num items for arrays
  uint16 depth = 1;
  uint8 numMipmaps = 1;
  // For cubemap, should be 6, -1 is for equirectangular map
  int8 numFaces = 0;

  uint32 arraySize = 1;

  // Custom address calculator
  TileBase *customTile = nullptr;

  // When set, process input data immediately
  // Data are treated as traditional layout described above TexelDataLayout
  // class, otherwise SendRasterData must be called for each face, mipmap and
  // layer
  const void *data = nullptr;

  // Override output data endpoint
  TexelOutput *texelOutput = nullptr;
  TexelContextFormat formatOverride = TexelContextFormat::Config;
  std::function<void(char *data, uint32 stride, uint32 numTexels)> postProcess =
      nullptr;
};

struct NewTexelContext {
  virtual ~NewTexelContext() = default;
  virtual void SendRasterData(const void *data, TexelInputLayout layout = {},
                              TexelInputFormat *formatOverrides = nullptr) = 0;
  virtual bool ShouldDoMipmaps() = 0;

  // Some better APIs support different format per mipmap
  // so typeOverrides is here to rectify correct layout data
  virtual TexelDataLayout ComputeTraditionalDataLayout(
      TexelInputFormatType typeOverrides[TexelDataLayout::MAX_MIPS] =
          nullptr) = 0;
};

struct AppContext : AppContextLocator {
  // Path to currently processed file within current filesystem
  AFileInfo workingFile;
  virtual std::istream &GetStream() = 0;
  virtual std::string GetBuffer(size_t size = -1, size_t begin = 0) = 0;
  // Creates context for extraction, can be created only once per context
  // Subsequent calls will return already created context
  virtual AppExtractContext *ExtractContext() = 0;
  // Create new file in system's filesystem
  // Provides single interfce, calling it mutiple times within the same context
  // will cause to close previous stream
  // To make mutiple files in single context, use ExtractContext() instead.
  // path can be relative, use with workingFile
  virtual NewFileContext NewFile(const std::string &path) = 0;

  // Create new image in system's filesystem
  // Provides single interfce, calling it mutiple times within the same context
  // will cause to close previous context
  // To make mutiple images in single context, use ExtractContext() instead.
  // path can be relative, use with workingFile, or null will use workingFile
  // Return value is nullptr if data are provided within NewTexelContextCreate
  virtual NewTexelContext *NewImage(NewTexelContextCreate ctx,
                                    const std::string *path = nullptr) = 0;

  virtual AppExtractContext *ExtractContext(std::string_view name) = 0;

  template <class C> void GetType(C &out, size_t offset = 0) {
    auto buffer = GetBuffer(sizeof(C), offset);
    memcpy(static_cast<void *>(&out), buffer.data(), buffer.size());
  }
};

struct AppContextStream {
  AppContextStream() = default;
  AppContextStream(const AppContextStream &) = delete;
  AppContextStream(AppContextStream &&other)
      : stream(other.stream), ctx(other.ctx) {
    other.ctx = nullptr;
  }
  AppContextStream(std::istream *str, AppContextLocator *ctx_)
      : stream(str), ctx(ctx_) {}
  ~AppContextStream() {
    if (*this) {
      ctx->DisposeFile(stream);
    }
  }

  auto Get() { return stream; }
  // Owner is responsible for disposing by calling AppContext::DisposeFile
  auto Release() {
    ctx = nullptr;
    return stream;
  }

  auto operator->() { return stream; }

  operator bool() const { return stream && ctx; }

  auto &operator=(AppContextStream &&other) {
    stream = other.stream;
    ctx = other.ctx;
    other.ctx = nullptr;
    return *this;
  }

private:
  std::istream *stream = nullptr;
  AppContextLocator *ctx = nullptr;
};

struct AppContextFoundStream : AppContextStream {
  AFileInfo path;
  using AppContextStream::AppContextStream;
  AppContextFoundStream(std::istream *str, AppContextLocator *ctx_,
                        const AFileInfo &workFile)
      : AppContextStream(str, ctx_), path(workFile) {}
};

extern "C" {
AppInfo_s AC_EXTERN *AppInitModule();
void AC_EXTERN AppAdditionalHelp(AppHelpContext *ctx, size_t indent);
bool AC_EXTERN AppInitContext(const std::string &dataFolder);
void AC_EXTERN AppProcessFile(AppContext *ctx);
// Returns total number of files within archive
size_t AC_EXTERN AppExtractStat(request_chunk requester);
AppPackContext AC_EXTERN *AppNewArchive(const std::string &folder);
void AC_EXTERN AppFinishContext();
};
