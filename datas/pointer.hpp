/*  Pointer from memory map

    Copyright 2020-2021 Lukas Cone

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
template <class T>
using use_push_back = decltype(std::declval<T>().push_back(
    std::declval<typename T::value_type>()));
template <class C>
constexpr static bool use_push_back_v = es::is_detected_v<use_push_back, C>;

template <class C> union PointerX64 {
  typedef C value_type;

private:
  const char *rawPtr;
  C *pointer;
  uint64 varPtr;

public:
  operator C *() { return pointer; }
  C &operator*() { return *pointer; }
  C *operator->() { return pointer; }
  PointerX64 &operator=(C *input) {
    pointer = input;
    return *this;
  }

  operator const C *() const { return pointer; }
  const C &operator*() const { return *pointer; }
  const C *operator->() const { return pointer; }

  template <class container> bool Check(container &store) const {
    if (std::any_of(store.begin(), store.end(),
                    [&](auto i) { return i == &varPtr; })) {
      return true;
    }

    return false;
  }

  template <class container>
  int Fixup(const char *root, container &storedPtrs) {
    if (!pointer) {
      return 0;
    }

    if (Check(storedPtrs)) {
      return -1;
    } else {
      if constexpr (use_push_back_v<container>) {
        storedPtrs.push_back(&varPtr);
      } else {
        storedPtrs.emplace(&varPtr);
      }
    }

    rawPtr = root + varPtr;
    return 1;
  }

  int Fixup(const char *root) {
    if (!pointer) {
      return 0;
    }

    rawPtr = root + varPtr;
    return 1;
  }

  void Swap();
};

template <class C, class Store = int32> struct PointerX86 {
  typedef C value_type;

private:
  Store varPtr = 0;

public:
#ifndef ES_COPYABLE_POINTER
  PointerX86(const PointerX86 &) = delete;
  PointerX86(PointerX86 &&) = delete;
  PointerX86() = default;
#else
  Store RawValue() const { return varPtr; }
#endif
  operator bool() const { return varPtr != 0; }

  C *Get() {
    return varPtr ? reinterpret_cast<C *>(reinterpret_cast<char *>(&varPtr) +
                                          varPtr)
                  : nullptr;
  }

  const C *Get() const {
    return varPtr ? reinterpret_cast<const C *>(
                        reinterpret_cast<const char *>(&varPtr) + varPtr)
                  : nullptr;
  }

  operator C *() { return Get(); }
  C &operator*() { return *static_cast<C *>(*this); }
  C *operator->() { return *this; }

  operator const C *() const { return Get(); }
  const C &operator*() const { return *static_cast<C *>(*this); }
  const C *operator->() const { return *this; }

  template <class container> bool Check(container &store) const {
    if (std::any_of(store.begin(), store.end(),
                    [&](auto i) { return i == &varPtr; })) {
      return true;
    }

    return false;
  }

  template <class container>
  int Fixup(const char *root, container &storedPtrs) {
    if (!varPtr) {
      return 0;
    }

    if (Check(storedPtrs)) {
      return -1;
    } else {
      if constexpr (use_push_back_v<container>) {
        storedPtrs.push_back(&varPtr);
      } else {
        storedPtrs.emplace(&varPtr);
      }
    }

    return FixupRelative(root);
  }

  int FixupRelative(const char *root) {
    char *rawAddr = const_cast<char *>(root + varPtr);
    *this = reinterpret_cast<C *>(rawAddr);
    return 1;
  }

  int Fixup(const char *root) {
    if (!varPtr) {
      return 0;
    }

    return FixupRelative(root);
  }

  PointerX86 &operator=(C *newDest) {
    uintptr_t _rawDest = reinterpret_cast<uintptr_t>(newDest);
    varPtr =
        static_cast<Store>(_rawDest - reinterpret_cast<uintptr_t>(&varPtr));

    return *this;
  }

  void Reset(Store value = 0) { varPtr = value; }

  void Swap();
};

template <class C>
concept IsFixable = requires(C t) {
  t.Fixup((const char *)nullptr);
};

template <class C>
concept NotFixable = !IsFixable<C>;

template <IsFixable... C> void FixupPointers(const char *root, C &...ptrs) {
  (ptrs.Fixup(root), ...);
}

template <IsFixable... C, NotFixable container>
bool FixupPointers(const char *root, container &store, C &...p) {
  return (... && (p.Fixup(root, store) != -1));
}

template <class... C, template <class F> class ptr, class container, class fn>
bool FixupPointersCB(const char *root, container &store, fn &&notFixedCB,
                     ptr<C> &...ptrs) {
  auto ptrArray = {reinterpret_cast<ptr<char> *>(&ptrs)...};
  for (auto p : ptrArray) {
    if (p.Check(store)) {
      return false;
    }
  }

  notFixedCB();
  FixupPointers(root, store, ptrs...);

  return true;
}

} // namespace es

template <class C> using esPointerX64 = es::PointerX64<C>;
template <class C> using esPointerX86 = es::PointerX86<C>;
