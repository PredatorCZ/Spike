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
#include "out_context.hpp"
#include "tmp_storage.hpp"
#include <list>
#include <mutex>
#include <optional>
#include <spanstream>

static std::mutex simpleIOLock;

struct AppContextShareImpl : AppContextShare {
  std::ostream &NewFile(const std::string &path) override {
    outFile.Open(basePath + path);
    return outFile.BaseStream();
  }

  void BaseOutputPath(const std::string &basePath_) override {
    basePath = basePath_;

    if (basePath.back() != '/') {
      basePath.push_back('/');
    }
  }

  JenHash Hash() override { return JenHash(FullPath()); }

  std::string FullPath() override {
    return basePath + std::string(workingFile.GetFullPath());
  }

  BinWritter outFile;
  std::string basePath;
};

struct SimpleIOContext : AppContextShareImpl {
  SimpleIOContext(const std::string &path) {
    workingFile.Load(path);
    mainFile.Open(path);
  }
  std::istream *OpenFile(const std::string &path);

  AppContextStream RequestFile(const std::string &path) override;

  AppContextFoundStream FindFile(const std::string &rootFolder,
                                 const std::string &pattern) override;
  std::istream &GetStream() override;
  std::string GetBuffer(size_t size, size_t begin) override;

  void DisposeFile(std::istream *str) override;

  AppExtractContext *ExtractContext() override {
    if (ectx) [[unlikely]] {
      throw std::logic_error(
          "ExtractContext has been already called within this context.");
    }

    std::string outPath(basePath);
    outPath += workingFile.GetFullPathNoExt();

    if (mainSettings.extractSettings.makeZIP) {
      if (workingFile.GetExtension() == ".zip") {
        outPath.append("_out");
      }

      outPath.append(".zip");

      auto uniq = std::make_unique<ZIPExtactContext>(outPath);
      uniq->forEachFile = forEachFile;
      ectx = std::move(uniq);
    } else {
      if (!mainSettings.extractSettings.folderPerArc) {
        outPath = workingFile.GetFolder();
      } else {
        es::mkdir(outPath);
        outPath.push_back('/');
      }

      auto uniq = std::make_unique<IOExtractContext>(outPath);
      uniq->forEachFile = forEachFile;
      ectx = std::move(uniq);
    }

    return ectx.get();
  }

  void Finish() override {
    if (ectx && mainSettings.extractSettings.makeZIP) {
      static_cast<ZIPExtactContext *>(ectx.get())->FinishZIP([] {
        printinfo("Generating cache.");
      });
    }
  }

private:
  BinReader mainFile;
  BinReader streamedFiles[32];
  uint32 usedFiles = 0;
  std::unique_ptr<AppExtractContext> ectx;
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

  return {OpenFile(sc.Files().front()), this, AFileInfo(sc.Files().front())};
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

std::istream &SimpleIOContext::GetStream() { return mainFile.BaseStream(); }

std::string SimpleIOContext::GetBuffer(size_t size, size_t begin) {
  mainFile.Push();
  mainFile.Seek(begin);
  std::string buffer;
  mainFile.ReadContainer(buffer,
                         size == size_t(-1) ? mainFile.GetSize() : size);
  mainFile.Pop();

  return buffer;
}

std::shared_ptr<AppContextShare> MakeIOContext(const std::string &path) {
  return std::make_unique<SimpleIOContext>(path);
}

struct ZIPIOContextInstance : AppContextShareImpl {
  ZIPIOContextInstance(const ZIPIOContextInstance &) = delete;
  ZIPIOContextInstance(ZIPIOContextInstance &&) = delete;
  ZIPIOContextInstance(ZIPIOContext *base_, ZIPIOEntry entry_)
      : base(base_), entry(entry_) {
    workingFile.Load(entry_.AsView());
    BaseOutputPath(base->basePath);
  }

  ~ZIPIOContextInstance() {
    if (stream) {
      base->DisposeFile(stream);
    }
  }

