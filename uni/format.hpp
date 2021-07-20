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
#include "datas/vectors_simd.hpp"
#include "datas/settings.hpp"
#include <memory>
#include <vector>

namespace uni {
enum class FormatType : uint16 {
  INVALID,
  INT,
  UINT,
  NORM,
  UNORM,
  FLOAT,
  UFLOAT
};

enum class DataType : uint16 {
  CUSTOM,
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

  bool operator<(const FormatDescr &input) const {
    return reinterpret_cast<const uint32 &>(input) >
           reinterpret_cast<const uint32 &>(*this);
  }
};

class FormatCodec {
public:
  typedef std::vector<IVector4A16> ivec;
  typedef std::vector<Vector4A16> fvec;

  // Samples single integer value
  // exceptions:
  //             runtime_error for incorrect call (format isn't int)
  virtual void PC_EXTERN GetValue(IVector4A16 &out, const char *input) const;

  // Samples single float value
  // exceptions:
  //             runtime_error for incorrect call (format is int)
  virtual void PC_EXTERN GetValue(Vector4A16 &out, const char *input) const;

  // Samples an array of values
  // count: if 0, count will be taken from 'out.size()'
  // stride: if 0, stride will be taken from format's size
  // exceptions:
  //             runtime_error if 0 < stride < format size
  //             runtime_error for incorrect call (format isn't int)
  virtual void PC_EXTERN Sample(ivec &out, const char *input, size_t count,
                      size_t stride = 0) const;

  // Samples an array of values
  // count: if 0, count will be taken from 'out.size()'
  // stride: if 0, stride will be taken from format's size
  // exceptions:
  //             runtime_error if 0 < stride < format size
  //             runtime_error for incorrect call (format is int)
  virtual void PC_EXTERN Sample(fvec &out, const char *input, size_t count,
                      size_t stride = 0) const;

  static FormatCodec PC_EXTERN &Get(const FormatDescr &input);
};

} // namespace uni
