/*  Spike is universal dedicated module handler
    This source contains context for input data
    Part of PreCore project

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

#include "context.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/stat.hpp"
#include "formats/ZIP_istream.inl"
#include <mutex>
#include <sstream>

static std::mutex simpleIOLock;

struct BinReaderEx : BinReader<> {
  using BinReader::baseStream;
  using BinReader::BinReader;
  using BinReader::Close_;
};

struct SimpleIOContext : AppContext {
  std::istream *OpenFile(const std::string &path);

  AppContextStream RequestFile(const std::string &path) override;

  AppContextStream FindFile(const std::string &rootFolder,
                            const std::string &pattern) override;

  void DisposeFile(std::istream *str) override;

private:
  BinReaderEx streamedFiles[32];
  uint32 usedFiles = 0;
};

struct ZIPContext : AppContext {
  AppContextStream RequestFile(const std::string &path) override;

  AppContextStream FindFile(const std::string &rootFolder,
                            const std::string &pattern) override;

  void DisposeFile(std::istream *str) override;
};

std::istream *SimpleIOContext::OpenFile(const std::string &path) {
  std::lock_guard<std::mutex> guard(simpleIOLock);
  for (size_t b = 0; b < 32; b++) {
    uint32 bit = 1 << b;
    if (!(usedFiles & bit)) {
      streamedFiles[b].Open(path);
      usedFiles ^= bit;
      return streamedFiles[b].baseStream;
    }
  }

  throw std::out_of_range("Maximum opened files reached!");
}

AppContextStream SimpleIOContext::RequestFile(const std::string &path) {
  AFileInfo wFile(workingFile);
  AFileInfo pFile(path);
  auto catchedFile = pFile.CatchBranch(wFile.GetFolder());
  return {OpenFile(catchedFile), this};
}

AppContextStream SimpleIOContext::FindFile(const std::string &rootFolder,
                                           const std::string &pattern) {
  DirectoryScanner sc;
  sc.AddFilter(pattern);
  sc.Scan(rootFolder);

  if (sc.Files().empty()) {
    throw es::FileNotFoundError(pattern);
  } else if (sc.Files().size() > 1) {
    throw std::runtime_error("Too many files found.");
  }

  return {OpenFile(sc.Files().front()), this};
}

void SimpleIOContext::DisposeFile(std::istream *str) {
  size_t index = 0;

  for (auto &f : streamedFiles) {
    if (f.baseStream == str) {
      uint32 bit = 1 << index;

      if (!(usedFiles & bit)) {
        throw std::runtime_error("Stream already freed.");
      }

      f.Close_();
      usedFiles ^= bit;
      return;
    }

    index++;
  }

  throw std::runtime_error("Requested stream not found!");
}

std::unique_ptr<AppContext> MakeIOContext() {
  return std::make_unique<SimpleIOContext>();
}

static std::mutex ZIPLock;

struct ZIPDataHolder {
  virtual ~ZIPDataHolder() = default;
};

struct ZIPIOContext_impl : ZIPIOContext {
  std::istream *OpenFile(const zip_entry &entry) override;

  AppContextStream RequestFile(const std::string &path) override;

  AppContextStream FindFile(const std::string &rootFolder,
                            const std::string &pattern) override;

  void DisposeFile(std::istream *str) override;

  ZIPIOContext_impl(const std::string &file, const PathFilter &pathFilter_,
                    const PathFilter &moduleFilter_)
      : rd(file), pathFilter(&pathFilter_), moduleFilter(&moduleFilter_) {
    Read();
    pathFilter = moduleFilter = nullptr;
  }

  ZIPIOContext_impl(const std::string &file) : rd(file) { Read(); }

private:
  void ReadEntry();
  void Read();

  std::map<std::istream *, std::unique_ptr<ZIPDataHolder>> openedFiles;
  BinReader<> rd;
  const PathFilter *pathFilter = nullptr;
  const PathFilter *moduleFilter = nullptr;
};

struct ZIPMemoryStream : ZIPDataHolder {
  std::istringstream stream;

  ZIPMemoryStream(std::string &&input)
      : stream(input, std::ios::in | std::ios::binary) {}
};

struct ZIPFileStream : ZIPDataHolder {
  BinReaderEx rd;
  std::string path;
  ZIPFileStream(const std::string &path_) : rd(path_), path(path_) {}
  ~ZIPFileStream() {
    es::Dispose(rd);
    try {
      es::RemoveFile(path);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  }
};

std::istream *ZIPIOContext_impl::OpenFile(const zip_entry &entry) {
  std::lock_guard<std::mutex> guard(ZIPLock);
  rd.Seek(entry.first);
  constexpr size_t memoryLimit = 16777216;

  if (entry.second > memoryLimit) {
    std::string path = es::GetTempFilename();
    {
      std::string semi;
      semi.resize(memoryLimit);
      const size_t numBlocks = entry.second / memoryLimit;
      const size_t restBytes = entry.second % memoryLimit;
      BinWritter wr(path);

      for (size_t b = 0; b < numBlocks; b++) {
        rd.ReadBuffer(&semi[0], memoryLimit);
        wr.WriteContainer(semi);
      }

      if (restBytes) {
        rd.ReadBuffer(&semi[0], restBytes);
        wr.WriteBuffer(semi.data(), restBytes);
      }
    }

    auto stoff = std::make_unique<ZIPFileStream>(path);
    std::istream *ptr = stoff->rd.baseStream;
    openedFiles.emplace(ptr, std::move(stoff));
    return ptr;
  } else {
    std::string semi;
    rd.ReadContainer(semi, entry.second);
    auto stoff = std::make_unique<ZIPMemoryStream>(std::move(semi));
    std::istream *ptr = &stoff->stream;
    openedFiles.emplace(ptr, std::move(stoff));
    return ptr;
  }
}

void ZIPIOContext_impl::DisposeFile(std::istream *str) {
  std::lock_guard<std::mutex> guard(ZIPLock);
  openedFiles.erase(str);
}

AppContextStream ZIPIOContext_impl::RequestFile(const std::string &path) {
  auto found = vfs.find(path);

  if (es::IsEnd(vfs, found)) {
    throw es::FileNotFoundError(path);
  }

  return {OpenFile(found->second), this};
}

AppContextStream ZIPIOContext_impl::FindFile(const std::string &,
                                             const std::string &pattern) {
  PathFilter filter;
  filter.AddFilter(pattern);

  for (auto &f : vfs) {
    es::string_view kvi(f.first);
    size_t lastSlash = kvi.find_last_of('/');
    kvi.remove_prefix(lastSlash + 1);

    if (filter.IsFiltered(kvi)) {
      return {OpenFile(f.second), this};
    }
  }

  throw es::FileNotFoundError(pattern);
}

void ZIPIOContext_impl::Read() {
  const size_t fileSize = rd.GetSize();
  while (rd.Tell() < fileSize) {
    ReadEntry();
  }
}

void ZIPIOContext_impl::ReadEntry() {
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
      size_t entrySize = hdr.compressedSize;

      if (hdr.compressedSize == 0xffffffff) {
        const size_t extraEnd = rd.Push() + hdr.extraFieldSize;

        while (rd.Tell() < extraEnd) {
          ZIP64Extra extra;
          rd.Read(extra.id);
          rd.Read(extra.size);

          if (extra.id == 1) {
            rd.Read(extra.compressedSize);
            entrySize = extra.compressedSize;
            break;
          } else {
            rd.Skip(extra.size);
          }
        }

        rd.Pop();
      }

      rd.Skip(hdr.extraFieldSize);
      auto entry = std::make_pair(rd.Tell(), entrySize);
      rd.Skip(entrySize);

      if (pathFilter && !pathFilter->IsFiltered(path)) {
        return;
      }

      if (moduleFilter && !moduleFilter->IsFiltered(path)) {
        return;
      }

      vfs.emplace(std::move(path), entry);
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

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file,
                                             const PathFilter &pathFilter,
                                             const PathFilter &moduleFilter) {
  return std::make_unique<ZIPIOContext_impl>(file, pathFilter, moduleFilter);
}

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file) {
  return std::make_unique<ZIPIOContext_impl>(file);
}
