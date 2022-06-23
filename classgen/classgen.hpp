/*  class layout utils
    Copyright 2021-2022 Lukas Cone

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
#include "datas/flags.hpp"
#include "datas/pointer.hpp"
#include "datas/endian.hpp"
#include <array>
#include <bit>
#include <set>
#include <stdexcept>

namespace clgen {
enum class LookupFlag : uint8 {
  Ptr,
  Padding,
};
using LookupFlags = es::Flags<LookupFlag>;
struct LayoutLookup {
  uint8 version;
  bool x64;
  bool gnuLayout;
  LookupFlags flags;

  LayoutLookup(uint8 version_, bool X64_, bool gnuLayout)
      : version(version_), x64(X64_), gnuLayout(gnuLayout) {}
  LayoutLookup(uint8 version_, bool X64_, bool gnuLayout, LookupFlags flags_)
      : version(version_), x64(X64_), gnuLayout(gnuLayout), flags(flags_) {}
  LayoutLookup(LayoutLookup other, LookupFlags flags_)
      : version(other.version), x64(other.x64), gnuLayout(other.gnuLayout),
        flags(flags_) {}
  LayoutLookup(const LayoutLookup &other) = default;
};

struct ClassDataHeader {
  union {
    struct {
      uint8 versionBegin;
      uint8 versionEnd;
      uint8 ptrSize;
      bool gnuLayout;
    };
    uint32 raw;
  };
  uint16 totalSize;

  bool operator==(LayoutLookup other) const {
    if (other.version >= versionBegin && other.version <= versionEnd) {
      if (uint8 ptrSize_ = other.x64 ? 8 : 4;
          other.flags == LookupFlag::Ptr && ptrSize != ptrSize_) {
        return false;
      }

      if (other.flags == LookupFlag::Padding && gnuLayout != other.gnuLayout) {
        return false;
      }

      return true;
    }

    return false;
  }

  bool operator==(const ClassDataHeader &other) const {
    return raw == other.raw;
  }

  bool operator<(const ClassDataHeader &other) const { return raw < other.raw; }
  const ClassDataHeader *operator->() const { return this; }
};

template <size_t N> struct ClassData : ClassDataHeader {
  std::array<int16, N> vtable;
  std::array<uint16, (N / 8) + (N % 8 ? 1 : 0)> swaps;
};

template <class IType> void EndianSwap(IType &interface) {
  size_t numItems = interface.layout->vtable.size();

  for (size_t i = 0; i < numItems; ++i) {
    const int16 offset = interface.m(i);

    if (offset < 0) {
      continue;
    }

    const size_t macro = i / 8;
    const size_t micro = i % 8;
    const uint16 macroTile = interface.layout->swaps.at(macro);
    const uint16 swapType = (macroTile >> (2 * micro)) & 3;

    switch (swapType) {
    case 1: {
      uint16 *data = reinterpret_cast<uint16 *>(interface.data + offset);
      FByteswapper(*data);
      break;
    }
    case 2: {
      uint32 *data = reinterpret_cast<uint32 *>(interface.data + offset);
      FByteswapper(*data);
      break;
    }
    case 3: {
      uint64 *data = reinterpret_cast<uint64 *>(interface.data + offset);
      FByteswapper(*data);
      break;
    }
    default:
      break;
    }
  }
}

template <size_t N>
auto GetLayout(const std::set<ClassData<N>> &layouts, LayoutLookup layout) {
  auto item = std::find_if(layouts.begin(), layouts.end(),
                           [&](auto &item) { return item == layout; });

  if (es::IsEnd(layouts, item)) {
    throw std::runtime_error("Specified layout was not found!");
  }

  return item.operator->();
}

template <class IType> struct Iterator : IType {
  using IType::IType;
  Iterator &Next(int by = 1) {
    this->data += this->layout->totalSize * by;
    return *this;
  }
};

template <class Type> struct Pointer {
  Pointer(char *data_, LayoutLookup layout_) : data{data_}, lookup{layout_} {
    layout.totalSize = layout.ptrSize = lookup.x64 ? 8 : 4;
    layout.gnuLayout = lookup.gnuLayout;
  }

  auto operator*() {
    if constexpr (std::is_class_v<Type>) {
      if (layout.ptrSize == 8) {
        return Type{*reinterpret_cast<char **>(data), lookup};
      }

      return Type{*reinterpret_cast<es::PointerX86<char> *>(data), lookup};
    } else {
      if (layout.ptrSize == 8) {
        return *reinterpret_cast<Type **>(data);
      }

      return static_cast<Type *>(
          *reinterpret_cast<es::PointerX86<Type> *>(data));
    }
  }

  char *data;
  LayoutLookup lookup;
  ClassDataHeader layout;
};
} // namespace clgen
