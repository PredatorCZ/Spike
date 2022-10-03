#include "batch.hpp"
#include "console.hpp"
#include "datas/master_printer.hpp"
#include "datas/stat.hpp"
#include <cinttypes>
#include <deque>
#include <future>

static constexpr bool CATCH_EXCEPTIONS = false;

struct WorkerThread {
  MultiThreadManagerImpl &manager;
  std::promise<void> state;
  std::function<void(std::string)> func{};
  void operator()();
};

struct MultiThreadManagerImpl {
  using FuncType = MultiThreadManager::FuncType;

  MultiThreadManagerImpl(size_t capacityPerThread)
      : capacity(capacityPerThread * std::thread::hardware_concurrency()) {
    const size_t minWorkerCount = std::thread::hardware_concurrency();

    for (size_t i = 0; i < minWorkerCount; i++) {
      std::promise<void> promise;
      states.emplace_back(promise.get_future());
      workers.emplace_back(WorkerThread{*this, std::move(promise)});
    }
  }

  void Push(FuncType item) {
    {
      std::unique_lock<std::mutex> lk(mutex);
      hasSpace.wait(lk, [&] { return queue.size() < capacity; });
      queue.push_back(std::move(item));
    }
    canProcess.notify_one();
  }

  FuncType Pop() {
    FuncType retval;
    {
      std::unique_lock<std::mutex> lk(mutex);
      canProcess.wait(
          lk, [&] { return !queue.empty() || (done && queue.empty()); });

      if (!queue.empty()) [[likely]] {
        retval = std::move(queue.front());
        queue.pop_front();
      }
    }
    hasSpace.notify_one();
    return retval;
  }

  ~MultiThreadManagerImpl() {
    done = true;
    canProcess.notify_all();
    for (auto &future : states) {
      if (future.valid()) {
        future.get();
      }
    }
    for (auto &w : workers) {
      if (w.joinable()) {
        w.join();
      }
    }
  }

  std::vector<std::thread> workers;
  std::vector<std::future<void>> states;
  std::deque<FuncType> queue;
  size_t capacity;
  bool done = false;
  std::mutex mutex;
  std::condition_variable canProcess;
  std::condition_variable hasSpace;
};

void WorkerThread::operator()() {
  while (true) {
    auto item = manager.Pop();

    if (!item) [[unlikely]] {
      break;
    }

    try {
      item();
    } catch (const std::exception &e) {
      if constexpr (CATCH_EXCEPTIONS) {
        state.set_exception(std::current_exception());
        return;
      } else {
        printerror(e.what());
      }
    } catch (...) {
      if constexpr (CATCH_EXCEPTIONS) {
        state.set_exception(std::current_exception());
        return;
      } else {
        printerror("Uncaught exception");
      }
    }
  }

  state.set_value();
}

MultiThreadManager::MultiThreadManager(size_t capacity_)
    : pi(std::make_unique<MultiThreadManagerImpl>(capacity_)) {}

MultiThreadManager::~MultiThreadManager() = default;

void MultiThreadManager::Push(FuncType item) { pi->Push(std::move(item)); }

void SimpleManager::Push(SimpleManager::FuncType item) {
  try {
    item();
  } catch (const std::exception &e) {
    if constexpr (CATCH_EXCEPTIONS) {
      throw;
    } else {
      printerror(e.what());
    }
  }
}

struct ScanningFoldersBar : LoadingBar {
  char buffer[512]{};
  size_t modifyPos = 0;

  ScanningFoldersBar(std::string_view folder)
      : LoadingBar({buffer, sizeof(buffer)}) {
    static constexpr std::string_view part1("Scanning folder: ");
    strncpy(buffer, part1.data(), part1.size());
    modifyPos = part1.size();
    strncpy(buffer + modifyPos, folder.data(), folder.size());
    modifyPos += folder.size();
  }

  void Update(size_t numFolders, size_t numFiles, size_t foundFiles) {
    snprintf(buffer + modifyPos, sizeof(buffer) - modifyPos,
             " %4" PRIuMAX " folders, %4" PRIuMAX " files, %4" PRIuMAX
             " found.",
             numFolders, numFiles, foundFiles);
  }
};

