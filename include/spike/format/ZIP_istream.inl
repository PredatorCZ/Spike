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
#include "spike/io/binreader_stream.hpp"

inline void ZIPLocalFile::Read(BinReaderRef rd) {
  rd.ReadBuffer(reinterpret_cast<char *>(this),
                getBlockSize(ZIPLocalFile, id, lastModFileDate));
  rd.ReadBuffer(reinterpret_cast<char *>(&crc),
                getBlockSize(ZIPLocalFile, crc, extraFieldSize));
}

inline void ZIPFile::Read(BinReaderRef rd) {
  rd.ReadBuffer(reinterpret_cast<char *>(this),
                getBlockSize(ZIPFile, id, internalFileAttrs));
  rd.ReadBuffer(reinterpret_cast<char *>(&externalFileAttrs), 8);
}

inline void ZIPSignature::Read(BinReaderRef rd) {
  rd.ReadBuffer(reinterpret_cast<char *>(this), 6);
}

inline void ZIPCentralDir::Read(BinReaderRef rd) {
  rd.ReadBuffer(reinterpret_cast<char *>(this), 22);
}

inline void ZIP64CentralDir::Read(BinReaderRef rd) {
  rd.Read(id);
  rd.ReadBuffer(reinterpret_cast<char *>(&dirRecord),
                getBlockSize(ZIP64CentralDir, dirRecord, diskNumber));
  rd.Read(startDiskNumber);
  rd.ReadBuffer(reinterpret_cast<char *>(&numDiskEntries),
                getBlockSize(ZIP64CentralDir, numDiskEntries, dirOffset));
}

inline void ZIP64CentralDirLocator::Read(BinReaderRef rd) {
  rd.ReadBuffer(reinterpret_cast<char *>(this), 20);
}
