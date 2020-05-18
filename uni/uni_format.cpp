/*  uni format module source
    part of uni module
    Copyright 2020 Lukas Cone

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

#include "datas/macroLoop.hpp"
#include "format.hpp"
#include <unordered_map>

using namespace uni;

template <FormatType ftype, DataType dtype> FormatCodec *_makeCodec() {
  return new _FormatCodecImpl_t<ftype, dtype>();
}

#define _MAKE_CODEC_BASE(cname)                                                \
  {{FormatType::INT, DataType::cname},                                         \
   _makeCodec<FormatType::INT, DataType::cname>},                              \
      {{FormatType::UINT, DataType::cname},                                    \
       _makeCodec<FormatType::UINT, DataType::cname>},                         \
      {{FormatType::UNORM, DataType::cname},                                   \
       _makeCodec<FormatType::UNORM, DataType::cname>},                        \
      {{FormatType::NORM, DataType::cname},                                    \
       _makeCodec<FormatType::NORM, DataType::cname>},

#define _MAKE_CODEC_FLOAT(cname)                                               \
  {{FormatType::FLOAT, DataType::cname},                                       \
   _makeCodec<FormatType::FLOAT, DataType::cname>},

#define _MAKE_CODEC_UFLOAT(cname)                                              \
  {{FormatType::UFLOAT, DataType::cname},                                      \
   _makeCodec<FormatType::UFLOAT, DataType::cname>},

namespace std {
template <> struct hash<FormatDescr> {
  constexpr uint32 operator()(const FormatDescr &t) const {
    return static_cast<uint32>(t.compType) | static_cast<uint32>(t.outType)
                                                 << 16;
  }
};
} // namespace std

static const std::unordered_map<FormatDescr, FormatCodec *(*)()> registry = {
    StaticFor(_MAKE_CODEC_BASE, R32G32B32A32, R32G32B32, R16G16B16A16, R32G32,
              R16G16B16, R32, R16G16, R10G10B10A2, R11G11B10, R8G8B8A8, R24G8,
              R8G8B8, R8G8, R16, R5G6B5, R5G5B5A1, R8)
        StaticFor(_MAKE_CODEC_FLOAT, R32G32B32A32, R32G32B32, R16G16B16A16,
                  R32G32, R16G16B16, R32, R16G16, R11G11B10, R16)
            StaticFor(_MAKE_CODEC_UFLOAT, R11G11B10)};

void FormatCodec::GetValue(IVector4A16 &, const char *) const {
  throw std::runtime_error("Invalid call for uni::format codec!");
}

void FormatCodec::GetValue(Vector4A16 &, const char *) const {
  throw std::runtime_error("Invalid call for uni::format codec!");
}

void FormatCodec::Sample(ivec &, const char *, size_t, size_t) const {
  throw std::runtime_error("Invalid call for uni::format codec!");
}

void FormatCodec::Sample(fvec &, const char *, size_t, size_t) const {
  throw std::runtime_error("Invalid call for uni::format codec!");
}

FormatCodec::ptr FormatCodec::Create(const FormatDescr &input) {
  auto ctor = registry.find(input);

  if (es::IsEnd(registry, ctor)) {
    return nullptr;
  }

  return FormatCodec::ptr(ctor->second());
}
