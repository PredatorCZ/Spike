#include "../datas/multi_thread.hpp"
#include "../datas/unit_testing.hpp"
#include <atomic>
#include <random>

int test_mt_thread00() {
  const size_t numTasks = 3;
  size_t pted[numTasks];
  std::atomic_size_t curIndex(0);
  es::print::PrintThreadID(true);

  RunThreadedQueue(numTasks, [&](size_t curTask) {
    printline("Hello thread.");
    auto curID = curIndex.fetch_add(1, std::memory_order::relaxed);
    pted[curID] = curTask;
  });

  for (size_t i = 0; i < numTasks; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(std::begin(pted), std::end(pted), i)));
  }

  es::print::PrintThreadID(false);

  return 0;
}

int test_mt_thread01() {
  const size_t numTasks = 30;
  size_t pted[numTasks];
  std::atomic_size_t curIndex(0);

  RunThreadedQueue(numTasks, [&](size_t curTask) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> randRange(10, 200);
    const auto cVal = randRange(eng);
    std::this_thread::sleep_for(std::chrono::milliseconds(cVal));
    auto curID = curIndex.fetch_add(1, std::memory_order::relaxed);
    pted[curID] = curTask;
  });

  for (size_t i = 0; i < numTasks; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(std::begin(pted), std::end(pted), i)));
  }

  return 0;
}
