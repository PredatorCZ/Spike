/*  class for writing data to a buffer

    Copyright 2018-2023 Lukas Cone

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
#include "detail/bincore.hpp"
#include "spike/type/detail/sc_type.hpp"
#include "spike/util/endian.hpp"
#include <string_view>

template <class _Traits, bool HandleEndian>
class BinWritterRef_t : public BinStreamNavi<_Traits> {
public:
  typedef BinStreamNavi<_Traits> navi_type;

  BinWritterRef_t() noexcept = default;
  BinWritterRef_t(typename _Traits::StreamType &stream) noexcept
      : navi_type(stream) {}

  operator BinWritterRef_t<_Traits, !HandleEndian>() {
    return reinterpret_cast<BinWritterRef_t<_Traits, !HandleEndian> &>(*this);
  }

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
      constexpr size_t size = sizeof(T);
      auto wrbuffer = [&] {
        WriteBuffer(reinterpret_cast<const char *>(input.data()),
                    size * input.size());
      };

      if constexpr (size > 1) {
        if (this->swapEndian) {
          for (auto &item : input) {
            Write(item);
          }
        } else {
          wrbuffer();
        }
      } else {
        wrbuffer();
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
  void WriteT(std::string_view input) const {
    using charType = std::string_view ::value_type;
    WriteBuffer(input.data(), input.size());
    Write<charType>(0);
  }

  // Write C wstring (utf16, utf32, other)
  void WriteT(std::wstring_view input) const {
    using charType = std::wstring_view::value_type;
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
      constexpr size_t typeSize = sizeof(std::remove_extent_t<T>);
      constexpr size_t totalSize = sizeof(T);
      auto wtbuffer = [&](auto *ptr) {
        WriteBuffer(reinterpret_cast<const char *>(ptr), totalSize);
      };

      if constexpr (HandleEndian && typeSize > 1 && use_swap_v<T>) {
        if (this->swapEndian) {
          auto outCopy = input;
          FByteswapper(outCopy, true);
          wtbuffer(&outCopy);
        } else {
          wtbuffer(&input);
        }
      } else {
        wtbuffer(&input);
      }
    }
  }

private:
  using Self = BinWritterRef_t<_Traits, HandleEndian>;
  template <class T>
  using use_write = decltype(std::declval<T>().Write(std::declval<Self>()));
  template <class C>
  constexpr static bool use_write_v = es::is_detected_v<use_write, C>;

  template <class T> using no_swap = decltype(std::declval<T>().NoSwap());
  template <class C>
  constexpr static bool use_swap_v = !es::is_detected_v<no_swap, C>;
};
