#pragma once
#include "datas/string_view.hpp"
#include "datas/supercore.hpp"
#include <iosfwd>
#include <string>

#ifdef MAIN_APP
#define AC_EXTERN
#else
#define AC_EXTERN ES_EXPORT
#endif

enum class AppMode_e {
  EXTRACT,
  CONVERT,
};

struct ReflectorFriend;
struct AppContextStream;

struct AppInfo_s {
  static constexpr uint32 CONTEXT_VERSION = 1;
  uint32 contextVersion;
  AppMode_e mode;
  es::string_view header;
  ReflectorFriend *settings;
  es::string_view *filters;
};

struct AppContext {
  std::string workingFile;
  virtual ~AppContext() = default;
  virtual AppContextStream RequestFile(const std::string &path) = 0;
  virtual void DisposeFile(std::istream *file) = 0;
  virtual AppContextStream FindFile(const std::string &rootFolder,
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

struct AppExtractContext {
  AppContext *ctx = nullptr;
  virtual ~AppExtractContext() = default;
  virtual void NewFile(const std::string &path) = 0;
  virtual void SendData(es::string_view data) = 0;
  virtual bool RequiresFolders() const = 0;
  virtual void AddFolderPath(const std::string &path) = 0;
  virtual void GenerateFolders() = 0;
};

extern "C" {
void AC_EXTERN AppAdditionalHelp(std::ostream &str, size_t indent);
bool AC_EXTERN AppInitContext(const std::string &dataFolder);
void AC_EXTERN AppProcessFile(std::istream &stream, AppContext *ctx);
void AC_EXTERN AppExtractFile(std::istream &stream, AppExtractContext *ctx);
};
