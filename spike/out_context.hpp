/*  Spike is universal dedicated module handler
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

#pragma once
#include "cache.hpp"
#include "datas/app_context.hpp"
#include "datas/binwritter.hpp"
#include "formats/ZIP.hpp"
#include <optional>
#include <set>
#include <sstream>
#include <vector>

struct CounterLine;

struct ZIPExtactContext : AppExtractContext {
  ZIPExtactContext(const std::string &outFile)
      : records(outFile), outputFile(outFile), entries(entriesStream),
        cache(std::in_place) {}
  ZIPExtactContext(const std::string &outFile, bool)
      : records(outFile), entries(entriesStream) {}
  ZIPExtactContext(const ZIPExtactContext &) = delete;
  ZIPExtactContext(ZIPExtactContext &&) = delete;

  void NewFile(const std::string &path) override;
  void SendData(std::string_view data) override;
  bool RequiresFolders() const override;
  void AddFolderPath(const std::string &path) override;
  void GenerateFolders() override;
  using cache_begin_cb = void (*)();
  void FinishZIP(cache_begin_cb cacheBeginCB);

  std::string prefixPath;
  std::function<void()> forEachFile;

private:
  friend struct ZIPMerger;
  BinWritter records;
  std::string outputFile;
  std::stringstream entriesStream;
  BinWritterRef entries;
  ZIPLocalFile zLocalFile{ZIPLocalFile::ID, 10};
  size_t curLocalFileOffset = 0;
  size_t numEntries = 0;
  size_t curFileSize = 0;
  std::string curFileName;
  std::optional<CacheGenerator> cache;
  std::vector<uint64> fileOffsets;
  void FinishFile(bool final = false);
};

struct ZIPMerger {
  ZIPMerger(const std::string &outFiles, const std::string &outEntries)
      : entries(outEntries), records(outFiles), entriesFile(outEntries),
        outFile(outFiles) {}
  ZIPMerger() = default;
  ZIPMerger(const ZIPMerger &) = delete;
  ZIPMerger(ZIPMerger&&) = delete;
  using cache_begin_cb = void (*)();
  void Merge(ZIPExtactContext &other, const std::string &recordsFile);
  void FinishMerge(cache_begin_cb cacheBeginCB);

private:
  BinWritter entries;
  BinWritter records;
  std::string entriesFile;
  std::string outFile;
  size_t numEntries = 0;
  CacheGenerator cache;
};

struct IOExtractContext : AppExtractContext, BinWritter {
  std::string outDir;
  std::set<std::string> folderTree;
  std::function<void()> forEachFile;

  IOExtractContext(const std::string &outDir_) : outDir(outDir_) {}

  void NewFile(const std::string &path) override;
  void SendData(std::string_view data) override;
  bool RequiresFolders() const override;
  void AddFolderPath(const std::string &path) override;
  void GenerateFolders() override;
};
