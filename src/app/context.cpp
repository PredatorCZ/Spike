/*  Spike is universal dedicated module handler

    Copyright 2021-2023 Lukas Cone

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

#include "spike/app/context.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/io/directory_scanner.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector_xml.hpp"
#include "spike/type/tchar.hpp"
#include "spike/util/pugiex.hpp"
#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

#if defined(_MSC_VER) || defined(__MINGW64__)
#include "spike/type/tchar.hpp"
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

MainAppConfFriend mainSettings{};
CLISettings cliSettings{};

// NOTE: ReflDesc flags, used for guis and logs
// MAX:n - Maximum number limit (numbers only)
// MIN:n - Minimum number limit (numbers only)
// HIDDEN - Hide text value (strings only)
// FILEPATH - Mark string as filepath for all files
// FILEPATH:<ext0>;<ext1>;<extn> - Mark string as filepath only for specified
//                                 extensions/patterns
// FOLDER - Mark string as folder

REFLECT(CLASS(MainAppConfFriend),
        MEMBERNAME(generateLog, "generate-log", "L",
                   ReflDesc{"Will generate text log of console output inside "
                            "application location."}),
        MEMBER(verbosity, "v",
               ReflDesc{"Prints more information per level.", "MAX:3"}),
        MEMBERNAME(extractSettings, "extract-settings"),
        MEMBERNAME(compressSettings, "compress-settings"),
        MEMBERNAME(texelSettings, "texel-settings"))

REFLECT(CLASS(CLISettings),
        MEMBER(out,
                   ReflDesc{"Output folder for processed files", "FOLDER"}))

REFLECT(
    CLASS(ExtractConf),
    MEMBERNAME(
        folderPerArc, "folder-per-archive", "F",
        ReflDesc{
            "When extracting, create folder that uses input archive's name as "
            "output dir."}),
    MEMBERNAME(makeZIP, "create-zip", "Z",
               ReflDesc{"Pack extracted files inside ZIP file named after "
                        "input archive. Your HDD will thank you."}), )

REFLECT(
    CLASS(CompressConf),
    MEMBERNAME(ratioThreshold, "ratio-threshold", "c",
               ReflDesc{
                   "Writes compressed data only when compression ratio is less "
                   "than specified threshold [0 - 100]%",
                   "MAX:100"}),
    MEMBERNAME(minFileSize, "min-file-size", "m",
               ReflDesc{"Files that are smaller than specified size won't be "
                        "compressed."}), );

REFLECT(ENUMERATION(CubemapFace), ENUM_MEMBER(NONE), ENUM_MEMBER(Right),
        ENUM_MEMBER(Left), ENUM_MEMBER(Up), ENUM_MEMBER(Down),
        ENUM_MEMBER(Front), ENUM_MEMBER(Back));

REFLECT(
    ENUMERATION(TexelContextFormat),
    ENUM_MEMBERDESC(
        DDS_Legacy,
        R"(Use legacy dds container but decode to uncompressed formats if necessary
Supported legacy formats:
  BC1, BC2, BC3, Grayscale, RGB8, RGBA8, RGB565, RGBA4
Other formats will be converted to 8bit channeled format like:
  BC4 -> Grayscale
  BC5 -> RGX8
  BC7 -> RGBA8 or RGB8
Support for mipmaps, cubemaps, volumetrics
snorm will be converted to unorm)"),
    ENUM_MEMBERDESC(DDS,
                    R"(dds, data format is unchanged
PVRTC and ETC will be converted to RGBA8
Support for mipmaps, cubemaps, arrays, volumetrics)"),
    ENUM_MEMBERDESC(QOI_BMP,
                    R"(Decode to Quite OK Image format.
Only for RGX, RGB and RGBA
2 channels will be converted to RGX8
Decode to BMP for P8, P4, Grayscale,
No mipmap support)"),
    ENUM_MEMBERDESC(QOI,
                    R"(Decode to Quite OK Image format.
Normal formats RGB and RGBA are kept instact
2 channels will be converted to RGX8
8bit (P8) and 4bit (P4) palletes will be expanded to RGB or RGBA
Grayscale will be expanded to RGB
No mipmap support)"));

REFLECT(CLASS(TexelConf),
        MEMBERNAME(outputFormat, "output-format",
                   ReflDesc{"Specify output format for images"}),
        MEMBERNAME(cubemapToEquirectangular, "single-cube",
                   ReflDesc{"Convert cubemaps into equirectangular layout"}),
        MEMBERNAME(processMipMaps, "process-mipmaps",
                   ReflDesc{"Save only largest mipmap for each mipmap chain"}))

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

APPContext::APPContext(const char *moduleName_, const std::string &appFolder_,
                       const std::string &appName_)
    : appFolder(appFolder_), appName(appName_) {
  moduleName = moduleName_;

  auto modulePath = [&] {
    DirectoryScanner esmScan;
    esmScan.AddFilter((std::string(1, '^') + moduleName) + "*.spk$");
    esmScan.Scan(appFolder);
    std::vector<VersionHandler> versionedFiles;

    for (auto &f : esmScan) {
      const size_t lastDotPos = f.find_last_of('.');
      const size_t slashPos = f.find_last_of('/');
      std::string_view extension(f.data() + lastDotPos);
      std::string_view fileName(f.data() + slashPos, lastDotPos - slashPos);
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

  if (mainSettings.verbosity > 1) {
    printinfo("Module open: " << modulePath);
  }

#if defined(_MSC_VER) || defined(__MINGW64__)
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
  AppInfo_s *info_ = InitModule();
  info = info_;

  if (info->contextVersion != AppInfo_s::CONTEXT_VERSION) {
    throw std::runtime_error("Module context version mismatch!");
  }

  info_->internalSettings = &mainSettings;

  tryAssign(AdditionalHelp, "AppAdditionalHelp");
  tryAssign(InitContext, "AppInitContext");
  tryAssign(FinishContext, "AppFinishContext");
  tryAssign(NewArchive, "AppNewArchive");
  tryAssign(ProcessFile, "AppProcessFile");
  tryAssign(ExtractStat, "AppExtractStat");

  if (NewArchive && ProcessFile) {
    throw std::logic_error("Module uses 2 or more contexts!");
  }
}

APPContext::APPContext(APPContext &&other)
    : APPContextCopyData(other), appFolder(std::move(other.appFolder)),
      appName(std::move(other.appName)) {
  if (dlHandle && dlHandle != other.dlHandle) {
    dlclose(dlHandle);
  }
  dlHandle = other.dlHandle;
  std::construct_at(&other);
}

APPContext &APPContext::operator=(APPContext &&other) {
  static_cast<APPContextCopyData &>(*this) = other;
  appFolder = std::move(other.appFolder);
  appName = std::move(other.appName);
  if (dlHandle && dlHandle != other.dlHandle) {
    dlclose(dlHandle);
  }
  dlHandle = other.dlHandle;
  std::construct_at(&other);
  return *this;
}

APPContext::~APPContext() {
  if (dlHandle) {
    dlclose(dlHandle);
  }
}

class ReflectorFriend : public Reflector {
public:
  using Reflector::GetReflectedInstance;
  using Reflector::GetReflectedType;
  using Reflector::SetReflectedValue;
};

static auto &MainSettings() {
  static ReflectorWrap<MainAppConfFriend> wrap(mainSettings);
  return reinterpret_cast<ReflectorFriend &>(wrap);
}

static auto &ExtractSettings() {
  static ReflectorWrap<ExtractConf> wrap(mainSettings.extractSettings);
  return reinterpret_cast<ReflectorFriend &>(wrap);
}

static auto &CompressSettings() {
  static ReflectorWrap<CompressConf> wrap(mainSettings.compressSettings);
  return reinterpret_cast<ReflectorFriend &>(wrap);
}

static auto &TexelSettings() {
  static ReflectorWrap<TexelConf> wrap(mainSettings.texelSettings);
  return reinterpret_cast<ReflectorFriend &>(wrap);
}

static auto &CliSettings() {
  static ReflectorWrap<CLISettings> wrap(cliSettings);
  return reinterpret_cast<ReflectorFriend &>(wrap);
}

static const reflectorStatic *RTTI(const ReflectorFriend &ref) {
  auto rawRTTI = ref.GetReflectedInstance();
  return static_cast<const ReflectedInstanceFriend &>(rawRTTI).Refl();
}

void APPContext::ResetSwitchSettings() {
  if (info->settings) {
    const size_t numValues = Settings().GetNumReflectedValues();
    for (size_t i = 0; i < numValues; i++) {
      auto rType = Settings().GetReflectedType(i);

      if (rType->type == REFType::Bool) {
        Settings().SetReflectedValue(i, "false");
      }
    }
  }

  mainSettings.generateLog = mainSettings.extractSettings.folderPerArc =
      mainSettings.extractSettings.makeZIP = false;
}

int APPContext::ApplySetting(std::string_view key, std::string_view value) {
  JenHash keyHash(key);
  ReflectorFriend *refl = nullptr;
  const ReflType *rType = nullptr;
  static ReflectorFriend *settings[]{
      /**/ //
      &Settings(),
      &MainSettings(),
      &CliSettings(),
      &TexelSettings(),
      &ExtractSettings(),
      &CompressSettings(),
  };

  for (auto s : settings) {
    if (!s) {
      continue;
    }

    rType = s->GetReflectedType(keyHash);
    if (rType) {
      refl = s;
      break;
    }
  }

  if (rType) {
    if (rType->type == REFType::Bool) {
      refl->SetReflectedValue(*rType, "true");
      return 0;
    } else {
      refl->SetReflectedValue(*rType, value);
      return 1;
    }
  } else {
    printerror("Invalid option: " << (key.size() > 1 ? "--" : "-") << key);
    return -1;
  }
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
  printStuff(::RTTI(TexelSettings()));

  if (ProcessFile) {
    printStuff(::RTTI(ExtractSettings()));
  } else if (NewArchive) {
    printStuff(::RTTI(CompressSettings()));
  }

  if (info->settings) {
    printStuff(RTTI());
  }
  printline("");
}

