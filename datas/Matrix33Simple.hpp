/*	Matrix33s class is a simple matrix3x3
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
#include "vectors.hpp"
class Matrix33s
{
public:
	Vector r1, r2, r3, r4;
	Matrix33s();
	Matrix33s(Vector row1, Vector row2, Vector row3):r1(row1), r2(row2) , r3(row3) {}
	Matrix33s(const Vector4 &quat);
	Matrix33s(Vector *rows) :r1(rows[0]), r2(rows[1]), r3(rows[2]) {}

	
	//Vector GetScale(){return Vector(r1.Length(),r2.Length(),r3.Length());} //orthogonal only
	//void Scale(Vector &input){r1 *= Vector(input.X,input.X,input.X);r2 *= Vector(input.Y,input.Y,input.Y);r2 *= Vector(input.Z,input.Z,input.Z);} //orthogonal only
	
	void Multiply(Matrix33s &input);
	void Transpose();
	void Inverse();
	void Normalize();
	void MakeIdentity();
	void Decompose(Vector &position, Vector4 &rotation, Vector &scale)const;
	void Compose(const Vector &position, const Vector4 &rotation, const Vector &scale);
	float Determinant() const;
	bool IsIdentity()const;
	bool IsOrthogonal()const;
	Matrix33s TransposeCopy() const;
	Matrix33s InverseCopy()const;
	Matrix33s MultiplyCopy(Matrix33s &input)const;
	Matrix33s NormalizeCopy() const;
	Vector RotatePoint(const Vector &input) const;
	void FromQuat(const Vector4 & q);
	void ToQuat(Vector4 & q)const;
};