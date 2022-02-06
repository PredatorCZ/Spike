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
#include "datas/stat.hpp"
#include "datas/tchar.hpp"
#include "out_context.hpp"
#include "project.h"
#include "tmp_storage.hpp"

#ifndef SPIKE_USE_THREADS
#define SPIKE_USE_THREADS NDEBUG
#endif

static const char appHeader0[] =
    "Simply drag'n'drop files/folders onto application or "
    "use as ";
static const char appHeader1[] =
    " [options] path1 path2 ...\nTool can detect and scan folders and "
    "uncompressed zip archives.";

struct ScanningFoldersBar : LoadingBar {
  char buffer[512]{};
  size_t modifyPos = 0;

  ScanningFoldersBar(es::string_view folder)
      : LoadingBar({buffer, sizeof(buffer)}) {
    static constexpr es::string_view part1("Scanning folder: ");
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
    snprintf(buffer, sizeof(buffer), "Extracted %4" PRIuMAX " files.",
             curitem.load(std::memory_order_relaxed));
    LoadingBar::PrintLine();
  }
};

struct UILines {
  size_t numFiles;
  ProgressBar *totalProgress;
  CounterLine *totalCount;
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
};

void ProcessZIPsExtractConvertMode(std::map<std::string, PathFilter> &zips,
                                   PathFilter &pathFilter, APPContext &ctx,
                                   UILines &lines) {
  size_t processedSoFar = lines.numFiles;
  size_t remainingZips = zips.size();

  for (auto &[path, filter] : zips) {
    remainingZips--;
    auto labelData = "Loading ZIP vfs: " + path;
    auto loadBar = AppendNewLogLine<LoadingBar>(labelData);
    const bool loadFiltered =
        ctx.info->arcLoadType == ArchiveLoadType::FILTERED;
    auto fctx = loadFiltered ? MakeZIPContext(path, filter, pathFilter)
                             : MakeZIPContext(path);
    AFileInfo zFile(path);
    std::vector<ZIPIOEntry> filesToProcess;

    if (!loadFiltered) {
      auto vfsIter = fctx->Iter();

      for (auto f : vfsIter) {
        auto item = f.AsView();
        if (size_t lastSlash = item.find_last_of("/\\");
            lastSlash != item.npos) {
          item.remove_prefix(lastSlash + 1);
        }

        if (pathFilter.IsFiltered(item) && filter.IsFiltered(item)) {
          filesToProcess.push_back(f);
        }
      }
    }

    std::string outPath = zFile.GetFullPathNoExt();
    ZIPMerger mainZip;

    if (!extractSettings.makeZIP) {
      es::mkdir(outPath);
      outPath.push_back('/');
      IOExtractContext ctx_(outPath);

      auto AddFolder = [&](auto &store) {
        for (decltype(auto) f : store) {
          AFileInfo cFile(f.AsView());

          if (extractSettings.folderPerArc) {
            ctx_.AddFolderPath(cFile.GetFullPathNoExt().to_string());
          } else {
            ctx_.AddFolderPath(cFile.GetFolder().to_string());
          }
        }
      };

      if (!loadFiltered) {
        AddFolder(filesToProcess);
      } else {
        auto vfsIter = fctx->Iter();
        AddFolder(vfsIter);
      }

      ctx_.GenerateFolders();
    } else {
      auto outZip = outPath + "_out.zip";
      new (&mainZip) ZIPMerger(outZip, RequestTempFile());
    }

    loadBar->Finish();
    ReleaseLogLines(loadBar);

    auto vfsIter = fctx->Iter();
    auto vfsIterBegin = vfsIter.begin();
    const size_t numFiles =
        loadFiltered ? vfsIter.base->Count() : filesToProcess.size();
    std::vector<size_t> archiveFiles;
    std::atomic_size_t numFilesToProcess{0};

    if (ctx.info->mode == AppMode_e::EXTRACT && ctx.ExtractStat) {
      auto scanBar = AppendNewLogLine<LoadingBar>("Processing extract stats.");
      archiveFiles.resize(numFiles);
      auto vfsInternalIter = fctx->Iter();
      auto vfsInternalIterBegin = vfsInternalIter.begin();
      RunThreadedQueue(numFiles, [&](size_t index) {
        auto &&fileEntry = [&] {
          if (!loadFiltered) {
            return filesToProcess[index];
          } else {
            return vfsInternalIterBegin++;
          }
        }();

        auto numFiles = ctx.ExtractStat(std::bind(
            [&](size_t offset, size_t size) {
              return fctx->GetChunk(fileEntry, offset, size);
            },
            std::placeholders::_1, std::placeholders::_2));

        archiveFiles[index] = numFiles;
        numFilesToProcess.fetch_add(numFiles, std::memory_order_relaxed);
      });

      ModifyElements([&](ElementAPI &api) {
        api.Remove(scanBar);

        const size_t minThreads =
            std::min(size_t(std::thread::hardware_concurrency()),
                     numFilesToProcess.load());

        if (minThreads <= lines.bars.size()) {
          return;
        }

        for (size_t t = 0; t < minThreads; t++) {
          auto progBar = std::make_unique<ProgressBar>("Thread:");
          auto progBarRaw = progBar.get();
          lines.bars.emplace(t, progBarRaw);
          api.Insert(std::move(progBar), lines.totalProgress, false);
        }
      });
    }

    if (lines.totalProgress) {
      lines.totalProgress->ItemCount(processedSoFar + numFilesToProcess +
                                         (remainingZips * lines.numFiles),
                                     processedSoFar);
    }
    processedSoFar += numFilesToProcess;

#if SPIKE_USE_THREADS
    RunThreadedQueue(numFiles, [&, &path = path](size_t index) {
      try {
#else
    for (size_t index = 0; index < numFiles; index++) {
#endif
        auto &&fileEntry = [&] {
          if (!loadFiltered) {
            return filesToProcess[index];
          } else {
            return vfsIterBegin++;
          }
        }();
        AFileInfo cFile(fileEntry.AsView());
        auto appCtx = std::make_unique<ZIPIOContextInstance>(fctx.get());
        appCtx->workingFile = fileEntry.AsView();
        auto currentBar = lines.ChooseBar();

        if (currentBar) {
          currentBar->ItemCount(archiveFiles.at(index));
        }

        if (ctx.info->mode == AppMode_e::EXTRACT) {
          printline("Extracting: " << path << '/' << fileEntry.AsView());
          std::unique_ptr<AppExtractContext> ectx;
          std::string recordsFile;

          if (extractSettings.makeZIP) {
            recordsFile = RequestTempFile();
            auto zCtx = std::make_unique<ZIPExtactContext>(recordsFile, false);

            if (extractSettings.folderPerArc) {
              zCtx->prefixPath = cFile.GetFullPathNoExt().to_string();
              zCtx->prefixPath.push_back('/');
            }

            zCtx->totalBar = lines.totalCount;
            zCtx->progBar = currentBar;

            ectx = std::move(zCtx);
          } else {
            auto outPath_ = outPath;

            if (extractSettings.folderPerArc) {
              outPath_ += cFile.GetFullPathNoExt().to_string();
              outPath_.push_back('/');
            } else {
              outPath_ += cFile.GetFolder().to_string();
            }

            auto uniq = std::make_unique<IOExtractContext>(outPath_);
            uniq->totalBar = lines.totalCount;
            uniq->progBar = currentBar;
            ectx = std::move(uniq);
          }

          ectx->ctx = appCtx.get();
          auto fileStream = fctx->OpenFile(fileEntry);
          ctx.ExtractFile(*fileStream, ectx.get());
          fctx->DisposeFile(fileStream);

          if (extractSettings.makeZIP) {
            auto zCtx = static_cast<ZIPExtactContext *>(ectx.get());
            mainZip.Merge(*zCtx, recordsFile);
            es::Dispose(ectx);
            es::RemoveFile(recordsFile);
          }
        } else {
          printline("Processing: " << path << '/' << fileEntry.AsView());
          auto fileStream = fctx->OpenFile(fileEntry);

          appCtx->outFile = outPath + appCtx->workingFile;
          ctx.ProcessFile(*fileStream, appCtx.get());
          (*lines.totalProgress)++;
          fctx->DisposeFile(fileStream);
        }
#if SPIKE_USE_THREADS
      } catch (const std::exception &e) {
        printerror(e.what());
      }
    });
#else
    }
#endif

    if (extractSettings.makeZIP) {
      ModifyElements([](ElementAPI &api) {
        api.Clean();
        api.Append(std::make_unique<LoadingBar>("Generating final ZIP."));
      });
      mainZip.FinishMerge([] { printinfo("Generating cache."); });
    }
  }
}