void DumpTypeMD(std::ostream &out, const ReflectorFriend &info,
                size_t indent = 0) {
  auto rtti = RTTI(info);

  auto gi = [&]() -> std::ostream & {
    static const char indents[]{"                "};
    return out << indents + (8 - indent) * 2;
  };

  for (size_t i = 0; i < rtti->nTypes; i++) {
    gi() << "- **" << rtti->typeNames[i] << "**\n\n";

    if (info.IsReflectedSubClass(i)) {
      auto sub = info.GetReflectedSubClass(i);
      ReflectorPureWrap subRef(sub);
      DumpTypeMD(
          out, static_cast<ReflectorFriend &>(static_cast<Reflector &>(subRef)),
          indent + 1);
      continue;
    }

    gi() << "  **CLI Long:** ***--" << rtti->typeNames[i] << "***\\\n";

    if (rtti->typeAliases && rtti->typeAliases[i]) {
      gi() << "  **CLI Short:** ***-" << rtti->typeAliases[i] << "***\n\n";
    }

    if (auto val = info.GetReflectedValue(i); !val.empty()) {
      gi() << "  **Default value:** " << val << "\n\n";
    }

    if (auto &rType = rtti->types[i]; rType.type == REFType::Enum) {
      auto refEnum =
          ReflectedEnum::Registry().at(JenHash(rType.asClass.typeHash));
      gi() << "  **Valid values:** ";

      if ([&] {
            for (size_t e = 0; e < refEnum->numMembers; e++) {
              if (refEnum->descriptions && refEnum->descriptions[e]) {
                return true;
              }
            }
            return false;
          }()) {
        out << "\n\n";
        indent++;

        for (size_t e = 0; e < refEnum->numMembers; e++) {
          gi() << "- " << refEnum->names[e];
          if (std::string_view desc(refEnum->descriptions[e]);
              desc.size() > 0) {
            size_t curLine = 0;
            size_t nextLine = desc.find('\n');
            out << ':';

            if (nextLine != desc.npos) {
              out << '\n';
              while (nextLine != desc.npos) {
                gi() << desc.substr(curLine, nextLine + 1 - curLine);
                curLine = nextLine + 1;
                nextLine = desc.find('\n', curLine);
              }
            } else {
              out << ' ' << desc << "\n\n";
            }

            out << "\n\n";
          } else {
            out << ", "
                << "\n\n";
          }
        }
        indent--;
      } else {
        for (size_t e = 0; e < refEnum->numMembers; e++) {
          out << refEnum->names[e] << ", ";
        }
      }

      out.seekp(out.tellp() - std::streamoff(2));
      out << "\n\n";
    }

    if (auto desc = rtti->typeDescs[i].part1; desc) {
      gi() << "  " << rtti->typeDescs[i].part1 << "\n\n";
    }
  }
}

