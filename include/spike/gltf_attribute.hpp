/*  GLTF attributes

    Copyright 2024 Lukas Cone

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
#include "uni/format.hpp"

enum class AttributeType : uint8 {
  Undefined,
  Position,
  Normal,
  Tangent,
  TextureCoordiante,
  BoneIndices,
  BoneWeights,
  VertexColor,
};

struct AttributeCodec {
  virtual ~AttributeCodec() = default;
  virtual void Sample(uni::FormatCodec::fvec &out, const char *input,
                      size_t stride = 0) const = 0;
  virtual void Transform(uni::FormatCodec::fvec &in) const = 0;
  virtual bool CanSample() const = 0;
  virtual bool CanTransform() const = 0;
  // Whenever Sample or Transform procudes normalized output
  virtual bool IsNormalized() const = 0;
};

struct AttributeUnormToSnorm : AttributeCodec {
  void Sample(uni::FormatCodec::fvec &, const char *, size_t) const override {}
  void Transform(uni::FormatCodec::fvec &in) const override {
    for (Vector4A16 &v : in) {
      v = v * 2 - 1;
    }
  }
  bool CanSample() const override { return false; }
  bool CanTransform() const override { return true; }
  bool IsNormalized() const override { return true; }
};

struct AttributeMad : AttributeCodec {
  void Sample(uni::FormatCodec::fvec &, const char *, size_t) const override {}
  void Transform(uni::FormatCodec::fvec &in) const override {
    for (Vector4A16 &v : in) {
      v = v * mul + add;
    }
  }
  bool CanSample() const override { return false; }
  bool CanTransform() const override { return true; }
  bool IsNormalized() const override { return false; }

  Vector4A16 mul;
  Vector4A16 add;
};

struct Attribute {
  uni::DataType type;
  uni::FormatType format;
  AttributeType usage;
  int8 offset = -1;
  AttributeCodec *customCodec = nullptr;
};
