/*  Cache format for seekless zip loading

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
#include "spike/util/supercore.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <variant>


struct CacheGeneratorImpl;
struct HybridLeaf;
struct ZipEntryLeaf;
struct CacheHeader;

struct CacheBaseHeader {
  static constexpr uint32 ID = CompileFourCC("SPCH");
  uint32 id = ID;
  uint8 version = 3;
  uint8 numLevels;
  uint16 maxPathSize;
  uint32 numFiles;
  uint32 zipCRC;
  uint64 zipSize;
  uint64 zipCheckupOffset;
};

struct WALThread;

struct CacheGenerator {
  CacheGenerator();
  ~CacheGenerator();
  void AddFile(std::string_view fileName, size_t zipOffset, size_t fileSize);
  void WaitAndWrite(BinWritterRef wr);
  CacheBaseHeader meta{};

  struct Metrics {
    size_t numSearchHits;
    size_t numSearchMisses;
  };
  static Metrics GlobalMetrics();

private:
  friend class WALThread;
  std::unique_ptr<WALThread> workThread;
  std::thread walThread;
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
  using variant_type = std::variant<std::string_view, std::string>;
  variant_type name;

  std::string_view AsView() const {
    return std::visit(
        [](auto &item) {
          return std::string_view{item.data(), item.size()};
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

struct Cache {
  ZIPIOEntry FindFile(std::string_view pattern);
  ZipEntry RequestFile(std::string_view path);
  void Mount(const void *data_) { data = data_; }

  std::unique_ptr<ZIPIOEntryRawIterator> Iter(ZIPIOEntryType type) const;

private:
  friend struct ZIPIOEntryRawIterator_impl;
  friend struct ZIPIOContextCached;
  const CacheHeader &Header() const;

  const void *data;
};
