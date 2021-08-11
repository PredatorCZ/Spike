/*  class for reading data from a buffer

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

#pragma once
#include "endian.hpp"
#include "internal/bincore.hpp"
#include "internal/sc_type.hpp"

template <class _Traits> class BinReaderRef_t : public BinStreamNavi<_Traits> {
  using _Traits::Read;

public:
  typedef BinStreamNavi<_Traits> navi_type;

  BinReaderRef_t() noexcept = default;
  BinReaderRef_t(typename _Traits::StreamType &stream) noexcept
      : navi_type(stream) {}

  void ReadBuffer(char *buffer, size_t size) const {
    _Traits::Read(buffer, size);
  }

  // Will read any container (vector, basic_string, etc..)
  // input -> must have resize(), operator[], value_type, begin(), end()
  template <class _containerClass,
            class T = typename _containerClass::value_type>
  void ReadContainer(_containerClass &input, size_t numitems) const {
    input.resize(numitems);

    if (!numitems) {
      return;
    }

    if constexpr (use_read_v<T>) {
      for (auto &item : input) {
        item.Read(*this);
      }
    } else {
      const size_t size = sizeof(T);
      ReadBuffer(reinterpret_cast<char *>(&input[0]), size * numitems);

      if (this->swapEndian && size > 1) {
        for (auto &item : input) {
          FByteswapper(item);
        }
      }
    }
  }

  // Will read any container (vector, basic_string, etc..)
  // Will read number of items first.
  // _countType -> type, class convertable into size_t, readable by this->Read
  // method
  // input -> must have resize(), operator[], value_type, begin(), end()
  template <class _countType = uint32, class _containerClass>
  void ReadContainer(_containerClass &input) const {
    _countType numElements;
    Read(numElements);
    ReadContainer(input, numElements);
  }

  // Will read any container (vector, basic_string, etc..) with lambda function
  // per element
  // input -> must have resize(), operator[], value_type, begin(), end()
  // func -> lambda with (BinReaderRef, <element type>), no return
  template <class _containerClass,
            class T = typename _containerClass::value_type, class _func>
  void ReadContainerLambda(_containerClass &input, size_t numitems,
                           _func func) const {
    input.resize(numitems);

    if (!numitems)
      return;

    for (auto &e : input) {
      func(*this, e);
    }
  }

  // Will read any container (vector, basic_string, etc..) with lambda function
  // per element
  // Will read number of items first.
  // _countType -> type, class convertable into size_t, readable by this->Read
  // method
  // input -> must have resize(), operator[], value_type, begin(), end()
  // func -> lambda with (BinReaderRef, <element type>), no return
  template <class _countType = uint32, class _containerClass, class _func>
  void ReadContainerLambda(_containerClass &input, _func func) const {
    _countType numElements;
    Read(numElements);
    ReadContainerLambda(input, numElements, func);
  }

  // Will read buffer until 0
  template <class _containerClass>
  void ReadString(_containerClass &input) const {
    typename _containerClass::value_type tmp;

    while ((Read(tmp), tmp) != 0 && !this->IsEOF())
      input.push_back(tmp);
  }

  template <typename T, size_t _size> void Read(T (&value)[_size]) const {
    const size_t size = sizeof(T);
    const size_t arraySize = size * _size;

    ReadBuffer(reinterpret_cast<char *>(value), arraySize);

    if (this->swapEndian && size > 1) {
      FByteswapper(value);
    }
  }

  // Read class with its Read(BinReaderRef_t) member method, or read type itself
  template <typename T> void Read(T &value) const {
    if constexpr (use_read_v<T>) {
      value.Read(*this);
    } else {
      const size_t size = sizeof(T);
      ReadBuffer(reinterpret_cast<char *>(&value), size);

      if (this->swapEndian && size > 1) {
        FByteswapper(value);
      }
    }
  }

private:
  using Self = BinReaderRef_t<_Traits>;
  template <class C, class = void> struct use_read : std::false_type {};
  template <class C>
  struct use_read<
      C, std::void_t<decltype(std::declval<C>().Read(std::declval<Self>()))>>
      : std::true_type {};
  template <class C> constexpr static bool use_read_v = use_read<C>::value;
};
