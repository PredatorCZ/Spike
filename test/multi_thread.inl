#include "../datas/multi_thread.hpp"
#include "../datas/unit_testing.hpp"
#include <atomic>
#include <random>
#include <vector>

int test_mt_thread00() {
  using vt_type = std::vector<size_t>;
  typename vt_type pted;
  std::atomic<typename vt_type *> apted(&pted);

  struct {
    size_t numQ;
    size_t curQ = 0;
    std::atomic<typename vt_type *> *atom;

    void RetreiveItem() { atom->load()->push_back(curQ); }
    operator bool() { return curQ < numQ; }
    void operator++(int) { curQ++; }
    size_t NumQueues() const { return numQ; }
  } queue;

  queue.numQ = 3;
  queue.atom = &apted;

  RunThreadedQueue(queue);

  for (size_t i = 0; i < queue.numQ; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(pted.begin(), pted.end(), i)));
  }

  return 0;
}

int test_mt_thread01() {
  using vt_type = std::vector<size_t>;
  typename vt_type pted;
  std::atomic<typename vt_type *> apted(&pted);

  struct {
    size_t numQ;
    size_t curQ = 0;
    std::atomic<typename vt_type *> *atom;

    size_t RetreiveItem() {
      std::random_device rd;
      std::default_random_engine eng(rd());
      std::uniform_int_distribution<int> randRange(10, 200);
      const auto cVal = randRange(eng);
      std::this_thread::sleep_for(std::chrono::milliseconds(cVal));
      atom->load()->push_back(curQ);
      return curQ;
    }
    operator bool() { return curQ < numQ; }
    void operator++(int) { curQ++; }
    size_t NumQueues() const { return numQ; }
  } queue;

  queue.numQ = 30;
  queue.atom = &apted;

  RunThreadedQueue(queue);

  for (size_t i = 0; i < queue.numQ; i++) {
    TEST_NOT_CHECK(es::IsEnd(pted, std::find(pted.begin(), pted.end(), i)));
  }

  return 0;
}