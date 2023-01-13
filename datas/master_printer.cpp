/*  a source for MasterPrinter class

    Copyright 2018-2021 Lukas Cone

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

#include "master_printer.hpp"
#include "flags.hpp"
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace es::print;

static struct MasterPrinter {
  struct FuncType {
    print_func func;
    bool useColor;

    FuncType(print_func fp, bool cl) : func(fp), useColor(cl) {}
  };

  std::vector<FuncType> functions;
  std::vector<queue_func> queues;
  std::stringstream buffer;
  std::mutex mutex;
  std::thread::id lockedThread;
  bool printThreadID = false;
  MPType cType = MPType::MSG;
} MASTER_PRINTER;

namespace es::print {

void AddPrinterFunction(print_func func, bool useColor) {
  for (auto &[func_, _] : MASTER_PRINTER.functions) {
    if (func_ == func) {
      return;
    }
  }
  MASTER_PRINTER.functions.emplace_back(func, useColor);
}

void AddQueuer(queue_func func) { MASTER_PRINTER.queues.push_back(func); }

std::ostream &Get(MPType type) {
  if (auto threadID = std::this_thread::get_id();
      !(MASTER_PRINTER.lockedThread == threadID)) {
    MASTER_PRINTER.mutex.lock();
    MASTER_PRINTER.lockedThread = threadID;
  }

  if (type != MPType::PREV) {
    MASTER_PRINTER.cType = type;
  }
  return MASTER_PRINTER.buffer;
}

void FlushAll() {
  Queuer que;
  que.payload = std::move(MASTER_PRINTER.buffer).str();
  std::thread::id threadID = std::this_thread::get_id();
  que.threadId = reinterpret_cast<uint32 &>(threadID);
  que.type = MASTER_PRINTER.cType;

  for (auto &[func, useColor] : MASTER_PRINTER.functions) {
    if (useColor) {
      if (que.type == MPType::WRN) {
        func("\033[38;2;255;255;0m");
      } else if (que.type == MPType::ERR) {
        func("\033[38;2;255;0;0m");
      } else if (que.type == MPType::INF) {
        func("\033[38;2;0;180;255m");
      }
    }

    if (MASTER_PRINTER.printThreadID) {
      func("Thread[0x");
      char buffer[65];
      snprintf(buffer, 65, "%X", que.threadId);
      func(buffer);
      func("] ");
    }

    if (que.type == MPType::WRN) {
      func("WARNING: ");
    } else if (que.type == MPType::ERR) {
      func("ERROR: ");
    }

    func(que.payload.data());

    if (useColor && que.type != MPType::MSG) {
      func("\033[0m");
    }
  }

  for (auto &q : MASTER_PRINTER.queues) {
    q(que);
  }

  MASTER_PRINTER.cType = MPType::MSG;
  MASTER_PRINTER.mutex.unlock();
  MASTER_PRINTER.lockedThread = {};
}

void PrintThreadID(bool yn) { MASTER_PRINTER.printThreadID = yn; }
} // namespace es::print
