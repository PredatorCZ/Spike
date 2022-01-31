/*  Spike is universal dedicated module handler
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

#include "datas/string_view.hpp"
#include <atomic>
#include <memory>

struct LogLine {
  virtual void PrintLine() = 0;
  virtual ~LogLine() = default;
};

struct CounterLine {
  CounterLine &operator++(int) {
    curitem.fetch_add(1, std::memory_order_relaxed);
    return *this;
  }

protected:
  std::atomic_size_t curitem{0};
};

struct ProgressBar : CounterLine, LogLine {
  ProgressBar(es::string_view label_) : label(label_) {}

  void PrintLine() override;

  void ItemCount(size_t numItems, size_t done = 0) {
    curitem = done;
    if (numItems) {
      itemDelta.store(1.f / numItems);
    } else {
      itemDelta.store(0.f);
    }
  }

protected:
  std::atomic<float> itemDelta;
  es::string_view label;
};

struct DetailedProgressBar : ProgressBar {
  using ProgressBar::ProgressBar;
  void PrintLine() override;
};

struct LoadingBar : LogLine {
  LoadingBar(es::string_view payload_) : payload(payload_) {}

  void PrintLine() override;

  void Finish(bool failed = false) { state = failed ? 2 : 1; }

private:
  std::atomic_int8_t state{0};
  size_t innerTick = 0;

protected:
  es::string_view payload;
};

void InitConsole();
void TerminateConsole();

// 0 = White text [default]
// 1 = Colored text
// 2 = Print Threads
// 3 = Colored text and print threads
void ConsolePrintDetail(uint8_t detail);

struct ElementAPI {
  void Append(std::unique_ptr<LogLine> &&item);
  // Only remove pointed line
  void Remove(LogLine *item);
  // Remove pointed line and print it's final result into text flow
  void Release(LogLine *line);

  void Insert(std::unique_ptr<LogLine> &&item, LogLine *where,
              bool after = true);

  void Clean();
};

using element_callback = void (*)(void *userData, ElementAPI &api);

void ModifyElements(void *userData, element_callback cb);

template <class C, class... Args> C *AppendNewLogLine(Args &&...args) {
  auto newLine = std::make_unique<C>(std::forward<Args>(args)...);
  auto newLineRaw = newLine.get();

  ModifyElements(&newLine, [](void *data, ElementAPI &api) {
    auto scanData = reinterpret_cast<decltype(newLine) *>(data);
    api.Append(std::move(*scanData));
  });

  return newLineRaw;
}

inline void ReleaseLogLine(LogLine *item) {
  ModifyElements(item, [](void *data, ElementAPI &api) {
    api.Release(static_cast<LogLine *>(data));
  });
}

template <class... Lines> void RemoveLogLines(Lines *...item) {
  auto payload = std::array<LogLine *, sizeof...(item)>{item...};
  ModifyElements(&payload, [](void *data, ElementAPI &api) {
    for (auto d : *static_cast<decltype(payload) *>(data)) {
      api.Remove(d);
    }
  });
}

inline void RemoveLogLine(LogLine *item) {
  ModifyElements(item, [](void *data, ElementAPI &api) {
    api.Remove(static_cast<LogLine *>(data));
  });
}
