/*  Spike is universal dedicated module handler
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

#pragma once
#include "datas/app_context.hpp"
#include "datas/binwritter.hpp"
#include "formats/ZIP.hpp"
#include <set>
#include <sstream>

struct ZIPExtactContext : AppExtractContext {
  ZIPExtactContext(const std::string &outFile)
      : records(outFile), entries(entriesStream) {}
  ZIPExtactContext(const ZIPExtactContext &) = delete;
  ZIPExtactContext(ZIPExtactContext &&) = delete;

  void NewFile(const std::string &path) override;
  void SendData(es::string_view data) override;
  bool RequiresFolders() const override;
  void AddFolderPath(const std::string &path) override;
  void GenerateFolders() override;
  void FinishZIP();

  std::string prefixPath;

private:
  friend struct ZIPMerger;
  BinWritter<> records;
  std::stringstream entriesStream;
  BinWritterRef entries;
  ZIPLocalFile zLocalFile{ZIPLocalFile::ID, 10};
  size_t curLocalFileOffset = 0;
  size_t numEntries = 0;
  size_t curFileSize = 0;
  std::string curFileName;
  void FinishFile();
};

struct ZIPMerger {
  ZIPMerger(const std::string &outFiles, const std::string &outEntries)
      : entries(outEntries), records(outFiles), entriesFile(outEntries) {}
  ZIPMerger() = default;

  void Merge(ZIPExtactContext &other, const std::string &recordsFile);
  void FinishMerge();

private:
  BinWritter<> entries;
  BinWritter<> records;
  std::string entriesFile;
  size_t numEntries = 0;
};

struct IOExtractContext : AppExtractContext, BinWritter<> {
  std::string outDir;
  std::set<std::string> folderTree;

  IOExtractContext(const std::string &outDir_) : outDir(outDir_) {}

  void NewFile(const std::string &path) override;
  void SendData(es::string_view data) override;
  bool RequiresFolders() const override;
  void AddFolderPath(const std::string &path) override;
  void GenerateFolders() override;
};
