#include "context.hpp"
#include "datas/directory_scanner.hpp"
#include <functional>
#include <memory>
#include <set>
#include <variant>

struct MultiThreadManagerImpl;

struct MultiThreadManager {
  using FuncType = std::function<void()>;

  MultiThreadManager(size_t capacity_);
  ~MultiThreadManager();

  void Push(FuncType item);
  void Wait();

private:
  std::unique_ptr<MultiThreadManagerImpl> pi;
};

struct SimpleManager {
  using FuncType = std::function<void()>;

  void Push(FuncType item);
  void Wait(){};
};

struct WorkerManager {
  using FuncType = std::function<void()>;
  std::variant<MultiThreadManager, SimpleManager> man{SimpleManager()};

  WorkerManager(const WorkerManager &) = delete;
  WorkerManager(WorkerManager &&) = delete;

  WorkerManager(size_t capacityPerThread) {
    if (capacityPerThread) {
      man.emplace<MultiThreadManager>(capacityPerThread);
    }
  }

  void Push(FuncType func) {
    std::visit([&](auto &item) { item.Push(std::move(func)); }, man);
  }

  void Wait() {
    std::visit([](auto &item) { item.Wait(); }, man);
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

  Batch(APPContext *ctx_, size_t queueCapacity)
      : ctx(ctx_), manager(queueCapacity) {
    for (auto &c : ctx->info->filters) {
      scanner.AddFilter(c);
    }
  }

  void AddFile(std::string path);

  void FinishBatch();
  void Clean() {
    manager.Wait();
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
  WorkerManager manager{0};
};
