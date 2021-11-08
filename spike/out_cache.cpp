/*  Cache format generator for seekless zip loading
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
#include "datas/binwritter_stream.hpp"
#include "datas/fileinfo.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct StringSlider {
  std::string buffer;

  size_t InsertString(es::string_view str) {
    auto found =
        std::search(buffer.begin(), buffer.end(),
                    // slightly faster than boyer_moore_searcher
                    std::boyer_moore_horspool_searcher(str.begin(), str.end()));

    if (found == buffer.end()) {
      // todo add tail compare?
      buffer.append(str.data(), str.size());
      return buffer.size() - str.size();
    } else {
      return std::distance(buffer.begin(), found);
    }
  }
};

struct SliderString {
  size_t offset;
  size_t size;
  StringSlider *base;

  bool operator<(const SliderString &other) const {
    return es::string_view(base->buffer.data() + offset, size) <
           es::string_view(other.base->buffer.data() + other.offset,
                           other.size);
  }
};

static constexpr size_t STRING_OFFSET = sizeof(CacheHeader);
static constexpr size_t HYBRIDLEAF_PARENTPTR = 8;
static constexpr size_t FINAL_PARENTPTR = 16;

struct FinalEntry : SliderString {
  size_t zipOffset;
  size_t zipSize;
  size_t totalFileNameSize;
  mutable size_t wrOffset;

  void Write(BinWritterRef wr, std::set<size_t> &fixups) const {
    wrOffset = wr.Tell();
    wr.Write(zipOffset);
    wr.Write(zipSize);
    fixups.emplace(wr.Tell());
    wr.Write<uint64>(0);

    if (size < 13) {
      wr.WriteBuffer(base->buffer.data() + offset, size);
      wr.Skip(12 - size);
    } else {
      fixups.emplace(wr.Tell());
      const int64 stringOffset = STRING_OFFSET + offset;
      const int64 thisOffset = wr.Tell();
      wr.Write(stringOffset - thisOffset);
      wr.Write<uint32>(0);
    }

    wr.Write<uint16>(size);
    wr.Write<uint16>(totalFileNameSize);
  }
};

struct HybridLeafGen {
  std::map<SliderString, const FinalEntry *> finals;
  std::map<SliderString, size_t> childrenIds;
  SliderString partName;

  size_t Write(BinWritterRef wr, const std::vector<size_t> &childrenOffsets,
               std::set<size_t> &fixups) {
    wr.ApplyPadding(8);
    for (auto &[_, id] : childrenIds) {
      const int64 childOffset = childrenOffsets.at(id);
      const int64 thisOffset = wr.Tell();
      fixups.emplace(thisOffset);
      wr.Write(childOffset - thisOffset);
    }

    const size_t retVal = wr.Tell() - 8;

    // fixup parent offset for children
    wr.Push();
    for (auto &[_, id] : childrenIds) {
      const int64 childOffset = childrenOffsets.at(id);
      wr.Seek(childOffset + HYBRIDLEAF_PARENTPTR);
      const int64 thisOffset = retVal;
      const int64 memberOffset = wr.Tell();
      wr.Write(thisOffset - memberOffset);
    }
    wr.Pop();

    if (partName.size) {
      // only root doesn't have partName & parent
      fixups.emplace(wr.Tell());
    }

    wr.Write<uint64>(0);
    if (!partName.size) {
      wr.Write<uint64>(0);
    } else if (partName.size < 9) {
      wr.WriteBuffer(partName.base->buffer.data() + partName.offset,
                     partName.size);
      wr.ApplyPadding(8);
    } else {
      const int64 pathPartOffset_ = STRING_OFFSET + partName.offset;
      const int64 thisOffset = wr.Tell();
      fixups.emplace(thisOffset);
      wr.Write(pathPartOffset_ - thisOffset);
    }

    wr.Write<uint16>(childrenIds.size());
    wr.Write<uint16>(partName.size);
    wr.Write<uint32>(finals.size());

    for (auto &[_, entry] : finals) {
      const int64 finalOffset = entry->wrOffset;
      const int64 thisOffset = wr.Tell();
      fixups.emplace(thisOffset);
      wr.Write(finalOffset - thisOffset);
    }

    wr.Push();
    for (auto &[_, entry] : finals) {
      const int64 finalOffset = entry->wrOffset;
      wr.Seek(finalOffset + FINAL_PARENTPTR);
      const int64 thisOffset = retVal;
      const int64 memberOffset = wr.Tell();
      wr.Write(thisOffset - memberOffset);
    }
    wr.Pop();

    return retVal;
  }
};

struct CacheGeneratorImpl {
  std::multiset<FinalEntry> totalCache;
  HybridLeafGen root{};
  std::vector<std::vector<HybridLeafGen>> levels;
  StringSlider slider;
  StringSlider sliderTiny;
  size_t maxPathSize = 0;

  void AddFile(es::string_view fileName, size_t offset, size_t size) {
    maxPathSize = std::max(fileName.size(), maxPathSize);
    std::vector<SliderString> parts;
    SliderString finalKey{};

    {
      AFileInfo f(fileName);
      auto fileParts = f.Explode();
      parts.reserve(fileParts.size());
      auto finalPart = fileParts.back();
      fileParts.pop_back();

      for (auto &p : fileParts) {
        auto &sliderRef = p.size() > 8 ? slider : sliderTiny;
        const size_t position = sliderRef.InsertString(p);
        parts.emplace_back(SliderString{position, p.size(), &sliderRef});
      }

      finalKey.size = finalPart.size();

      if (finalPart.size() < 13) {
        const size_t position = sliderTiny.InsertString(finalPart);
        finalKey.offset = position;
        finalKey.base = &sliderTiny;
      } else {
        const size_t position = slider.InsertString(finalPart);
        finalKey.offset = position;
        finalKey.base = &slider;
      }
    }

    const size_t numLevels = parts.size();

    if (numLevels > levels.size()) {
      levels.resize(numLevels);
    }

    auto AddFinal = [&](HybridLeafGen &where) {
      FinalEntry entry{finalKey};
      entry.zipOffset = offset;
      entry.zipSize = size;
      entry.totalFileNameSize = fileName.size();
      auto entryIter = totalCache.emplace(entry);
      where.finals.emplace(finalKey, entryIter.operator->());
    };

    if (!numLevels) {
      AddFinal(root);
      return;
    }

    auto AddChild = [&](HybridLeafGen &where, size_t index) {
      auto found = where.childrenIds.find(parts.at(index));

      if (es::IsEnd(where.childrenIds, found)) {
        where.childrenIds.emplace(parts.at(index), levels.at(index).size());
        levels.at(index).emplace_back();
        auto &levelItem = levels.at(index).back();
        levelItem.partName = parts.at(index);
        return &levelItem;
      } else {
        return &levels.at(index).at(found->second);
      }
    };

    auto *leaf = AddChild(root, 0);

    for (size_t l = 1; l < numLevels; l++) {
      leaf = AddChild(*leaf, l);
    }

    AddFinal(*leaf);
  }

  void Write(BinWritterRef wr, CacheBaseHeader &meta) {
    meta.numFiles = totalCache.size();
    meta.numLevels = levels.size() + 1;
    meta.maxPathSize = maxPathSize;
    CacheHeader hdr{meta};
    wr.Write(hdr);
    wr.WriteContainer(slider.buffer);
    wr.ApplyPadding();
    std::set<size_t> fixups;

    {
      const int64 thisOffset = offsetof(CacheHeader, entries);
      fixups.emplace(thisOffset);
      const int64 entriesOffset = wr.Tell();
      hdr.entries =
          reinterpret_cast<ZipEntryLeaf *>(entriesOffset - thisOffset);
    }

    for (auto &f : totalCache) {
      f.Write(wr, fixups);
    }

    {
      std::vector<size_t> childrenOffsets;

      for (int64 l = levels.size() - 1; l >= 0; l--) {
        std::vector<size_t> newChildrenOffsets;

        for (auto &l : levels.at(l)) {
          auto retVal = l.Write(wr, childrenOffsets, fixups);
          newChildrenOffsets.push_back(retVal);
        }

        std::swap(childrenOffsets, newChildrenOffsets);
      }

      const int64 rootOffset = root.Write(wr, childrenOffsets, fixups);
      const int64 thisOffset = offsetof(CacheHeader, root);
      fixups.emplace(thisOffset);
      hdr.root = reinterpret_cast<HybridLeaf *>(rootOffset - thisOffset);
    }

    hdr.cacheSize = wr.Tell();

    std::stringstream str;
    BinWritterRef wrs(str);
    size_t lastFixup = 0;
    size_t numRleFixups = 0;

    for (auto f : fixups) {
      if (!lastFixup) {
        lastFixup = f;
        continue;
      }

      if (f == lastFixup + 8 * (numRleFixups + 1)) {
        numRleFixups++;
        continue;
      }

      if (numRleFixups) {
        wr.Write<buint128>(lastFixup);
        wr.Write<buint128>(numRleFixups);
        hdr.numRleFixups++;
        numRleFixups = 0;
      } else {
        wrs.Write<buint128>(lastFixup);
        hdr.numSimpleFixups++;
      }

      lastFixup = f;
    }

    if (numRleFixups) {
      wr.Write<buint128>(lastFixup);
      wr.Write<buint128>(numRleFixups);
      hdr.numRleFixups++;
    } else {
      wrs.Write<buint128>(lastFixup);
      hdr.numSimpleFixups++;
    }

    {
      auto buff = str.str();
      es::Dispose(str);
      wr.WriteContainer(buff);
    }

    wr.Push();
    wr.Seek(0);
    wr.Write(hdr);
    wr.Pop();
  }
};

CacheGenerator::~CacheGenerator() = default;
CacheGenerator::CacheGenerator() : pi(std::make_unique<CacheGeneratorImpl>()) {}
void CacheGenerator::AddFile(es::string_view fileName, size_t zipOffset,
                             size_t fileSize) {
  pi->AddFile(fileName, zipOffset, fileSize);
}
void CacheGenerator::Write(BinWritterRef wr) { pi->Write(wr, meta); }