  AppContextStream RequestFile(const std::string &path) override {
    return base->RequestFile(path);
  }
  void DisposeFile(std::istream *file) override { base->DisposeFile(file); }
  AppContextFoundStream FindFile(const std::string &rootFolder,
                                 const std::string &pattern) override {
    return base->FindFile(rootFolder, pattern);
  }

  std::istream &GetStream() override {
    if (!stream) {
      stream = base->OpenFile(entry);
    }
    return *stream;
  }
  std::string GetBuffer(size_t size, size_t begin) override {
    BinReaderRef rd(GetStream());
    rd.Push();
    rd.Seek(begin);
    std::string buffer;
    rd.ReadContainer(buffer, size == size_t(-1) ? rd.GetSize() : size);
    rd.Pop();
    return buffer;
  }

  AppExtractContext *ExtractContext() override {
    if (ectx) [[unlikely]] {
      throw std::logic_error(
          "ExtractContext has been already called within this context.");
    }

    if (mainSettings.extractSettings.makeZIP) {
      base->InitMerger();
      entriesPath = RequestTempFile();
      auto uniq = std::make_unique<ZIPExtactContext>(entriesPath, false);
      if (mainSettings.extractSettings.folderPerArc) {
        uniq->prefixPath = workingFile.GetFullPathNoExt();
        uniq->prefixPath.push_back('/');
      }
      uniq->forEachFile = forEachFile;
      ectx = std::move(uniq);
    } else {
      std::string outPath;
      if (!mainSettings.extractSettings.folderPerArc) {
        outPath = workingFile.GetFolder();
      } else {
        outPath = basePath;
        outPath += workingFile.GetFullPathNoExt();
        mkdirs(outPath);
        outPath.push_back('/');
      }

      auto uniq = std::make_unique<IOExtractContext>(outPath);
      uniq->forEachFile = forEachFile;
      ectx = std::move(uniq);
    }

    return ectx.get();
  }

  void Finish() override {
    if (ectx && mainSettings.extractSettings.makeZIP) {
      base->Merge(static_cast<ZIPExtactContext *>(ectx.get()), entriesPath);
      es::Dispose(ectx);
      es::RemoveFile(entriesPath);
    }
  }

  ZIPIOContext *base;
  std::istream *stream = nullptr;
  ZipEntry entry;
  std::unique_ptr<AppExtractContext> ectx;
  std::string entriesPath;
};

std::shared_ptr<AppContextShare> ZIPIOContext::Instance(ZIPIOEntry entry) {
  return std::make_unique<ZIPIOContextInstance>(this, entry);
}

static std::mutex ZIPLock;

struct ZIPDataHolder {
  virtual ~ZIPDataHolder() = default;
};

struct ZIPIOContext_implbase : ZIPIOContext {
  ZIPIOContext_implbase(const std::string &file) : zipMount(file) {}
  std::istream *OpenFile(const ZipEntry &entry) override;
  std::string GetChunk(const ZipEntry &entry, size_t offset,
                       size_t size) const override;
  void DisposeFile(std::istream *str) override;

  void Merge(ZIPExtactContext *eCtx, const std::string &records) override {
    merger->Merge(*eCtx, records);
  }

  void InitMerger() override {
    if (!merger) {
      merger.emplace(basePath + "_out.zip", RequestTempFile());
    }
  }

  void Finish() override {
    if (merger) {
      merger->FinishMerge([] { printinfo("Generating cache."); });
    }
  }

protected:
  std::list<std::spanstream> openedFiles;
  es::MappedFile zipMount;
  std::optional<ZIPMerger> merger;
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
  auto dataBegin = static_cast<char *>(zipMount.data) + entry.offset;
  auto dataEnd = dataBegin + entry.size;

