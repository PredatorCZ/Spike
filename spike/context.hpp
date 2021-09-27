/*  Spike is universal dedicated module handler
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

#pragma once
#include "datas/app_context.hpp"
#include "datas/reflector.hpp"
#include <map>
#include <unordered_map>

extern struct MainAppConf : ReflectorBase<MainAppConf> {
  bool generateLog = false;
} mainSettings;

extern struct MainAppExtractConf : ReflectorBase<MainAppExtractConf> {
  bool makeZIP = true;
  bool folderPerArc = true;
} extractSettings;

class ReflectorFriend : public Reflector {
public:
  using Reflector::GetReflectedInstance;
  using Reflector::GetReflectedType;
  using Reflector::SetReflectedValue;
};

inline auto &MainSettings() {
  return reinterpret_cast<ReflectorFriend &>(mainSettings);
}

inline auto &ExtractSettings() {
  return reinterpret_cast<ReflectorFriend &>(extractSettings);
}

struct APPContextCopyData {
  template <class C> using func = std::add_pointer_t<C>;
  func<decltype(AppAdditionalHelp)> AdditionalHelp;
  func<decltype(AppInitContext)> InitContext;
  func<decltype(AppProcessFile)> ProcessFile;
  func<decltype(AppExtractFile)> ExtractFile;
  AppInfo_s *info;

protected:
  const char *moduleName;
};

struct APPContext : APPContextCopyData {
  APPContext(const char *moduleName_, const std::string &appFolder_);
  APPContext() = default;
  APPContext(const APPContext &) = delete;
  APPContext(APPContext &&other)
      : APPContextCopyData(other), dlHandle(other.dlHandle),
        appFolder(std::move(other.appFolder)) {
    other.dlHandle = nullptr;
  }
  ~APPContext();

  auto &operator=(APPContext &&other) {
    static_cast<APPContextCopyData &>(*this) = other;
    appFolder = std::move(other.appFolder);
    dlHandle = other.dlHandle;
    other.dlHandle = nullptr;
    return *this;
  }

  ReflectorFriend &Settings() { return *info->settings; }
  const ReflectorFriend &Settings() const { return *info->settings; }

  void PrintCLIHelp() const;
  void SetupModule();
  void FromConfig();

private:
  void *dlHandle = nullptr;
  std::string appFolder;

  const reflectorStatic *RTTI() const;
  void CreateLog();
  void GetHelp(std::ostream &str, const reflectorStatic *ref, size_t level = 1);
  void GetHelp(std::ostream &str);
};

struct ZIPIOContext : AppContext {
  using zip_entry = std::pair<size_t, size_t>;

  virtual std::istream *OpenFile(const zip_entry &entry) = 0;

  std::map<std::string, zip_entry> vfs;
  //std::unordered_map<std::string, zip_entry> vfs;
};

struct ZIPIOContextInstance : AppContext {
  ZIPIOContextInstance(ZIPIOContext *base_) : base(base_) {}
  AppContextStream RequestFile(const std::string &path) {
    return base->RequestFile(path);
  }
  void DisposeFile(std::istream *file) { base->DisposeFile(file); }
  AppContextStream FindFile(const std::string &rootFolder,
                            const std::string &pattern) {
    return base->FindFile(rootFolder, pattern);
  }

private:
  ZIPIOContext *base;
};

std::unique_ptr<AppContext> MakeIOContext();
std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file);
