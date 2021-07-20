/*  esMatrix44 class is a simple affine matrix 4x4
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

#pragma once
#include "vectors_simd.hpp"
#include "settings.hpp"

class esMatrix44 {
public:
  Vector4A16 v[4];
  PC_EXTERN esMatrix44();
  esMatrix44(const Vector4A16 &row1, const Vector4A16 &row2,
             const Vector4A16 &row3)
      : v{row1, row2, row3, {}} {}
  esMatrix44(const Vector4A16 &row1, const Vector4A16 &row2,
             const Vector4A16 &row3, const Vector4A16 &row4)
      : v{row1, row2, row3, row4} {}
  PC_EXTERN esMatrix44(const Vector4A16 &quat);
  esMatrix44(const Vector4A16 *rows) : v{rows[0], rows[1], rows[2], rows[3]} {}

  void PC_EXTERN MakeIdentity();
  void PC_EXTERN Decompose(Vector4A16 &position, Vector4A16 &rotation,
                 Vector4A16 &scale) const;
  void PC_EXTERN Compose(const Vector4A16 &position, const Vector4A16 &rotation,
               const Vector4A16 &scale);
  Vector4A16 PC_EXTERN RotatePoint(const Vector4A16 &input) const;
  void PC_EXTERN FromQuat(const Vector4A16 &q);
  Vector4A16 PC_EXTERN ToQuat() const;
  // Transpose 3x3
  void PC_EXTERN Transpose();
  // Transpose 4x4
  void PC_EXTERN TransposeFull();

  esMatrix44 operator*(const esMatrix44 &right) const {
    return esMatrix44(*this) *= right;
  }

  const Vector4A16 &operator[](size_t index) const { return v[index]; }
  Vector4A16 &operator[](size_t index) { return v[index]; }

  const Vector4A16 &r1() const { return v[0]; }
  const Vector4A16 &r2() const { return v[1]; }
  const Vector4A16 &r3() const { return v[2]; }
  const Vector4A16 &r4() const { return v[3]; }
  Vector4A16 &r1() { return v[0]; }
  Vector4A16 &r2() { return v[1]; }
  Vector4A16 &r3() { return v[2]; }
  Vector4A16 &r4() { return v[3]; }

  esMatrix44 PC_EXTERN &operator*=(const esMatrix44 &right);
  esMatrix44 PC_EXTERN operator-() const;

  void SwapEndian() { FByteswapper(v); }
};
