#include "../datas/multi_thread.hpp"
#include "../datas/unit_testing.hpp"
#include <atomic>
#include <random>
#include <vector>

int test_mt_thread00() {
  using vt_type = std::vector<size_t>;
  vt_type pted;
  std::atomic<vt_type *> apted(&pted);
  const size_t numTasks = 3;
  pted.reserve(numTasks);

  RunThreadedQueue(numTasks,
                   [&](size_t curTask) { apted.load()->push_back(curTask); });

  for (size_t i = 0; i < numTasks; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(pted.begin(), pted.end(), i)));
  }

  return 0;
}

int test_mt_thread01() {
  using vt_type = std::vector<size_t>;
  vt_type pted;
  std::atomic<vt_type *> apted(&pted);
  const size_t numTasks = 30;
  pted.reserve(numTasks);

  RunThreadedQueue(numTasks, [&](size_t curTask) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> randRange(10, 200);
    const auto cVal = randRange(eng);
    std::this_thread::sleep_for(std::chrono::milliseconds(cVal));
    apted.load()->push_back(curTask);
  });

  for (size_t i = 0; i < numTasks; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(pted.begin(), pted.end(), i)));
  }

  return 0;
}