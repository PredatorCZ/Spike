/*  ZIP format stream handlers

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

#include "ZIP.hpp"
#include "spike/io/binwritter_stream.hpp"

inline void ZIPLocalFile::Write(BinWritterRef wr) const {
  wr.WriteBuffer(reinterpret_cast<const char *>(this),
                 getBlockSize(ZIPLocalFile, id, lastModFileDate));
  wr.WriteBuffer(reinterpret_cast<const char *>(&crc),
                 getBlockSize(ZIPLocalFile, crc, extraFieldSize));
}

inline void ZIPFile::Write(BinWritterRef wr) const {
  wr.WriteBuffer(reinterpret_cast<const char *>(this),
                 getBlockSize(ZIPFile, id, internalFileAttrs));
  wr.WriteBuffer(reinterpret_cast<const char *>(&externalFileAttrs), 8);
}

inline void ZIPSignature::Write(BinWritterRef wr) const {
  wr.WriteBuffer(reinterpret_cast<const char *>(this), 6);
}

inline void ZIPCentralDir::Write(BinWritterRef wr) const {
  wr.WriteBuffer(reinterpret_cast<const char *>(this), 22);
}

inline void ZIP64CentralDir::Write(BinWritterRef wr) const {
  wr.Write(id);
  wr.WriteBuffer(reinterpret_cast<const char *>(&dirRecord),
                 getBlockSize(ZIP64CentralDir, dirRecord, diskNumber));
  wr.Write(startDiskNumber);
  wr.WriteBuffer(reinterpret_cast<const char *>(&numDiskEntries),
                 getBlockSize(ZIP64CentralDir, numDiskEntries, dirOffset));
}

inline void ZIP64CentralDirLocator::Write(BinWritterRef wr) const {
  wr.WriteBuffer(reinterpret_cast<const char *>(this), 20);
}

inline void ZIP64Extra::Write(BinWritterRef wr) const {
  wr.Write(id);
  wr.Push();
  uint16 size = 0;
  wr.Write(size);

  if (uncompressedSize) {
    size += sizeof(uncompressedSize);
    wr.Write(uncompressedSize);
  }

  if (compressedSize) {
    size += sizeof(compressedSize);
    wr.Write(compressedSize);
  }

  if (localHeaderOffset) {
    size += sizeof(localHeaderOffset);
    wr.Write(localHeaderOffset);
  }

  if (startDiskNumber) {
    size += sizeof(startDiskNumber);
    wr.Write(startDiskNumber);
  }

  wr.Push(wr.StackIndex1);
  wr.Pop();
  wr.Write(size);
  wr.Pop(wr.StackIndex1);
}
