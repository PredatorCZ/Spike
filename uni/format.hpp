/*  uni format module
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

#pragma once
#include "datas/VectorsSimd.hpp"

namespace uni {
enum class FormatType : uint16 { INT, UINT, NORM, UNORM, FLOAT, UFLOAT };

enum class DataType : uint16 {
  UNKNOWN,
  R32G32B32A32, // 128
  R32G32B32,    // 96
  R16G16B16A16, // 64
  R32G32,       // 64
  R16G16B16,    // 48
  R32,          // 32
  R16G16,       // 32
  R10G10B10A2,  // 32
  R11G11B10,    // 32
  R8G8B8A8,     // 32
  R24G8,        // 32
  R8G8B8,       // 24
  R8G8,         // 16
  R16,          // 16
  R5G6B5,       // 16
  R5G5B5A1,     // 16
  R8,           // 8
};

/*  SUPPORTED COMBINATIONS
INT, UINT, NORM, UNORM:
  [ALL]
FLOAT:
  [IEEE-754 standard, single]
  R32G32B32A32
  R32G32B32
  R32G32
  R32
  [IEEE-754 standard, half]
  R16G16B16A16
  R16G16B16
  R16G16
  R16
  R11G11B10 [sign bit, 5b exponent, 4b/5b mantissa]
UFLOAT:
  R11G11B10 [5b exponent, 5b/6b mantissa]
*/

struct FormatDescr {
  FormatType outType;
  DataType compType;

  bool operator==(const FormatDescr &input) const {
    return reinterpret_cast<const uint32 &>(input) ==
           reinterpret_cast<const uint32 &>(*this);
  }
};

class FormatCodec {
public:
  typedef std::unique_ptr<FormatCodec> ptr;
  virtual void GetValue(IVector4A16 &out, const char *input) const;
  virtual void GetValue(Vector4A16 &out, const char *input) const;

  static ptr Create(const FormatDescr &input);
};

template <FormatType, DataType> class FormatCodec_t : public FormatCodec {};

} // namespace uni

#include "internal/format.inl"
