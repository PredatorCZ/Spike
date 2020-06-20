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
#include <type_traits>

#define _CHECK_FAILED_TMP(...)                                                 \
  printerror("Check failed " << __FILE__ << '(' << __LINE__                    \
                             << "): " __VA_ARGS__)

namespace es {
template <class C, class D>
auto append_scan(const D &input, const char *varName, int)
    -> decltype(std::declval<std::ostream>() << std::declval<C>(), void()) {
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ < 7)
  printer << varName;
#else
  printer << input;
#endif
};

template <class C, class D>
void append_scan(const D &, const char *varName, ...) {
  printer << varName;
}

template <class C> struct ValuePrinter {
  const C &value;
  const char *valName;

  ValuePrinter(const C &input, const char *vlName)
      : value(input), valName(vlName) {}

  friend std::ostream &operator<<(std::ostream &str, const ValuePrinter &val) {
    append_scan<C, C>(val.value, val.valName, 0);
    return str;
  }
};

template <class A, class B>
MasterPrinterThread &PrintCheckFailed(const A &aVal, const B &bVal,
                                      const char *aName, const char *bName,
                                      const char *op, const char *file,
                                      int line) {
  printerror("Check failed " << file << '(' << line << "): " << aName << op
                             << bName << ", " << ValuePrinter<A>(aVal, aName)
                             << op << ValuePrinter<B>(bVal, bName));
  return printer;
}
} // namespace es

#define TEST_CHECK(val)                                                        \
  if (!val) {                                                                  \
    _CHECK_FAILED_TMP(#val);                                                   \
    return 1;                                                                  \
  }

#define TEST_EQUAL(val1, val2)                                                 \
  if (val1 != val2) {                                                          \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " != ", __FILE__,           \
                         __LINE__);                                            \
    return 1;                                                                  \
  }

#define TEST_GT(val1, val2)                                                    \
  if (val1 <= val2) {                                                          \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " <= ", __FILE__,           \
                         __LINE__);                                            \
    return 1;                                                                  \
  }

#define TEST_LT(val1, val2)                                                    \
  if (val1 >= val2) {                                                          \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " >= ", __FILE__,           \
                         __LINE__);                                            \
    return 1;                                                                  \
  }

#define TEST_GT_EQ(val1, val2)                                                 \
  if (val1 < val2) {                                                           \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " < ", __FILE__, __LINE__); \
    return 1;                                                                  \
  }

#define TEST_LE_EQ(val1, val2)                                                 \
  if (val1 > val2) {                                                           \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " > ", __FILE__, __LINE__); \
    return 1;                                                                  \
  }

#define TEST_NOT_CHECK(val)                                                    \
  if (val) {                                                                   \
    _CHECK_FAILED_TMP(#val);                                                   \
    return 1;                                                                  \
  }

#define TEST_NOT_EQUAL(val1, val2)                                             \
  if (val1 == val2) {                                                          \
    es::PrintCheckFailed(val1, val2, #val1, #val2, " == ", __FILE__,           \
                         __LINE__);                                            \
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
