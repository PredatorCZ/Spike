/*  Cache format loader for seekless zip loading
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

#include "cache.hpp"
#include "datas/base_128.hpp"
#include "datas/binreader_stream.hpp"
#include "datas/except.hpp"
#include "datas/fileinfo.hpp"

struct ZipEntryLeaf : ZipEntry {
  HybridLeaf *parent;
  union {
    const char *fileNamePtr;
    const char fileName[8];
  };
  union {
    const char extendedFileName[4];
    uint32 tailData;
  };
  uint16 fileNameSize;
  uint16 totalFileNameSize;

  es::string_view Name() const {
    if (fileNameSize < 13) {
      // basically uses fileName + extendedFileName datablock
      return {fileName, fileNameSize};
    } else {
      return {fileNamePtr, fileNameSize};
    }
  }

  bool operator<(es::string_view sw) const { return Name() < sw; }
};

struct ChildrenIter {
  using value_type = const HybridLeaf *;
  value_type const *begin_;
  value_type const *end_;

  auto begin() const { return begin_; }
  auto end() const { return end_; }
};

struct FinalsIter {
  using value_type = const ZipEntryLeaf *;
  value_type const *begin_;
  value_type const *end_;

  auto begin() const { return begin_; }
  auto end() const { return end_; }
};

struct HybridLeaf {
  HybridLeaf *children[1];
  HybridLeaf *parent;
  union {
    const char *pathPartPtr;
    const char pathPart[8];
  };
  uint16 numChildren;
  uint16 pathPartSize;
  uint32 numFinals;
  ZipEntryLeaf *entries[1];

  ChildrenIter Children() const {
    return {children - numChildren + 1, children + 1};
  }

  FinalsIter Finals() const { return {entries, entries + numFinals}; }

  es::string_view Name() const {
    if (pathPartSize < 9) {
      return {pathPart, pathPartSize};
    } else {
      return {pathPartPtr, pathPartSize};
    }
  }
};

ZIPIOEntry Cache::FindFile(es::string_view pattern) {
  auto begin = Header().entries;
  auto end = begin + Header().numFiles;
  bool clampBegin = pattern.front() == '^';
  bool clampEnd = pattern.back() == '$';

  if (clampBegin) {
    pattern.remove_prefix(1);
  }

  if (clampEnd) {
    pattern.remove_suffix(1);
  }

  auto wildcharPos = pattern.find_first_of('*');
  bool useWildchar = wildcharPos != pattern.npos;

  if (useWildchar) {
    auto part1 = pattern.substr(0, wildcharPos);
    auto part2 = pattern.substr(wildcharPos + 1);

    // cases ^foo*bar or ^foo*bar$
    if (clampBegin) {
      auto found = std::lower_bound(begin, end, part1);

      if (found == end) {
        return {};
      }

      auto foundName = found->Name();

      while (foundName.begins_with(part1)) {
        if (clampEnd) {
          if (foundName.ends_with(part2)) {
            return {*found, foundName};
          }
        } else if (foundName.find(part2, part1.size()) != foundName.npos) {
          return {*found, foundName};
        }

        found++;

        if (found == end) {
          return {};
        }

        foundName = found->Name();
      }

      return {};
    }

    // cases foo*bar$ only
    if (clampEnd) {
      for (auto p = begin; p != end; p++) {
        const auto foundName = p->Name();

        if (foundName.ends_with(part2)) {
          auto foundName2 = foundName;
          foundName2.remove_suffix(part2.size());

          if (foundName2.find(part1) != foundName.npos) {
            return {*p, foundName};
          }
        }
      }

      return {};
    }

    // cases foo*bar only
    for (auto p = begin; p != end; p++) {
      const auto foundName = p->Name();

      if (auto found = foundName.find(part1); found != foundName.npos) {
        auto foundName2 = foundName;
        foundName2.remove_prefix(found + part1.size());

        if (foundName2.find(part2) != foundName.npos) {
          return {*p, foundName};
        }
      }
    }

    return {};
  }

  if (clampBegin && clampEnd) {
    auto found = std::lower_bound(begin, end, pattern);

    if (found == end) {
      return {};
    }

    auto foundName = found->Name();

    if (foundName == pattern) {
      return {*found, foundName};
    }

    return {};
  } else if (clampBegin) {
    auto found = std::lower_bound(begin, end, pattern);

    if (found == end) {
      return {};
    }

    auto foundName = found->Name();

    if (foundName.begins_with(pattern)) {
      return {*found, foundName};
    }

    return {};
  } else if (clampEnd) {
    for (auto p = begin; p != end; p++) {
      auto foundName = p->Name();
      if (foundName.ends_with(pattern)) {
        return {*p, foundName};
      }
    }

    return {};
  }

  for (auto p = begin; p != end; p++) {
    auto foundName = p->Name();
    if (foundName.find(pattern) != foundName.npos) {
      return {*p, foundName};
    }
  }

  return {};
}

bool operator<(const HybridLeaf *leaf, es::string_view sw) {
  return leaf->Name() < sw;
}

bool operator<(const ZipEntryLeaf *leaf, es::string_view sw) {
  return leaf->operator<(sw);
}

ZipEntry Cache::RequestFile(es::string_view path) {
  AFileInfo pp(path);
  auto parts = pp.Explode();

  if (parts.size() > Header().numLevels) {
    return {};
  }

  auto rootChildren = Header().root->Children();

  auto find = [&](auto children, size_t level) ->
      typename decltype(children)::value_type {
        auto found =
            std::lower_bound(children.begin(), children.end(), parts.at(level));

        if (es::IsEnd(children, found) || (*found)->Name() != parts.at(level)) {
          return nullptr;
        }

        return *found;
      };

  auto foundLevel = find(rootChildren, 0);

  if (!foundLevel) {
    return {};
  }

  for (size_t l = 1; l < parts.size() - 1; l++) {
    foundLevel = find(foundLevel->Children(), l);

    if (!foundLevel) {
      return {};
    }
  }

  auto foundFinal = find(foundLevel->Finals(), parts.size() - 1);

  if (!foundFinal) {
    return {};
  }

  return *foundFinal;
}

void Cache::Load(BinReaderRef rd) {
  CacheHeader hdr;
  rd.Push();
  rd.Read(hdr);

  if (hdr.id != hdr.ID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  if (hdr.version != 2) {
    throw es::InvalidVersionError(hdr.version);
  }

  rd.Pop();
  Load(rd, hdr.cacheSize);
}

void Cache::Load(BinReaderRef rd, size_t size) {
  rd.ReadContainer(buffer, size);
  auto &hdr = Header();

  for (size_t f = 0; f < hdr.numRleFixups; f++) {
    buint128 offset;
    buint128 count;
    rd.Read(offset);
    rd.Read(count);

    for (size_t s = 0; s <= count; s++) {
      auto begin = &buffer[0] + offset + s * 8;
      auto rOffset = reinterpret_cast<int64 *>(begin);
      *rOffset = reinterpret_cast<int64>(begin) + *rOffset;
    }
  }

  for (size_t f = 0; f < hdr.numSimpleFixups; f++) {
    buint128 offset;
    rd.Read(offset);

    auto begin = &buffer[0] + offset;
    auto rOffset = reinterpret_cast<int64 *>(begin);
    *rOffset = reinterpret_cast<int64>(begin) + *rOffset;
  }
}

struct ZIPIOEntryRawIterator_impl : ZIPIOEntryRawIterator {
  ZIPIOEntryRawIterator_impl(const Cache &map, ZIPIOEntryType type_)
      : base(&map), type(type_), current(map.Header().entries),
        end(current + map.Header().numFiles) {}

  void Make(std::string &buff, const HybridLeaf *parent) const {
    if (parent) {
      Make(buff, parent->parent);
      auto parentName = parent->Name();
      buff.append(parentName.data(), parentName.size());
      if (parent->parent) {
        buff.push_back('/');
      }
    }
  }

  ZIPIOEntry Make() const {
    if (type == ZIPIOEntryType::View) {
      return {*current, current->Name()};
    }

    std::string fullPath;
    fullPath.reserve(current->totalFileNameSize);
    Make(fullPath, current->parent);
    fullPath.append(current->Name());

    return {*current, fullPath};
  }

  ZIPIOEntry Fist() const override {
    if (current == end) {
      return {};
    }
    return Make();
  }

  ZIPIOEntry Next() const override {
    if (current == end) {
      return {};
    }
    current++;
    if (current == end) {
      return {};
    }

    return Make();
  }
  size_t Count() const override { return base->Header().numFiles; }

  const Cache *base;
  ZIPIOEntryType type;
  mutable const ZipEntryLeaf *current;
  const ZipEntryLeaf *end;
};

std::unique_ptr<ZIPIOEntryRawIterator> Cache::Iter(ZIPIOEntryType type) const {
  return std::make_unique<ZIPIOEntryRawIterator_impl>(*this, type);
}

#ifndef NDEBUG
#include "datas/master_printer.hpp"
#include <cstring>

void Dump(const HybridLeaf *leaf, char *buffer, const char *wholeBuffer) {
  buffer[0] = '/';
  buffer++;
  for (auto &f : leaf->Finals()) {
    es::string_view sw(wholeBuffer, buffer);
    printinfo(sw << f->Name());
  }

  for (auto &c : leaf->Children()) {
    auto name = c->Name();
    memcpy(buffer, name.data(), name.size());
    char *newBuffer = buffer + name.size();
    Dump(c, newBuffer, wholeBuffer);
  }
}

void DumpEntries(const CacheHeader &cc) {
  auto begin = cc.entries;
  auto end = begin + cc.numFiles;

  while (begin < end) {
    printinfo(begin->Name());
    begin++;
  }
}
#endif
