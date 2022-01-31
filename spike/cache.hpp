/*  Cache format for seekless zip loading
    Part of PreCore's Spike project

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
#include "datas/bincore_fwd.hpp"
#include "datas/string_view.hpp"
#include "datas/supercore.hpp"
#include <memory>
#include <variant>

struct CacheGeneratorImpl;
struct HybridLeaf;
struct ZipEntryLeaf;

struct CacheBaseHeader {
  static constexpr uint32 ID = CompileFourCC("SPCH");
  uint32 id = ID;
  uint8 version = 1;
  uint8 numLevels;
  uint16 maxPathSize;
  uint32 numFiles;
  uint32 zipCRC;
  uint64 zipSize;
};

struct CacheGenerator {
  CacheGenerator();
  ~CacheGenerator();
  void AddFile(es::string_view fileName, size_t zipOffset, size_t fileSize);
  void Write(BinWritterRef wr);
  CacheBaseHeader meta{};

private:
  std::unique_ptr<CacheGeneratorImpl> pi;
};

struct ZipEntry {
  uint64 offset;
  uint64 size = 0;
};

enum class ZIPIOEntryType {
  String,
  View,
};

struct ZIPIOEntry : ZipEntry {
  using variant_type = std::variant<es::string_view, std::string>;
  variant_type name;

  es::string_view AsView() const {
    return std::visit(
        [](auto &item) {
          return es::string_view{item.data(), item.size()};
        },
        name);
  }

  operator bool() const;
};

struct ZIPIOEntryRawIterator {
  virtual ZIPIOEntry Fist() const = 0;
  virtual ZIPIOEntry Next() const = 0;
  virtual size_t Count() const = 0;
  virtual ~ZIPIOEntryRawIterator() = default;
};

struct CacheHeader : CacheBaseHeader {
  uint32 numSimpleFixups;
  uint32 numRleFixups;
  uint32 cacheSize;
  HybridLeaf *root;
  ZipEntryLeaf *entries;
};

struct Cache {
  ZIPIOEntry FindFile(es::string_view pattern);
  ZipEntry RequestFile(es::string_view path);
  void Load(BinReaderRef rd);

  std::unique_ptr<ZIPIOEntryRawIterator> Iter(ZIPIOEntryType type) const;

private:
  friend struct ZIPIOEntryRawIterator_impl;
  void Load(BinReaderRef rd, size_t size);
  const CacheHeader &Header() const {
    return *reinterpret_cast<const CacheHeader *>(buffer.data());
  }

  std::string buffer;
};
