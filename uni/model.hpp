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

  virtual const char *RawBuffer() const = 0;
  virtual size_t Stride() const = 0;
  virtual size_t Offset() const = 0;
  virtual FormatDescr Type() const = 0;
  virtual BBOX UnpackData() const = 0;
  virtual UnpackDataType_e UnpackDataType() const = 0;
  virtual ~PrimitiveDescriptor() {}
};

typedef Element<const List<PrimitiveDescriptor>> PrimitiveDescriptorsConst;
typedef Element<List<PrimitiveDescriptor>> PrimitiveDescriptors;

class Primitive {
public:
  enum class IndexType_e { None, Line, Triangle, Strip, Fan };

  virtual const char *RawIndexBuffer() const = 0;
  virtual const char *RawVertexBuffer(size_t id) const = 0;
  virtual PrimitiveDescriptorsConst Descriptors() const = 0;
  virtual IndexType_e IndexType() const = 0;
  virtual size_t IndexSize() const = 0;
  virtual size_t NumVertices() const = 0;
  virtual size_t NumVertexBuffers() const = 0;
  virtual size_t NumIndices() const = 0;
  virtual ~Primitive() {}
};
} // namespace uni
