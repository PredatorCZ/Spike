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

#pragma once
#include "cache.hpp"
#include "spike/app/texel.hpp"
#include "spike/crypto/jenkinshash.hpp"
#include "spike/util/pugi_fwd.hpp"
#include <map>
#include <mutex>
#include <optional>

class PathFilter;
struct reflectorStatic;
class ReflectorFriend;

struct MainAppConfFriend : MainAppConf {
  using MainAppConf::extractSettings;
  using MainAppConf::generateLog;
  TexelConf texelSettings;
};

extern struct MainAppConfFriend mainSettings;

struct CLISettings {
  std::string out;
};

extern struct CLISettings cliSettings;

template <class func> class APPOptionalCall {
public:
  using value_type = func;
  APPOptionalCall() = default;
  APPOptionalCall(const APPOptionalCall &) = default;
  APPOptionalCall(APPOptionalCall &&) = default;
  APPOptionalCall &operator=(const APPOptionalCall &) = default;
  APPOptionalCall &operator=(APPOptionalCall &&) = default;
  APPOptionalCall(func obj) : object(obj) {}
  APPOptionalCall &operator=(func obj) {
    object = obj;
    return *this;
  }

  operator bool() const { return object != nullptr; }
  template <class... C> auto operator()(C &&...params) {
    using return_type = decltype(object(std::forward<C>(params)...));

    if constexpr (!std::is_void_v<return_type>) {
      if (object) {
        return object(std::forward<C>(params)...);
      }
      return return_type{};
    } else {
      if (object) {
        object(std::forward<C>(params)...);
      }
    }
  }

private:
  func object = nullptr;
};

struct APPContextCopyData {
  template <class C> using func = std::add_pointer_t<C>;
  template <class C> using opt_func = APPOptionalCall<std::add_pointer_t<C>>;
  opt_func<decltype(AppProcessFile)> ProcessFile;
  opt_func<decltype(AppExtractStat)> ExtractStat;
  opt_func<decltype(AppNewArchive)> NewArchive;
  opt_func<decltype(AppFinishContext)> FinishContext;
  opt_func<decltype(AppAdditionalHelp)> AdditionalHelp;
  const AppInfo_s *info = nullptr;

protected:
  const char *moduleName;
  opt_func<decltype(AppInitContext)> InitContext;
};

struct APPContext : APPContextCopyData {
  APPContext(const char *moduleName_, const std::string &appFolder_,
             const std::string &appName_);
  APPContext() = default;
  APPContext(const APPContext &) = delete;
  APPContext(APPContext &&other);
  ~APPContext();

  APPContext &operator=(APPContext &&other);

  ReflectorFriend &Settings() { return *info->settings; }
  const ReflectorFriend &Settings() const { return *info->settings; }

  void PrintCLIHelp() const;
  void SetupModule();
  void FromConfig();
  void ResetSwitchSettings();
  std::string GetClassName(pugi::xml_node node) const;
  void GetMarkdownDoc(std::ostream &out, pugi::xml_node node) const;
  int ApplySetting(std::string_view key, std::string_view value);

private:
  void *dlHandle = nullptr;
  std::string appFolder;
  std::string appName;

  const reflectorStatic *RTTI() const;
  void CreateLog();
  void GetHelp(std::ostream &str, const reflectorStatic *ref, size_t level = 1);
  void GetHelp(std::ostream &str);
};

struct ZIPIOEntryIterator {
  ZIPIOEntryRawIterator &base;
  ZIPIOEntry current;
  std::mutex iterMtx;

  ZIPIOEntry operator++() {
    std::lock_guard<std::mutex> lg(iterMtx);
    auto retVal = current;
    current = base.Next();
    return retVal;
  }
  ZIPIOEntry operator++(int) { return operator++(); }
  ZIPIOEntry operator*() const { return current; }
  bool operator!=(const ZIPIOEntry &) { return current; }
};

struct ZIPIOContextIterator {
  std::unique_ptr<ZIPIOEntryRawIterator> base;
  ZIPIOEntryIterator begin() const { return {*base.get(), base->Fist(), {}}; }
  ZIPIOEntry end() const { return {}; }
};

struct AppContextShare : AppContext {
  virtual void BaseOutputPath(std::string basePath_) = 0;
  virtual void Finish() = 0;
  virtual JenHash Hash() = 0;
  virtual std::string FullPath() = 0;
  const std::vector<std::string> &SupplementalFiles() override;
  std::function<void()> forEachFile;
  std::optional<std::vector<std::string>> supplementals;
};

struct ZIPExtactContext;

struct ZIPIOContext : AppContextLocator {
  virtual std::istream *OpenFile(const ZipEntry &entry) = 0;
  virtual ZIPIOContextIterator
      Iter(ZIPIOEntryType = ZIPIOEntryType::String) const = 0;
  virtual std::string GetChunk(const ZipEntry &entry, size_t offset,
                               size_t size) const = 0;
  virtual std::shared_ptr<AppContextShare> Instance(ZIPIOEntry entry);
  virtual void Merge(ZIPExtactContext *eCtx, const std::string &records) = 0;
  virtual void InitMerger() = 0;
  virtual void Finish() = 0;
  const std::vector<std::string> &SupplementalFiles() override;
  std::string basePath;
  std::optional<std::vector<std::string>> supplementals;
};

std::shared_ptr<AppContextShare> MakeIOContext(
    const std::string &path,
    std::optional<std::vector<std::string>> supplementals = std::nullopt);
std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file,
                                             const PathFilter &pathFilter,
                                             const PathFilter &moduleFilter);
std::unique_ptr<ZIPIOContext> MakeZIPContext(const std::string &file);
