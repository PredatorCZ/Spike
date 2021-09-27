/*  Spike is universal dedicated module handler
    This source contains code for CLI master app
    Part of PreCore project

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

struct BinReaderEx : BinReader<> {
  using BinReader::baseStream;
  using BinReader::BinReader;
};

static const char appHeader0[] =
    "Simply drag'n'drop files/folders onto application or "
    "use as ";
static const char appHeader1[] =
    " [options] path1 path2 ...\nTool can detect and scan folders and "
    "uncompressed zip archives.";

void ExtractConvertMode(int argc, TCHAR *argv[], APPContext &ctx,
                        const std::vector<bool> &markedFiles) {
  DirectoryScanner sc;
  {
    es::string_view *curFilter = ctx.info->filters;

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
      printline("Scanning: " << fileName);
      sc.Clear();
      sc.Scan(fileName);

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

  es::print::PrintThreadID(true);

  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
      BinReaderEx cRead(files[index]);
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

          ectx = std::make_unique<ZIPExtactContext>(outPath);
        } else {
          if (!extractSettings.folderPerArc) {
            outPath = cFile.GetFolder();
          } else {
            es::mkdir(outPath);
            outPath.push_back('/');
          }

          ectx = std::make_unique<IOExtractContext>(outPath);
        }

        ectx->ctx = appCtx.get();
        ctx.ExtractFile(*cRead.baseStream, ectx.get());

        if (extractSettings.makeZIP) {
          static_cast<ZIPExtactContext *>(ectx.get())->FinishZIP();
        }
      } else {
        printline("Processing: " << files[index]);
        ctx.ProcessFile(*cRead.baseStream, appCtx.get());
      }
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  if (zips.empty()) {
    return;
  }

  for (auto &zip : zips) {
    printline("Loading ZIP vfs: " << zip.first);
    const bool loadFiltered =
        ctx.info->arcLoadType == ArchiveLoadType::FILTERED;
    auto fctx = loadFiltered ? MakeZIPContext(zip.first, zip.second, pathFilter)
                             : MakeZIPContext(zip.first);
    AFileInfo zFile(zip.first);
    std::vector<decltype(fctx->vfs)::value_type> filesToProcess;

    if (!loadFiltered) {
      for (auto &f : fctx->vfs) {
        if (pathFilter.IsFiltered(f.first) && zip.second.IsFiltered(f.first)) {
          filesToProcess.emplace_back(f);
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
        for (auto &f : store) {
          AFileInfo cFile(f.first);

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
        AddFolder(fctx->vfs);
      }

      ctx_.GenerateFolders();
    } else {
      auto outZip = outPath + "_out.zip";
      new (&mainZip) ZIPMerger(outZip, es::GetTempFilename());
    }

    const size_t numFiles =
        loadFiltered ? fctx->vfs.size() : filesToProcess.size();
    auto vfsIter = fctx->vfs.begin();
    std::mutex vfsMutex;

    RunThreadedQueue(numFiles, [&](size_t index) {
      // for (size_t index = 0; index < numFiles; index++) {
      try {
        auto &&fileEntry = [&] {
          if (!loadFiltered) {
            return filesToProcess[index];
          } else {
            std::lock_guard<std::mutex> guard(vfsMutex);
            return *vfsIter++;
          }
        }();
        AFileInfo cFile(fileEntry.first);
        auto appCtx = std::make_unique<ZIPIOContextInstance>(fctx.get());
        appCtx->workingFile = fileEntry.first;

        if (ctx.info->mode == AppMode_e::EXTRACT) {
          printline("Extracting: " << zip.first << '/' << fileEntry.first);
          std::unique_ptr<AppExtractContext> ectx;
          std::string recordsFile;

          if (extractSettings.makeZIP) {
            recordsFile = es::GetTempFilename();
            auto zCtx = std::make_unique<ZIPExtactContext>(recordsFile);

            if (extractSettings.folderPerArc) {
              zCtx->prefixPath = cFile.GetFullPathNoExt().to_string();
              zCtx->prefixPath.push_back('/');
            }

            ectx = std::move(zCtx);
          } else {
            auto outPath_ = outPath;

            if (extractSettings.folderPerArc) {
              outPath_ += cFile.GetFullPathNoExt().to_string();
              outPath_.push_back('/');
            } else {
              outPath_ += cFile.GetFolder().to_string();
            }
            ectx = std::make_unique<IOExtractContext>(outPath_);
          }

          ectx->ctx = appCtx.get();
          auto fileStream = fctx->OpenFile(fileEntry.second);
          ctx.ExtractFile(*fileStream, ectx.get());
          fctx->DisposeFile(fileStream);

          if (extractSettings.makeZIP) {
            auto zCtx = static_cast<ZIPExtactContext *>(ectx.get());
            mainZip.Merge(*zCtx, recordsFile);
            es::Dispose(ectx);
            es::RemoveFile(recordsFile);
          }
        } else {
          printline("Processing: " << zip.first << '/' << fileEntry.first);
          auto fileStream = fctx->OpenFile(fileEntry.second);

          appCtx->workingFile.insert(0, outPath);
          ctx.ProcessFile(*fileStream, appCtx.get());
          fctx->DisposeFile(fileStream);
        }
      } catch (const std::exception &e) {
        printerror(e.what());
      }
    });

    if (extractSettings.makeZIP) {
      mainZip.FinishMerge();
    }
  }
}

void PackMode(int argc, TCHAR *argv[], APPContext &ctx,
              const std::vector<bool> &markedFiles) {
  PathFilter moduleFilter;
  {
    es::string_view *curFilter = ctx.info->filters;

    while (!curFilter->empty()) {
      moduleFilter.AddFilter(*curFilter);
      curFilter++;
    }
  }

  using paths = std::vector<std::string>;
  std::map<std::string, paths> zips;
  std::mutex zipsMutex;

  RunThreadedQueue(argc, [&](size_t index) {
    if (!markedFiles[index]) {
      return;
    }

    auto fileName = std::to_string(argv[index]);
    auto type = FileType(fileName);

    try {
      switch (type) {
      case FileType_e::Directory: {
        printline("Scanning: " << fileName);
        DirectoryScanner sc(moduleFilter);
        sc.Scan(fileName);
        AppPackStats stats;
        stats.numFiles = sc.Files().size();

        for (auto &f : sc) {
          stats.totalSizeFileNames += f.size() + 1;
        }

        auto archiveContext = ctx.NewArchive(fileName, stats);

        for (auto &f : sc) {
          BinReaderEx cRead(f);
          es::string_view relativeFilepath(f);
          relativeFilepath.remove_prefix(fileName.size());
          archiveContext->SendFile(relativeFilepath, *cRead.baseStream);
        }

        archiveContext->Finish();
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
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  es::Dispose(markedFiles);

  if (zips.empty()) {
    return;
  }

  for (auto &zip : zips) {
    printline("Loading ZIP vfs: " << zip.first);
    const bool loadFiltered =
        ctx.info->arcLoadType == ArchiveLoadType::FILTERED;
    auto fctx = [&] {
      if (loadFiltered) {
        PathFilter mainFilter;
        for (auto &z : zip.second) {
          mainFilter.AddFilter(z);
        }

        return MakeZIPContext(zip.first, mainFilter, moduleFilter);
      }

      return MakeZIPContext(zip.first);
    }();

    RunThreadedQueue(zip.second.size(), [&](size_t index) {
      try {
        PathFilter zFilter;
        auto zipPath = zip.first.substr(0, zip.first.size() - 4);
        auto &zFolder = zip.second[index];

        if (!zFolder.empty()) {
          zFilter.AddFilter(zFolder);
          zipPath += '/' + zFolder.substr(1);
        }

        AppPackStats stats;
        std::vector<bool> markedFiles(fctx->vfs.size(), false);
        size_t curIndex = 0;

        for (auto &f : fctx->vfs) {
          if (moduleFilter.IsFiltered(f.first) && zFilter.IsFiltered(f.first)) {
            markedFiles[curIndex] = true;
            stats.numFiles++;
            stats.totalSizeFileNames += f.first.size() + 1;
          }

          curIndex++;
        }

        curIndex = 0;
        auto archiveContext = ctx.NewArchive(zipPath, stats);

        for (auto &f : fctx->vfs) {
          if (markedFiles[curIndex++]) {
            auto fileStream = fctx->OpenFile(f.second);
            es::string_view zFile(f.first);
            zFile.remove_prefix(zFolder.size());
            archiveContext->SendFile(zFile, *fileStream);
            fctx->DisposeFile(fileStream);
          }
        }

        archiveContext->Finish();
      } catch (const std::exception &e) {
        printerror(e.what());
      }
    });
  }
}

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  es::print::AddPrinterFunction(UPrintf);

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

  printline(ctx.info->header);
  printline(appHeader0 << appLocation.GetFilename() << ' ' << moduleName
                       << appHeader1);

  if (IsHelp(argv[2])) {
    ctx.PrintCLIHelp();
    return 0;
  }

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

  ctx.SetupModule();

  if (ctx.info->mode == AppMode_e::PACK) {
    PackMode(argc, argv, ctx, markedFiles);
  } else {
    ExtractConvertMode(argc, argv, ctx, markedFiles);
  }
}
