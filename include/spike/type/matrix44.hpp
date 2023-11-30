/*  Matrix44 class is a simple affine matrix 4x4

    Copyright 2018-2023 Lukas Cone

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
#include "spike/type/vectors_simd.hpp"
#include "spike/util/settings.hpp"

namespace es {
class Matrix44 {
public:
  mreal32x4a16 v[4];
  PC_EXTERN Matrix44();
  Matrix44(real32x4a16 &row1, real32x4a16 &row2,
           real32x4a16 &row3)
      : v{row1, row2, row3, {0, 0, 0, 1}} {}
  Matrix44(real32x4a16 &row1, real32x4a16 &row2,
           real32x4a16 &row3, real32x4a16 &row4)
      : v{row1, row2, row3, row4} {}
  PC_EXTERN Matrix44(real32x4a16 &quat);
  Matrix44(real32x4a16 *rows) : v{rows[0], rows[1], rows[2], rows[3]} {}

  void PC_EXTERN MakeIdentity();
  void PC_EXTERN Decompose(mreal32x4a16 &position, mreal32x4a16 &rotation,
                           mreal32x4a16 &scale) const;
  void PC_EXTERN Compose(real32x4a16 &position, real32x4a16 &rotation,
                         real32x4a16 &scale);
  void PC_EXTERN FromQuat(real32x4a16 &q);
  real32x4a16 PC_EXTERN ToQuat() const;
  // Transpose 3x3
  void PC_EXTERN Transpose();
  // Transpose 4x4
  void PC_EXTERN TransposeFull();

  Matrix44 &operator*=(const Matrix44 &right) { return *this = *this * right; }

  real32x4a16 &operator[](size_t index) const { return v[index]; }
  mreal32x4a16 &operator[](size_t index) { return v[index]; }

  real32x4a16 &r1() const { return v[0]; }
  real32x4a16 &r2() const { return v[1]; }
  real32x4a16 &r3() const { return v[2]; }
  real32x4a16 &r4() const { return v[3]; }
  mreal32x4a16 &r1() { return v[0]; }
  mreal32x4a16 &r2() { return v[1]; }
  mreal32x4a16 &r3() { return v[2]; }
  mreal32x4a16 &r4() { return v[3]; }

  Matrix44 PC_EXTERN operator*(const Matrix44 &right) const;
  friend real32x4a16 PC_EXTERN operator*(real32x4a16 &point,
                                        const Matrix44 &mtx);
  Matrix44 PC_EXTERN operator-() const;

  void SwapEndian() { FByteswapper(v); }
};
} // namespace es
