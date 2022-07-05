/*  a source for Matrix44 class
    more info in README for PreCore Project

    Copyright 2018-2022 Lukas Cone

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

#include "matrix44.hpp"
#include "glm/ext.hpp"
#include "glm/glm.hpp"

using namespace es;

static const Vector4A16 &AsVec(const glm::vec4 &in) {
  return reinterpret_cast<const Vector4A16 &>(in);
}

static const Vector4A16 &AsVec(const glm::quat &in) {
  return reinterpret_cast<const Vector4A16 &>(in);
}

static const glm::quat &AsQuat(const Vector4A16 &in) {
  return reinterpret_cast<const glm::quat &>(in);
}

static const glm::vec4 &AsVec(const Vector4A16 &in) {
  return reinterpret_cast<const glm::vec4 &>(in);
}

static const Matrix44 &AsMat4(const glm::mat4 &in) {
  return reinterpret_cast<const Matrix44 &>(in);
}

static const glm::mat4 &AsMat4(const Matrix44 &in) {
  return reinterpret_cast<const glm::mat4 &>(in);
}

[[maybe_unused]] static glm::mat4 &AsMat4(Matrix44 &in) {
  return reinterpret_cast<glm::mat4 &>(in);
}

static_assert(sizeof(glm::mat4) == sizeof(Matrix44));
static_assert(alignof(glm::mat4) == alignof(Matrix44));
static_assert(sizeof(glm::vec4) == sizeof(Vector4A16));
static_assert(alignof(glm::vec4) == alignof(Vector4A16));
static_assert(sizeof(glm::quat) == sizeof(Vector4A16));
static_assert(alignof(glm::quat) == alignof(Vector4A16));

void Matrix44::Decompose(Vector4A16 &position, Vector4A16 &rotation,
                         Vector4A16 &scale) const {
  position = r4();
  scale.X = r1().Length();
  scale.Y = r2().Length();
  scale.Z = r3().Length();

  if (r1().Dot(Vector4A16(Vector(r2()).Cross(r3()), 0.0f)) < 0)
    scale *= -1;

  Matrix44 tmp(*this);
  tmp.r1() /= scale.X;
  tmp.r2() /= scale.Y;
  tmp.r3() /= scale.Z;
  rotation = tmp.ToQuat();
}

void Matrix44::Compose(const Vector4A16 &position, const Vector4A16 &rotation,
                       const Vector4A16 &scale) {
  FromQuat(rotation);
  r4() = position;
  r4().w = 1.f;
  r1() *= scale.X;
  r2() *= scale.Y;
  r3() *= scale.Z;
}

void Matrix44::MakeIdentity() {
  r1() = Vector4A16(1.0f, 0.0f, 0.0f, 0.0f);
  r2() = Vector4A16(0.0f, 1.0f, 0.0f, 0.0f);
  r3() = Vector4A16(0.0f, 0.0f, 1.0f, 0.0f);
  r4() = Vector4A16(0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix44::Matrix44() { MakeIdentity(); }

Matrix44::Matrix44(const Vector4A16 &quat) {
  MakeIdentity();
  FromQuat(quat);
}

void Matrix44::FromQuat(const Vector4A16 &q) {
  *this = AsMat4(glm::mat4_cast(glm::quat(AsQuat(q))));
}

Vector4A16 Matrix44::ToQuat() const {
  return AsVec(glm::quat_cast(AsMat4(*this)));
}

void Matrix44::Transpose() {
  __m128 tmp00 =
      _mm_shuffle_ps(r1()._data, r2()._data, _MM_SHUFFLE(1, 0, 1, 0));
  __m128 tmp01 =
      _mm_shuffle_ps(r1()._data, r2()._data, _MM_SHUFFLE(2, 1, 2, 1));

  r1()._data = _mm_shuffle_ps(tmp00, r3()._data, _MM_SHUFFLE(3, 0, 2, 0));
  r2()._data = _mm_shuffle_ps(tmp01, r3()._data, _MM_SHUFFLE(3, 1, 2, 0));
  r3()._data = _mm_shuffle_ps(tmp01, r3()._data, _MM_SHUFFLE(3, 2, 3, 1));
}

void Matrix44::TransposeFull() {
  _MM_TRANSPOSE4_PS(r1()._data, r2()._data, r3()._data, r4()._data);
}

namespace es {
Vector4A16 operator*(const Vector4A16 &point, const es::Matrix44 &mtx) {
  auto result = AsVec(point) * AsMat4(mtx);
  return AsVec(result);
}
} // namespace es

Matrix44 Matrix44::operator*(const Matrix44 &right) const {
  auto &thisMat = AsMat4(*this);
  auto &rightMat = AsMat4(right);
  return AsMat4(thisMat * rightMat);
}

Matrix44 Matrix44::operator-() const {
  auto thisMat = glm::inverse(AsMat4(*this));
  return AsMat4(thisMat);
}
