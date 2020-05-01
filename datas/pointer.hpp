/*  Pointer from memory map

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
#include "supercore.hpp"
#include <algorithm>
#include <vector>

namespace es {
thread_local extern std::vector<void *> usedPts;

template <class C, typename B> union Pointer_t {
  typedef C value_type;

private:
  char *rawPtr;
  C *pointer;
  B varPtr;

public:
  operator C *() { return pointer; }
  C &operator*() { return *pointer; }
  C *operator->() { return pointer; }
  Pointer_t &operator=(C *input) {
    pointer = input;
    return *this;
  }

  operator const C *() const { return pointer; }
  const C &operator*() const { return *pointer; }
  const C *operator->() const { return pointer; }

  bool Fixed() const {
    return std::any_of(usedPts.begin(), usedPts.end(),
                       [&](const void *i) { return i == &varPtr; });
  }

  bool Fixup(char *root, bool noCheck = false) {
    if (!pointer || (!noCheck && Fixed())) {
      return false;
    }

    rawPtr = root + varPtr;
    usedPts.push_back(&varPtr);
    return true;
  }
};

template <class C> struct PointerX86 {
  typedef C value_type;

private:
  uint32 varPtr;

public:
  operator C *() {
    return reinterpret_cast<C *>(reinterpret_cast<char *>(&varPtr) + varPtr);
  }

  C &operator*() { return *static_cast<C *>(*this); }
  C *operator->() { return *this; }

  operator const C *() const {
    return reinterpret_cast<const C *>(reinterpret_cast<const char *>(&varPtr) +
                                       varPtr);
  }
  const C &operator*() const { return *static_cast<C *>(*this); }
  const C *operator->() const { return *this; }

  bool Fixed() const {
    return std::any_of(usedPts.begin(), usedPts.end(),
                       [&](const void *i) { return i == &varPtr; });
  }

  bool Fixup(char *root, bool noCheck = false) {
    if (!varPtr || (!noCheck && Fixed()))
      return false;

    uintptr_t rawAddr = reinterpret_cast<uintptr_t>(root) + varPtr;
    varPtr =
        static_cast<uint32>(rawAddr - reinterpret_cast<uintptr_t>(&varPtr));
    usedPts.push_back(&varPtr);
    return true;
  }
};

template <class C, bool x64> struct _PointerX86 {
  typedef PointerX86<C> value_type;
};
template <class C> struct _PointerX86<C, false> {
  typedef Pointer_t<C, uint32> value_type;
};

} // namespace es

template <class C> using esPointerX64 = es::Pointer_t<C, uint64>;
template <class C>
using esPointerX86 = typename es::_PointerX86<C, ES_X64>::value_type;

static inline void ClearESPointers() { es::usedPts.clear(); }