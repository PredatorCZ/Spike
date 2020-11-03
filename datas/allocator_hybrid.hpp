/*  es::allocator_hybrid for container classes, allows to link
    external buffer. more info in README for PreCore Project

    Copyright 2018-2020 Lukas Cone

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
#ifndef _DECLSPEC_ALLOCATOR
#define _DECLSPEC_ALLOCATOR
#endif

#include <cstddef>
#include <type_traits>

namespace es {

struct allocator_hybrid_base {
  template <class C> static void DisposeStorage(C &input) noexcept {
    input = C();
  }

  template <class C>
  static void LinkStorage(C &input, typename C::pointer start,
                          typename C::pointer end) {
    input = C(start, end, C::allocator_type(start));
  }

  template <class C>
  static void LinkStorage(C &input, typename C::pointer start, size_t count) {
    input = C(start, start + count, typename C::allocator_type(start));
  }
};

template <class _Ty> struct allocator_hybrid : allocator_hybrid_base {
  typedef _Ty value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef std::true_type propagate_on_container_copy_assignment;
  typedef std::true_type propagate_on_container_move_assignment;

  pointer buffer;

  allocator_hybrid() noexcept : buffer(nullptr) {}

  allocator_hybrid(pointer iBuff) noexcept : buffer(iBuff){};

  template <class U>
  allocator_hybrid(const allocator_hybrid<U> &) noexcept : allocator_hybrid() {}

  allocator_hybrid(const allocator_hybrid<value_type> &input) noexcept {
    buffer = input.buffer;
  }

  _DECLSPEC_ALLOCATOR pointer allocate(size_t count) {
    if (!buffer)
      return static_cast<pointer>(operator new(count * sizeof(value_type)));
    else
      return buffer;
  }

  void deallocate(pointer ptr, size_t) {
    if (buffer == ptr) {
      buffer = nullptr;
      return;
    }

    operator delete(ptr);
  }
};

template <class T, class O>
bool operator==(const allocator_hybrid<T> &t,
                const allocator_hybrid<O> &o) noexcept {
  return static_cast<void *>(t.buffer) == static_cast<void *>(o.buffer);
}
template <class T, class O>
bool operator!=(const allocator_hybrid<T> &t,
                const allocator_hybrid<O> &o) noexcept {
  return !(t == o);
}
} // namespace es
