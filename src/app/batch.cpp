#include "spike/app/batch.hpp"
#include "nlohmann/json.hpp"
#include "spike/app/console.hpp"
#include "spike/io/binreader.hpp"
#include "spike/io/stat.hpp"
#include "spike/master_printer.hpp"
#include <cinttypes>
#include <deque>
#include <future>

#ifdef NDEBUG
static constexpr bool CATCH_EXCEPTIONS = false;
#else
static constexpr bool CATCH_EXCEPTIONS = true;
#endif

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
      pthread_setname_np(workers.back().native_handle(), "batch_worker");
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

  void Wait() {
    if (queue.empty() && workingWorkers == 0) {
      return;
    }
    std::mutex mutex_;
    std::unique_lock<std::mutex> lk(mutex_);
    finsihedBatch.wait(lk,
                       [&] { return queue.empty() && workingWorkers == 0; });
  }

  FuncType Pop() {
    FuncType retval;
    {
      std::unique_lock<std::mutex> lk(mutex);
      canProcess.wait(
          lk, [&] { return !queue.empty() || (done && queue.empty()); });

      if (!queue.empty()) [[likely]] {
        workingWorkers++;
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
  std::atomic_int32_t workingWorkers;
  size_t capacity;
  bool done = false;
  std::mutex mutex;
  std::condition_variable canProcess;
  std::condition_variable hasSpace;
  std::condition_variable finsihedBatch;
};

void WorkerThread::operator()() {
  while (true) {
    {
      auto item = manager.Pop();

      if (!item) [[unlikely]] {
        manager.workingWorkers--;
        manager.finsihedBatch.notify_all();
        break;
      }

      if constexpr (CATCH_EXCEPTIONS) {
        item();

        if (auto curException = std::current_exception(); curException) {
          state.set_exception(curException);
        }
      } else {
        try {
          item();
        } catch (const std::exception &e) {
          PrintError(e.what());
        } catch (...) {
          PrintError("Uncaught exception");
        }
      }
    }

    manager.workingWorkers--;
    manager.finsihedBatch.notify_all();
  }

  state.set_value();
}

MultiThreadManager::MultiThreadManager(size_t capacity_)
    : pi(std::make_unique<MultiThreadManagerImpl>(capacity_)) {}

MultiThreadManager::~MultiThreadManager() = default;

void MultiThreadManager::Push(FuncType item) { pi->Push(std::move(item)); }

void MultiThreadManager::Wait() { pi->Wait(); }

void SimpleManager::Push(SimpleManager::FuncType item) {
  if constexpr (CATCH_EXCEPTIONS) {
    item();
  } else {
    try {
      item();
    } catch (const std::exception &e) {
      PrintError(e.what());
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

Batch::Batch(APPContext *ctx_, size_t queueCapacity)
    : ctx(ctx_), manager(queueCapacity) {
  if (ctx->info->batchControlFilters.empty()) {
    for (auto &c : ctx->info->filters) {
      scanner.AddFilter(c);
      loaderFilter.AddFilter(c);
    }
  } else {
    scanner.AddFilter(std::string_view("batch.json$"));

    for (auto &c : ctx->info->batchControlFilters) {
      loaderFilter.AddFilter(c);
      batchControlFilter.AddFilter(c);
    }

    for (auto &c : ctx->info->filters) {
      loaderFilter.AddFilter(c);
      supplementalFilter.AddFilter(c);
    }
  }
}

void Batch::AddBatch(nlohmann::json &batch, const std::string &batchPath) {
  for (auto &group : batch) {
    std::vector<std::string> supplementals;
    std::string controlPath;

    for (std::string item : group) {
      if (batchControlFilter.IsFiltered(item)) {
        if (!controlPath.empty()) {
          PrintError("Dupicate main file in batch group: ", item);
          continue;
        }

        controlPath = batchPath + item;
        continue;
      }

      supplementals.emplace_back(batchPath + item);
    }

    if (controlPath.empty()) {
      PrintError("Main file not provided for batch group");
      continue;
    }

    manager.Push(
        [&, iCtx{MakeIOContext(controlPath, std::move(supplementals))}] {
          forEachFile(iCtx.get());
          iCtx->Finish();
        });
  }
}

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
    scanner.Clear();
    scanner.Scan(path);
    scanBar->Finish();
    if (keepFinishLines) {
      ReleaseLogLines(scanBar);
    } else {
      RemoveLogLines(scanBar);
    }

    if (updateFileCount && scanner.Files().size()) {
      updateFileCount(scanner.Files().size() - 1);
    }

    if (forEachFolder) {
      forEachFolder(path, scanner.Files().size());
    }

    for (auto &f : scanner) {
      manager.Push([&, iCtx{MakeIOContext(f)}] {
        forEachFile(iCtx.get());
        iCtx->Finish();
      });
    }

    manager.Wait();

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
        auto fctx = loadFiltered ? MakeZIPContext(path, loaderFilter, {})
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
        };

        if (forEachFolder) {
          auto zipPath = path.substr(0, path.size() - 4);
          size_t numFiles = 0;

          Iterate([&](auto &) { numFiles++; });

          forEachFolder(zipPath, numFiles);
        }

        Iterate([&](auto &f) {
          manager.Push([&, zInstance(fctx->Instance(f))] {
            forEachFile(zInstance.get());
            zInstance->Finish();
          });
        });

        manager.Wait();

        if (forEachFolderFinish) {
          forEachFolderFinish();
        }

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
      if (!ctx->info->batchControlFilters.empty()) {
        if (!path.ends_with("batch.json")) {
          PrintError("Expected json bach, got: ", path);
          break;
        }

        BinReader mainFile(path);
        std::string pathDir(AFileInfo(path).GetFolder());
        nlohmann::json batch(nlohmann::json::parse(mainFile.BaseStream()));

        if (updateFileCount) {
          updateFileCount(batch.size());
        }

        AddBatch(batch, pathDir);
      } else {
        manager.Push([&, iCtx{MakeIOContext(path)}] {
          forEachFile(iCtx.get());
          iCtx->Finish();
        });
      }
      break;
    }
    PrintError("Invalid path: ", path);
    break;
  }
  }
}

void Batch::FinishBatch() {
  for (auto &[zip, paths] : zips) {
    auto labelData = "Loading ZIP vfs: " + zip;
    auto loadBar = AppendNewLogLine<LoadingBar>(labelData);
    const bool loadFiltered = ctx->info->filteredLoad;
    auto fctx = loadFiltered ? MakeZIPContext(zip, loaderFilter, paths)
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
    };

    if (forEachFolder) {
      auto zipPath = zip.substr(0, zip.size() - 4);
      size_t numFiles = 0;

      Iterate([&](auto &) { numFiles++; });

      forEachFolder(std::move(zipPath), numFiles);
    }

    Iterate([&](auto &f) {
      manager.Push([&, zInstance(fctx->Instance(f))] {
        forEachFile(zInstance.get());
        zInstance->Finish();
      });
    });

    manager.Wait();

    if (forEachFolderFinish) {
      forEachFolderFinish();
    }

    fctx->Finish();
  }
}
