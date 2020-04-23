/*  Virtual Iterator for abstraction classes
    part of uni module
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
#include <functional>

namespace uni {
template <class containerClass, size_t (containerClass::*Counter)() const,
          class returnType, returnType (containerClass::*Accessor)(size_t) const>
class VirtualIterator {
  const containerClass *tClass;
  size_t iterPos;
  static const size_t npos = 0xffffffff;

public:
  explicit VirtualIterator(const containerClass *cls, size_t _num = npos)
      : iterPos(_num >= npos ? std::bind(Counter, tClass)() : _num), tClass(cls) {
  }

  VirtualIterator &operator++() {
    iterPos++;
    return *this;
  }
  VirtualIterator operator++(int) {
    VirtualIterator retval = *this;
    ++(*this);
    return retval;
  }
  bool operator==(VirtualIterator input) const { return iterPos == input.iterPos; }
  bool operator!=(VirtualIterator input) const { return iterPos != input.iterPos; }

  template <class ptrTest = returnType>
  typename std::enable_if<
      std::is_pointer<ptrTest>::value,
      typename std::remove_pointer<returnType>::type &>::type
  operator*() const {
    return *std::bind(Accessor, tClass, iterPos)();
  }

  template <class ptrTest = returnType>
  typename std::enable_if<!std::is_pointer<ptrTest>::value, returnType>::type
  operator*() const {
    return std::bind(Accessor, tClass, iterPos)();
  }
};
} // namespace uni