void APPContext::GetMarkdownDoc(std::ostream &out, pugi::xml_node node) const {
  const char *className = "[[MODULE CLASS NAME]]";
  const char *description = "[[MODULE DESCRIPTION]]";

  if (info->settings) {
    className = RTTI()->className;
  }

  if (node) {
    if (auto child = node.attribute("name"); child) {
      className = child.as_string();
    }
    description = node.text().as_string();
  }

  out << "## " << className << "\n\n### Module command: " << moduleName
      << "\n\n"
      << description << "\n\n";

  if (!info->settings) {
    return;
  }

  out << "### Settings\n\n";

  DumpTypeMD(out, Settings());
}

void APPContext::SetupModule() {
  if (mainSettings.generateLog) {
    CreateLog();
  }

  if (InitContext && !InitContext(appFolder + "data/")) {
    throw std::runtime_error("Error while initializing context.");
  }
}

using stream_type = BinWritter_t<BinCoreOpenMode::Text>;
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
      std::string_view desc2;

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
  PrintStuff(TexelSettings());
  if (ProcessFile) {
    PrintStuff(ExtractSettings());
  } else if (NewArchive) {
    PrintStuff(CompressSettings());
  }

  if (info->settings) {
    PrintStuff(Settings());
  }

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
    std::string_view elName = ref->typeNames[i];
    auto elDesc = ref->typeDescs[i];
    fillIndent() << elName << std::endl;

    if (elDesc.part1) {
      fillIndent(1) << elDesc.part1 << std::endl;
    }

    auto fl = ref->types[i];

    if (fl.type == REFType::Class || fl.type == REFType::BitFieldClass) {
      GetHelp(str, reflectorStatic::Registry().at(JenHash(fl.asClass.typeHash)),
              level + 1);
    } else if (fl.type == REFType::Array || fl.type == REFType::ArrayClass) {
      const auto &arr = fl.asArray;

      if (arr.type == REFType::Class || arr.type == REFType::BitFieldClass) {
        GetHelp(str,
                reflectorStatic::Registry().at(JenHash(arr.asClass.typeHash)),
                level + 1);
      }
    } else if (fl.type == REFType::Enum) {
      auto refEnum = ReflectedEnum::Registry().at(JenHash(fl.asClass.typeHash));
      fillIndent(1) << "Values: ";

      if ([&] {
            for (size_t e = 0; e < refEnum->numMembers; e++) {
              if (refEnum->descriptions && refEnum->descriptions[e]) {
                return true;
              }
            }
            return false;
          }()) {
        str << std::endl;

        for (size_t e = 0; e < refEnum->numMembers; e++) {
          fillIndent(2) << refEnum->names[e];
          if (refEnum->descriptions[e]) {
            if (std::string_view desc(refEnum->descriptions[e]);
                desc.size() > 0) {
              size_t curLine = 0;
              size_t nextLine = desc.find('\n');
              str << ':';

              if (nextLine != desc.npos) {
                str << '\n';
                while (nextLine != desc.npos) {
                  fillIndent(3) << desc.substr(curLine, nextLine + 1 - curLine);
                  curLine = nextLine + 1;
                  nextLine = desc.find('\n', curLine);
                }
              } else {
                str << ' ' << desc << std::endl;
              }

              str << std::endl;
            }
          } else {
            str << ", " << std::endl;
          }
        }
      } else {
        for (size_t e = 0; e < refEnum->numMembers; e++) {
          str << refEnum->names[e] << ", ";
        }
        str << std::endl;
      }
    }
  }
}