  std::lock_guard<std::mutex> guard(ZIPLock);
  auto &str = openedFiles.emplace_back(std::span<char>(dataBegin, dataEnd),
                                       std::ios::binary | std::ios::in);
  return &str;
}

std::string ZIPIOContext_implbase::GetChunk(const ZipEntry &entry,
                                            size_t offset, size_t size) const {
  auto dataBegin = static_cast<char *>(zipMount.data) + entry.offset + offset;
  auto dataEnd = dataBegin + size;

  return {dataBegin, dataEnd};
}

void ZIPIOContext_implbase::DisposeFile(std::istream *str) {
  std::lock_guard<std::mutex> guard(ZIPLock);
  openedFiles.remove_if([&](auto &spanStr) {
    return static_cast<std::istream *>(&spanStr) == str;
  });
}

struct ZIPIOContextIter_impl : ZIPIOEntryRawIterator {
  using map_type = std::map<std::string_view, ZipEntry>;
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
  void Read();
  const PathFilter *pathFilter = nullptr;
  const PathFilter *moduleFilter = nullptr;
  std::map<std::string_view, ZipEntry> vfs;
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
    std::string_view kvi(f.first);
    size_t lastSlash = kvi.find_last_of('/');
    kvi.remove_prefix(lastSlash + 1);

    if (filter.IsFiltered(kvi)) {
      return {OpenFile(f.second), this, AFileInfo(f.first)};
    }
  }

  throw es::FileNotFoundError(pattern);
}

