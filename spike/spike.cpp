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

#include "batch.hpp"
#include "console.hpp"
#include "datas/binwritter.hpp"
#include "datas/master_printer.hpp"
#include "datas/pugiex.hpp"
#include "datas/stat.hpp"
#include "datas/tchar.hpp"
#include "project.h"
#include "tmp_storage.hpp"
#include <thread>

static constexpr uint8 CONSOLE_DETAIL = 1 | uint8(USE_THREADS) << 1;

static const char appHeader0[] =
    "Simply drag'n'drop files/folders onto application or "
    "use as ";
static const char appHeader1[] =
    " [options] path1 path2 ...\nTool can detect and scan folders and "
    "uncompressed zip archives.";

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
      if (totalCount) {
        // Wait a little bit for internal queues to finish printing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void PackModeBatch(Batch &batch) {
  struct PackData {
    size_t index = 0;
    AppPackContext *archiveContext = nullptr;
    std::string pbarLabel;
    DetailedProgressBar *progBar = nullptr;
    std::string folderPath;
  };

  auto payload = std::make_shared<PackData>();

  batch.forEachFolder = [payload, ctx = batch.ctx](const std::string &path,
                                                   AppPackStats stats) {
    payload->folderPath = path;
    payload->archiveContext = ctx->NewArchive(path, stats);
    payload->pbarLabel = "Folder id " + std::to_string(payload->index++);
    payload->progBar =
        AppendNewLogLine<DetailedProgressBar>(payload->pbarLabel);
    payload->progBar->ItemCount(stats.numFiles);
    ConsolePrintDetail(CONSOLE_DETAIL);
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
      totalFiles += numFiles;
    }

    ~ExtractStatsMaker() { RemoveLogLines(scanBar); }
  };

  batch.keepFinishLines = false;
  auto sharedData = std::make_shared<ExtractStatsMaker>();
  ConsolePrintDetail(CONSOLE_DETAIL);
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
  ConsolePrintDetail(CONSOLE_DETAIL);
  batch.forEachFile = [payload = std::make_shared<UILines>(*stats),
                       archiveFiles =
                           std::make_shared<decltype(stats->archiveFiles)>(
                               std::move(stats->archiveFiles)),
                       ctx = batch.ctx](AppContextShare *iCtx) {
    auto currentBar = payload->ChooseBar();
    if (currentBar) {
      currentBar->ItemCount(archiveFiles->at(iCtx->Hash()));
    }

    iCtx->forEachFile = [=] {
      if (currentBar) {
        (*currentBar)++;
      }

      if (payload->totalCount) {
        (*payload->totalCount)++;
      }
    };

    printline("Processing: " << iCtx->FullPath());
    ctx->ProcessFile(iCtx);
    if (payload->totalProgress) {
      (*payload->totalProgress)++;
    }
  };
}

void ProcessBatch(Batch &batch, size_t numFiles) {
  ConsolePrintDetail(CONSOLE_DETAIL);
  batch.forEachFile = [payload = std::make_shared<UILines>(numFiles),
                       ctx = batch.ctx](AppContextShare *iCtx) {
    printline("Processing: " << iCtx->FullPath());
    ctx->ProcessFile(iCtx);
    if (payload->totalProgress) {
      (*payload->totalProgress)++;
    }
    if (payload->totalCount) {
      (*payload->totalCount)++;
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
      batch.Clean();
      stats.get()->totalFiles += totalFiles;
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
