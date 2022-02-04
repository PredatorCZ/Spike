/*  Spike is universal dedicated module handler
    This source contains context for input data
    Part of PreCore project

    Copyright 2021-2022 Lukas Cone

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
#include "tmp_storage.hpp"
#include <mutex>
#include <sstream>

static std::mutex simpleIOLock;

struct SimpleIOContext : AppContext {
  std::istream *OpenFile(const std::string &path);

  AppContextStream RequestFile(const std::string &path) override;

  AppContextFoundStream FindFile(const std::string &rootFolder,
                                 const std::string &pattern) override;

  void DisposeFile(std::istream *str) override;

private:
  BinReader streamedFiles[32];
  uint32 usedFiles = 0;
};

struct ZIPContext : AppContext {
  AppContextStream RequestFile(const std::string &path) override;

  AppContextFoundStream FindFile(const std::string &rootFolder,
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
      return &streamedFiles[b].BaseStream();
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

AppContextFoundStream SimpleIOContext::FindFile(const std::string &rootFolder,
                                                const std::string &pattern) {
  DirectoryScanner sc;
  sc.AddFilter(pattern);
  sc.Scan(rootFolder);

  if (sc.Files().empty()) {
    throw es::FileNotFoundError(pattern);
  } else if (sc.Files().size() > 1) {
    throw std::runtime_error("Too many files found.");
  }

  return {OpenFile(sc.Files().front()), this, sc.Files().front()};
}

void SimpleIOContext::DisposeFile(std::istream *str) {
  size_t index = 0;

  for (auto &f : streamedFiles) {
    if (&f.BaseStream() == str) {
      uint32 bit = 1 << index;

      if (!(usedFiles & bit)) {
        throw std::runtime_error("Stream already freed.");
      }

      es::Dispose(f);
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

struct ZIPIOContext_implbase : ZIPIOContext {
  ZIPIOContext_implbase(const std::string &file) : rd(file) {}
  std::istream *OpenFile(const ZipEntry &entry) override;
  std::string GetChunk(const ZipEntry &entry, size_t offset,
                       size_t size) const override;
  void DisposeFile(std::istream *str) override;

protected:
  std::map<std::istream *, std::unique_ptr<ZIPDataHolder>> openedFiles;
  BinReader rd;
};

struct ZIPMemoryStream : ZIPDataHolder {
  std::istringstream stream;

  ZIPMemoryStream(std::string &&input)
      : stream(input, std::ios::in | std::ios::binary) {}
};

struct ZIPFileStream : ZIPDataHolder {
  BinReader rd;
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

std::istream *ZIPIOContext_implbase::OpenFile(const ZipEntry &entry) {
  std::lock_guard<std::mutex> guard(ZIPLock);
  rd.Seek(entry.offset);
  constexpr size_t memoryLimit = 16777216;

  if (entry.size > memoryLimit) {
    std::string path = RequestTempFile();
    {
      std::string semi;
      semi.resize(memoryLimit);
      const size_t numBlocks = entry.size / memoryLimit;
      const size_t restBytes = entry.size % memoryLimit;
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
    std::istream *ptr = &stoff->rd.BaseStream();
    openedFiles.emplace(ptr, std::move(stoff));
    return ptr;
  } else {
    std::string semi;
    rd.ReadContainer(semi, entry.size);
    auto stoff = std::make_unique<ZIPMemoryStream>(std::move(semi));
    std::istream *ptr = &stoff->stream;
    openedFiles.emplace(ptr, std::move(stoff));
    return ptr;
  }
}

std::string ZIPIOContext_implbase::GetChunk(const ZipEntry &entry,
                                            size_t offset, size_t size) const {
  std::lock_guard<std::mutex> guard(ZIPLock);
  rd.Seek(entry.offset + offset);
  std::string retVal;
  rd.ReadContainer(retVal, size);
  return retVal;
}

void ZIPIOContext_implbase::DisposeFile(std::istream *str) {
  std::lock_guard<std::mutex> guard(ZIPLock);
  openedFiles.erase(str);
}

struct ZIPIOContextIter_impl : ZIPIOEntryRawIterator {
  using map_type = std::map<std::string, ZipEntry>;
  ZIPIOContextIter_impl(const map_type &map)
      : base(&map), current(map.begin()), end(map.end()) {}
  ZIPIOEntry Fist() const override {
    if (current == end) {
      return {};
    }
    return {current->second, current->first};
  }
  ZIPIOEntry Next() const override {
    if (current == end) {
      return {};
    }
    current++;
    if (current == end) {
      return {};
    }

    return {current->second, current->first};
  }
  size_t Count() const override { return base->size(); }

  const map_type *base;
  mutable map_type::const_iterator current;
  map_type::const_iterator end;
};

struct ZIPIOContext_impl : ZIPIOContext_implbase {
  AppContextStream RequestFile(const std::string &path) override;

  AppContextFoundStream FindFile(const std::string &rootFolder,
                                 const std::string &pattern) override;

  ZIPIOContextIterator Iter(ZIPIOEntryType) const override {
    return {std::make_unique<ZIPIOContextIter_impl>(vfs)};
  }

  ZIPIOContext_impl(const std::string &file, const PathFilter &pathFilter_,
                    const PathFilter &moduleFilter_)
      : ZIPIOContext_implbase(file), pathFilter(&pathFilter_),
        moduleFilter(&moduleFilter_) {
    Read();
    pathFilter = moduleFilter = nullptr;
  }

  ZIPIOContext_impl(const std::string &file) : ZIPIOContext_implbase(file) {
    Read();
  }

private:
  void ReadEntry();
  void Read();
  const PathFilter *pathFilter = nullptr;
  const PathFilter *moduleFilter = nullptr;
  std::map<std::string, ZipEntry> vfs;
};

AppContextStream ZIPIOContext_impl::RequestFile(const std::string &path) {
  auto found = vfs.find(path);

  if (es::IsEnd(vfs, found)) {
    throw es::FileNotFoundError(path);
  }

  return {OpenFile(found->second), this};
}

AppContextFoundStream ZIPIOContext_impl::FindFile(const std::string &,
                                                  const std::string &pattern) {
  PathFilter filter;
  filter.AddFilter(pattern);

  for (auto &f : vfs) {
    es::string_view kvi(f.first);
    size_t lastSlash = kvi.find_last_of('/');
    kvi.remove_prefix(lastSlash + 1);

    if (filter.IsFiltered(kvi)) {
      return {OpenFile(f.second), this, f.first};
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
      ZipEntry entry{rd.Tell(), entrySize};
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

ZIPIOEntry::operator bool() const {
  return std::visit([](auto &name) { return !name.empty(); }, name);
}

struct ZIPIOContextCached : ZIPIOContext_implbase {
  AppContextStream RequestFile(const std::string &path) override {

    auto found = cache.RequestFile(path);

    if (!found.size) {
      throw es::FileNotFoundError(path);
    }

    return {OpenFile(found), this};
  }

  AppContextFoundStream FindFile(const std::string &,
                                 const std::string &pattern) override {
    auto found = cache.FindFile(pattern);

    if (!found.size) {
      throw es::FileNotFoundError(pattern);
    }

    return std::visit(
        [&](auto &item) -> AppContextFoundStream {
          return {OpenFile(found), this, item};
        },
        found.name);
  }

  ZIPIOContextIterator Iter(ZIPIOEntryType type) const override {
    return {cache.Iter(type)};
  }

  ZIPIOContextCached(const std::string &file, BinReaderRef cacheFile)
      : ZIPIOContext_implbase(file) {

    ZIPLocalFile zFile;
    rd.Read(zFile);
    std::string name;
    rd.ReadContainer(name, zFile.fileNameSize);
    if (name != "__cache_checkup__") {
      throw std::runtime_error(
          "Cache checkup was not found in zip root entry.");
    }

    CacheBaseHeader hdr;
    rd.Read(hdr);
    CacheBaseHeader cacheHdr;
    cacheFile.Push();
    cacheFile.Read(cacheHdr);
    cacheFile.Pop();

    if (memcmp(&hdr, &cacheHdr, sizeof(hdr))) {
      throw std::runtime_error("Cache header and zip checkup are different.");
    }

    cache.Load(cacheFile);
  }

private:
  Cache cache;
};

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file,
                                             const PathFilter &pathFilter,
                                             const PathFilter &moduleFilter) {
  return std::make_unique<ZIPIOContext_impl>(file, pathFilter, moduleFilter);
}

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file) {
  std::string cacheFile = file + ".cache";
  BinReader rd;
  try {
    rd.Open(cacheFile);
  } catch (const std::exception &e) {
    printwarning("Failed loading cache: " << e.what());
    return std::make_unique<ZIPIOContext_impl>(file);
  }
  try {
    printinfo("Found zip cache: " << cacheFile);
    return std::make_unique<ZIPIOContextCached>(file, rd);
  } catch (const std::exception &e) {
    printwarning("Failed loading cache: " << e.what());
    return std::make_unique<ZIPIOContext_impl>(file);
  }
}
