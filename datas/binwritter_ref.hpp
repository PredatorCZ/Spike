/*  class for writing data to a buffer

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
#include "internal/bincore.hpp"
#include "internal/sc_type.hpp"
#include <cstring>

template <class _Traits> class BinWritterRef_t : public BinStreamNavi<_Traits> {
public:
  typedef BinStreamNavi<_Traits> navi_type;

  BinWritterRef_t() noexcept = default;
  BinWritterRef_t(typename _Traits::StreamType &stream) noexcept
      : navi_type(stream) {}

  void WriteBuffer(const char *buffer, size_t size) const {
    _Traits::Write(buffer, size);
  }

  // Will write any container (vector, basic_string, etc..)
  // input -> must have size(), data(), value_type
  template <class _containerClass,
            class T = typename _containerClass::value_type>
  void WriteContainer(const _containerClass &input) const {
    _WriteElements<T>(input.data(), input.size(), 0);
  }

  // Will write any container (vector, basic_string, etc..)
  // Will write number of items first.
  // _countType -> type, class castable from size_t, writeable by this->Write
  // method (optional)
  // input -> must have size(), data(), value_type
  template <class _countType = uint32, class _containerClass>
  void WriteContainerWCount(const _containerClass &input) const {
    const _countType numElements = static_cast<_countType>(input.size());
    Write(numElements);
    WriteContainer(input);
  }

  // Will write any container (vector, basic_string, etc..) via lambda function
  // per element
  // Will write number of items first.
  // _countType -> type, class castable from size_t, writeable by this->Write
  // method (optional)
  // input -> must have size(), data(), value_type
  // func -> lambda with (BinWritterRef, <element type>), no return
  template <class _countType = uint32, class _containerClass, class _func>
  void WriteContainerLambda(_containerClass &input, _func func) const {
    _countType numElements = static_cast<_countType>(input.size());
    Write(numElements);

    for (const auto &v : input) {
      func(*this, v);
    }
  }

  // Write C string
  // cut : will remove \0
  void WriteT(const char *input, bool cut = false) const {
    WriteBuffer(input, strlen(input) + (cut ? 0 : 1));
  }

  // Write C wstring (utf16, utf32, other)
  // cut : will remove \0
  void WriteT(const wchar_t *input, bool cut = false) const {
    size_t size = wcslen(input);
    if (!cut)
      size++;
    size_t capacity = size * sizeof(wchar_t);
#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian) {
      std::wstring outBuffer = input;

      if (cut)
        outBuffer.resize(size);

      WriteContainer(outBuffer);
    } else
#endif
      WriteBuffer(reinterpret_cast<const char *>(input), capacity);
  }

  // SFINAE
  // Write class with its Write(BinWritterRef_t) const member method, or writes
  // type itself
  template <class T> void Write(const T &input) const {
    _WriteSingle<T>(input, 0);
  }

private:
  template <class C, class D>
  auto _WriteSingle(const D &input, int) const
      -> decltype(std::declval<C>().Write(*this), void()) {
    input.Write(*this);
  };

  template <class C, class T> void _WriteSingle(const T &value, ...) const {
    const size_t capacity = sizeof(T);
#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian && capacity > 1) {
      T outCopy = value;
      FByteswapper(outCopy);
      WriteBuffer(reinterpret_cast<const char *>(&outCopy), capacity);
    } else
#endif
      WriteBuffer(reinterpret_cast<const char *>(&value), capacity);
  }

  template <class C, class D>
  auto _WriteElements(const D *input, size_t numElements, int) const
      -> decltype(std::declval<C>().Write(*this), void()) {
    for (size_t e = 0; e < numElements; e++)
      (input + e)->Write(*this);
  };

  template <class C, class T>
  void _WriteElements(const T *value, size_t numElements, ...) const {
    const size_t size = sizeof(T);
#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian && size > 1) {
      for (size_t e = 0; e < numElements; e++)
        Write(*(value + e));
    } else
#endif
      WriteBuffer(reinterpret_cast<const char *>(value), size * numElements);
  }
};
