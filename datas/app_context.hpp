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
#include "datas/string_view.hpp"
#include "datas/supercore.hpp"
#include <functional>
#include <iosfwd>
#include <string>

#ifdef MAIN_APP
#define AC_EXTERN
#else
#define AC_EXTERN ES_EXPORT
#endif

enum class AppMode_e : uint8 {
  EXTRACT,
  CONVERT,
  PACK,
};

// Archive only (ZIP,) load only filtered entries or load all entries.
enum class ArchiveLoadType : uint8 {
  FILTERED,
  ALL,
};

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
  static constexpr uint32 CONTEXT_VERSION = 1;
  uint32 contextVersion;
  AppMode_e mode;
  ArchiveLoadType arcLoadType;
  es::string_view header;
  ReflectorFriend *settings = nullptr;
  es::string_view *filters = nullptr;
  const MainAppConf *internalSettings = nullptr;
};

struct AppContext {
  // Used with ZIP to separate zip and system filesystem
  // Used only for locating additional files (RequestFile, FindFile)
  std::string workingFile;
  // Used only in CONVERT mode, represents presumed file location within system
  // filesystem
  std::string outFile;
  virtual ~AppContext() = default;
  virtual AppContextStream RequestFile(const std::string &path) = 0;
  virtual void DisposeFile(std::istream *file) = 0;
  virtual AppContextFoundStream FindFile(const std::string &rootFolder,
                                         const std::string &pattern) = 0;
};

struct AppContextStream {
  AppContextStream() = default;
  AppContextStream(const AppContextStream &) = delete;
  AppContextStream(AppContextStream &&other)
      : stream(other.stream), ctx(other.ctx) {
    other.ctx = nullptr;
  }
  AppContextStream(std::istream *str, AppContext *ctx_)
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
  AppContext *ctx = nullptr;
};

struct AppContextFoundStream : AppContextStream {
  std::string workingFile;
  using AppContextStream::AppContextStream;
  AppContextFoundStream(std::istream *str, AppContext *ctx_,
                        const std::string &workFile)
      : AppContextStream(str, ctx_), workingFile(workFile) {}
};

struct AppExtractContext {
  AppContext *ctx = nullptr;
  virtual ~AppExtractContext() = default;
  virtual void NewFile(const std::string &path) = 0;
  virtual void SendData(es::string_view data) = 0;
  virtual bool RequiresFolders() const = 0;
  virtual void AddFolderPath(const std::string &path) = 0;
  virtual void GenerateFolders() = 0;
};

// Every call is multi-threaded
struct AppPackContext {
  virtual ~AppPackContext() = default;
  virtual void SendFile(es::string_view path, std::istream &stream) = 0;
  virtual void Finish() = 0;
};

struct AppPackStats {
  size_t numFiles;
  size_t totalSizeFileNames;
};

using request_chunk = std::function<std::string(size_t offset, size_t size)>;

extern "C" {
const AppInfo_s AC_EXTERN *AppInitModule();
void AC_EXTERN AppAdditionalHelp(std::ostream &str, size_t indent);
bool AC_EXTERN AppInitContext(const std::string &dataFolder);
void AC_EXTERN AppProcessFile(std::istream &stream, AppContext *ctx);
void AC_EXTERN AppExtractFile(std::istream &stream, AppExtractContext *ctx);
// Returns total number of files within archive
size_t AC_EXTERN AppExtractStat(request_chunk requester);
AppPackContext AC_EXTERN *AppNewArchive(const std::string &folder,
                                        const AppPackStats &stats);
void AC_EXTERN AppFinishContext();
};
