/*  Spike is universal dedicated module handler

    Copyright 2021-2023 Lukas Cone

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

#include "spike/app/console.hpp"
#include "spike/master_printer.hpp"
#include "spike/type/tchar.hpp"
#include <algorithm>
#include <csignal>
#include <mutex>
#include <thread>
#include <vector>

const size_t nextTickMS = 100;

const char8_t *loopchars[] = {
    u8"\u2807", u8"\u280B", u8"\u2819", u8"\u2838", u8"\u2834", u8"\u2826",
};

const char8_t *barchars[] = {
    u8"\u2801", u8"\u2803", u8"\u2807", u8"\u2847",
    u8"\u284F", u8"\u285F", u8"\u287F", u8"\u28FF",
};

void ProgressBar::PrintLine() {
  const size_t width = 50;
  const float normState = std::min(curitem * itemDelta, 1.f);
  const size_t state = normState * width;
  es::Print(label.data());
  es::Print("\033[38;2;168;204;140m");

  for (size_t i = 0; i < state; i++) {
#ifdef USEWIN
    es::Print("#");
#else
    es::Print(u8"\u25A0");
#endif
  }

  for (size_t i = state; i < width; i++) {
    es::Print(" ");
  }

  char percBuffer[16]{};
  snprintf(percBuffer, sizeof(percBuffer), "\033[0m %3u%% ",
           uint32(normState * 100));
  es::Print(percBuffer);
}

void DetailedProgressBar::PrintLine() {
  const size_t goal = curitem;
  const size_t width = 50;
  const size_t parts = 8;
  const float normStateGoal = std::min(goal * itemDelta, 1.f);
  float normState = normStateGoal;

  if (lastItem < normStateGoal) {
    lastItem += (normStateGoal - lastItem) / 5;
    normState = lastItem;
  } else {
    lastItem = normStateGoal;
  }

  const size_t stateMacro = normState * width;
  const size_t state = size_t(normState * width * parts) % parts;

  es::Print(label.data());
  es::Print("\033[38;2;168;204;140m");

  for (size_t i = 0; i < stateMacro; i++) {
    es::Print(barchars[7]);
  }

  if (state) {
    es::Print(barchars[state]);
  }

  for (size_t i = stateMacro + bool(state); i < width; i++) {
    es::Print(" ");
  }

  char percBuffer[16]{};
  snprintf(percBuffer, sizeof(percBuffer), "\033[0m %3u%% ",
           uint32(normState * 100));
  es::Print(percBuffer);
}

void LoadingBar::PrintLine() {
  if (!state) {
    const size_t loopTick = (innerTick / 100) % 6;
    es::Print(loopchars[loopTick]);
    es::Print(" ");
  } else if (state == 1) {
    es::Print(u8"\033[38;2;168;220;140m\u2714 \033[0m");
  } else {
    es::Print(u8"\u274C ");
  }

  es::Print(payload.data());
  innerTick += nextTickMS;
}

struct LineMsg {
  std::unique_ptr<LogLine> newLine;
  LogLine *line = nullptr;
  bool after = false;

  LineMsg(std::unique_ptr<LogLine> &&item) : newLine(std::move(item)) {}
  LineMsg(std::unique_ptr<LogLine> &&item, LogLine *point, bool after_)
      : newLine(std::move(item)), line(point), after(after_) {}
  LineMsg(LogLine *point) : line(point) {}
  LineMsg(LogLine *point, bool release) : line(point), after(release) {}
};

static std::vector<es::print::Queuer> messageQueues[2];
static std::vector<std::unique_ptr<LogLine>> lineStack;
static std::vector<LineMsg> lineQueue[2];
static std::atomic_bool currentlyUsedMessageQueue;
static std::atomic_bool currentlyUsedLineQueue;
static std::atomic_bool stopLogger;
static std::atomic_bool eraseAllLines;
static std::thread logger;
static std::atomic_uint8_t printDetail{0};
static std::atomic_uint64_t newPrintDetailSince{0};

void ReceiveQueue(const es::print::Queuer &que) {
  messageQueues[currentlyUsedMessageQueue].push_back(que);
}

void MakeLogger() {
  size_t innerTick = 0;
  currentlyUsedMessageQueue = false;
  currentlyUsedLineQueue = false;
  bool mustClear = false;

  while (!stopLogger) {
    if (mustClear) {
      es::Print("\033[J");
      mustClear = false;
    }

    if (bool mqo = currentlyUsedMessageQueue; !messageQueues[mqo].empty()) {
      currentlyUsedMessageQueue = !mqo;
      uint8 fullDetail = printDetail;
      uint64 newDetailSince = newPrintDetailSince.exchange(0);
      size_t curQue = 0;

      for (auto &l : messageQueues[mqo]) {
        uint8 detail = newDetailSince <= curQue ? fullDetail : fullDetail >> 4;
        bool colored = detail & 1;
        if (colored) {
          using es::print::MPType;
          switch (l.type) {
          case MPType::ERR:
            es::Print(u8"\033[38;2;255;50;50m\u26D4 ");
            break;
          case MPType::INF:
            es::Print(u8"\033[38;2;50;120;255m \u2139 ");
            break;
          case MPType::WRN:
            es::Print(u8"\033[38;2;255;255;50m\u26A0  ");
            break;
          default:
            es::Print(u8"\033[0m   ");
            colored = false;
            break;
          }
        }

        if (detail & 2) {
          char threadBuffer[16]{};
          snprintf(threadBuffer, sizeof(threadBuffer), "[0x%.8X] ", l.threadId);
          es::Print(threadBuffer);
        }

        es::Print(l.payload.data());

        if (colored) {
          es::Print(u8"\033[0m");
        }

        curQue++;
      }

      messageQueues[mqo].clear();
    }

    if (eraseAllLines) {
      lineQueue[0].clear();
      lineQueue[1].clear();
      lineStack.clear();
      eraseAllLines = false;
    }

    if (bool lqo = currentlyUsedLineQueue; !lineQueue[lqo].empty()) {
      currentlyUsedLineQueue = !lqo;

      for (auto &l : lineQueue[lqo]) {
        if (l.line) {
          if (l.newLine) {
            auto found = std::find_if(
                lineStack.begin(), lineStack.end(),
                [&](auto &item_) { return item_.get() == l.line; });
            lineStack.insert(std::next(found, l.after), std::move(l.newLine));
          } else {
            if (l.after) {
              l.line->PrintLine();
              es::Print("\n");
            }

            lineStack.erase(std::find_if(
                lineStack.begin(), lineStack.end(),
                [&](auto &item_) { return item_.get() == l.line; }));
          }
        } else {
          lineStack.emplace_back(std::move(l.newLine));
        }
      }

      lineQueue[lqo].clear();
    }

    for (auto &l : lineStack) {
      l->PrintLine();
      es::Print("\n");
    }

    for (size_t i = 0; i < lineStack.size(); i++) {
      es::Print("\033[A");
    }

    mustClear = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(nextTickMS));
    innerTick += nextTickMS;
  }
}

void TerminateConsoleDontWait() {
  eraseAllLines = true;
  stopLogger = true;
  if (logger.joinable()) {
    logger.join();
  }

  es::Print("\033[?25h"); // Enable cursor
}

void InitConsole() {
  es::Print("\033[?25l"); // Disable cursor
  es::print::AddQueuer(ReceiveQueue);
  logger = std::thread{MakeLogger};
  pthread_setname_np(logger.native_handle(), "console_logger");
  auto terminate = [](int sig) {
    TerminateConsoleDontWait();
    printf("+------------------------------------------------+\n");
    printf("| APPLICATION HAVE CLOSED UNEXPECTEDLY, CODE: %.2i |\n", sig);
    printf("+------------------------------------------------+\n");
    std::exit(sig);
  };

  std::signal(SIGTERM, terminate);
  std::signal(SIGABRT, terminate);
  std::signal(SIGINT, terminate);
  std::signal(SIGSEGV, terminate);
#ifdef SIGBUS
  std::signal(SIGBUS, terminate);
#endif
}

void TerminateConsole() {
  while (!messageQueues[0].empty() && !messageQueues[1].empty()) {
  }

  TerminateConsoleDontWait();
}

void ConsolePrintDetail(uint8 detail) {
  newPrintDetailSince = messageQueues[currentlyUsedMessageQueue].size();
  uint8 oldDetail = printDetail;
  printDetail = detail | oldDetail << 4;
}

void ElementAPI::Append(std::unique_ptr<LogLine> &&item) {
  lineQueue[currentlyUsedLineQueue].emplace_back(std::move(item));
}

void ElementAPI::Remove(LogLine *item) {
  lineQueue[currentlyUsedLineQueue].emplace_back(item);
}

void ElementAPI::Release(LogLine *line) {
  lineQueue[currentlyUsedLineQueue].emplace_back(line, true);
}

void ElementAPI::Clean() { eraseAllLines = true; }

void ElementAPI::Insert(std::unique_ptr<LogLine> &&item, LogLine *where,
                        bool after) {
  lineQueue[currentlyUsedLineQueue].emplace_back(std::move(item), where, after);
}

void ModifyElements_(element_callback cb) {
  static std::mutex accessMutex;
  std::lock_guard lg(accessMutex);
  static ElementAPI EAPI;
  cb(EAPI);
}