void ExtractConvertMode(int argc, TCHAR *argv[], APPContext &ctx,
                        const std::vector<bool> &markedFiles) {
  DirectoryScanner sc;

  if (es::string_view *curFilter = ctx.info->filters) {
    while (!curFilter->empty()) {
      sc.AddFilter(*curFilter);
      curFilter++;
    }
  }

  std::vector<std::string> files;
  std::map<std::string, PathFilter> zips;

  for (int a = 2; a < argc; a++) {
    if (!markedFiles[a]) {
      continue;
    }

    auto fileName = std::to_string(argv[a]);
    auto type = FileType(fileName);

    switch (type) {
    case FileType_e::Directory: {
      auto scanBar = AppendNewLogLine<ScanningFoldersBar>(fileName);
      sc.Clear();
      sc.scanCbData = scanBar;
      sc.scanCb = [](void *data, size_t numFolders, size_t numFiles,
                     size_t foundFiles) {
        auto barData = static_cast<ScanningFoldersBar *>(data);
        barData->Update(numFolders, numFiles, foundFiles);
      };
      sc.Scan(fileName);
      scanBar->Finish();
      ReleaseLogLines(scanBar);

      std::transform(std::make_move_iterator(sc.begin()),
                     std::make_move_iterator(sc.end()),
                     std::back_inserter(files),
                     [](auto &&item) { return std::move(item); });

      break;
    }
    case FileType_e::File: {
      const size_t found = fileName.find(".zip");
      if (found != fileName.npos) {
        if (found + 4 == fileName.size()) {
          zips.emplace(std::make_pair(std::move(fileName), PathFilter{}));
        } else if (fileName[found + 4] != '/') {
          files.emplace_back(std::move(fileName));
        } else {
          auto sub = fileName.substr(0, found + 5);
          auto foundZip = zips.find(sub);
          auto filterString = "^" + fileName.substr(found + 5);

          if (es::IsEnd(zips, foundZip)) {
            PathFilter zFilter;
            zFilter.AddFilter(filterString);
            zips.emplace(std::make_pair(std::move(sub), std::move(zFilter)));
          } else {
            foundZip->second.AddFilter(filterString);
          }
        }
      } else {
        files.emplace_back(std::move(fileName));
      }
      break;
    }
    default: {
      printerror("Invalid path: " << fileName);
      break;
    }
    }
  }

  PathFilter pathFilter = sc;

  es::Dispose(markedFiles);
  es::Dispose(sc);

  if (!files.empty()) {
    printline("Total files to process: " << files.size());
  }

  if (!zips.empty()) {
    printline("Total ZIP files to process: " << zips.size());
  }

  ConsolePrintDetail(3);
  std::atomic_size_t numFilesToProcess{0};
  std::vector<size_t> archiveFiles;
  UILines uiLines;
  uiLines.totalProgress = nullptr;
  uiLines.totalCount = nullptr;
  uiLines.numFiles = files.size();

  if (ctx.info->mode == AppMode_e::EXTRACT) {
    if (ctx.ExtractStat) {
      auto scanBar = AppendNewLogLine<LoadingBar>("Processing extract stats.");
      archiveFiles.resize(files.size());
      RunThreadedQueue(files.size(), [&](size_t index) {
        try {
          BinReader cRead(files[index]);
          auto numFiles = ctx.ExtractStat(std::bind(
              [&](size_t offset, size_t size) {
                cRead.Seek(offset);
                std::string data;
                cRead.ReadContainer(data, size);
                return data;
              },
              std::placeholders::_1, std::placeholders::_2));

          archiveFiles[index] = numFiles;
          numFilesToProcess.fetch_add(numFiles, std::memory_order_relaxed);
        } catch (const std::exception &e) {
          printerror(e.what());
        }
      });

      ModifyElements([&](ElementAPI &api) {
        api.Remove(scanBar);

        const size_t minThreads =
            std::min(size_t(std::thread::hardware_concurrency()), files.size());

        if (minThreads < 2) {
          return;
        }

        for (size_t t = 0; t < minThreads; t++) {
          auto progBar = std::make_unique<ProgressBar>("Thread:");
          auto progBarRaw = progBar.get();
          uiLines.bars.emplace(t, progBarRaw);
          api.Append(std::move(progBar));
        }
      });

      auto prog = AppendNewLogLine<DetailedProgressBar>("Total: ");
      prog->ItemCount(std::max(numFilesToProcess.load(), size_t(1)) *
                      (zips.size() + 1));
      uiLines.totalCount = prog;
      uiLines.totalProgress = prog;
    } else {
      uiLines.totalCount = AppendNewLogLine<ProcessedFiles>();
    }
  } else {
    auto prog = AppendNewLogLine<DetailedProgressBar>("Total: ");
    prog->ItemCount(std::max(files.size(), size_t(1)) *
                    std::max(zips.size(), size_t(1)));
    uiLines.totalCount = prog;
    uiLines.totalProgress = prog;
  }

#if SPIKE_USE_THREADS
  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
#else
  for (size_t index = 0; index < files.size(); index++) {
#endif
      auto currentBar = uiLines.ChooseBar();
      if (currentBar) {
        currentBar->ItemCount(archiveFiles.at(index));
      }
      BinReader cRead(files[index]);
      AFileInfo cFile(files[index]);
      auto appCtx = MakeIOContext();
      appCtx->workingFile = files[index];

      if (ctx.info->mode == AppMode_e::EXTRACT) {
        printline("Extracting: " << files[index]);
        std::unique_ptr<AppExtractContext> ectx;
        std::string outPath = cFile.GetFullPathNoExt().to_string();

        if (extractSettings.makeZIP) {
          if (cFile.GetExtension() == ".zip") {
            outPath.append("_out");
          }

          outPath.append(".zip");

          auto uniq = std::make_unique<ZIPExtactContext>(outPath);
          uniq->totalBar = uiLines.totalCount;
          uniq->progBar = currentBar;
          ectx = std::move(uniq);
        } else {
          if (!extractSettings.folderPerArc) {
            outPath = cFile.GetFolder();
          } else {
            es::mkdir(outPath);
            outPath.push_back('/');
          }

          auto uniq = std::make_unique<IOExtractContext>(outPath);
          uniq->totalBar = uiLines.totalCount;
          uniq->progBar = currentBar;
          ectx = std::move(uniq);
        }

        ectx->ctx = appCtx.get();
        ctx.ExtractFile(cRead.BaseStream(), ectx.get());

        if (extractSettings.makeZIP) {
          static_cast<ZIPExtactContext *>(ectx.get())->FinishZIP([] {
            printinfo("Generating cache.");
          });
        }
      } else {
        appCtx->outFile = files[index];
        printline("Processing: " << files[index]);
        ctx.ProcessFile(cRead.BaseStream(), appCtx.get());
        (*uiLines.totalProgress)++;
      }
#if SPIKE_USE_THREADS
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });
#else
  }
