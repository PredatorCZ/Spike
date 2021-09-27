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
    " [options] path1 path2 ...\nTool can detect and scan folders.";

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

      auto SetOpt = [&](auto type_, ReflectorFriend &what) {
        if (type_->type == REFType::Bool) {
          what.SetReflectedValue(*type_, "true");
        } else {
          auto valStr = std::to_string(argv[++a]);
          what.SetReflectedValue(*type_, valStr);
        }
      };

      auto PrintFail = [&] {
        printerror("Invalid option: " << (optStr.size() > 1 ? "--" : "-")
                                      << optStr);
      };

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

  DirectoryScanner sc;
  {
    es::string_view *curFilter = ctx.info->filters;

    while (!curFilter->empty()) {
      sc.AddFilter(*curFilter);
      curFilter++;
    }
  }

  std::vector<std::string> files;
  std::vector<std::string> zips;

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
      es::string_view ext(std::prev(fileName.end(), 4).operator->(),
                          fileName.end().operator->());
      if (ext == ".zip") {
        zips.emplace_back(std::move(fileName));
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
    return 0;
  }

  for (auto &zip : zips) {
    printline("Loading ZIP vfs: " << zip);
    auto fctx = MakeZIPContext(zip);
    AFileInfo zFile(zip);
    std::vector<decltype(fctx->vfs)::value_type> filesToProcess;

    for (auto &f : fctx->vfs) {
      if (pathFilter.IsFiltered(f.first)) {
        filesToProcess.emplace_back(f);
      }
    }

    std::string outPath = zFile.GetFullPathNoExt();
    ZIPMerger mainZip;

    if (!extractSettings.makeZIP) {
      es::mkdir(outPath);
      outPath.push_back('/');
      IOExtractContext ctx_(outPath);

      for (auto &f : filesToProcess) {
        AFileInfo cFile(f.first);

        if (extractSettings.folderPerArc) {
          ctx_.AddFolderPath(cFile.GetFullPathNoExt().to_string());
        } else {
          ctx_.AddFolderPath(cFile.GetFolder().to_string());
        }
      }

      ctx_.GenerateFolders();
    } else {
      auto outZip = outPath + "_out.zip";
      new (&mainZip) ZIPMerger(outZip, es::GetTempFilename());
    }

    RunThreadedQueue(filesToProcess.size(), [&](size_t index) {
      // for (size_t index = 0; index < filesToProcess.size(); index++) {
      try {
        auto &fileEntry = filesToProcess[index];
        AFileInfo cFile(fileEntry.first);
        auto appCtx = std::make_unique<ZIPIOContextInstance>(fctx.get());
        appCtx->workingFile = fileEntry.first;

        if (ctx.info->mode == AppMode_e::EXTRACT) {
          printline("Extracting: " << zip << '/' << fileEntry.first);
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
          printline("Processing: " << zip << '/' << fileEntry.first);
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

  return 0;
}
