/*  Multi Threading manager

    Copyright 2019-2020 Lukas Cone

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
#include <vector>
#include <thread>

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
