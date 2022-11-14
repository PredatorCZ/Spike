/*  App context classes for spike modules
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

#pragma once
#include "datas/fileinfo.hpp"
#include <cstring>
#include <functional>
#include <iosfwd>
#include <span>
#include <string>

#ifdef MAIN_APP
#define AC_EXTERN
#else
#define AC_EXTERN ES_EXPORT
#endif

class ReflectorFriend;
struct AppContextStream;
struct AppContextFoundStream;

struct ExtractConf {
  bool makeZIP = true;
  bool folderPerArc = true;
  void ReflectorTag();
};

struct CompressConf {
  uint32 ratioThreshold = 90;
  uint32 minFileSize = 0x80;
  void ReflectorTag();
};

struct MainAppConf {
  CompressConf compressSettings;
  uint8 verbosity = 0;

protected:
  bool generateLog = false;
  ExtractConf extractSettings;
};

struct AppInfo_s {
  static constexpr uint32 CONTEXT_VERSION = 2;
  uint32 contextVersion = CONTEXT_VERSION;
  // No RequestFile or FindFile is being called
  bool filteredLoad = false;
  // AppProcessFile is called by thread
  bool multithreaded = true;
  std::string_view header;
  ReflectorFriend *settings = nullptr;
  std::span<std::string_view> filters{};
  const MainAppConf *internalSettings = nullptr;
};

struct AppExtractContext {
  virtual ~AppExtractContext() = default;
  virtual void NewFile(const std::string &path) = 0;
  virtual void SendData(std::string_view data) = 0;
  virtual bool RequiresFolders() const = 0;
  virtual void AddFolderPath(const std::string &path) = 0;
  virtual void GenerateFolders() = 0;
};

// Every call is multi-threaded
struct AppPackContext {
  virtual ~AppPackContext() = default;
  virtual void SendFile(std::string_view path, std::istream &stream) = 0;
  virtual void Finish() = 0;
};

struct AppPackStats {
  size_t numFiles;
  size_t totalSizeFileNames;
};

struct AppHelpContext {
  virtual ~AppHelpContext() = default;
  virtual std::ostream &GetStream(const std::string &tag) = 0;
};

using request_chunk = std::function<std::string(size_t offset, size_t size)>;

struct AppContextLocator {
  virtual ~AppContextLocator() = default;
  virtual AppContextStream RequestFile(const std::string &path) = 0;
  virtual void DisposeFile(std::istream *file) = 0;
  virtual AppContextFoundStream FindFile(const std::string &rootFolder,
                                         const std::string &pattern) = 0;
};

struct AppContext : AppContextLocator {
  // Path to currently processed file within current filesystem
  AFileInfo workingFile;
  virtual std::istream &GetStream() = 0;
  virtual std::string GetBuffer(size_t size = -1, size_t begin = 0) = 0;
  // Creates context for extraction, can be called only once per context
  virtual AppExtractContext *ExtractContext() = 0;
  // Create new file in system's filesystem
  // Provides single interfce, calling it mutiple times within the same context
  // will cause to close previous stream
  // To make mutiple files in single context, use ExtractContext() instead.
  // path can be relative, use with workingFile
  virtual std::ostream &NewFile(const std::string &path) = 0;

  template <class C> void GetType(C &out, size_t offset = 0) {
    auto buffer = GetBuffer(sizeof(C), offset);
    memcpy(&out, buffer.data(), buffer.size());
  }
};

struct AppContextStream {
  AppContextStream() = default;
  AppContextStream(const AppContextStream &) = delete;
  AppContextStream(AppContextStream &&other)
      : stream(other.stream), ctx(other.ctx) {
    other.ctx = nullptr;
  }
  AppContextStream(std::istream *str, AppContextLocator *ctx_)
      : stream(str), ctx(ctx_) {}
  ~AppContextStream() {
    if (*this) {
      ctx->DisposeFile(stream);
    }
  }

  auto Get() { return stream; }
  // Owner is responsible for disposing by calling AppContext::DisposeFile
  auto Release() {
    ctx = nullptr;
    return stream;
  }

  auto operator->() { return stream; }

  operator bool() const { return stream && ctx; }

  auto &operator=(AppContextStream &&other) {
    stream = other.stream;
    ctx = other.ctx;
    other.ctx = nullptr;
    return *this;
  }

private:
  std::istream *stream = nullptr;
  AppContextLocator *ctx = nullptr;
};

struct AppContextFoundStream : AppContextStream {
  AFileInfo path;
  using AppContextStream::AppContextStream;
  AppContextFoundStream(std::istream *str, AppContextLocator *ctx_,
                        const AFileInfo &workFile)
      : AppContextStream(str, ctx_), path(workFile) {}
};

extern "C" {
AppInfo_s AC_EXTERN *AppInitModule();
void AC_EXTERN AppAdditionalHelp(AppHelpContext *ctx, size_t indent);
bool AC_EXTERN AppInitContext(const std::string &dataFolder);
void AC_EXTERN AppProcessFile(AppContext *ctx);
// Returns total number of files within archive
size_t AC_EXTERN AppExtractStat(request_chunk requester);
AppPackContext AC_EXTERN *AppNewArchive(const std::string &folder,
                                        const AppPackStats &stats);
void AC_EXTERN AppFinishContext();
};
