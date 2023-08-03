/*  render model module
    part of uni module
    Copyright 2020-2023 Lukas Cone

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
#include "format.hpp"
#include "list.hpp"
#include "spike/type/matrix44.hpp"

namespace uni {
struct RTSValue;
struct BBOX {
  Vector4A16 min;
  Vector4A16 max;
};

class PrimitiveDescriptor : public Base {
public:
  enum class UnpackDataType_e {
    None,
    Add,  // x + min
    Mul,  // x * min
    Madd, // max + x * min
  };

  enum class Usage_e : uint8 {
    Undefined,
    Position,
    Normal,
    Tangent,
    BiTangent,
    TextureCoordiante,
    BoneIndices,
    BoneWeights,
    VertexColor,
    VertexIndex,
    PositionDelta,
  };

  // Get already indexed & offseted vertex buffer
  virtual const char *RawBuffer() const = 0;
  virtual size_t Stride() const = 0;
  virtual size_t Offset() const = 0;
  virtual size_t Index() const = 0;
  virtual Usage_e Usage() const = 0;
  virtual FormatDescr Type() const = 0;
  virtual FormatCodec &Codec() const { return FormatCodec::Get(Type()); }
  virtual BBOX UnpackData() const = 0;
  virtual UnpackDataType_e UnpackDataType() const = 0;
  void Resample(FormatCodec::fvec &data) const;
  void Resample(FormatCodec::fvec &data, const es::Matrix44 &transform) const;
};

typedef Element<const List<PrimitiveDescriptor>> PrimitiveDescriptorsConst;
typedef Element<List<PrimitiveDescriptor>> PrimitiveDescriptors;

class IndexArray : public Base {
public:
  virtual const char *RawIndexBuffer() const = 0;
  virtual size_t IndexSize() const = 0;
  virtual size_t NumIndices() const = 0;
};

typedef Element<const List<IndexArray>> IndexArraysConst;
typedef Element<List<IndexArray>> IndexArrays;

class VertexArray : public Base {
public:
  virtual PrimitiveDescriptorsConst Descriptors() const = 0;
  virtual size_t NumVertices() const = 0;
};

typedef Element<const List<VertexArray>> VertexArraysConst;
typedef Element<List<VertexArray>> VertexArrays;

class Primitive : public Base {
public:
  enum class IndexType_e { None, Line, Triangle, Strip, Fan };

  virtual IndexType_e IndexType() const = 0;
  virtual std::string Name() const = 0;
  virtual size_t SkinIndex() const = 0;
  virtual int64 LODIndex() const = 0;
  virtual size_t MaterialIndex() const = 0;
  virtual size_t VertexArrayIndex(size_t id) const = 0;
  virtual size_t IndexArrayIndex() const = 0;
  virtual size_t NumVertexArrays() const = 0;
};

typedef Element<const List<Primitive>> PrimitivesConst;
typedef Element<List<Primitive>> Primitives;

class PC_EXTERN Skin : public Base {
public:
  virtual size_t NumNodes() const = 0;
  virtual TransformType TMType() const = 0;
  virtual void GetTM(RTSValue &out, size_t index) const;
  virtual void GetTM(es::Matrix44 &out, size_t index) const;
  virtual size_t NodeIndex(size_t index) const = 0;
};

typedef Element<const List<Skin>> SkinsConst;
typedef Element<List<Skin>> Skins;

using ResourcesConst = Element<const List<std::string>>;
using Resources = Element<List<std::string>>;

class Material : public Base {
public:
  virtual size_t Version() const = 0;
  virtual std::string Name() const = 0;
  virtual std::string TypeName() const = 0;
};

using MaterialsConst = Element<const List<Material>>;
using Materials = Element<List<Material>>;

class Model : public Base {
public:
  virtual PrimitivesConst Primitives() const = 0;
  virtual IndexArraysConst Indices() const = 0;
  virtual VertexArraysConst Vertices() const = 0;
  virtual SkinsConst Skins() const = 0;
  virtual ResourcesConst Resources() const = 0;
  virtual MaterialsConst Materials() const = 0;
};
} // namespace uni

#include "detail/model.inl"
