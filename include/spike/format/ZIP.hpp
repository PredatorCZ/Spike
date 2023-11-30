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
    muint32 id;
    muint32 crc;
    muint32 compressedSize;
    muint32 uncompressedSize;
  } sig;
  struct {
    muint32 crc;
    muint32 compressedSize;
    muint32 uncompressedSize;
  } noSig;
};

struct ZIPExtraData {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x08");
  muint32 id;
  muint32 dataSize;
};

struct ZIPLocalFile {
  static constexpr uint32 ID = CompileFourCC("PK\x03\x04");
  muint32 id;
  muint16 extractVersion;
  ZIPLocalFlags flags;
  ZIPCompressionMethod compression;
  muint16 lastModFileTime;
  muint16 lastModFileDate;
  muint16 _pad;
  muint32 crc;
  muint32 compressedSize;
  muint32 uncompressedSize;
  muint16 fileNameSize;
  muint16 extraFieldSize;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPFile {
  static constexpr uint32 ID = CompileFourCC("PK\x01\x02");
  muint32 id;
  muint16 madeBy;
  muint16 extractVersion;
  ZIPLocalFlags flags;
  ZIPCompressionMethod compression;
  muint16 lastModFileTime;
  muint16 lastModFileDate;
  muint32 crc;
  muint32 compressedSize;
  muint32 uncompressedSize;
  muint16 fileNameSize;
  muint16 extraFieldSize;
  muint16 fileCommentSize;
  muint16 diskNumberStart;
  muint16 internalFileAttrs;
  muint32 externalFileAttrs;
  muint32 localHeaderOffset;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPSignature {
  static constexpr uint32 ID = CompileFourCC("PK\x05\x05");
  muint32 id;
  muint16 dataSize;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIPCentralDir {
  static constexpr uint32 ID = CompileFourCC("PK\x05\x06");
  muint32 id;
  muint16 diskNumber;
  muint16 startDiskNumber;
  muint16 numDiskEntries;
  muint16 numDirEntries;
  muint32 dirSize;
  muint32 dirOffset;
  muint16 commentSize;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64CentralDir {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x06");
  muint32 id;
  muint64 dirRecord;
  muint16 madeBy;
  muint16 extractVersion;
  muint32 diskNumber;
  muint32 startDiskNumber;
  muint64 numDiskEntries;
  muint64 numDirEntries;
  muint64 dirSize;
  muint64 dirOffset;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64CentralDirLocator {
  static constexpr uint32 ID = CompileFourCC("PK\x06\x07");
  muint32 id;
  muint32 diskNumber;
  muint64 centralDirOffset;
  muint32 numDisks;

  void Read(BinReaderRef rd);
  void Write(BinWritterRef wr) const;
};

struct ZIP64Extra {
  muint16 id = 1;
  muint16 size;
  muint64 uncompressedSize = 0;
  muint64 compressedSize = 0;
  muint64 localHeaderOffset = 0;
  muint32 startDiskNumber = 0;

  void Write(BinWritterRef wr) const;
};
