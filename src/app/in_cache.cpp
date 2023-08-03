/*  Cache format loader for seekless zip loading

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

#include "spike/app/cache.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/fileinfo.hpp"

template <class C, size_t Align> struct CachePointer {
  using value_type = C;
  static constexpr size_t TYPE_ALIGN = Align;

private:
  int32 varPtr;

public:
  CachePointer() = default;
  CachePointer(const CachePointer &) = delete;
  CachePointer(CachePointer &&) = delete;
  operator C *() {
    return varPtr ? reinterpret_cast<C *>(reinterpret_cast<char *>(&varPtr) +
                                          (varPtr * TYPE_ALIGN))
                  : nullptr;
  }

  C &operator*() { return *static_cast<C *>(*this); }
  C *operator->() { return *this; }

  operator const C *() const {
    return varPtr ? reinterpret_cast<const C *>(
                        reinterpret_cast<const char *>(&varPtr) +
                        (varPtr * TYPE_ALIGN))
                  : nullptr;
  }
  const C &operator*() const { return *static_cast<const C *>(*this); }
  const C *operator->() const { return *this; }
};

using HybridLeafPtr = CachePointer<HybridLeaf, 4>;
using TextPtr = CachePointer<const char, 1>;

struct ZipEntryLeaf : ZipEntry {
  HybridLeafPtr parent;
  uint16 fileNameSize;
  uint16 totalFileNameSize;
  union {
    TextPtr fileNamePtr;
    const char fileName[8];
  };

  std::string_view Name() const {
    if (fileNameSize < 9) {
      return {fileName, fileNameSize};
    } else {
      return {fileNamePtr, fileNameSize};
    }
  }

  bool operator<(std::string_view sw) const { return Name() < sw; }
};

using ZipEntryLeafPtr = CachePointer<ZipEntryLeaf, 4>;

struct ChildrenIter {
  using value_type = HybridLeafPtr;
  value_type const *begin_;
  value_type const *end_;

  auto begin() const { return begin_; }
  auto end() const { return end_; }
};

struct FinalsIter {
  using value_type = ZipEntryLeafPtr;
  value_type const *begin_;
  value_type const *end_;

  auto begin() const { return begin_; }
  auto end() const { return end_; }
};

struct HybridLeaf {
  HybridLeafPtr children[1];
  HybridLeafPtr parent;
  union {
    TextPtr pathPartPtr;
    const char pathPart[4];
  };
  uint16 numChildren;
  uint16 pathPartSize;
  uint32 numFinals;
  ZipEntryLeafPtr entries[1];

  ChildrenIter Children() const {
    return {children - numChildren + 1, children + 1};
  }

  FinalsIter Finals() const { return {entries, entries + numFinals}; }

  std::string_view Name() const {
    if (pathPartSize < 5) {
      return {pathPart, pathPartSize};
    } else {
      return {pathPartPtr, pathPartSize};
    }
  }
};

struct CacheHeader : CacheBaseHeader {
  uint32 cacheSize;
  HybridLeafPtr root;
  ZipEntryLeafPtr entries;
};

const CacheHeader &Cache::Header() const {
  return *static_cast<const CacheHeader *>(data);
}

ZIPIOEntry Cache::FindFile(std::string_view pattern) {
  const ZipEntryLeaf *begin = Header().entries;
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

      while (foundName.starts_with(part1)) {
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

    if (foundName.starts_with(pattern)) {
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

bool operator<(const HybridLeaf *leaf, std::string_view sw) {
  return leaf->Name() < sw;
}

bool operator<(const ZipEntryLeaf *leaf, std::string_view sw) {
  return leaf->operator<(sw);
}

ZipEntry Cache::RequestFile(std::string_view path) {
  AFileInfo pp(path);
  auto parts = pp.Explode();

  auto find = [&](auto children, size_t level) {
    auto found =
        std::lower_bound(children.begin(), children.end(), parts.at(level));

    if (es::IsEnd(children, found) || (*found)->Name() != parts.at(level)) {
      static const typename decltype(children)::value_type null{};
      return std::ref(null);
    }

    return std::ref(*found);
  };

  if (parts.size() > Header().numLevels) {
    return {};
  } else if (parts.size() == 1) {
    auto rootFinals = Header().root->Finals();
    auto foundFinal = find(rootFinals, parts.size() - 1);

    if (!foundFinal.get()) {
      return {};
    }

    return *foundFinal.get();
  }

  auto rootChildren = Header().root->Children();
  auto foundLevel = find(rootChildren, 0);

  if (!foundLevel.get()) {
    return {};
  }

  for (size_t l = 1; l < parts.size() - 1; l++) {
    foundLevel = find(foundLevel.get()->Children(), l);

    if (!foundLevel.get()) {
      return {};
    }
  }

  auto foundFinal = find(foundLevel.get()->Finals(), parts.size() - 1);

  if (!foundFinal.get()) {
    return {};
  }

  return *foundFinal.get();
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
#include "spike/master_printer.hpp"
#include <cstring>

void Dump(const HybridLeaf *leaf, char *buffer, const char *wholeBuffer) {
  buffer[0] = '/';
  buffer++;
  for (auto &f : leaf->Finals()) {
    std::string_view sw(wholeBuffer, buffer);
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
  const ZipEntryLeaf *begin = cc.entries;
  auto end = begin + cc.numFiles;

  while (begin < end) {
    printinfo(begin->Name());
    begin++;
  }
}
#endif