void APPContext::GetHelp(std::ostream &str) {
  str << moduleName << " settings." << std::endl;
  ::GetHelp(str, RTTI());
}

struct AppHelpContextImpl : AppHelpContext {
  std::map<std::string, std::stringstream> tagBuffers;

  std::ostream &GetStream(const std::string &tag) override {
    return tagBuffers[tag] = std::stringstream{};
  }
};

void APPContext::FromConfig() {
  auto configName = (appFolder + appName) + ".config";
  pugi::xml_document doc = {};

  auto TryFile = [](auto cb) {
    constexpr size_t numTries = 10;
    size_t curTry = 0;

    for (; curTry < numTries; curTry++) {
      try {
        cb();
      } catch (const es::FileNotFoundError &) {
      } catch (const es::FileInvalidAccessError &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        continue;
      }

      break;
    }

    if (curTry == numTries) {
      throw std::runtime_error("Cannot access config. File is locked.");
    }
  };

  TryFile([&] {
    auto flags = XMLDefaultParseFlags;
    flags += XMLParseFlag::Comments;
    doc = XMLFromFile(configName, flags);
    ReflectorXMLUtil::LoadV2(MainSettings(), doc.child("common"));

    if (info->settings) {
      ReflectorXMLUtil::LoadV2(Settings(), doc.child(moduleName));
    }
  });

  {
    {
      AppHelpContextImpl helpCtx;

      if (auto commentNode = doc.find_child([](pugi::xml_node &node) {
            return node.type() == pugi::xml_node_type::node_comment &&
                   std::string_view(node.value()).starts_with("common");
          });
          commentNode) {
        std::string_view comment(commentNode.value());
        std::string_view lastTag;
        size_t lastPos = 0;

        while (true) {
          lastPos = comment.find("<-tag:", lastPos);

          if (!lastTag.empty()) {
            std::string tagName(lastTag);
            lastTag.substr(0, comment.size());
            size_t dataBegin = lastTag.find_first_of('\n');

            if (dataBegin != lastTag.npos) {
              dataBegin++;
              const size_t dataEnd =
                  lastPos != lastTag.npos ? lastPos : lastTag.size();
              helpCtx.GetStream(tagName)
                  << lastTag.substr(dataBegin, dataEnd - dataBegin);
            }

            lastTag = {};
          }

          if (lastPos == comment.npos) {
            break;
          }

          const size_t tagBegin = lastPos += 6;

          lastPos = comment.find("->", lastPos);

          if (lastPos == comment.npos) {
            break;
          }

          auto tagName = comment.substr(tagBegin, lastPos - tagBegin);
          lastTag = es::TrimWhitespace(tagName);
        }

        doc.remove_child(commentNode);
      }

      std::stringstream str;
      str << "common settings." << std::endl;
      ::GetHelp(str, ::RTTI(MainSettings()));
      AdditionalHelp(&helpCtx, 1);

      for (auto &[tag, data] : helpCtx.tagBuffers) {
        str << "\t<-tag: " << tag << "->\n" << data.str();
      }

      auto buff = std::move(str).str();
      pugi::xml_node commonNode;
      auto commentNode = doc.append_child(pugi::node_comment);
      commentNode.set_value(buff.data());

      if (commonNode = doc.child("common"); commonNode) {
        doc.insert_move_after(commonNode, commentNode);
      } else {
        commonNode = doc.append_child("common");
      }
      ReflectorXMLUtil::SaveV2a(MainSettings(), commonNode,
                                ReflectorXMLUtil::Flags_StringAsAttribute);
    }

    if (info->settings) {
      if (auto commentNode = doc.find_child([&](pugi::xml_node &node) {
            return node.type() == pugi::xml_node_type::node_comment &&
                   std::string_view(node.value()).starts_with(moduleName);
          });
          commentNode) {
        doc.remove_child(commentNode);
      }

      std::stringstream str;
      GetHelp(str);
      auto buff = std::move(str).str();
      pugi::xml_node node;
      auto commentNode = doc.append_child(pugi::node_comment);
      commentNode.set_value(buff.data());

      if (node = doc.child(moduleName); node) {
        doc.insert_move_after(node, commentNode);
      } else {
        node = doc.append_child(moduleName);
      }
      ReflectorXMLUtil::SaveV2a(Settings(), node,
                                {ReflectorXMLUtil::Flags_StringAsAttribute});
    }

    TryFile([&] {
      XMLToFile(configName, doc,
                {XMLFormatFlag::WriteBOM, XMLFormatFlag::IndentAttributes});
    });
  }
}
