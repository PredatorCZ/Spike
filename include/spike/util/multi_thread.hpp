/*  Multi Threading manager

    Copyright 2019-2022 Lukas Cone

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
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

// lmBody(size_t index)
template <class lmBody> void RunThreadedQueue(size_t numTasks, lmBody &&fc) {
  const size_t numHWThreads = std::thread::hardware_concurrency();
  const size_t numThreads = std::min(numHWThreads, numTasks);
  size_t curTask = 0;
  using future_type = std::future<decltype(fc(size_t()))>;
  std::vector<future_type> workingThreads(numThreads);

  for (auto &wt : workingThreads) {
    wt = std::async(std::launch::async, fc, curTask);
    curTask++;
  }

  while (curTask < numTasks) {
    for (auto &wt : workingThreads) {
      if (curTask >= numTasks) {
        break;
      }

      if ((wt.wait_for(std::chrono::milliseconds(2)) ==
           std::future_status::ready)) {
        wt = std::async(std::launch::async, fc, curTask);
        curTask++;
      }
    }
  }

  for (auto &wt : workingThreads) {
    wt.wait();
  }
}

// lmBody(size_t index)
template <class lmBody> void RunThreadedQueueEx(size_t numTasks, lmBody &&fc) {
  const size_t numHWThreads = std::thread::hardware_concurrency();
  const size_t numThreads = std::min(numHWThreads, numTasks);
  size_t curTask = 0;
  std::vector<std::pair<std::future<void>, std::thread>> threads(numThreads);

  auto Invoke = [&](size_t index, std::promise<void> &&state) {
    try {
      fc(index);
    } catch (...) {
      state.set_exception(std::current_exception());
    }
    state.set_value();
  };

  auto NewTask = [&](auto &data) {
    auto &[future, thread] = data;
    std::promise<void> state;

    if (future.valid()) {
      future.get();
    }

    if (thread.joinable()) {
      thread.join();
    }

    future = state.get_future();
    thread = std::thread(Invoke, curTask, std::move(state));

    curTask++;
  };

  for (auto &thread : threads) {
    NewTask(thread);
  }

  while (curTask < numTasks) {
    for (auto &thread : threads) {
      auto &[future, _] = thread;
      if (curTask >= numTasks) {
        break;
      }

      if ((future.wait_for(std::chrono::milliseconds(2)) ==
           std::future_status::ready)) {
        NewTask(thread);
      }
    }
  }

  for (auto &[future, thread] : threads) {
    if (future.valid()) {
      future.get();
    }

    if (thread.joinable()) {
      thread.join();
    }
  }
}