// Warning: Unaligned accesses
// Note: Multiple central directories? (unlikely)
void ZIPIOContext_impl::Read() {
  auto curEnd = static_cast<char *>(zipMount.data) + zipMount.dataSize -
                (sizeof(ZIPCentralDir) - 2);
  auto curLocator = reinterpret_cast<const ZIPCentralDir *>(curEnd);

  if (curLocator->id != ZIPCentralDir::ID) {
    int numIters = 4096;
    while (numIters > 0) {
      curEnd--;
      numIters--;

      curLocator = reinterpret_cast<const ZIPCentralDir *>(curEnd);
      if (curLocator->id == ZIPCentralDir::ID) {
        break;
      }
    }
  }

  if (curLocator->id != ZIPCentralDir::ID) {
    throw std::runtime_error("Cannot find ZIP central directory");
  }

  uint64 dirOffset = 0;
  uint64 numEntries = 0;
  uint64 dirSize = 0;

  if (curLocator->dirOffset == -1U || curLocator->numDirEntries == uint16(-1) ||
      curLocator->dirSize == -1U) {
    curEnd -= sizeof(ZIP64CentralDir) - 8;
    auto curLocatorX64 = reinterpret_cast<const ZIP64CentralDir *>(curEnd);
    if (curLocatorX64->id != ZIP64CentralDir::ID) {
      int numIters = 4096;
      while (numIters > 0) {
        curEnd--;
        numIters--;

        curLocatorX64 = reinterpret_cast<const ZIP64CentralDir *>(curEnd);
        if (curLocatorX64->id == ZIP64CentralDir::ID) {
          break;
        }
      }
    }

    if (curLocatorX64->id != ZIP64CentralDir::ID) {
      throw std::runtime_error("Cannot find ZIPx64 central directory");
    }

    std::spanstream entriesSpan(
        std::span<char>(curEnd, static_cast<char *>(zipMount.data) +
                                          zipMount.dataSize),
        std::ios::binary | std::ios::in);
    BinReaderRef rd(entriesSpan);
    ZIP64CentralDir x64CentraDir;
    rd.Read(x64CentraDir);

    dirOffset = x64CentraDir.dirOffset;
    numEntries = x64CentraDir.numDirEntries;
    dirSize = x64CentraDir.dirSize;
  } else {
    dirOffset = curLocator->dirOffset;
    numEntries = curLocator->numDirEntries;
    dirSize = curLocator->dirSize;
  }

  auto entriesBegin = static_cast<char *>(zipMount.data) + dirOffset;
  auto entriesEnd = entriesBegin + dirSize;
  std::spanstream entriesSpan(std::span<char>(entriesBegin, entriesEnd),
                              std::ios::binary | std::ios::in);
  BinReaderRef rd(entriesSpan);
  std::spanstream localStreamSpan(
      std::span<char>(static_cast<char *>(zipMount.data), entriesBegin),
      std::ios::binary | std::ios::in);
  BinReaderRef localRd(localStreamSpan);

  for (size_t d = 0; d < numEntries; d++) {
    uint32 id;
    rd.Push();
    rd.Read(id);
    rd.Pop();

    switch (id) {
    case ZIPFile::ID: {
      ZIPFile hdr;
      rd.Read(hdr);

      [&] {
        std::string_view entryName(entriesBegin + rd.Tell(), hdr.fileNameSize);
        rd.Skip(hdr.fileNameSize);

        if (!hdr.compressedSize) {
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

        size_t entrySize = hdr.compressedSize;
        size_t localOffset = hdr.localHeaderOffset;

        if (hdr.compressedSize == -1U || hdr.uncompressedSize == -1U ||
            hdr.localHeaderOffset == -1U) {
          const size_t extraEnd = rd.Push() + hdr.extraFieldSize;

          while (rd.Tell() < extraEnd) {
            ZIP64Extra extra;
            rd.Read(extra.id);
            rd.Read(extra.size);

            if (extra.id == 1) {
              if (hdr.uncompressedSize == -1U) {
                rd.Read(extra.uncompressedSize);
                entrySize = extra.uncompressedSize;
              }
              if (hdr.compressedSize == -1U) {
                rd.Read(extra.compressedSize);
              }
              if (hdr.localHeaderOffset == -1U) {
                rd.Read(extra.localHeaderOffset);
                localOffset = extra.localHeaderOffset;
              }
              break;
            } else {
              rd.Skip(extra.size);
            }
          }

          rd.Pop();
        }

        if (pathFilter && !pathFilter->IsFiltered(entryName)) {
          return;
        }

        if (moduleFilter && !moduleFilter->IsFiltered(entryName)) {
          return;
        }

        ZIPLocalFile localEntry;
        localRd.Seek(localOffset);
        localRd.Read(localEntry);
        ZipEntry entry;
        entry.size = entrySize;
        entry.offset = localRd.Tell() + localEntry.extraFieldSize +
                       localEntry.fileNameSize;
        vfs.emplace(entryName, entry);
      }();

      rd.Skip(hdr.extraFieldSize + hdr.fileCommentSize);
      break;
    }
    default:
      using std::to_string;
      throw std::runtime_error("Invalid dir entry " + to_string(id) + " at " +
                               to_string(rd.Tell() + dirOffset));
    }
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
          return {OpenFile(found), this, AFileInfo(item)};
        },
        found.name);
  }

  ZIPIOContextIterator Iter(ZIPIOEntryType type) const override {
    return {cache.Iter(type)};
  }

  ZIPIOContextCached(const std::string &file, es::MappedFile &&cacheFile)
      : ZIPIOContext_implbase(file), cacheMount(std::move(cacheFile)) {
    cache.Mount(cacheMount.data);
    auto &cacheHdr = reinterpret_cast<const CacheBaseHeader &>(cache.Header());
    auto zipData = static_cast<const char *>(zipMount.data);
    auto zipHeader = reinterpret_cast<const CacheBaseHeader *>(
        zipData + cacheHdr.zipCheckupOffset);

    if (memcmp(zipHeader, &cacheHdr, sizeof(cacheHdr))) {
      throw std::runtime_error("Cache header and zip checkup are different.");
    }
  }

private:
  Cache cache;
  es::MappedFile cacheMount;
};

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file,
                                             const PathFilter &pathFilter,
                                             const PathFilter &moduleFilter) {
  return std::make_unique<ZIPIOContext_impl>(file, pathFilter, moduleFilter);
}

std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file) {
  std::string cacheFile = file + ".cache";
  es::MappedFile mf;
  try {
    mf = es::MappedFile(cacheFile);
  } catch (const std::exception &e) {
    printwarning("Failed loading cache: " << e.what());
    return std::make_unique<ZIPIOContext_impl>(file);
  }
  try {
    printinfo("Found zip cache: " << cacheFile);
    return std::make_unique<ZIPIOContextCached>(file, std::move(mf));
  } catch (const std::exception &e) {
    printwarning("Failed loading cache: " << e.what());
    return std::make_unique<ZIPIOContext_impl>(file);
  }
}
