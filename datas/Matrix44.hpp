/*      esMatrix44 class is a simple affine matrix 4x4
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

#pragma once
#include "VectorsSimd.hpp"

class esMatrix44 {
public:
  Vector4A16 r1, r2, r3, r4;
  esMatrix44();
  esMatrix44(const Vector4A16 &row1, const Vector4A16 &row2,
             const Vector4A16 &row3)
      : r1(row1), r2(row2), r3(row3) {}
  esMatrix44(const Vector4A16 &quat);
  esMatrix44(const Vector4A16 *rows)
      : r1(rows[0]), r2(rows[1]), r3(rows[2]), r4(rows[3]) {}

  void MakeIdentity();
  void Decompose(Vector4A16 &position, Vector4A16 &rotation,
                 Vector4A16 &scale) const;
  void Compose(const Vector4A16 &position, const Vector4A16 &rotation,
               const Vector4A16 &scale);
  Vector4A16 RotatePoint(const Vector4A16 &input) const;
  void FromQuat(const Vector4A16 &q);
  Vector4A16 ToQuat() const;
};