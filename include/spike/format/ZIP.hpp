/*  ZIP format structures

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
#include "spike/io/bincore_fwd.hpp"
#include "spike/type/flags.hpp"

enum class ZIPCompressionMethod : uint16 {
  Store,
  Shrunk,
  Reduce1,
  Reduce2,
  Reduce3,
  Reduce4,
  Implode,
  Tokenize,
  Deflate,
  Deflate64,
  BZIP2 = 12,
  LZMA = 14,
  CMPSC = 16,
  TERSE = 18,
  IBM_LZ77,
  JPEG = 96,
  WavPack,
  PPMd,
  AEx
};

enum class ZIPLocalFlag : uint16 {
  Encrypted,
  CFlag0,
  CFlag1,
  DataDescriptor,
  EnhancedDeflate,
  PatchedData,
  StronkEncryption,
  StringsInUTF8 = 11,
  StronkEncryptSpecific = 13,
};

enum class ImplodeFlag {
  Dict8k, // 8k sliding window used instead of 4k
  SF3,    // Use 3 Shannon-Fano trees instead of 2
};

enum class DeflateMethod {
  Normal,    //-en
  Maximum,   //-exx/-ex
  Fast,      //-ef
  SuperFast, //-es
};

enum class LZMAFlag {
  EOS, // Use end of stream marker
};

using ZIPLocalFlags = es::Flags<ZIPLocalFlag>;

union ZIPDataDescriptor {
  struct {
    uint32 id;
    uint32 crc;
    uint32 compressedSize;
    uint32 uncompressedSize;
  } sig;
  struct {
    uint32 crc;
    uint32 compressedSize;
    uint32 uncompressedSize;
  } noSig;
};

struct ZIPExtraData {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x08");
  uint32 id;
  uint32 dataSize;
};

struct ZIPLocalFile {
  static constexpr uint32 ID = CompileFourCC("PK\x03\x04");
  uint32 id;
  uint16 extractVersion;
  ZIPLocalFlags flags{};
  ZIPCompressionMethod compression{};
  uint16 lastModFileTime{};
  uint16 lastModFileDate{};
  uint16 _pad{};
  uint32 crc{};
  uint32 compressedSize{};
  uint32 uncompressedSize{};
  uint16 fileNameSize{};
  uint16 extraFieldSize{};

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPFile {
  static constexpr uint32 ID = CompileFourCC("PK\x01\x02");
  uint32 id;
  uint16 madeBy;
  uint16 extractVersion;
  ZIPLocalFlags flags;
  ZIPCompressionMethod compression;
  uint16 lastModFileTime;
  uint16 lastModFileDate;
  uint32 crc;
  uint32 compressedSize;
  uint32 uncompressedSize;
  uint16 fileNameSize;
  uint16 extraFieldSize;
  uint16 fileCommentSize;
  uint16 diskNumberStart;
  uint16 internalFileAttrs;
  uint32 externalFileAttrs;
  uint32 localHeaderOffset;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPSignature {
  static constexpr uint32 ID = CompileFourCC("PK\x05\x05");
  uint32 id;
  uint16 dataSize;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPCentralDir {
  static constexpr uint32 ID = CompileFourCC("PK\x05\x06");
  uint32 id;
  uint16 diskNumber;
  uint16 startDiskNumber;
  uint16 numDiskEntries;
  uint16 numDirEntries;
  uint32 dirSize;
  uint32 dirOffset;
  uint16 commentSize;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64CentralDir {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x06");
  uint32 id;
  uint64 dirRecord;
  uint16 madeBy;
  uint16 extractVersion;
  uint32 diskNumber;
  uint32 startDiskNumber;
  uint64 numDiskEntries;
  uint64 numDirEntries;
  uint64 dirSize;
  uint64 dirOffset;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64CentralDirLocator {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x07");
  uint32 id;
  uint32 diskNumber;
  uint64 centralDirOffset;
  uint32 numDisks;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64Extra {
  uint16 id = 1;
  uint16 size;
  uint64 uncompressedSize = 0;
  uint64 compressedSize = 0;
  uint64 localHeaderOffset = 0;
  uint32 startDiskNumber = 0;

  void Write(BinWritterRef wr) const;
};
