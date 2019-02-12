/*	a source for Matrix33s class
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
#include "Matrix33Simple.hpp"

float Matrix33s::Determinant() const { return r1.X*r2.Y*r3.Z + r2.X*r3.Y*r1.Z + r3.X*r1.Y*r2.Z - r1.Z*r2.Y*r3.X - r1.X*r2.Z*r3.Y - r1.Y*r2.X*r3.Z; }
	
Matrix33s _Multiply(const Matrix33s &left, const Matrix33s &right)
{
	Matrix33s result(left);
	
	result.r1.X = left.r1.X*right.r1.X + left.r1.Y*right.r2.X + left.r1.Z*right.r3.X;
	result.r1.Y = left.r1.X*right.r1.Y + left.r1.Y*right.r2.Y + left.r1.Z*right.r3.Y;
	result.r1.Z = left.r1.X*right.r1.Z + left.r1.Y*right.r2.Z + left.r1.Z*right.r3.Z;

	result.r2.X = left.r2.X*right.r1.X + left.r2.Y*right.r2.X + left.r2.Z*right.r3.X;
	result.r2.Y = left.r2.X*right.r1.Y + left.r2.Y*right.r2.Y + left.r2.Z*right.r3.Y;
	result.r2.Z = left.r2.X*right.r1.Z + left.r2.Y*right.r2.Z + left.r2.Z*right.r3.Z;

	result.r3.X = left.r3.X*right.r1.X + left.r3.Y*right.r2.X + left.r3.Z*right.r3.X;
	result.r3.Y = left.r3.X*right.r1.Y + left.r3.Y*right.r2.Y + left.r3.Z*right.r3.Y;
	result.r3.Z = left.r3.X*right.r1.Z + left.r3.Y*right.r2.Z + left.r3.Z*right.r3.Z;
	
	result.r4 = right.r4 + right.RotatePoint(left.r4);

	return result;
}
#define _mtx33sidenproxima(num,num2) num - 1 < FLT_EPSILON

bool Matrix33s::IsIdentity() const
{
	const float epsilon = FLT_EPSILON;
	return (r1.X - 1 < epsilon) & (r1.Y < epsilon) & (r1.Z < epsilon) & (r2.X < epsilon) & (r2.Y - 1 < epsilon) & (r2.Z < epsilon) & (r3.X < epsilon) & (r3.Y < epsilon) & (r3.Z - 1 < epsilon);
}
bool Matrix33s::IsOrthogonal() const
{
	Matrix33s result = TransposeCopy();
	return _Multiply(result, *this).IsIdentity();
}

void Matrix33s::Multiply(Matrix33s &input) { *this = _Multiply(*this,input); }
Matrix33s Matrix33s::MultiplyCopy(Matrix33s &input)const { return _Multiply(*this, input); }

Matrix33s _Transpose(const Matrix33s &mtx)
{
	Matrix33s result(mtx);
	result.r1 = Vector(mtx.r1.X, mtx.r2.X, mtx.r3.X);
	result.r2 = Vector(mtx.r1.Y, mtx.r2.Y, mtx.r3.Y);
	result.r3 = Vector(mtx.r1.Z, mtx.r2.Z, mtx.r3.Z);
	return result;
}

void Matrix33s::Transpose() { *this = _Transpose(*this); }
Matrix33s Matrix33s::TransposeCopy() const { return _Transpose(*this); }

Matrix33s _Inverse(const Matrix33s &mtx)
{
	float idet = 1.f / mtx.Determinant();
	Matrix33s result(mtx);
	result.r1.X = (mtx.r2.Y*mtx.r3.Z - mtx.r3.Y*mtx.r2.Z)*idet;
	result.r1.Y = -(mtx.r1.Y*mtx.r3.Z - mtx.r3.Y*mtx.r1.Z)*idet;
	result.r1.Z = (mtx.r1.Y*mtx.r2.Z - mtx.r2.Y*mtx.r1.Z)*idet;
	result.r2.X = -(mtx.r2.X*mtx.r3.Z - mtx.r3.X*mtx.r2.Z)*idet;
	result.r2.Y = (mtx.r1.X*mtx.r3.Z - mtx.r3.X*mtx.r1.Z)*idet;
	result.r2.Z = -(mtx.r1.X*mtx.r2.Z - mtx.r1.Z*mtx.r2.X)*idet;
	result.r3.X = (mtx.r2.X*mtx.r3.Y - mtx.r3.X*mtx.r2.Y)*idet;
	result.r3.Y = -(mtx.r1.X*mtx.r3.Y - mtx.r3.X*mtx.r1.Y)*idet;
	result.r3.Z = (mtx.r1.X*mtx.r2.Y - mtx.r2.X*mtx.r1.Y)*idet;
	
	result.r4 = -result.RotatePoint(mtx.r4);

	return result;
}

void Matrix33s::Inverse() { *this = _Inverse(*this); }
Matrix33s Matrix33s::InverseCopy() const { return _Inverse(*this); }

void _Normalize(Matrix33s &mtx) { mtx.r1.Normalize(); mtx.r2.Normalize(); mtx.r3.Normalize(); }
void Matrix33s::Normalize() { _Normalize(*this); }

void Matrix33s::MakeIdentity() { r1 = Vector(1, 0, 0); r2 = Vector(0, 1, 0); r3 = Vector(0, 0, 1); r4 = Vector(0, 0, 0); }

void Matrix33s::Decompose(Vector & position, Vector4 & rotation, Vector & scale)const
{
	position = r4;
	scale.X = r1.Length();
	scale.Y = r2.Length();
	scale.Z = r3.Length();

	if (r1.Dot(r2.Cross(r3)) < 0)
		scale *= -1;

	Matrix33s tmp(*this);
	tmp.r1 /= scale.X;
	tmp.r2 /= scale.Y;
	tmp.r3 /= scale.Z;
	tmp.ToQuat(rotation);
}

void Matrix33s::Compose(const Vector & position, const Vector4 & rotation, const Vector & scale)
{
	FromQuat(rotation);
	r4 = position;
	r1 *= scale.X;
	r2 *= scale.Y;
	r3 *= scale.Z;
}

Matrix33s Matrix33s::NormalizeCopy() const { Matrix33s result(*this); result.Normalize(); return result; }

Matrix33s::Matrix33s() { MakeIdentity(); }

Matrix33s::Matrix33s(const Vector4 & quat) { FromQuat(quat); }

Vector Matrix33s::RotatePoint(const Vector &point) const
{ 
	Vector res;
	res.X = r1.X*point.X + r2.X*point.Y + r3.X*point.Z;
	res.Y = r1.Y*point.X + r2.Y*point.Y + r3.Y*point.Z;
	res.Z = r1.Z*point.X + r2.Z*point.Y + r3.Z*point.Z;
	return res; 
}

void Matrix33s::FromQuat(const Vector4 &q)
{
	const float xx = powf(q.X, 2);
	const float xy = q.X * q.Y;
	const float xz = q.X * q.Z;
	const float xw = q.X * q.W;

	const float yy = powf(q.Y, 2);
	const float yz = q.Y * q.Z;
	const float yw = q.Y * q.W;

	const float zz = powf(q.Z, 2);
	const float zw = q.Z * q.W;

	r1.X = 1.f - 2.f * (yy + zz);
	r1.Y = 2.f * (xy - zw);
	r1.Z = 2.f * (xz + yw);

	r2.X = 2.f * (xy + zw);
	r2.Y = 1.f - 2.f * (xx + zz);
	r2.Z = 2.f * (yz - xw);

	r3.X = 2.f * (xz - yw);
	r3.Y = 2.f * (yz + xw);
	r3.Z = 1.f - 2.f * (xx + yy);
}

void Matrix33s::ToQuat(Vector4 &q) const
{
	float trace = 0;
	if (r3.Z + FLT_EPSILON < 0.f)
	{
		if (r1.X >r2.Y)
		{
			trace = 1 + r1.X - r2.Y - r3.Z;
			q.X = trace;
			q.Y = r2.X + r1.Y;
			q.Z = r1.Z + r3.X;
			q.W = r3.Y - r2.Z;
		}
		else
		{
			trace = 1- r1.X + r2.Y - r3.Z;
			q.X = r2.X + r1.Y;
			q.Y = trace;
			q.Z = r3.Y + r2.Z;
			q.W = r1.Z - r3.X;
		}
	}
	else
	{
		if (r1.X <-r2.Y)
		{
			trace = 1- r1.X - r2.Y + r3.Z;
			q.X = r1.Z + r3.X;
			q.Y = r3.Y + r2.Z;
			q.Z = trace;
			q.W = r2.X - r1.Y;
		}
		else
		{
			trace = 1 + r1.X + r2.Y + r3.Z;
			q.X = r3.Y - r2.Z;
			q.Y = r1.Z - r3.X;
			q.Z = r2.X - r1.Y;
			q.W = trace;
		}
	}
	q *= 0.5f / sqrtf(trace);
	
	
	/*const float X = r1.X - r2.Y - r3.Z;
	const float Y = r2.Y - r1.X - r3.Z;
	const float Z = r3.Z - r1.X - r2.Y;

	int traceform = 0;
	float trace = r1.X + r2.Y + r3.Z;
	if (X > trace)
	{
		trace = X;
		traceform = 1;
	}
	if (Y > trace)
	{
		trace = Y;
		traceform = 2;
	}
	if (Z > trace)
	{
		trace = Z;
		traceform = 3;
	}
	trace += 1.f;

	switch (traceform)
	{
	case 0:
		q.X = r2.Z - r3.Y;
		q.Y = r3.X - r1.Z;
		q.Z = r1.Y - r2.X;
		q.W = trace;
		break;
	case 1:
		q.X = trace;
		q.Y = r1.Y + r2.X;
		q.Z = r3.X + r1.Z;
		q.W = r2.Z - r3.Y;
		break;
	case 2:
		q.X = r1.Y + r2.X;
		q.Y = trace;
		q.Z = r2.Z + r3.Y;
		q.W = r3.X - r1.Z;
		break;
	case 3:
		q.X = r3.X + r1.Z;
		q.Y = r2.Z + r3.Y;
		q.Z = trace;
		q.W = r1.Y - r2.X;
		break;
	}
	q *= 0.5f / sqrtf(trace);*/
	
	
	
	/* this will produce conjugated quaternion
	float trace = r1.X + r2.Y + r3.Z;
	if (trace > 0) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		q.W = 0.25f / s;
		q.X = (r3.Y - r2.Z) * s;
		q.Y = (r1.Z - r3.X) * s;
		q.Z = (r2.X - r1.Y) * s;
	}
	else if (r1.X > r2.Y && r1.X > r3.Z) {
		float s = 2.0f * sqrtf(1.0f + r1.X - r2.Y - r3.Z);
		q.W = (r3.Y - r2.Z) / s;
		q.X = 0.25f * s;
		q.Y = (r1.Y + r2.X) / s;
		q.Z = (r1.Z + r3.X) / s;
	}
	else if (r2.Y > r3.Z) {
		float s = 2.0f * sqrtf(1.0f + r2.Y - r1.X - r3.Z);
		q.W = (r1.Z - r3.X) / s;
		q.X = (r1.Y + r2.X) / s;
		q.Y = 0.25f * s;
		q.Z = (r2.Z + r3.Y) / s;
	}
	else {
		float s = 2.0f * sqrtf(1.0f + r3.Z - r1.X - r2.Y);
		q.W = (r2.X - r1.Y) / s;
		q.X = (r1.Z + r3.X) / s;
		q.Y = (r2.Z + r3.Y) / s;
		q.Z = 0.25f * s;
	}*/
}