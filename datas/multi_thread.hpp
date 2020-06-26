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
#include <thread>

struct __exampleTraits {
  void RetreiveItem();  // calling working function here
  operator bool();      // true if queue is not finished
  void operator++(int); // increase done queues by 1
  size_t NumQueues() const;
};

template <class Traits> void RunThreadedQueue(Traits &traits) {
  const size_t numQueues = traits.NumQueues();
  const size_t numHWThreads = std::thread::hardware_concurrency();
  const size_t numThreads = std::min(numHWThreads, numQueues);
  using async_type = decltype(&Traits::RetreiveItem);
  using future_type = decltype(std::async(&Traits::RetreiveItem, traits));
  std::vector<future_type> workingThreads(numThreads);

  for (auto &wt : workingThreads) {
    wt = std::async(&Traits::RetreiveItem, traits);
    traits++;
  }

  while (traits) {
    for (auto &wt : workingThreads) {
      if (!traits) {
        break;
      }

      if ((wt.wait_for(std::chrono::milliseconds(2)) ==
           std::future_status::ready)) {
        wt = std::async(&Traits::RetreiveItem, traits);
        traits++;
      }
    }
  }

  for (auto &wt : workingThreads) {
    wt.wait();
  }
}