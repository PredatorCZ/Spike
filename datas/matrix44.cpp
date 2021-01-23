/*  a source for esMatrix44 class
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

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

void esMatrix44::Decompose(Vector4A16 &position, Vector4A16 &rotation,
                           Vector4A16 &scale) const {
  position = r4;
  scale.X = r1.Length();
  scale.Y = r2.Length();
  scale.Z = r3.Length();

  if (r1.Dot(Vector4A16(Vector(r2).Cross(r3), 0.0f)) < 0)
    scale *= -1;

  esMatrix44 tmp(*this);
  tmp.r1 /= scale.X;
  tmp.r2 /= scale.Y;
  tmp.r3 /= scale.Z;
  rotation = tmp.ToQuat();
}

void esMatrix44::Compose(const Vector4A16 &position, const Vector4A16 &rotation,
                         const Vector4A16 &scale) {
  FromQuat(rotation);
  r4 = position;
  r1 *= scale.X;
  r2 *= scale.Y;
  r3 *= scale.Z;
}

void esMatrix44::MakeIdentity() {
  r1 = Vector4A16(1.0f, 0.0f, 0.0f, 0.0f);
  r2 = Vector4A16(0.0f, 1.0f, 0.0f, 0.0f);
  r3 = Vector4A16(0.0f, 0.0f, 1.0f, 0.0f);
  r4 = Vector4A16(0.0f, 0.0f, 0.0f, 1.0f);
}

esMatrix44::esMatrix44() { MakeIdentity(); }

esMatrix44::esMatrix44(const Vector4A16 &quat) {
  MakeIdentity();
  FromQuat(quat);
}

Vector4A16 esMatrix44::RotatePoint(const Vector4A16 &point) const {
  auto mtxCopy = *this;
  mtxCopy.Transpose();

  const float v0 = mtxCopy.r1.Dot(point);
  const float v1 = mtxCopy.r2.Dot(point);
  const float v2 = mtxCopy.r3.Dot(point);

  return Vector4A16(v0, v1, v2, 1.0f);
}

void esMatrix44::FromQuat(const Vector4A16 &q) {
  Vector4A16 xo(q.X);
  xo *= q;

  Vector4A16 yo(q.Y);
  yo *= q;

  Vector4A16 zo(q.Z);
  zo *= q;

  // yo.z, yo.y, xo.z, xo.w
  const auto tmp00 =
      _mm_shuffle_ps(yo._data, xo._data, _MM_SHUFFLE(3, 2, 1, 2));
  // xo.y, xo.x, yo.z, yo.y
  const Vector4A16 r20(
      _mm_shuffle_ps(xo._data, yo._data, _MM_SHUFFLE(1, 2, 0, 1)));
  // yo.y, xo.y, xo.z, xo.z
  const Vector4A16 r10(
      _mm_shuffle_ps(r20._data, xo._data, _MM_SHUFFLE(2, 2, 0, 3)));
  // xo.z, yo.z, xo.x, xo.x
  const Vector4A16 r30(
      _mm_shuffle_ps(tmp00, xo._data, _MM_SHUFFLE(0, 0, 0, 2)));

  // zo.z, zo.w, yo.w, yo.w
  const Vector4A16 r11 =
      Vector4A16(_mm_shuffle_ps(zo._data, yo._data, _MM_SHUFFLE(3, 3, 3, 2))) *
      Vector4A16(1.f, -1.f, 1.f, 0.f);
  // zo.w, zo.z, xo.w, xo.w
  const Vector4A16 r21 =
      Vector4A16(_mm_shuffle_ps(zo._data, xo._data, _MM_SHUFFLE(3, 3, 2, 3))) *
      Vector4A16(1.f, 1.f, -1.f, 0.f);
  // xo.w, yo.y, yo.w, yo.w
  const auto tmp06 = _mm_shuffle_ps(tmp00, yo._data, _MM_SHUFFLE(3, 3, 1, 3));
  // yo.w, xo.w, yo.y, yo.y
  const Vector4A16 r31 =
      Vector4A16(_mm_shuffle_ps(tmp06, tmp06, _MM_SHUFFLE(1, 1, 0, 3))) *
      Vector4A16(-1.f, 1.f, 1.f, 0.f);

  r1 = Vector4A16(1.0f, 0.0f, 0.0f, 0.0f) +
       (r10 + r11) * Vector4A16(-2.f, 2.0f, 2.0f, 0.0f);
  r2 = Vector4A16(0.0f, 1.0f, 0.0f, 0.0f) +
       (r20 + r21) * Vector4A16(2.f, -2.0f, 2.0f, 0.0f);
  r3 = Vector4A16(0.0f, 0.0f, 1.0f, 0.0f) +
       (r30 + r31) * Vector4A16(2.f, 2.0f, -2.0f, 0.0f);
}

Vector4A16 esMatrix44::ToQuat() const {
  const bool traceType0 = r3.Z < 0.f;
  const bool traceType1 = traceType0 ? r1.X > r2.Y : r1.X < -r2.Y;
  // r1.x, r1.z, r2.x, r2.z
  const auto tmp10 =
      _mm_shuffle_ps(r1._data, r2._data, _MM_SHUFFLE(2, 0, 2, 0));
  // r3.x, r1.y, r1.z, r1.w
  const auto tmp11 = _mm_move_ss(r1._data, r3._data);

  // r1.x, r2.z, r2.x, r1.z
  Vector4A16 tmp00(_mm_shuffle_ps(tmp10, tmp10, _MM_SHUFFLE(1, 2, 3, 0)));
  // r3.z, r3.y, r1.y, r3.x
  Vector4A16 tmp01(_mm_shuffle_ps(r3._data, tmp11, _MM_SHUFFLE(0, 1, 1, 2)));
  // r2.y, 0, 0, 0
  Vector4A16 tmp03(_mm_shuffle_ps(r2._data, r2._data, _MM_SHUFFLE(3, 3, 3, 1)));

  if (traceType0) {
    if (traceType1) {
      tmp00 *= Vector4A16(1.f, -1.f, 1.f, 1.f);
      tmp01 *= Vector4A16(-1.f, 1.f, 1.f, 1.f);
      tmp03 *= -1.f;
      tmp00 = Vector4A16(1.f, 0, 0, 0) + tmp00 + tmp01 + tmp03;
      tmp00 *= 0.5f / sqrtf(tmp00.X);

      return Vector4A16(
          _mm_shuffle_ps(tmp00._data, tmp00._data, _MM_SHUFFLE(1, 3, 2, 0)));
    } else {
      tmp00 *= Vector4A16(-1.f, 1.f, 1.f, 1.f);
      tmp01 *= Vector4A16(-1.f, 1.f, 1.f, -1.f);
      tmp00 = Vector4A16(1.f, 0, 0, 0) + tmp00 + tmp01 + tmp03;
      tmp00 *= 0.5f / sqrtf(tmp00.X);
      tmp00 *= -1.0f;

      return Vector4A16(
          _mm_shuffle_ps(tmp00._data, tmp00._data, _MM_SHUFFLE(3, 1, 0, 2)));
    }
  } else {
    if (traceType1) {
      tmp00 *= Vector4A16(-1.f, 1.f, 1.f, 1.f);
      tmp01 *= Vector4A16(1.f, 1.f, -1.f, 1.f);
      tmp03 *= -1.f;
      tmp00 = Vector4A16(1.f, 0, 0, 0) + tmp00 + tmp01 + tmp03;
      tmp00 *= 0.5f / sqrtf(tmp00.X);

      return Vector4A16(
          _mm_shuffle_ps(tmp00._data, tmp00._data, _MM_SHUFFLE(2, 0, 1, 3)));
    } else {
      tmp00 *= Vector4A16(1.f, -1.f, 1.f, 1.f);
      tmp01 *= Vector4A16(1.f, 1.f, -1.f, -1.f);
      tmp00 = Vector4A16(1.f, 0, 0, 0) + tmp00 + tmp01 + tmp03;
      tmp00 *= 0.5f / sqrtf(tmp00.X);

      return Vector4A16(
          _mm_shuffle_ps(tmp00._data, tmp00._data, _MM_SHUFFLE(0, 2, 3, 1)));
    }
  }
}

void esMatrix44::Transpose() {
  __m128 tmp00 = _mm_shuffle_ps(r1._data, r2._data, _MM_SHUFFLE(1, 0, 1, 0));
  __m128 tmp01 = _mm_shuffle_ps(r1._data, r2._data, _MM_SHUFFLE(2, 1, 2, 1));

  r1._data = _mm_shuffle_ps(tmp00, r3._data, _MM_SHUFFLE(3, 0, 2, 0));
  r2._data = _mm_shuffle_ps(tmp01, r3._data, _MM_SHUFFLE(3, 1, 2, 0));
  r3._data = _mm_shuffle_ps(tmp01, r3._data, _MM_SHUFFLE(3, 2, 3, 1));
}

esMatrix44 &esMatrix44::operator*=(const esMatrix44 &right) {
  esMatrix44 rCopy(right);
  rCopy.Transpose();

  const float v00 = r1.Dot(rCopy.r1);
  const float v01 = r1.Dot(rCopy.r2);
  const float v02 = r1.Dot(rCopy.r3);

  const float v10 = r2.Dot(rCopy.r1);
  const float v11 = r2.Dot(rCopy.r2);
  const float v12 = r2.Dot(rCopy.r3);

  const float v20 = r3.Dot(rCopy.r1);
  const float v21 = r3.Dot(rCopy.r2);
  const float v22 = r3.Dot(rCopy.r3);

  const float v30 = rCopy.r1.Dot(r4);
  const float v31 = rCopy.r2.Dot(r4);
  const float v32 = rCopy.r3.Dot(r4);

  r1 = Vector4A16(v00, v01, v02, 0.0f);
  r2 = Vector4A16(v10, v11, v12, 0.0f);
  r3 = Vector4A16(v20, v21, v22, 0.0f);
  r4 = right.r4 + Vector4A16(v30, v31, v32, 0.0f);

  return *this;
}

// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html#_transform_matrix_inverse
esMatrix44 esMatrix44::operator-() const {
  esMatrix44 retVal(*this);
  retVal.Transpose();

  Vector4A16 sizeSqr = retVal.r1 * retVal.r1;
  sizeSqr += retVal.r2 * retVal.r2;
  sizeSqr += retVal.r3 * retVal.r3;

  const Vector4A16 vEps(0.000001f);
  const Vector4A16 oneVal(1.0f);
  const Vector4A16 ltMask(_mm_cmplt_ps(sizeSqr._data, vEps._data));

  sizeSqr = oneVal / ((ltMask & oneVal) + (~ltMask & sizeSqr));

  retVal.r1 *= sizeSqr;
  retVal.r2 *= sizeSqr;
  retVal.r3 *= sizeSqr;

  const float v30 = r1.Dot(r4);
  const float v31 = r2.Dot(r4);
  const float v32 = r3.Dot(r4);

  retVal.r4 = Vector4A16(v30, v31, v32, 1.0f).QConjugate();

  return retVal;
}
