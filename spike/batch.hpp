#include "context.hpp"
#include "datas/directory_scanner.hpp"
#include <functional>
#include <memory>
#include <set>
#include <variant>

static constexpr bool USE_THREADS = true;

struct MultiThreadManagerImpl;

struct MultiThreadManager {
  using FuncType = std::function<void()>;

  MultiThreadManager(size_t capacity_);
  ~MultiThreadManager();

  void Push(FuncType item);

private:
  std::unique_ptr<MultiThreadManagerImpl> pi;
};

struct SimpleManager {
  using FuncType = std::function<void()>;

  void Push(FuncType item);
};

struct WorkerManager {
  using FuncType = std::function<void()>;
  std::variant<MultiThreadManager, SimpleManager> man{SimpleManager()};

  WorkerManager(const WorkerManager &) = delete;
  WorkerManager(WorkerManager &&) = delete;

  WorkerManager(size_t capacityPerThread) {
    if constexpr (USE_THREADS) {
      man.emplace<MultiThreadManager>(capacityPerThread);
    }
  }

  void Push(FuncType func) {
    std::visit([&](auto &item) { item.Push(std::move(func)); }, man);
  }
};

struct Batch {
  APPContext *ctx;
  std::function<void(const std::string &path, AppPackStats)> forEachFolder;
  std::function<void()> forEachFolderFinish;
  std::function<void(AppContextShare *)> forEachFile;
  bool keepFinishLines = true;

  Batch(const Batch &) = delete;
  Batch(Batch &&) = delete;

  Batch(APPContext *ctx_) : ctx(ctx_) {
    for (auto &c : ctx->info->filters) {
      scanner.AddFilter(c);
    }
  }

  void AddFile(std::string path);

  void FinishBatch();
  void Clean() {
    es::Dispose(rootZips);
    es::Dispose(zips);
    scanner.Clear();
    es::Dispose(forEachFile);
    es::Dispose(forEachFolderFinish);
    es::Dispose(forEachFolder);
    keepFinishLines = true;
  }

private:
  std::set<std::string> rootZips;
  std::map<std::string, PathFilter> zips;
  DirectoryScanner scanner;
  WorkerManager manager{50};
};