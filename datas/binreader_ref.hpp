/*  class for reading data from a buffer

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

    if (!numitems)
      return;

    _ReadElements<T>(&input[0], numitems, 0);
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

  // SFINAE
  // Read class with its Read(BinReaderRef_t) member method, or read type itself
  template <typename T> void Read(T &value) const { _ReadSingle<T>(value, 0); }

private:
  template <class C, class D>
  auto _ReadSingle(D &input, int) const
      -> decltype(std::declval<C>().Read(*this), void()) {
    input.Read(*this);
  };

  template <class C, class T> void _ReadSingle(T &value, ...) const {
    const size_t size = sizeof(T);
    ReadBuffer(reinterpret_cast<char *>(&value), size);

    if (this->swapEndian && size > 1) {
      FByteswapper(value);
    }
  }

  template <class C, class D>
  auto _ReadElements(D *input, size_t numElements, int) const
      -> decltype(std::declval<C>().Read(*this), void()) {
    for (size_t e = 0; e < numElements; e++)
      (input + e)->Read(*this);
  };

  template <class C, class T>
  void _ReadElements(T *value, size_t numElements, ...) const {
    const size_t size = sizeof(T);
    ReadBuffer(reinterpret_cast<char *>(value), size * numElements);

    if (this->swapEndian && size > 1) {
      for (size_t e = 0; e < numElements; e++) {
        FByteswapper(*(value + e));
      }
    }
  }
};
