/*  multi threaded ZIP virtual file system

    Copyright 2021 Lukas Cone

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
#include "formats/ZIP_istream.inl"
#include <map>
#include <mutex>
#include <sstream>

struct ZIPVFSFile {
  std::istringstream buffer;
  BinReaderRef reader;

  operator BinReaderRef() const { return reader; }

  ZIPVFSFile(std::string &&str) : buffer(str), reader(buffer){};
};

class ZIPVFS : std::map<std::string, std::pair<size_t, size_t>> {
  using parent = std::map<std::string, std::pair<size_t, size_t>>;

public:
  ZIPVFS(BinReaderRef rd_) : rd(rd_) { Read(); }
  using parent::at;
  using parent::begin;
  using parent::empty;
  using parent::end;
  using parent::find;
  using parent::size;

  ZIPVFSFile Get(mapped_type value) const {
    std::string holder;
    std::lock_guard<std::mutex> guard(rdMutex);
    rd.Seek(value.first);
    rd.ReadContainer(holder, value.second);
    ZIPVFSFile retVal(std::move(holder));
    return retVal;
  }

  ZIPVFSFile Get(const_iterator iter) const { return Get(iter->second); }

  ZIPVFSFile Get(const std::string &fileName) const {
    return Get(at(fileName));
  }

private:
  BinReaderRef rd;
  mutable std::mutex rdMutex;

  void ReadEntry() {
    uint32 id;
    rd.Push();
    rd.Read(id);
    rd.Pop();

    switch (id) {
    case ZIPLocalFile::ID: {
      ZIPLocalFile hdr;
      rd.Read(hdr);

      [&] {
        if (!hdr.compressedSize) {
          rd.Skip(hdr.fileNameSize + hdr.extraFieldSize + hdr.uncompressedSize);
          return;
        }

        if (hdr.flags[ZIPLocalFlag::Encrypted]) {
            throw std::runtime_error("ZIP cannot have encrypted files!");
        }

        if (hdr.compression != ZIPCompressionMethod::Store) {
          throw std::runtime_error("ZIP cannot have compressed files!");
        }

        if (!hdr.fileNameSize) {
          throw std::runtime_error("ZIP local file's path must be specified!");
        }

        std::string path;
        rd.ReadContainer(path, hdr.fileNameSize);
        rd.Skip(hdr.extraFieldSize);
        emplace(std::move(path), std::make_pair(rd.Tell(), hdr.compressedSize));
        rd.Skip(hdr.compressedSize);
      }();
      break;
    }

    case ZIPFile::ID: {
      ZIPFile hdr;
      rd.Read(hdr);
      rd.Skip(hdr.fileNameSize + hdr.extraFieldSize + hdr.fileCommentSize);
      break;
    };

    case ZIPCentralDir::ID: {
      ZIPCentralDir hdr;
      rd.Read(hdr);
      rd.Skip(hdr.commentSize);
      break;
    }

    case ZIP64CentralDir::ID: {
      ZIP64CentralDir hdr;
      rd.Read(hdr);
      break;
    }

    case ZIP64CentralDirLocator::ID: {
      ZIP64CentralDirLocator hdr;
      rd.Read(hdr);
      break;
    }

    case ZIPSignature::ID: {
      ZIPSignature hdr;
      rd.Read(hdr);
      rd.Skip(hdr.dataSize);
      break;
    }

    case ZIPExtraData::ID: {
      ZIPExtraData hdr;
      rd.Read(hdr);
      rd.Skip(hdr.dataSize);
      break;
    }

    default:
      using std::to_string;
      throw std::runtime_error("Invalid block " + to_string(id) + " at " +
                               to_string(rd.Tell()));
    }
  }

  void Read() {
    const size_t fileSize = rd.GetSize();
    while (rd.Tell() < fileSize) {
      ReadEntry();
    }
  }
};
