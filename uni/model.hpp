/*  render model module
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
#include "format.hpp"
#include "list.hpp"
#include <vector>

namespace uni {
struct BBOX {
  Vector4A16 min;
  Vector4A16 max;
};

class PrimitiveDescriptor {
public:
  enum class UnpackDataType_e {
    Add,  // x + min
    Mul,  // x * min
    Madd, // max + x * min
    Lerp  // min + (max - min) * x, x = [0, 1]
  };

  const char *RawBuffer() const = 0;
  size_t Stride() const = 0;
  size_t Offset() const = 0;
  FormatDescr Type() const = 0;
  BBOX UnpackData() const = 0;
  UnpackDataType_e UnpackDataType() const = 0;
};

typedef Element<const List<PrimitiveDescriptor>> PrimitiveDescriptorsConst;
typedef Element<List<PrimitiveDescriptor>> PrimitiveDescriptors;

class Primitive {
public:
  enum class IndexType_e { None, Line, Triangle, Strip, Fan };

  const char *RawIndexBuffer() const = 0;
  const char *RawVertexBuffer(size_t id) const = 0;
  PrimitiveDescriptorsConst Descriptors() const = 0;
  IndexType_e IndexType() const = 0;
  size_t IndexSize() const = 0;
  size_t NumVertices() const = 0;
  size_t NumVertexBuffers() const = 0;
  size_t NumIndices() const = 0;
} // namespace uni
