/*  Contains macros for unit testing

    Copyright 2020 Lukas Cone

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
#include "macroLoop.hpp"
#include "masterprinter.hpp"

#define _CHECK_FAILED_TMP(...)                                                 \
  printerror("Check failed " << __FILE__ << '(' << __LINE__                    \
                             << "): " __VA_ARGS__)

#define TEST_CHECK(val)                                                        \
  if (!val) {                                                                  \
    _CHECK_FAILED_TMP(#val);                                                   \
    return 1;                                                                  \
  }

#define TEST_EQUAL(val1, val2)                                                 \
  if (val1 != val2) {                                                          \
    _CHECK_FAILED_TMP(#val1 " != " #val2);                                     \
    return 1;                                                                  \
  }

#define TEST_NOT_CHECK(val)                                                    \
  if (val) {                                                                   \
    _CHECK_FAILED_TMP(#val);                                                   \
    return 1;                                                                  \
  }

#define TEST_NOT_EQUAL(val1, val2)                                             \
  if (val1 == val2) {                                                          \
    _CHECK_FAILED_TMP(#val1 " != " #val2);                                     \
    return 1;                                                                  \
  }

#define TEST_FUNC(funcName, ...)                                               \
  {                                                                            \
    int _tstValLoc = funcName(__VA_ARGS__);                                    \
    _tstVal += _tstValLoc;                                                     \
    if (_tstValLoc) {                                                          \
      printerror("Method " #funcName " failed!");                              \
    }                                                                          \
  }

#define TEST_CASES(resultVar, ...)                                             \
  printline("Testing " << VA_NARGS(__VA_ARGS__) << " cases.");                 \
  int _tstVal = 0;                                                             \
  StaticFor(VA_NARGS_EVAL, __VA_ARGS__);                                       \
  printline("Testing result: " << VA_NARGS(__VA_ARGS__) - _tstVal              \
                               << " out of " << VA_NARGS(__VA_ARGS__)          \
                               << " successed.");                              \
  resultVar = _tstVal
