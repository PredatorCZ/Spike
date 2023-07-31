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
#include "console.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/fileinfo.hpp"
#include "tmp_storage.hpp"
#include <algorithm>
#include <barrier>
#include <cinttypes>
#include <fstream>
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
  std::vector<std::thread> workingThreads;
  std::map<std::thread::id, int64> threadResults;
  struct SearchArgs {
    size_t chunkBegin;
    size_t splitPoint;
    std::string_view str;
    std::boyer_moore_horspool_searcher<std::string_view::iterator> *searcher;
    bool endThread;
  };
  std::map<std::thread::id, SearchArgs> threadArgs;

  // All threads finished searching, notify main thread and resume ready state
  struct {
    std::map<std::thread::id, int64> *threadResults;
    std::atomic_bool *finishedSearch;
    int64 *resultOffset;
    void operator()() noexcept {
      // PrintInfo("Processing worker data");
      *resultOffset = -1;
      for (auto [_, off] : *threadResults) {
        if (off >= 0) {
          *resultOffset = off;
          break;
        }
      }

      // PrintInfo("Search end");

      *finishedSearch = true;
      finishedSearch->notify_all();
    }
  } workersDone;

  std::barrier<decltype(workersDone)> workerSync;

  // Call when threads are in ready state and notify main thread
  struct {
    std::atomic_bool *runWorkers;
    void operator()() noexcept {
      runWorkers->wait(false);
      *runWorkers = false;
      runWorkers->notify_all();
    }
  } workersReadyCb;
  std::barrier<decltype(workersReadyCb)> workersReady;

  std::atomic_bool stopWorkers;
  std::atomic_bool finishedSearch;
  std::atomic_bool runWorkers;

  int64 resultOffset = -1;

  std::atomic_bool &allowThreads;

