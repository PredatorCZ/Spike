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
#include <cstring>

template <class _Traits> class BinWritterRef_t : public BinStreamNavi<_Traits> {
public:
  typedef BinStreamNavi<_Traits> navi_type;

  BinWritterRef_t() noexcept = default;
  BinWritterRef_t(typename _Traits::StreamType &stream) noexcept
      : _Traits(stream) {}

  void WriteBuffer(const char *buffer, size_t size) const {
    _Traits::Write(buffer, size);
  }

  // Will write any std container using std::allocator class, eg. vector,
  // basic_string, etc..
  // swapType : will force not to swap endianess, when used with class that does
  // not have SwapEndian method or is not defined for structural swap
  template <class _containerClass,
            class T = typename _containerClass::value_type>
  void WriteContainer(_containerClass &input, _e_swapEndian) const {
    const size_t elesize = sizeof(T);
    const size_t capacity = input.size() * elesize;
#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian && elesize > 1 &&
        swapType == BinSteamEndian::EndianSwap::SWAP) {
      for (const T &a : input) {
        T outCopy = a;
        FByteswapper(outCopy);
        WriteBuffer(reinterpret_cast<const char *>(&outCopy), elesize);
      }
    } else
#endif
      WriteBuffer(reinterpret_cast<const char *>(&input[0]), capacity);
  }

  // Will write any std container using std::allocator class, eg. vector,
  // basic_string, etc.. Will write number of items first.
  // swapType : will force not to swap endianess, when used with class that does
  // not have SwapEndian method or is not defined for structural swap
  template <class _countType = int, class _containerClass>
  const void WriteContainerWCount(_containerClass &input, _e_swapEndian) const {
    const _countType numElements = static_cast<_countType>(input.size());
    Write(numElements);
#ifdef ES_ENDIAN_DEFINED
    WriteContainer(input, swapType);
#else
    WriteContainer(input);
#endif
  }

  // cut : will remove \0
  void WriteT(const char *input, bool cut = false) const {
    WriteBuffer(input, strlen(input) + (cut ? 0 : 1));
  }

  // cut : will remove \0
  void WriteT(const wchar_t *input, bool cut = false) const {
    size_t size = wcslen(input);
    if (!cut)
      size++;
    size_t capacity = size * 2;
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

  // swapType : will force not to swap endianess, when used with class that does
  // not have SwapEndian method or is not defined for structural swap
  template <class T> void Write(const T input, _e_swapEndian) const {
    const size_t capacity = sizeof(T);
#ifdef ES_ENDIAN_DEFINED
    if (swapType && this->swapEndian && capacity > 1) {
      T outCopy = input;
      FByteswapper(outCopy);
      WriteBuffer(reinterpret_cast<const char *>(&outCopy), capacity);
    } else
#endif
      WriteBuffer(reinterpret_cast<const char *>(&input), capacity);
  }
};