#endif

  if (!zips.empty()) {
    size_t curBar = 0;
    decltype(uiLines.bars) newBars;

    for (auto &[_, bar] : uiLines.bars) {
      if (bar) {
        newBars.emplace(curBar++, bar);
      }
    }

    std::swap(uiLines.bars, newBars);
    ProcessZIPsExtractConvertMode(zips, pathFilter, ctx, uiLines);
  }

  if (ctx.info->mode == AppMode_e::EXTRACT && !ctx.ExtractStat) {
    auto data = static_cast<ProcessedFiles *>(uiLines.totalCount);
    data->Finish();
    ReleaseLogLines(data);
  }
}

void PackMode(int argc, TCHAR *argv[], APPContext &ctx,
              const std::vector<bool> &markedFiles) {
  PathFilter moduleFilter;

  if (es::string_view *curFilter = ctx.info->filters) {
    while (!curFilter->empty()) {
      moduleFilter.AddFilter(*curFilter);
      curFilter++;
    }
  }

  using paths = std::vector<std::string>;
  std::map<std::string, paths> zips;
  std::mutex zipsMutex;

  for (size_t index = 0; index < argc; index++) {
    if (!markedFiles[index]) {
      continue;
    }

    auto fileName = std::to_string(argv[index]);
    auto type = FileType(fileName);
    switch (type) {
    case FileType_e::Directory: {
      auto scanBar = AppendNewLogLine<ScanningFoldersBar>(fileName);
      DirectoryScanner sc(moduleFilter);
      sc.scanCbData = scanBar;
      sc.scanCb = [](void *data, size_t numFolders, size_t numFiles,
                     size_t foundFiles) {
        auto barData = static_cast<ScanningFoldersBar *>(data);
        barData->Update(numFolders, numFiles, foundFiles);
      };
      sc.Scan(fileName);
      scanBar->Finish();
      ReleaseLogLines(scanBar);

      AppPackStats stats{};
      stats.numFiles = sc.Files().size();

      for (auto &f : sc) {
        stats.totalSizeFileNames += f.size() + 1;
      }

      auto archiveContext = ctx.NewArchive(fileName, stats);
      auto folderIndex = "Folder id " + std::to_string(index);
      auto statBar = AppendNewLogLine<DetailedProgressBar>(folderIndex);
      statBar->ItemCount(sc.Files().size());
      ConsolePrintDetail(3);

      RunThreadedQueue(sc.Files().size(), [&](size_t index) {
        try {
          BinReader cRead(sc.Files().at(index));
          es::string_view relativeFilepath(sc.Files().at(index));
          relativeFilepath.remove_prefix(fileName.size());
          archiveContext->SendFile(relativeFilepath, cRead.BaseStream());
          (*statBar)++;
        } catch (const std::exception &e) {
          printerror(e.what());
        }
      });

      ConsolePrintDetail(1);
      archiveContext->Finish();
      RemoveLogLines(statBar);
      break;
    }
    default:
    case FileType_e::File: {
      const size_t found = fileName.find(".zip");
      if (found != fileName.npos) {
        std::lock_guard<std::mutex> zipGuard(zipsMutex);
        if (found + 4 == fileName.size()) {
          zips.emplace(std::make_pair(std::move(fileName), paths{""}));
          break;
        } else if (fileName[found + 4] == '/') {
          auto sub = fileName.substr(0, found + 4);
          auto foundZip = zips.find(sub);
          auto filterString = "^" + fileName.substr(found + 5);

          if (es::IsEnd(zips, foundZip)) {
            paths pVec;
            pVec.push_back(std::move(filterString));
            zips.emplace(std::make_pair(std::move(sub), std::move(pVec)));
          } else {
            foundZip->second.push_back(std::move(filterString));
          }

          break;
        }
      }
      printerror("Invalid path: " << fileName);
    }
    }
  }

  es::Dispose(markedFiles);

  if (zips.empty()) {
    return;
  }

  for (auto &[path, paths] : zips) {
    auto labelData = "Loading ZIP vfs: " + path;
    auto loadBar = AppendNewLogLine<LoadingBar>(labelData);
    const bool loadFiltered =
        ctx.info->arcLoadType == ArchiveLoadType::FILTERED;
    auto fctx = [&, &paths_ = paths, &path_ = path] {
      if (loadFiltered) {
        PathFilter mainFilter;
        for (auto &z : paths_) {
          mainFilter.AddFilter(z);
        }

        return MakeZIPContext(path_, mainFilter, moduleFilter);
      }

      return MakeZIPContext(path_);
    }();

    loadBar->Finish();
    ReleaseLogLines(loadBar);

    for (size_t index = 0; index < paths.size(); index++) {
      PathFilter zFilter;
      auto zipPath = path.substr(0, path.size() - 4);
      auto &zFolder = paths[index];

      if (!zFolder.empty()) {
        zFilter.AddFilter(zFolder);
        zipPath += '/' + zFolder.substr(1);
      }

      AppPackStats stats{};
      auto vfsIter = fctx->Iter();
      std::vector<bool> markedFiles(vfsIter.base->Count(), false);
      size_t curIndex = 0;

      for (auto f : vfsIter) {
        auto item = f.AsView();
        if (moduleFilter.IsFiltered(item) && zFilter.IsFiltered(item)) {
          markedFiles[curIndex] = true;
          stats.numFiles++;
          stats.totalSizeFileNames += item.size() + 1;
        }

        curIndex++;
      }

      curIndex = 0;
      auto archiveContext = ctx.NewArchive(zipPath, stats);
      vfsIter = fctx->Iter();
      auto vfsIterBegin = vfsIter.begin();
      auto folderIndex = "Zip id " + std::to_string(index);
      auto statBar = AppendNewLogLine<DetailedProgressBar>(folderIndex);
      statBar->ItemCount(stats.numFiles);
      ConsolePrintDetail(3);

      RunThreadedQueueEx(markedFiles.size(), [&](size_t index) {
        auto f = vfsIterBegin++;

        if (!markedFiles[index]) {
          return;
        }

        auto fileStream = fctx->OpenFile(f);
        es::string_view zFile(f.AsView());
        zFile.remove_prefix(zFolder.size());
        try {
          archiveContext->SendFile(f.AsView(), *fileStream);
        } catch (const std::exception &e) {
          printerror(e.what());
        }
        fctx->DisposeFile(fileStream);
        (*statBar)++;
      });

      ConsolePrintDetail(1);
      archiveContext->Finish();
      RemoveLogLines(statBar);
    }
  }
}

