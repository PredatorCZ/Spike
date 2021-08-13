/*  class for writing data to a buffer

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
#include "string_view.hpp"

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
    if constexpr (use_write_v<T>) {
      for (auto &item : input) {
        item.Write(*this);
      }
    } else {
      const size_t size = sizeof(T);
      if (this->swapEndian && size > 1) {
        for (auto &item : input) {
          Write(item);
        }
      } else {
        WriteBuffer(reinterpret_cast<const char *>(input.data()),
                    size * input.size());
      }
    }
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
  void WriteT(es::string_view input) const {
    using charType = es::string_view::value_type;
    WriteBuffer(input.data(), input.size());
    Write<charType>(0);
  }

  // Write C wstring (utf16, utf32, other)
  void WriteT(es::wstring_view input) const {
    using charType = es::wstring_view::value_type;
    if (this->swapEndian) {
      WriteContainer(input);
    } else {
      const size_t capacity = input.size() * sizeof(charType);
      WriteBuffer(reinterpret_cast<const char *>(input.data()), capacity);
    }

    Write<charType>(0);
  }

  // Write class with its Write(BinWritterRef_t) const member method, or writes
  // type itself
  template <class T> void Write(const T &input) const {
    if constexpr (use_write_v<T>) {
      input.Write(*this);
    } else {
      const size_t capacity = sizeof(T);
      if (this->swapEndian && capacity > 1) {
        auto outCopy = input;
        FByteswapper(outCopy, true);
        WriteBuffer(reinterpret_cast<const char *>(&outCopy), capacity);
      } else {
        WriteBuffer(reinterpret_cast<const char *>(&input), capacity);
      }
    }
  }

private:
  using Self = BinWritterRef_t<_Traits>;
  template <class T>
  using use_write = decltype(std::declval<T>().Write(std::declval<Self>()));
  template <class C>
  constexpr static bool use_write_v = es::is_detected_v<use_write, C>;
};
