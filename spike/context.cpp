/*  Spike is universal dedicated module handler
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

#include "context.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/master_printer.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/tchar.hpp"
#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

#ifdef _MSC_VER
#include "datas/tchar.hpp"
#include <windows.h>
auto dlsym(void *handle, const char *name) {
  return GetProcAddress((HMODULE)handle, name);
}

void dlclose(void *handle) { FreeLibrary((HMODULE)handle); }

auto dlerror() {
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);

  std::string retVal = std::to_string((LPTSTR)lpMsgBuf);
  LocalFree(lpMsgBuf);
  return retVal;
}
#else
#include <dlfcn.h>
#endif

MainAppConf mainSettings;
MainAppExtractConf extractSettings;

REFLECT(CLASS(MainAppConf),
        MEMBERNAME(generateLog, "generate-log", "L",
                   ReflDesc{"Will generate text log of console output inside "
                            "application location."}), )

REFLECT(
    CLASS(MainAppExtractConf),
    MEMBERNAME(
        folderPerArc, "folder-per-archive", "F",
        ReflDesc{
            "When extracting, create folder that uses input archive's name as "
            "output dir."}),
    MEMBERNAME(makeZIP, "create-zip", "Z",
               ReflDesc{"Pack extracted files inside ZIP file named after "
                        "input archive. Your HDD will thank you."}), )

struct ReflectedInstanceFriend : ReflectedInstance {
  const reflectorStatic *Refl() const { return rfStatic; }
};

struct VersionHandler {
  uint32 versions[4]{};
  const std::string *path;

  bool operator<(const VersionHandler &other) const {
    if (versions[0] == other.versions[0]) {
      if (versions[1] == other.versions[1]) {
        if (versions[2] == other.versions[2]) {
          return versions[3] < other.versions[3];
        } else {
          return versions[2] < other.versions[2];
        }
      } else {
        return versions[1] < other.versions[1];
      }
    } else {
      return versions[0] < other.versions[0];
    }
  }
};

APPContext::APPContext(const char *moduleName_, const std::string &appFolder_)
    : appFolder(appFolder_) {
  moduleName = moduleName_;

  auto modulePath = [&] {
    DirectoryScanner esmScan;
    esmScan.AddFilter((std::string(1, '^') + moduleName) + '.');
    esmScan.Scan(appFolder);
    std::vector<VersionHandler> versionedFiles;

    for (auto &f : esmScan) {
      const size_t lastDotPos = f.find_last_of('.');
      const size_t slashPos = f.find_last_of('/');
      es::string_view extension(f.data() + lastDotPos);
      es::string_view fileName(f.data() + slashPos, lastDotPos - slashPos);

      if (extension != ".spk") {
        continue;
      }

      char *nextDot = nullptr;
      const size_t versionDotPos = fileName.find_first_of('.');

      if (versionDotPos == fileName.npos) {
        continue;
      }

      const char *versionBegin = fileName.data() + versionDotPos;
      size_t curIndex = 0;
      VersionHandler currentHandler;
      currentHandler.path = &f;
      auto &versions = currentHandler.versions;

      auto ChooseOne = [&] {
        if (*versionBegin != '.') {
          return false;
        }

        versionBegin++;
        const uint32 newVersion = std::strtoul(versionBegin, &nextDot, 10);

        if (versionBegin == nextDot) {
          return false;
        }

        versionBegin = nextDot;
        versions[curIndex++] = newVersion;

        return true;
      };

      if (ChooseOne()) {
        if (ChooseOne()) {
          if (ChooseOne()) {
            ChooseOne();
          }
        }
      }

      versionedFiles.push_back(currentHandler);
    }

    std::sort(versionedFiles.begin(), versionedFiles.end());

    if (versionedFiles.empty()) {
      throw std::runtime_error(std::string("Couldn't find module: ") +
                               moduleName);
    }

    return *versionedFiles.back().path;
  }();

  auto postError = [] {
    throw std::runtime_error(std::string("APPContext Error: ") + dlerror());
  };

  auto assign = [&](auto &value, auto name) {
    using type_ = std::decay_t<decltype(value)>;
    value = reinterpret_cast<type_>(dlsym(dlHandle, name));

    if (!value) {
      postError();
    }
  };

  auto tryAssign = [&](auto &value, auto name) {
    using type_ = typename std::decay_t<decltype(value)>::value_type;
    return value = reinterpret_cast<type_>(dlsym(dlHandle, name));
  };

#ifdef _MSC_VER
  auto modPath = ToTSTRING(modulePath);
  dlHandle = LoadLibrary(modPath.data());
#else
  dlHandle = dlopen(modulePath.data(), RTLD_NOW);
#endif
  if (!dlHandle) {
    postError();
  }

  func<decltype(AppInitModule)> InitModule;
  assign(InitModule, "AppInitModule");
  info = InitModule();

  if (info->contextVersion > AppInfo_s::CONTEXT_VERSION) {
    throw std::runtime_error("Module context version mismatch!");
  }

  tryAssign(AdditionalHelp, "AppAdditionalHelp");
  tryAssign(InitContext, "AppInitContext");
  tryAssign(FinishContext, "AppFinishContext");

  if (info->mode == AppMode_e::EXTRACT) {
    assign(ExtractFile, "AppExtractFile");
    tryAssign(ExtractStat, "AppExtractStat");
  } else if (info->mode == AppMode_e::PACK) {
    assign(NewArchive, "AppNewArchive");
  } else {
    assign(ProcessFile, "AppProcessFile");
    extractSettings.makeZIP = false;
    extractSettings.folderPerArc = false;
  }
}

APPContext::~APPContext() {
  if (dlHandle) {
    dlclose(dlHandle);
  }
}

static const reflectorStatic *RTTI(const ReflectorFriend &ref) {
  auto rawRTTI = ref.GetReflectedInstance();
  return static_cast<const ReflectedInstanceFriend &>(rawRTTI).Refl();
}

void APPContext::PrintCLIHelp() const {
  printline("Options:" << std::endl);

  auto printStuff = [](auto rtti) {
    for (size_t i = 0; i < rtti->nTypes; i++) {
      if (rtti->typeAliases && rtti->typeAliases[i]) {
        es::print::Get() << "-" << rtti->typeAliases[i] << ", ";
      }

      es::print::Get() << "--" << rtti->typeNames[i];
      es::print::Get() << "  = " << rtti->typeDescs[i].part1 << std::endl;
    }
  };

  printStuff(::RTTI(MainSettings()));

  if (info->mode == AppMode_e::EXTRACT) {
    printStuff(::RTTI(ExtractSettings()));
  }

  printStuff(RTTI());
  printline("");
}

void APPContext::SetupModule() {
  if (mainSettings.generateLog) {
    CreateLog();
  }

  if (InitContext && !InitContext(appFolder)) {
    throw std::runtime_error("Error while initializing context.");
  }
}

void APPContext::FromConfig() {
  auto configName = (appFolder + moduleName) + ".config";
  try {
    auto doc = XMLFromFile(configName);
    auto node = ReflectorXMLUtil::LoadV2(Settings(), doc, true);
    ReflectorXMLUtil::LoadV2(mainSettings, node);

    if (info->mode == AppMode_e::EXTRACT) {
      ReflectorXMLUtil::LoadV2(extractSettings, node);
    }
  } catch (const es::FileNotFoundError &) {
  }
  {
    pugi::xml_document doc = {};
    std::stringstream str;
    GetHelp(str);
    AdditionalHelp(str, 1);
    auto buff = str.str();
    doc.append_child(pugi::node_comment).set_value(buff.data());

    auto node =
        ReflectorXMLUtil::SaveV2a(Settings(), doc,
                                  {ReflectorXMLUtil::Flags_ClassNode,
                                   ReflectorXMLUtil::Flags_StringAsAttribute});
    ReflectorXMLUtil::SaveV2a(mainSettings, node,
                              ReflectorXMLUtil::Flags_StringAsAttribute);

    if (info->mode == AppMode_e::EXTRACT) {
      ReflectorXMLUtil::SaveV2a(extractSettings, node,
                                ReflectorXMLUtil::Flags_StringAsAttribute);
    }
    XMLToFile(configName, doc,
              {XMLFormatFlag::WriteBOM, XMLFormatFlag::IndentAttributes});
  }
}

using stream_type = BinWritter<BinCoreOpenMode::Text>;
static stream_type &GetStream() {
  static stream_type outStream;
  return outStream;
}

static std::ostream &GetLogger() { return GetStream().BaseStream(); }

static void printf(const char *str) { GetLogger() << str; }

const reflectorStatic *APPContext::RTTI() const { return ::RTTI(Settings()); }

void APPContext::CreateLog() {
  time_t curTime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm timeStruct = *localtime(&curTime);
  TCHAR dateBuffer[128]{};
  const size_t dateBufferSize = sizeof(dateBuffer) / sizeof(TCHAR);

  _tcsftime(dateBuffer, dateBufferSize, _T("_%y_%m_%d-%H.%M.%S"), &timeStruct);
  auto logName = appFolder + moduleName + std::to_string(dateBuffer) + ".txt";
  GetStream().Open(logName);
  es::print::AddPrinterFunction(printf, false);

  _tcsftime(dateBuffer, dateBufferSize, _T("%c %Z"), &timeStruct);

  GetLogger() << "Current time: " << std::to_string(dateBuffer) << std::endl;
  GetLogger() << "Number of concurrent threads: "
              << std::thread::hardware_concurrency() << std::endl;
  GetLogger() << "Configuration:" << std::endl;

  auto PrintStuff = [](auto &what) {
    const size_t numSettings = what.GetNumReflectedValues();
    auto rtti = ::RTTI(what);

    for (size_t t = 0; t < numSettings; t++) {
      es::string_view desc2;

      if (rtti->typeDescs && rtti->typeDescs[t].part2) {
        desc2 = rtti->typeDescs[t].part2;
      }

      Reflector::KVPair pair = what.GetReflectedPair(t);

      GetLogger() << '\t' << pair.name << ": ";

      if (desc2 == "HIDDEN") {
        GetLogger() << "--hidden--";
      } else {
        GetLogger() << pair.value;
      }

      GetLogger() << std::endl;
    }
  };

  PrintStuff(MainSettings());
  if (info->mode == AppMode_e::EXTRACT) {
    PrintStuff(ExtractSettings());
  }
  PrintStuff(Settings());

  GetLogger() << std::endl;
}

void GetHelp(std::ostream &str, const reflectorStatic *ref, size_t level = 1) {
  auto fillIndent = [&](size_t mod = 0) -> std::ostream & {
    for (size_t i = 0; i < level + mod; i++) {
      str << '\t';
    }

    return str;
  };

  for (size_t i = 0; i < ref->nTypes; i++) {
    es::string_view elName = ref->typeNames[i];
    auto elDesc = ref->typeDescs[i];
    fillIndent() << elName << std::endl;

    if (elDesc.part1) {
      fillIndent(1) << elDesc.part1 << std::endl;
    }

    auto fl = ref->types[i];

    if (fl.type == REFType::Class || fl.type == REFType::BitFieldClass) {
      GetHelp(str, REFSubClassStorage.at(JenHash(fl.asClass.typeHash)),
              level + 1);
    } else if (fl.type == REFType::Array || fl.type == REFType::ArrayClass) {
      const auto &arr = fl.asArray;

      if (arr.type == REFType::Class || arr.type == REFType::BitFieldClass) {
        GetHelp(str, REFSubClassStorage.at(JenHash(arr.asClass.typeHash)),
                level + 1);
      }
    } else if (fl.type == REFType::Enum) {
      auto refEnum = REFEnumStorage.at(JenHash(fl.asClass.typeHash));
      fillIndent(1) << "Values: ";

      for (size_t e = 0; e < refEnum->numMembers; e++) {
        str << refEnum->names[e] << ", ";
      }

      str << std::endl;
    }
  }
}

void APPContext::GetHelp(std::ostream &str) {
  auto ref = RTTI();
  str << ref->className << " settings." << std::endl;
  ::GetHelp(str, ref);
  ::GetHelp(str, ::RTTI(MainSettings()));

  if (info->mode == AppMode_e::EXTRACT) {
    ::GetHelp(str, ::RTTI(ExtractSettings()));
  }
}