int Main(int argc, TCHAR *argv[]) {
  ConsolePrintDetail(1);

  if (argc < 2) {
    printerror("Insufficient argument count, expected module name.");
    return 1;
  }

  if (argc < 3) {
    printerror("Insufficient argument count, expected parameters.");
    return 1;
  }

  AFileInfo appLocation(std::to_string(*argv));
  auto appFolder = appLocation.GetFolder().to_string();
  auto moduleName = std::to_string(argv[1]);
  APPContext ctx;

  try {
    ctx = APPContext(moduleName.data(), appFolder);
  } catch (const std::exception &e) {
    printerror(e.what());
    return 2;
  }

  bool dontLoadConfig = false;
  std::vector<bool> markedFiles(size_t(argc), false);
  ConsolePrintDetail(0);

  printline(ctx.info->header);
  printline(appHeader0 << appLocation.GetFilename() << ' ' << moduleName
                       << appHeader1);

  if (IsHelp(argv[2])) {
    ctx.PrintCLIHelp();
    return 0;
  }

  ConsolePrintDetail(1);

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

        const size_t numValues = ctx.Settings().GetNumReflectedValues();
        for (size_t i = 0; i < numValues; i++) {
          auto rType = ctx.Settings().GetReflectedType(i);
          if (rType->type == REFType::Bool) {
            ctx.Settings().SetReflectedValue(i, "false");
          }
        }

        extractSettings.folderPerArc = extractSettings.makeZIP = false;
      }();

      dontLoadConfig = true;
      opt++;

      if (opt[0] == '-') {
        opt++;
      }

      auto optStr = std::to_string(opt);
      JenHash optHash(optStr);

      ReflectorFriend *refl = nullptr;
      auto rType = ctx.Settings().GetReflectedType(optHash);

      if (rType) {
        refl = &ctx.Settings();
      } else {
        rType = MainSettings().GetReflectedType(optHash);

        if (rType) {
          refl = &MainSettings();
        } else if (ctx.info->mode == AppMode_e::EXTRACT) {
          rType = ExtractSettings().GetReflectedType(optHash);

          if (rType) {
            refl = &ExtractSettings();
          }
        }
      }

      if (rType) {
        if (rType->type == REFType::Bool) {
          refl->SetReflectedValue(*rType, "true");
        } else {
          auto valStr = std::to_string(argv[++a]);
          refl->SetReflectedValue(*rType, valStr);
        }
      } else {
        printerror("Invalid option: " << (optStr.size() > 1 ? "--" : "-")
                                      << optStr);
        continue;
      }

    } else {
      markedFiles[a] = true;
    }
  }

  if (!dontLoadConfig) {
    ctx.FromConfig();
  }

  InitTempStorage();
  ctx.SetupModule();

  if (ctx.info->mode == AppMode_e::PACK) {
    PackMode(argc, argv, ctx, markedFiles);
  } else {
    ExtractConvertMode(argc, argv, ctx, markedFiles);
  }

  if (ctx.FinishContext) {
    ctx.FinishContext();
  }

  return 0;
}

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  es::SetupWinApiConsole();
  InitConsole();
  CleanTempStorages();

  int retVal = Main(argc, argv);

  CleanCurrentTempStorage();
  TerminateConsole();
  return retVal;
}
