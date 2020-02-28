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
#include "internal/bincore.hpp"

template <class _Traits> class BinReaderRef_t : public BinStreamNavi<_Traits> {
  using _Traits::Read;
public:
  typedef BinStreamNavi<_Traits> navi_type;

  BinReaderRef_t() noexcept = default;
  BinReaderRef_t(typename _Traits::StreamType &stream) noexcept
      : _Traits::_Traits(stream) {}

  void ReadBuffer(char *buffer, size_t size) const { _Traits::Read(buffer, size); }

  // Will read any std container using std::allocator class, eg. vector,
  // basic_string, etc.. swapType : will force not to swap endianess, when used
  // with class that does not have SwapEndian method or is not defined for
  // structural swap
  template <class _containerClass,
            class T = typename _containerClass::value_type>
  void ReadContainer(_containerClass &input, size_t numitems,
                     _e_swapEndian) const {
    input.resize(numitems);

    if (!numitems)
      return;

    const size_t elesize = sizeof(T);
    const size_t arrsize = elesize * numitems;

    ReadBuffer(reinterpret_cast<char *>(&input[0]), arrsize);

#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian && swapType && elesize > 1)
      for (T &a : input)
        FByteswapper(a);
#endif
  }

  // Will read any std container using std::allocator class, eg. vector,
  // basic_string, etc.. Will read number of items first swapType : will force
  // not to swap endianess, when used with class that does not have SwapEndian
  // method or is not defined for structural swap
  template <class _countType = int, class _containerClass>
  void ReadContainer(_containerClass &input, _e_swapEndian) const {
    _countType numElements;
    Read(numElements);
#ifdef ES_ENDIAN_DEFINED
    ReadContainer(input, numElements, swapType);
#else
    ReadContainer(input, numElements);
#endif
  }

  // Will read buffer until 0
  template <class _containerClass>
  void ReadString(_containerClass &input) const {
    typename _containerClass::value_type tmp;

    while ((Read(tmp), tmp) != 0 && !this->IsEOF())
      input.push_back(tmp);
  }

  // swapType : will force not to swap endianess, when used with class that does
  // not have SwapEndian method or is not defined for structural swap
  template <typename T>
  void Read(T &value, const size_t size = sizeof(T), _e_swapEndian) const {
    ReadBuffer(reinterpret_cast<char *>(&value), size);

#ifdef ES_ENDIAN_DEFINED
    if (this->swapEndian && swapType && size > 1)
      FByteswapper(value);
#endif
  }
};
