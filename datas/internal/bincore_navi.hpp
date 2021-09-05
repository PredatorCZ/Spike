/*  Navigation class for Binary writter/reader

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
#include <cstddef>

template <class _Traits> class BinStreamNavi : public _Traits {
  size_t savePos[2]{};
  size_t localPos = 0;

protected:
  BinStreamNavi() noexcept = default;
  BinStreamNavi(typename _Traits::StreamType &stream) noexcept
      : _Traits(stream) {}

public:
  enum StackIndex { StackIndex0, StackIndex1 };
  using _Traits::Skip;
  size_t Push(StackIndex index = StackIndex0) {
    return savePos[index] = _Traits::Tell();
  }
  void Pop(StackIndex index = StackIndex0) { _Traits::Seek(savePos[index]); }

  size_t Tell() const { return _Traits::Tell() - localPos; }

  void Seek(size_t position,
            typename _Traits::seekdir vay = _Traits::beg) const {
    _Traits::Seek(vay == _Traits::beg ? position + localPos : position, vay);
  }

  template <typename T> void Skip() const { this->Skip(sizeof(T)); }

  void SetRelativeOrigin(size_t newOrigin, bool useSeek = true) {
    localPos = newOrigin;

    if (useSeek)
      Seek(0);
  }

  void ResetRelativeOrigin(bool useSeek = true) {
    SetRelativeOrigin(0, useSeek);
  };

  void ApplyPadding(int allignBytes = 16) const {
    const int mask = allignBytes - 1;
    const size_t iterPos = Tell();
    const int result = iterPos & mask;

    if (!result)
      return;

    this->Skip(allignBytes - result);
  }

  size_t GetSize() const {
    const size_t savepos = this->Tell();
    this->Seek(0, _Traits::end);
    const size_t curSize = this->Tell();
    this->Seek(savepos);
    return curSize;
  }
};