public:
  StringSlider(std::atomic_bool &allowThreads)
      : workersDone{&threadResults, &finishedSearch, &resultOffset},
        workerSync(std::thread::hardware_concurrency(), workersDone),
        workersReadyCb{&runWorkers},
        workersReady(std::thread::hardware_concurrency(), workersReadyCb),
        allowThreads(allowThreads) {
    const size_t numThreads = std::thread::hardware_concurrency();

    auto SearcherThread = [this, numThreads] {
      while (true) {
        // PrintInfo("Thread wait");
        workersReady.arrive_and_wait();

        // PrintInfo("Thread begin");

        if (stopWorkers) {
          return;
        }

        try {
          SearchArgs &args = threadArgs.at(std::this_thread::get_id());

          std::string_view item;
          if (args.endThread) {
            item = std::string_view(buffer.data() + args.chunkBegin,
                                    buffer.end().operator->());
          } else {
            item = std::string_view(buffer.data() + args.chunkBegin,
                                    args.splitPoint + args.str.size());
          }

          auto found = std::search(item.begin(), item.end(), *args.searcher);
          int64 offset = -1;

          if (found != item.end()) {
            offset =
                std::distance(const_cast<const char *>(buffer.data()), &*found);
          }

          threadResults.at(std::this_thread::get_id()) = offset;
        } catch (...) {
          std::terminate();
        }

        // PrintInfo("Thread end");
        workerSync.arrive_and_wait();
        // PrintInfo("Thread end wait");
      }
    };

    workingThreads.resize(numThreads);
    for (size_t i = 0; i < numThreads; i++) {
      workingThreads.at(i) = std::thread(SearcherThread);
      auto &curThread = workingThreads.at(i);
      threadArgs.emplace(curThread.get_id(), SearchArgs{});
      threadResults.emplace(curThread.get_id(), -1);
      pthread_setname_np(curThread.native_handle(), "cache_srch_wrkr");
    }
  }

  ~StringSlider() {
    stopWorkers = true;
    runWorkers = true;
    runWorkers.notify_all();

    for (auto &w : workingThreads) {
      if (w.joinable()) {
        w.join();
      }
    }
  }

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

    if (size_t bSize = buffer.size(); allowThreads && bSize > 1'000'000) {
      const size_t numThreads = workingThreads.size();
      size_t splitPoint = bSize / numThreads;
      size_t chunkBegin = 0;

      for (size_t i = 0; i < numThreads; i++) {
        threadArgs.at(workingThreads.at(i).get_id()) = SearchArgs{
            .chunkBegin = chunkBegin,
            .splitPoint = splitPoint,
            .str = str,
            .searcher = &searcher,
            .endThread = i + 1 == numThreads,
        };
        chunkBegin += splitPoint;
      }

      // PrintInfo("Notifying workers");
      finishedSearch = false;
      runWorkers.wait(true);
      runWorkers = true;
      runWorkers.notify_all();
      // PrintInfo("Waiting for workers");
      finishedSearch.wait(false);
      // PrintInfo("Search done");

      if (resultOffset >= 0) {
        return std::next(buffer.begin(), resultOffset);
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
  std::atomic_bool allowThreads;
  StringSlider slider{allowThreads};
  StringSlider sliderTiny{allowThreads};
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

  void Write(BinWritterRef wr, CacheBaseHeader &hdr,
             DetailedProgressBar *progress) {
    hdr.numFiles = totalCache.size();
    hdr.numLevels = levels.size() + 1;
    hdr.maxPathSize = maxPathSize;
    wr.Write(hdr);
    wr.Skip(12);
    wr.WriteContainer(slider.buffer);
    wr.ApplyPadding();

    if (progress) {
      progress->ItemCount(totalCache.size() + levels.size());
    }

    const int32 entriesOffset = (wr.Tell() - ENTRIES_OFFSET) / 4;
    int32 rootOffset;

    for (auto &f : totalCache) {
      f.Write(wr);
      if (progress) {
        (*progress)++;
      }
    }

    {
      std::vector<size_t> childrenOffsets;

      for (int64 l = levels.size() - 1; l >= 0; l--) {
        if (progress) {
          (*progress)++;
        }
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

struct WALThread {
  std::string walFile;
  std::ofstream walStreamIn;
  std::ifstream walStreamOut;
  std::atomic_size_t sharedCounter;
  std::atomic_bool isDone;
  std::atomic<CounterLine *> totalCount{nullptr};
  CounterLine *totalCountOwned{nullptr};
  CacheGeneratorImpl generator;
  std::promise<void> state;
  std::future<void> exception;

  WALThread()
      : walFile(RequestTempFile()), walStreamIn(walFile), walStreamOut(walFile),
        exception(state.get_future()) {
    if (walStreamIn.fail()) {
      throw std::runtime_error("Failed to create wal file.");
    }
    if (walStreamOut.fail()) {
      throw std::runtime_error("Failed to open wal file.");
    }
  }

  WALThread(WALThread &&) = delete;
  WALThread(const WALThread &) = delete;

  void Loop() {
    size_t lastOffset = 0;

    while (!isDone || sharedCounter > 0) {
      if (sharedCounter == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        continue;
      }

      if (!totalCountOwned && totalCount) {
        totalCountOwned = totalCount;
      }

      std::string curData;
      if (std::getline(walStreamOut, curData).eof()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        walStreamOut.clear();
        walStreamOut.seekg(lastOffset);
        continue;
      }

      lastOffset = walStreamOut.tellg();

      char path[0x1000]{};
      size_t zipOffset;
      size_t fileSize;
      std::sscanf(curData.c_str(), "%[^;];%zx;%zx", path, &zipOffset,
                  &fileSize);
      sharedCounter--;
      generator.AddFile(path, zipOffset, fileSize);

      if (totalCountOwned) {
        (*totalCountOwned)++;
      }

      if (auto curException = std::current_exception(); curException) {
        state.set_exception(curException);
        std::terminate();
      }
    }

    state.set_value();
  }
};

CacheGenerator::CacheGenerator()
    : workThread(std::make_unique<WALThread>()),
      walThread([&] { workThread->Loop(); }) {
  pthread_setname_np(walThread.native_handle(), "cache_wal");
}

CacheGenerator::~CacheGenerator() = default;

void CacheGenerator::AddFile(std::string_view fileName, size_t zipOffset,
                             size_t fileSize) {
  workThread->walStreamIn << fileName << ';' << std::hex << zipOffset << ';'
                          << fileSize << '\n';
  if (workThread->walStreamIn.fail()) {
    throw std::runtime_error("Failed to add file to WAL stream");
  }
  workThread->sharedCounter++;
}

void CacheGenerator::WaitAndWrite(BinWritterRef wr) {
  workThread->isDone = true;
  workThread->generator.allowThreads = true;
  es::Dispose(workThread->walStreamIn);
  DetailedProgressBar *prog = nullptr;

  if (size_t count = workThread->sharedCounter; count > 10) {
    prog = AppendNewLogLine<DetailedProgressBar>("Cache: ");
    prog->ItemCount(count);
    workThread->totalCount = prog;
  }

  if (workThread->exception.valid()) {
    workThread->exception.get();
  }

  if (walThread.joinable()) {
    walThread.join();
  }

  es::Dispose(workThread->walStreamOut);
  std::remove(workThread->walFile.c_str());

  workThread->generator.Write(wr, meta, prog);
}
