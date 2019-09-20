/*      a source for esMatrix44 class
        more info in README for PreCore Project

        Copyright 2018-2019 Lukas Cone

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

#include "Matrix44.hpp"

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

esMatrix44::esMatrix44(const Vector4A16 &quat) { FromQuat(quat); }

Vector4A16 esMatrix44::RotatePoint(const Vector4A16 &point) const {
  return {r1.X * point.X + r2.X * point.Y + r3.X * point.Z,
          r1.Y * point.X + r2.Y * point.Y + r3.Y * point.Z,
          r1.Z * point.X + r2.Z * point.Y + r3.Z * point.Z, 0.0f};
}

void esMatrix44::FromQuat(const Vector4A16 &q) {
  const float xx = powf(q.X, 2);
  const float xy = q.X * q.Y;
  const float xz = q.X * q.Z;
  const float xw = q.X * q.W;

  const float yy = powf(q.Y, 2);
  const float yz = q.Y * q.Z;
  const float yw = q.Y * q.W;

  const float zz = powf(q.Z, 2);
  const float zw = q.Z * q.W;

  r1 = Vector4A16(1.f - 2.f * (yy + zz), 2.f * (xy - zw), 2.f * (xz + yw), 0.f);
  r2 = Vector4A16(2.f * (xy + zw), 1.f - 2.f * (xx + zz), 2.f * (yz - xw), 0.f);
  r3 = Vector4A16(2.f * (xz - yw), 2.f * (yz + xw), 1.f - 2.f * (xx + yy), 0.f);
}

Vector4A16 esMatrix44::ToQuat() const {
  float trace = 0;
  Vector4A16 q;

  if (r3.Z + FLT_EPSILON < 0.f) {
    if (r1.X > r2.Y) {
      trace = 1 + r1.X - r2.Y - r3.Z;
      q.X = trace;
      q.Y = r2.X + r1.Y;
      q.Z = r1.Z + r3.X;
      q.W = r3.Y - r2.Z;
    } else {
      trace = 1 - r1.X + r2.Y - r3.Z;
      q.X = r2.X + r1.Y;
      q.Y = trace;
      q.Z = r3.Y + r2.Z;
      q.W = r1.Z - r3.X;
    }
  } else {
    if (r1.X < -r2.Y) {
      trace = 1 - r1.X - r2.Y + r3.Z;
      q.X = r1.Z + r3.X;
      q.Y = r3.Y + r2.Z;
      q.Z = trace;
      q.W = r2.X - r1.Y;
    } else {
      trace = 1 + r1.X + r2.Y + r3.Z;
      q.X = r3.Y - r2.Z;
      q.Y = r1.Z - r3.X;
      q.Z = r2.X - r1.Y;
      q.W = trace;
    }
  }

  return q *= 0.5f / sqrtf(trace);
}