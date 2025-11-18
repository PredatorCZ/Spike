/*  uni format module source
    part of uni module
    Copyright 2020-2024 Lukas Cone

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

#include "spike/uni/detail/format_full.hpp"
#include "spike/util/macroLoop.hpp"
#include <unordered_map>

using namespace uni;

// This class holds vtable for each _FormatCodecImpl_t.
// Huge gamble, but there is no better way. (for C++14)
// TODO: maybe use std::any for C++17
class variant {
  mutable uint64 vtdata;

public:
  template <FormatType ftype, DataType dtype>
  variant(_FormatCodecImpl_t<ftype, dtype> cted) {
    static_assert(sizeof(decltype(cted)) <= sizeof(vtdata), "Invalid class!");
    vtdata = reinterpret_cast<uint64 &>(cted);
  }

  variant(const variant &) = default;
  variant(variant &&) = default;

  operator FormatCodec &() const {
    return reinterpret_cast<FormatCodec &>(vtdata);
  }
};

template <FormatType ftype, DataType dtype> auto MakePair() {
  return std::make_pair(FormatDescr{ftype, dtype},
                        variant(_FormatCodecImpl_t<ftype, dtype>{}));
}

#define _MAKE_CODEC_BASE(cname)                                                \
  MakePair<FormatType::INT, DataType::cname>(),                                \
      MakePair<FormatType::UINT, DataType::cname>(),                           \
      MakePair<FormatType::UNORM, DataType::cname>(),                          \
      MakePair<FormatType::NORM, DataType::cname>(),

namespace std {
template <> struct hash<FormatDescr> {
  constexpr uint32 operator()(const FormatDescr &t) const {
    return static_cast<uint32>(t.compType) | static_cast<uint32>(t.outType)
                                                 << 16;
  }
};
} // namespace std

static const std::unordered_map<FormatDescr, variant> registry = {
    StaticFor(_MAKE_CODEC_BASE, R32G32B32A32, R32G32B32, R16G16B16A16, R32G32,
              R16G16B16, R32, R16G16, R10G10B10A2, R11G11B10, R8G8B8A8, R24G8,
              R8G8B8, R8G8, R16, R5G6B5, R5G5B5A1, R8)

        MakePair<FormatType::FLOAT, DataType::R32G32B32A32>(),
    MakePair<FormatType::FLOAT, DataType::R32G32B32>(),
    MakePair<FormatType::FLOAT, DataType::R32G32>(),
    MakePair<FormatType::FLOAT, DataType::R32>(),
    MakePair<FormatType::FLOAT, DataType::R16G16B16A16>(),
    MakePair<FormatType::FLOAT, DataType::R16G16B16>(),
    MakePair<FormatType::FLOAT, DataType::R16G16>(),
    MakePair<FormatType::FLOAT, DataType::R16>(),
    MakePair<FormatType::FLOAT, DataType::R11G11B10>(),
    MakePair<FormatType::UFLOAT, DataType::R11G11B10>(),
};

void FormatCodec::GetValue(IVector4A16 &, const char *) const {
  throw es::RuntimeError("Invalid call for uni::format codec!");
}

void FormatCodec::GetValue(Vector4A16 &, const char *) const {
  throw es::RuntimeError("Invalid call for uni::format codec!");
}

void FormatCodec::Sample(ivec &, const char *, size_t, size_t) const {
  throw es::RuntimeError("Invalid call for uni::format codec!");
}

void FormatCodec::Sample(fvec &, const char *, size_t, size_t) const {
  throw es::RuntimeError("Invalid call for uni::format codec!");
}

FormatCodec &FormatCodec::Get(const FormatDescr &input) {
  return registry.at(input);
}
