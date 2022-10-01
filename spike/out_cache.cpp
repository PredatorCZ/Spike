/*  Cache format generator for seekless zip loading
    Part of PreCore's Spike project

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

#include "cache.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/fileinfo.hpp"
#include <algorithm>
#include <functional>
#include <future>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct StringSlider {
  std::string buffer;

private:
  using future_type = std::future<int64>;
  std::vector<future_type> workingThreads;

public:
  StringSlider() { workingThreads.resize(std::thread::hardware_concurrency()); }

  std::string::iterator FindString(std::string_view str) {
    if (str.size() > buffer.size()) [[unlikely]] {
      if (std::string_view(str).starts_with(buffer)) [[unlikely]] {
        buffer = str;
        return buffer.begin();
      } else {
        return buffer.end();
      }
    }
    auto searcher = std::boyer_moore_horspool_searcher(str.begin(), str.end());

    if (size_t bSize = buffer.size(); bSize > 1'000'000) {
      const size_t numThreads = workingThreads.size();
      size_t splitPoint = bSize / numThreads;
      size_t chunkBegin = 0;

      for (size_t i = 0; i < numThreads; ++i) {
        workingThreads[i] =
            std::async(std::launch::async, [=, &buffer = buffer] {
              std::string_view item;
              if (i + 1 == numThreads) {
                item = std::string_view(buffer.data() + chunkBegin,
                                        buffer.end().operator->());
              } else {
                item = std::string_view(buffer.data() + chunkBegin,
                                        splitPoint + str.size());
              }

              auto found = std::search(item.begin(), item.end(), searcher);
              int64 offset = -1;

              if (found != item.end()) {
                offset = std::distance(const_cast<const char *>(buffer.data()),
                                       &*found);
              }

              return offset;
            });
        chunkBegin += splitPoint;
      }

      int64 offset = -1;

      for (auto &wt : workingThreads) {
        wt.wait();
        if (int64 off = wt.get(); off >= 0 && offset < 0) {
          offset = off;
        }
      }

      if (offset >= 0) {
        return std::next(buffer.begin(), offset);
      }

      return buffer.end();
    } else {
      return std::search(buffer.begin(), buffer.end(), searcher);
    }
  }

  size_t InsertString(std::string_view str) {
    auto found = FindString(str);

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
    return std::string_view(base->buffer.data() + offset, size) <
           std::string_view(other.base->buffer.data() + other.offset,
                            other.size);
  }
};

static constexpr size_t STRING_OFFSET = sizeof(CacheBaseHeader) + 12;
static constexpr size_t ENTRIES_OFFSET = sizeof(CacheBaseHeader) + 8;
static constexpr size_t ROOT_OFFSET = sizeof(CacheBaseHeader) + 4;
static constexpr size_t HYBRIDLEAF_PARENTPTR = 4;
static constexpr size_t FINAL_PARENTPTR = 16;

struct FinalEntry : SliderString {
  size_t zipOffset;
  size_t zipSize;
  size_t totalFileNameSize;
  mutable size_t wrOffset = 0;

  void Write(BinWritterRef wr) const {
    wrOffset = wr.Tell();
    wr.Write(zipOffset);
    wr.Write(zipSize);
    wr.Write<uint32>(0);
    wr.Write<uint16>(size);
    wr.Write<uint16>(totalFileNameSize);

    if (size < 9) {
      wr.WriteBuffer(base->buffer.data() + offset, size);
      wr.ApplyPadding(8);
    } else {
      const int32 stringOffset = STRING_OFFSET + offset;
      const int32 thisOffset = wr.Tell();
      wr.Write(stringOffset - thisOffset);
      wr.Write<uint32>(0);
    }
  }
};

struct HybridLeafGen {
  std::map<SliderString, const FinalEntry *> finals;
  std::map<SliderString, size_t> childrenIds;
  SliderString partName;

  size_t Write(BinWritterRef wr, const std::vector<size_t> &childrenOffsets) {
    wr.ApplyPadding(4);
    for (auto &[_, id] : childrenIds) {
      const int32 childOffset = childrenOffsets.at(id);
      const int32 thisOffset = wr.Tell();
      wr.Write((childOffset - thisOffset) / 4);
    }

    const size_t retVal = wr.Tell() - HYBRIDLEAF_PARENTPTR;

    // fixup parent offset for children
    wr.Push();
    for (auto &[_, id] : childrenIds) {
      const size_t childOffset = childrenOffsets.at(id);
      wr.Seek(childOffset + HYBRIDLEAF_PARENTPTR);
      const int32 thisOffset = retVal;
      const int32 memberOffset = wr.Tell();
      wr.Write((thisOffset - memberOffset) / 4);
    }
    wr.Pop();

    wr.Write<uint32>(0);
    if (!partName.size) {
      wr.Write<uint32>(0);
    } else if (partName.size < 5) {
      wr.WriteBuffer(partName.base->buffer.data() + partName.offset,
                     partName.size);
      wr.ApplyPadding(4);
    } else {
      const int32 pathPartOffset_ = STRING_OFFSET + partName.offset;
      const int32 thisOffset = wr.Tell();
      wr.Write(pathPartOffset_ - thisOffset);
    }

    wr.Write<uint16>(childrenIds.size());
    wr.Write<uint16>(partName.size);
    wr.Write<uint32>(finals.size());

    for (auto &[_, entry] : finals) {
      const int32 finalOffset = entry->wrOffset;
      const int32 thisOffset = wr.Tell();
      wr.Write((finalOffset - thisOffset) / 4);
    }

    wr.Push();
    for (auto &[_, entry] : finals) {
      const size_t finalOffset = entry->wrOffset;
      wr.Seek(finalOffset + FINAL_PARENTPTR);
      const int32 thisOffset = retVal;
      const int32 memberOffset = wr.Tell();
      wr.Write((thisOffset - memberOffset) / 4);
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

  void AddFile(std::string_view fileName, size_t offset, size_t size) {
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
        auto &sliderRef = p.size() > 4 ? slider : sliderTiny;
        const size_t position = sliderRef.InsertString(p);
        parts.emplace_back(SliderString{position, p.size(), &sliderRef});
      }

      finalKey.size = finalPart.size();

      if (finalPart.size() < 9) {
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
      FinalEntry entry{finalKey, offset, size, fileName.size()};
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

  void Write(BinWritterRef wr, CacheBaseHeader &hdr) {
    hdr.numFiles = totalCache.size();
    hdr.numLevels = levels.size() + 1;
    hdr.maxPathSize = maxPathSize;
    wr.Write(hdr);
    wr.Skip(12);
    wr.WriteContainer(slider.buffer);
    wr.ApplyPadding();

    const int32 entriesOffset = (wr.Tell() - ENTRIES_OFFSET) / 4;
    int32 rootOffset;

    for (auto &f : totalCache) {
      f.Write(wr);
    }

    {
      std::vector<size_t> childrenOffsets;

      for (int64 l = levels.size() - 1; l >= 0; l--) {
        std::vector<size_t> newChildrenOffsets;

        for (auto &l : levels.at(l)) {
          auto retVal = l.Write(wr, childrenOffsets);
          newChildrenOffsets.push_back(retVal);
        }

        std::swap(childrenOffsets, newChildrenOffsets);
      }

      const int32 rootOffset_ = root.Write(wr, childrenOffsets);
      rootOffset = (rootOffset_ - ROOT_OFFSET) / 4;
    }

    const uint32 cacheSize = wr.Tell();

    wr.Push();
    wr.Seek(0);
    wr.Write(hdr);
    wr.Write(cacheSize);
    wr.Write(rootOffset);
    wr.Write(entriesOffset);
    wr.Pop();
  }
};

CacheGenerator::~CacheGenerator() = default;
CacheGenerator::CacheGenerator() : pi(std::make_unique<CacheGeneratorImpl>()) {}
void CacheGenerator::AddFile(std::string_view fileName, size_t zipOffset,
                             size_t fileSize) {
  pi->AddFile(fileName, zipOffset, fileSize);
}
void CacheGenerator::Write(BinWritterRef wr) { pi->Write(wr, meta); }
