/*  Spike is universal dedicated module handler
    This source contains code for CLI master app
    Part of PreCore project

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

#include "console.hpp"
#include "context.hpp"
#include "datas/binreader.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/stat.hpp"
#include "datas/tchar.hpp"
#include "out_context.hpp"
#include "project.h"
#include "tmp_storage.hpp"

#ifndef SPIKE_USE_THREADS
#define SPIKE_USE_THREADS 0
#endif

static constexpr bool CATCH_EXCEPTIONS = false;

static const char appHeader0[] =
    "Simply drag'n'drop files/folders onto application or "
    "use as ";
static const char appHeader1[] =
    " [options] path1 path2 ...\nTool can detect and scan folders and "
    "uncompressed zip archives.";

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

struct ProcessedFiles : LoadingBar, CounterLine {
  char buffer[128]{};

  ProcessedFiles() : LoadingBar({buffer, sizeof(buffer)}) {}
  void PrintLine() override {
    snprintf(buffer, sizeof(buffer), "Processed %4" PRIuMAX " files.",
             curitem.load(std::memory_order_relaxed));
    LoadingBar::PrintLine();
  }
};

struct ExtractStats {
  std::map<JenHash, size_t> archiveFiles;
  size_t totalFiles = 0;
};

struct UILines {
  ProgressBar *totalProgress{nullptr};
  CounterLine *totalCount{nullptr};
  std::map<uint32, ProgressBar *> bars;
  std::mutex barsMutex;

  auto ChooseBar() {
    if (bars.empty()) {
      return (ProgressBar *)(nullptr);
    }

    auto threadId = std::this_thread::get_id();
    auto id = reinterpret_cast<const uint32 &>(threadId);
    auto found = bars.find(id);

    if (es::IsEnd(bars, found)) {
      std::lock_guard<std::mutex> lg(barsMutex);
      auto retVal = bars.begin()->second;
      bars.emplace(id, retVal);
      bars.erase(bars.begin());

      return retVal;
    }

    return found->second;
  };

  UILines(const ExtractStats &stats) {
    ModifyElements([&](ElementAPI &api) {
      const size_t minThreads =
          std::min(size_t(std::thread::hardware_concurrency()),
                   stats.archiveFiles.size());

      if (minThreads < 2) {
        return;
      }

      for (size_t t = 0; t < minThreads; t++) {
        auto progBar = std::make_unique<ProgressBar>("Thread:");
        auto progBarRaw = progBar.get();
        bars.emplace(t, progBarRaw);
        api.Append(std::move(progBar));
      }
    });

    auto prog = AppendNewLogLine<DetailedProgressBar>("Total: ");
    prog->ItemCount(stats.totalFiles);
    totalCount = prog;
  }

  UILines(size_t totalInputFiles) {
    totalCount = AppendNewLogLine<ProcessedFiles>();
    auto prog = AppendNewLogLine<DetailedProgressBar>("Total: ");
    prog->ItemCount(totalInputFiles);
    totalProgress = prog;
  }

  ~UILines() {
    ModifyElements([&](ElementAPI &api) {
      if (totalProgress) {
        auto data = static_cast<ProcessedFiles *>(totalCount);
        data->Finish();
        api.Release(data);
      }
      api.Clean();
    });
  }
};

void ScanModules(const std::string &appFolder, const std::string &appName) {
  DirectoryScanner sc;
  sc.AddFilter(std::string_view(".spk$"));
  sc.Scan(appFolder);

  for (auto &m : sc) {
    try {
      AFileInfo modulePath(m);
      auto moduleName = modulePath.GetFilename();
      const size_t firstDotPos = moduleName.find_first_of('.');
      std::string moduleNameStr(moduleName.substr(0, firstDotPos));
      APPContext ctx(moduleNameStr.data(), appFolder, appName);
      ctx.FromConfig();
    } catch (const std::runtime_error &e) {
      printerror(e.what());
    }
  }
}

void GenerateDocumentation(const std::string &appFolder,
                           const std::string &appName,
                           const std::string &templatePath) {
  DirectoryScanner sc;
  sc.AddFilter(std::string_view(".spk$"));
  sc.Scan(appFolder);
  std::set<std::string> modules;
  pugi::xml_document doc;

  if (!templatePath.empty()) {
    doc = XMLFromFile(templatePath);
  }

  for (auto &m : sc) {
    try {
      AFileInfo modulePath(m);
      auto moduleName = modulePath.GetFilename();
      const size_t firstDotPos = moduleName.find_first_of('.');
      std::string moduleNameStr(moduleName.substr(0, firstDotPos));
      modules.emplace(moduleNameStr);
    } catch (const std::runtime_error &e) {
      printerror(e.what());
    }
  }

  BinWritter_t<BinCoreOpenMode::Text> wr(appFolder + "/README.md");
  const char *toolsetName = "[[TOOLSET NAME]]";

  if (auto child = doc.child("toolset_name"); child) {
    toolsetName = child.text().as_string();
  }

  const char *toolsetDescription = "[[TOOLSET DESCRIPTION]]";

  if (auto child = doc.child("toolset_description"); child) {
    toolsetDescription = child.text().as_string();
  }

  wr.BaseStream() << "# " << toolsetName << "\n\n"
                  << toolsetDescription << "\n\n";

  for (auto &m : modules) {
    pugi::xml_node node = doc.child(m.data());
    APPContext ctx(m.data(), appFolder, appName);
    ctx.GetMarkdownDoc(wr.BaseStream(), node);
  }

  if (auto child = doc.child("toolset_footer"); child) {
    wr.BaseStream() << child.text().as_string();
  }
}

#include <deque>

struct WorkerManager {
  using FuncType = std::function<void()>;

  WorkerManager(size_t capacity_);

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
      canProcess.wait(lk, [&] { return done || !queue.empty(); });

      if (!done) [[likely]] {
        retval = std::move(queue.front());
        queue.pop_front();
      }
    }
    hasSpace.notify_one();
    return retval;
  }

  ~WorkerManager() {
    done = true;
    canProcess.notify_all();
    for (auto &w : workers) {
      w.join();
    }
  }

private:
  std::vector<std::thread> workers;
  std::deque<FuncType> queue;
  size_t capacity;
  bool done = false;
  std::mutex mutex;
  std::condition_variable canProcess;
  std::condition_variable hasSpace;
};

struct WorkerThread {
  WorkerManager &manager;
  std::function<void(std::string)> func{};

  void operator()() {
    while (true) {
      auto item = manager.Pop();

      if (!item) [[unlikely]] {
        break;
      }

      try {
        item();
      } catch (const std::exception &e) {
        printerror(e.what());
      }
    }
  }
};

WorkerManager::WorkerManager(size_t capacity_) : capacity(capacity_) {
  const size_t minWorkerCount = std::thread::hardware_concurrency();

  for (size_t i = 0; i < minWorkerCount; i++) {
    workers.emplace_back(WorkerThread{*this});
  }
}

struct Batch {
  APPContext *ctx;
  std::function<void(const std::string &path, AppPackStats)> forEachFolder;
  std::function<void()> forEachFolderFinish;
  std::function<void(AppContextShare *)> forEachFile;

  Batch(APPContext *ctx_) : ctx(ctx_) {
    for (auto &c : ctx->info->filters) {
      scanner.AddFilter(c);
    }
  }

  void AddFile(std::string path) {
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
      ReleaseLogLines(scanBar);

      if (forEachFolder) {
        AppPackStats stats{};
        stats.numFiles = scanner.Files().size();

        for (auto &f : scanner) {
          stats.totalSizeFileNames += f.size() + 1;
        }

        forEachFolder(path, stats);
      }

      for (auto &f : scanner) {
        manager.Push([&, iCtx{MakeIOContext(f)}] { forEachFile(iCtx.get()); });
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
          loadBar->Finish();
          ReleaseLogLines(loadBar);

          if (forEachFolder) {
            auto zipPath = path.substr(0, path.size() - 4);
            AppPackStats stats{};
            auto vfsIter = fctx->Iter();

            for (auto f : vfsIter) {
              auto item = f.AsView();
              if (scanner.IsFiltered(item)) {
                stats.numFiles++;
                stats.totalSizeFileNames += item.size() + 1;
              }
            }

            forEachFolder(std::move(zipPath), stats);
          }

          auto vfsIter = fctx->Iter();

          if (!loadFiltered) {
            for (auto f : vfsIter) {
              auto item = f.AsView();
              if (size_t lastSlash = item.find_last_of("/\\");
                  lastSlash != item.npos) {
                item.remove_prefix(lastSlash + 1);
              }

              if (scanner.IsFiltered(item)) {
                AFileInfo zFile(path);
                manager.Push([&, zInstance = fctx->Instance(f)] {
                  forEachFile(zInstance.get());
                });
              }
            }
          } else {
            for (auto f : vfsIter) {
              AFileInfo zFile(path);
              manager.Push([&, zInstance = fctx->Instance(f)] {
                forEachFile(zInstance.get());
              });
            }
          }
          break;
        } else if (path[found + 4] == '/') {
          auto sub = path.substr(0, found + 4);
          if (rootZips.contains(sub)) {
            break;
          }

          auto foundZip = zips.find(sub);
          auto filterString = "^" + path.substr(found + 5);

          if (es::IsEnd(zips, foundZip)) {
            std::vector<std::string> pVec;
            pVec.push_back(std::move(filterString));
            zips.emplace(std::move(sub), std::move(pVec));
          } else {
            foundZip->second.push_back(std::move(filterString));
          }
          break;
        }
      }

      if (type == FileType_e::File) {
        manager.Push(
            [&, iCtx{MakeIOContext(path)}] { forEachFile(iCtx.get()); });
        break;
      }
      printerror("Invalid path: " << path);
      break;
    }
    }
  }

  void FinishBatch() {
    for (auto &[zip, paths] : zips) {
      // TODO make context and stuff
    }
  }

private:
  std::set<std::string> rootZips;
  std::map<std::string, std::vector<std::string>> zips;
  DirectoryScanner scanner;
  WorkerManager manager{50 * std::thread::hardware_concurrency()};
};

void PackModeBatch(Batch &batch) {
  struct PackData {
    size_t index = 0;
    AppPackContext *archiveContext = nullptr;
    std::string pbarLabel;
    DetailedProgressBar *progBar = nullptr;
    const std::string *folderPath = nullptr;
  };

  auto payload = std::make_shared<PackData>();

  batch.forEachFolder = [payload, ctx = batch.ctx](const std::string &path,
                                                   AppPackStats stats) {
    payload->folderPath = &path;
    payload->archiveContext = ctx->NewArchive(path, stats);
    payload->pbarLabel = "Folder id " + std::to_string(payload->index++);
    payload->progBar = AppendNewLogLine<DetailedProgressBar>(payload->pbarLabel);
    payload->progBar->ItemCount(stats.numFiles);
    ConsolePrintDetail(3);
  };

  batch.forEachFile = [payload](AppContextShare *iCtx) {
    payload->archiveContext->SendFile(iCtx->workingFile.GetFullPath(),
                                      iCtx->GetStream());
    (*payload->progBar)++;
  };

  batch.forEachFolderFinish = [payload] {
    ConsolePrintDetail(1);
    payload->archiveContext->Finish();
    RemoveLogLines(payload->progBar);
  };
}

auto ExtractStatBatch(Batch &batch) {
  struct ExtractStatsMaker : ExtractStats {
    std::mutex mtx;
    LoadingBar *scanBar;

    void Push(AppContextShare *ctx, size_t numFiles) {
      std::unique_lock<std::mutex> lg(mtx);
      archiveFiles.emplace(ctx->Hash(), numFiles);
      totalFiles++;
    }
  };

  auto sharedData = std::make_shared<ExtractStatsMaker>();
  ConsolePrintDetail(3);
  sharedData->scanBar =
      AppendNewLogLine<LoadingBar>("Processing extract stats.");

  batch.forEachFile = [payload = sharedData,
                       ctx = batch.ctx](AppContextShare *iCtx) {
    payload->Push(iCtx, ctx->ExtractStat(std::bind(
                            [&](size_t offset, size_t size) {
                              return iCtx->GetBuffer(size, offset);
                            },
                            std::placeholders::_1, std::placeholders::_2)));
  };

  return sharedData;
}

void ProcessBatch(Batch &batch, ExtractStats *stats) {
  batch.forEachFile = [payload = std::make_shared<UILines>(*stats),
                       archiveFiles =
                           std::make_shared<decltype(stats->archiveFiles)>(
                               std::move(stats->archiveFiles)),
                       ctx = batch.ctx](AppContextShare *iCtx) {
    auto currentBar = payload->ChooseBar();
    if (currentBar) {
      currentBar->ItemCount(archiveFiles->at(iCtx->Hash()));
    }

    iCtx->MountUI(payload->totalCount, currentBar);
    printline("Processing: " << iCtx->workingFile.GetFullPath());
    ctx->ProcessFile(iCtx);
    if (payload->totalProgress) {
      (*payload->totalProgress)++;
    }
  };
}

void ProcessBatch(Batch &batch, size_t numFiles) {
  batch.forEachFile = [payload = std::make_shared<UILines>(numFiles),
                       ctx = batch.ctx](AppContextShare *iCtx) {
    iCtx->MountUI(payload->totalCount, nullptr);
    printline("Processing: " << iCtx->workingFile.GetFullPath());
    ctx->ProcessFile(iCtx);
    if (payload->totalProgress) {
      (*payload->totalProgress)++;
    }
  };
}

int Main(int argc, TCHAR *argv[]) {
  ConsolePrintDetail(1);
  AFileInfo appLocation(std::to_string(*argv));
  std::string appFolder(appLocation.GetFolder());
  std::string appName(appLocation.GetFilename());

  if (argc < 2) {
    printwarning(
        "No parameters provided, entering scan mode and generating config.");
    ScanModules(appFolder, appName);
    return 0;
  }

  auto moduleName = std::to_string(argv[1]);

  if (moduleName == "--make-doc") {
    std::string templatePath;

    if (argc < 3) {
      printwarning("Expexted template path!");
    } else {
      templatePath = std::to_string(argv[2]);
    }

    GenerateDocumentation(appFolder, appName, templatePath);
    return 0;
  }

  if (argc < 3) {
    printerror("Insufficient argument count, expected parameters.");
    return 1;
  }

  APPContext ctx;

  try {
    ctx = APPContext(moduleName.data(), appFolder, appName);
  } catch (const std::exception &e) {
    printerror(e.what());
    return 2;
  }

  ConsolePrintDetail(0);

  printline(ctx.info->header);
  printline(appHeader0 << appLocation.GetFilename() << ' ' << moduleName
                       << appHeader1);

  if (IsHelp(argv[2])) {
    ctx.PrintCLIHelp();
    return 0;
  }

  ConsolePrintDetail(1);
  bool dontLoadConfig = false;
  std::vector<bool> markedFiles(size_t(argc), false);
  size_t totalFiles = 0;

  // Handle cli options and switches
  for (int a = 2; a < argc; a++) {
    auto opt = argv[a];

    if (opt[0] == '-') {
      // We won't use config file, reset all booleans to false,
      // so we can properly use cli switches
      [&] {
        if (dontLoadConfig) {
          return;
        }

        printinfo("CLI option detected, config won't be loaded, all booleans "
                  "set to false!");
        ctx.ResetSwitchSettings();
      }();

      dontLoadConfig = true;
      opt++;

      if (opt[0] == '-') {
        opt++;
      }

      auto optStr = std::to_string(opt);
      auto valStr = std::to_string(argv[a + 1]);

      if (auto retVal = ctx.ApplySetting(optStr, valStr); retVal > 0) {
        a++;
      }

    } else {
      markedFiles[a] = true;
      totalFiles++;
    }
  }

  if (!dontLoadConfig) {
    printinfo("Loading config: " << appName << ".config");
    ctx.FromConfig();
  }

  InitTempStorage();
  ctx.SetupModule();

  Batch batch(&ctx);

  if (ctx.NewArchive) {
    PackModeBatch(batch);
  } else {
    if (ctx.ExtractStat) {
      auto stats = ExtractStatBatch(batch);
      for (int a = 2; a < argc; a++) {
        if (!markedFiles.at(a)) {
          continue;
        }
        batch.AddFile(std::to_string(argv[a]));
      }

      batch.FinishBatch();
      ProcessBatch(batch, stats.get());
    } else {
      ProcessBatch(batch, totalFiles);
    }
  }

  for (int a = 2; a < argc; a++) {
    if (!markedFiles.at(a)) {
      continue;
    }
    batch.AddFile(std::to_string(argv[a]));
  }

  batch.FinishBatch();

  if (ctx.FinishContext) {
    ctx.FinishContext();
  }

  return 0;
}

int _tmain(int argc, TCHAR *argv[]) {
  es::SetupWinApiConsole();
  InitConsole();
  CleanTempStorages();

  int retVal = Main(argc, argv);

  CleanCurrentTempStorage();
  TerminateConsole();
  return retVal;
}