void Batch::AddFile(std::string path) {
  auto type = FileType(path);
  switch (type) {
  case FileType_e::Directory: {
    auto scanBar = AppendNewLogLine<ScanningFoldersBar>(path);
    scanner.scanCbData = scanBar;
    scanner.scanCb = [](void *data, size_t numFolders, size_t numFiles,
                        size_t foundFiles) {
      auto barData = static_cast<ScanningFoldersBar *>(data);
      barData->Update(numFolders, numFiles, foundFiles);
    };
    scanner.Scan(path);
    scanBar->Finish();
    if (keepFinishLines) {
      ReleaseLogLines(scanBar);
    } else {
      RemoveLogLines(scanBar);
    }

    if (forEachFolder) {
      AppPackStats stats{};
      stats.numFiles = scanner.Files().size();

      for (auto &f : scanner) {
        stats.totalSizeFileNames += f.size() + 1;
      }

      forEachFolder(path, stats);
    }

    for (auto &f : scanner) {
      manager.Push([&, iCtx{MakeIOContext(f)}] {
        forEachFile(iCtx.get());
        iCtx->Finish();
      });
    }

    if (forEachFolderFinish) {
      forEachFolderFinish();
    }

    break;
  }

  default: {
    const size_t found = path.find(".zip");
    if (found != path.npos) {
      if (found + 4 == path.size()) {
        if (rootZips.contains(path)) {
          break;
        }

        if (zips.contains(path)) {
          zips.erase(path);
        }
        rootZips.emplace(path);

        auto labelData = "Loading ZIP vfs: " + path;
        auto loadBar = AppendNewLogLine<LoadingBar>(labelData);
        const bool loadFiltered = ctx->info->filteredLoad;
        auto fctx = loadFiltered ? MakeZIPContext(path, scanner, {})
                                 : MakeZIPContext(path);

        AFileInfo zFile(path);
        fctx->basePath = zFile.GetFullPathNoExt();

        loadBar->Finish();
        if (keepFinishLines) {
          ReleaseLogLines(loadBar);
        } else {
          RemoveLogLines(loadBar);
        }

        auto Iterate = [&](auto &&what) {
          auto vfsIter = fctx->Iter();

          if (loadFiltered) {
            for (auto f : vfsIter) {
              auto item = f.AsView();
              if (size_t lastSlash = item.find_last_of("/\\");
                  lastSlash != item.npos) {
                item.remove_prefix(lastSlash + 1);
              }

              if (scanner.IsFiltered(item)) {
                what(f);
              }
            }
          } else {
            for (auto f : vfsIter) {
              what(f);
            }
          }
        };

        if (forEachFolder) {
          auto zipPath = path.substr(0, path.size() - 4);
          AppPackStats stats{};

          Iterate([&](auto &f) {
            stats.numFiles++;
            stats.totalSizeFileNames += f.AsView().size() + 1;
          });

          forEachFolder(std::move(zipPath), stats);
        }

        Iterate([&](auto &f) {
          manager.Push([&, zInstance = fctx->Instance(f)] {
            forEachFile(zInstance.get());
            zInstance->Finish();
          });
        });

        fctx->Finish();
        break;
      } else if (path[found + 4] == '/') {
        auto sub = path.substr(0, found + 4);
        if (rootZips.contains(sub)) {
          break;
        }

        auto foundZip = zips.find(sub);
        auto filterString = "^" + path.substr(found + 5);

        if (es::IsEnd(zips, foundZip)) {
          PathFilter pVec;
          pVec.AddFilter(filterString);
          zips.emplace(std::move(sub), std::move(pVec));
        } else {
          foundZip->second.AddFilter(filterString);
        }
        break;
      }
    }

    if (type == FileType_e::File) {
      manager.Push([&, iCtx{MakeIOContext(path)}] {
        forEachFile(iCtx.get());
        iCtx->Finish();
      });
      break;
    }
    printerror("Invalid path: " << path);
    break;
  }
  }
}

void Batch::FinishBatch() {
  for (auto &[zip, paths] : zips) {
    auto labelData = "Loading ZIP vfs: " + zip;
    auto loadBar = AppendNewLogLine<LoadingBar>(labelData);
    const bool loadFiltered = ctx->info->filteredLoad;
    auto fctx = loadFiltered ? MakeZIPContext(zip, scanner, paths)
                             : MakeZIPContext(zip);

    AFileInfo zFile(zip);
    fctx->basePath = zFile.GetFullPathNoExt();

    loadBar->Finish();
    if (keepFinishLines) {
      ReleaseLogLines(loadBar);
    } else {
      RemoveLogLines(loadBar);
    }

    auto Iterate = [&, &paths = paths](auto &&what) {
      auto vfsIter = fctx->Iter();

      if (loadFiltered) {
        for (auto f : vfsIter) {
          auto item = f.AsView();
          if (size_t lastSlash = item.find_last_of("/\\");
              lastSlash != item.npos) {
            item.remove_prefix(lastSlash + 1);
          }

          if (scanner.IsFiltered(item) && paths.IsFiltered(f.AsView())) {
            what(f);
          }
        }
      } else {
        for (auto f : vfsIter) {
          what(f);
        }
      }
    };

    if (forEachFolder) {
      auto zipPath = zip.substr(0, zip.size() - 4);
      AppPackStats stats{};

      Iterate([&](auto &f) {
        stats.numFiles++;
        stats.totalSizeFileNames += f.AsView().size() + 1;
      });

      forEachFolder(std::move(zipPath), stats);
    }

    Iterate([&](auto &f) {
      manager.Push([&, zInstance = fctx->Instance(f)] {
        forEachFile(zInstance.get());
        zInstance->Finish();
      });
    });

    fctx->Finish();
  }
}
