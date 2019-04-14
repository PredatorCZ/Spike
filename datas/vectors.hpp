/*	Vector, Vector2, Vector4 classes
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

#ifndef ES_VECTORS_DEFINED
#define ES_VECTORS_DEFINED
#include "supercore.hpp"
#include <ostream>
#include <float.h>

#define FLTCMP(a, b) (a <= (b + FLT_EPSILON) && a >= (b - FLT_EPSILON))

template<typename T>
class t_Vector2
{
public:
	T X,Y;
	t_Vector2(void) :X(0), Y(0) {}
	t_Vector2(const T inx, const T iny) { X = inx; Y = iny; }
	
	t_Vector2& operator+=(const t_Vector2& input) { X += input.X; Y += input.Y; return *this; }
	t_Vector2& operator-=(const t_Vector2& input) { X -= input.X; Y -= input.Y; return *this; }
	t_Vector2& operator*=(const t_Vector2& input) { X *= input.X; Y *= input.Y; return *this; }
	t_Vector2& operator/=(const t_Vector2& input) { X /= input.X; Y /= input.Y; return *this; }

	t_Vector2 operator+(const t_Vector2& input) const { return t_Vector2(X + input.X, Y + input.Y); }
	t_Vector2 operator-(const t_Vector2& input) const { return t_Vector2(X - input.X, Y - input.Y); }
	t_Vector2 operator*(const t_Vector2& input) const { return t_Vector2(X * input.X, Y * input.Y); }
	t_Vector2 operator/(const t_Vector2& input) const { return t_Vector2(X / input.X, Y / input.Y); }

	t_Vector2& operator+=(const T& input) { X += input; Y += input; return *this; }
	t_Vector2& operator-=(const T& input) { X -= input; Y -= input; return *this; }
	t_Vector2& operator*=(const T& input) { X *= input; Y *= input; return *this; }
	t_Vector2& operator/=(const T& input) { X /= input; Y /= input; return *this; }

	t_Vector2 operator+(const T& input) const { return t_Vector2(X + input, Y + input); }
	t_Vector2 operator-(const T& input) const { return t_Vector2(X - input, Y - input); }
	t_Vector2 operator*(const T& input) const { return t_Vector2(X * input, Y * input); }
	t_Vector2 operator/(const T& input) const { return t_Vector2(X / input, Y / input); }

	t_Vector2 operator&(const T& input) const { return t_Vector2(X & input, Y & input); }
	t_Vector2 operator|(const T& input) const { return t_Vector2(X | input, Y | input); }
	t_Vector2 operator>>(const int input) const { return t_Vector2(X >> input, Y >> input); }
	t_Vector2 operator<<(const int input) const { return t_Vector2(X << input, Y << input); }

	t_Vector2& operator&=(const T& input) { X &= input; Y &= input; return *this; }
	t_Vector2& operator|=(const T& input) { X |= input; Y |= input; return *this; }
	t_Vector2& operator>>=(const int& input) { X >>= input; Y >>= input; return *this; }
	t_Vector2& operator<<=(const int& input) { X <<= input; Y <<= input; return *this; }

	t_Vector2& operator-() { X *= -1; Y *= -1; return *this; }

	ES_FORCEINLINE T& operator [](char pos) { return *(reinterpret_cast<T*>(this) + pos); }
	ES_FORCEINLINE const T& operator [](char pos) const { return *(reinterpret_cast<const T*>(this) + pos); }

	template<typename _T = T>
	typename std::enable_if<std::is_integral<_T>::value, bool>::type operator==(const t_Vector2 &input) const { return (X == input.X && Y == input.Y); }

	template<typename _T = T>
	typename std::enable_if<std::is_floating_point<_T>::value, bool>::type operator==(const t_Vector2 &input) const { return FLTCMP(X, input.X) && FLTCMP(Y, input.Y); }

	ES_FORCEINLINE bool operator!=(const t_Vector2 &input) const { return !(*this == input); }

	ES_FORCEINLINE bool IsSymetrical() const { return (X == Y); }

	template<typename T2>ES_FORCEINLINE t_Vector2<T2> Convert(void) const { return t_Vector2<T2>(static_cast<T2>(X), static_cast<T2>(Y)); }
	ES_FORCEINLINE std::string ToString() const
	{
		return std::to_string(X) + ' ' + std::to_string(Y);
	}
	ES_FORCEINLINE std::wstring ToWString() const
	{
		return std::to_wstring(X) + L' ' + std::to_wstring(Y);
	}

	ES_FORCEINLINE T Length() const { return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2))); }
	ES_FORCEINLINE int Sign() const { return X * Y < 0 ? -1 : 1; }
	ES_FORCEINLINE T Dot(const t_Vector2& input) const { return X * input.X + Y * input.Y; }
	friend ES_INLINE std::ostream& operator<<(std::ostream &strm, const t_Vector2<T> &v) { return strm << v.X << " " << v.Y; }

	t_Vector2 &Normalize()
	{
		T len = Length();
		if (!len) return *this;
		X /= len;
		Y /= len;
		return *this;
	}
#ifdef ES_ENDIAN_DEFINED 
	ES_FORCEINLINE const void SwapEndian()
	{
		FByteswapper(X);
		FByteswapper(Y);
	}
#endif
};
typedef t_Vector2<float> Vector2;
typedef Vector2 FVector2;
typedef t_Vector2<int> IVector2;
typedef t_Vector2<short> SVector2;
typedef t_Vector2<char> CVector2;
typedef t_Vector2<uint> UIVector2;
typedef t_Vector2<ushort> USVector2;
typedef t_Vector2<uchar> UCVector2;

template<typename T>
class t_Vector
{
public:
	T X,Y,Z;
	t_Vector(void) : X(0), Y(0), Z(0) {}
	t_Vector(const T inx, const T iny, const T inz) { X = inx; Y = iny; Z = inz; }


	t_Vector operator+(const t_Vector& input) const { return t_Vector(X + input.X, Y + input.Y, Z + input.Z); }
	t_Vector operator-(const t_Vector& input) const { return t_Vector(X - input.X, Y - input.Y, Z - input.Z); }
	t_Vector operator*(const t_Vector& input) const { return t_Vector(X * input.X, Y * input.Y, Z * input.Z); }
	t_Vector operator/(const t_Vector& input) const { return t_Vector(X / input.X, Y / input.Y, Z / input.Z); }
	
	t_Vector& operator+=(const t_Vector& input) { X += input.X; Y += input.Y; Z += input.Z; return *this; }
	t_Vector& operator-=(const t_Vector& input) { X -= input.X; Y -= input.Y; Z -= input.Z; return *this; }
	t_Vector& operator*=(const t_Vector& input) { X *= input.X; Y *= input.Y; Z *= input.Z; return *this; }
	t_Vector& operator/=(const t_Vector& input) { X /= input.X; Y /= input.Y; Z /= input.Z; return *this; }

	t_Vector operator*(const T& input) const { return t_Vector(X * input, Y * input, Z * input); }
	t_Vector operator+(const T& input) const { return t_Vector(X + input, Y + input, Z + input); }
	t_Vector operator/(const T& input) const { return t_Vector(X / input, Y / input, Z / input); }
	t_Vector operator-(const T& input) const { return t_Vector(X - input, Y - input, Z - input); }

	t_Vector& operator*=(const T& input) { X *= input; Y *= input; Z *= input; return *this; }
	t_Vector& operator/=(const T& input) { X /= input; Y /= input; Z /= input; return *this; }
	t_Vector& operator+=(const T& input) { X += input; Y += input; Z += input; return *this; }
	t_Vector& operator-=(const T& input) { X -= input; Y -= input; Z -= input; return *this; }

	t_Vector operator&(const T& input) const { return t_Vector(X & input, Y & input, Z & input); }
	t_Vector operator|(const T& input) const { return t_Vector(X | input, Y | input, Z | input); }
	t_Vector operator>>(const int input) const { return t_Vector(X >> input, Y >> input, Z >> input); }
	t_Vector operator<<(const int input) const { return t_Vector(X << input, Y << input, Z << input); }

	t_Vector& operator&=(const T& input) { X &= input; Y &= input; Z &= input; return *this; }
	t_Vector& operator|=(const T& input) { X |= input; Y |= input; Z |= input; return *this; }
	t_Vector& operator>>=(const int& input) { X >>= input; Y >>= input; Z >>= input; return *this; }
	t_Vector& operator<<=(const int& input) { X <<= input; Y <<= input; Z <<= input; return *this; }


	t_Vector& operator-() { X *= -1; Y *= -1; Z *= -1; return *this; }
	
	template<typename _T = T>
	typename std::enable_if<std::is_integral<_T>::value, bool>::type operator==(const t_Vector &input) const { return (X == input.X && Y == input.Y && Z == input.Z); }

	template<typename _T = T>
	typename std::enable_if<std::is_floating_point<_T>::value, bool>::type operator==(const t_Vector &input) const { return FLTCMP(X, input.X) && FLTCMP(Y, input.Y) && FLTCMP(Z, input.Z); }

	ES_FORCEINLINE bool operator!=(const t_Vector &input) const { return !(*this == input); }
		
	ES_FORCEINLINE T& operator [](char pos) { return *(reinterpret_cast<T*>(this) + pos); }
	ES_FORCEINLINE const T& operator [](char pos) const { return *(reinterpret_cast<const T*>(this) + pos); }
	
	ES_FORCEINLINE bool IsSymetrical() const { return (X == Y) && (X == Z); }

	template<typename T2>ES_FORCEINLINE t_Vector<T2> Convert(void) const { return t_Vector<T2>(static_cast<T2>(X), static_cast<T2>(Y), static_cast<T2>(Z)); }

	ES_FORCEINLINE std::string ToString() const
	{
		return std::to_string(X)+ ' ' + std::to_string(Y)+ ' ' + std::to_string(Z);
	}
	ES_FORCEINLINE std::wstring ToStringW() const
	{
		return std::to_wstring(X) + L' ' + std::to_wstring(Y) + L' ' + std::to_wstring(Z);
	}
	ES_FORCEINLINE int Sign() const { return X * Y * Z < 0 ? -1 : 1; }
	ES_FORCEINLINE T Length() const { return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2))); }
	ES_FORCEINLINE T Dot(const t_Vector& input) const { return X*input.X + Y*input.Y + Z*input.Z; }
	ES_FORCEINLINE t_Vector Cross(const t_Vector& input)const { return t_Vector((Y*input.Z - Z*input.Y), (Z*input.X - X*input.Z), (X*input.Y - Y*input.X)); }
	friend ES_INLINE std::ostream& operator<<(std::ostream &strm, const t_Vector<T> &v) { return strm << v.X << " " << v.Y << " " << v.Z; }

	t_Vector &Normalize()
	{
		T len = Length();
		if (!len) return *this;
		X /=len;
		Y /=len;
		Z /=len;
		return *this;
	}
#ifdef ES_ENDIAN_DEFINED 
	ES_FORCEINLINE const void SwapEndian()
	{
		FByteswapper(X);
		FByteswapper(Y);
		FByteswapper(Z);
	}

#endif
};
typedef t_Vector<float> Vector;
typedef Vector FVector;
typedef t_Vector<int> IVector;
typedef t_Vector<short> SVector;
typedef t_Vector<char> CVector;
typedef t_Vector<uint> UIVector;
typedef t_Vector<ushort> USVector;
typedef t_Vector<uchar> UCVector;

template<typename T>
class t_Vector4
{
public:
	T X,Y,Z,W;
	t_Vector4(void){Z = 0;X = 0;Y = 0;W = 0;}
	t_Vector4(const T inx, const T iny, const T inz, const T inw) { X = inx; Y = iny; Z = inz; W = inw; }

	t_Vector4& operator+=(const t_Vector4& input) { X += input.X; Y += input.Y; Z += input.Z; W += input.W; return *this; }
	t_Vector4& operator-=(const t_Vector4& input) { X -= input.X; Y -= input.Y; Z -= input.Z; W -= input.W; return *this; }
	t_Vector4& operator*=(const t_Vector4& input) { X *= input.X; Y *= input.Y; Z *= input.Z; W *= input.W; return *this; }
	t_Vector4& operator/=(const t_Vector4& input) { X /= input.X; Y /= input.Y; Z /= input.Z; W /= input.W; return *this; }

	t_Vector4 operator+(const t_Vector4& input) const { return t_Vector4(X + input.X, Y + input.Y, Z + input.Z, W + input.W); }
	t_Vector4 operator-(const t_Vector4& input) const { return t_Vector4(X - input.X, Y - input.Y, Z - input.Z, W - input.W); }
	t_Vector4 operator*(const t_Vector4& input) const { return t_Vector4(X * input.X, Y * input.Y, Z * input.Z, W * input.W); }
	t_Vector4 operator/(const t_Vector4& input) const { return t_Vector4(X / input.X, Y / input.Y, Z / input.Z, W / input.W); }

	t_Vector4& operator+=(const T& input) { X += input; Y += input; Z += input; W += input; return *this; }
	t_Vector4& operator-=(const T& input) { X -= input; Y -= input; Z -= input; W -= input; return *this; }
	t_Vector4& operator*=(const T& input) { X *= input; Y *= input; Z *= input; W *= input; return *this; }
	t_Vector4& operator/=(const T& input) { X /= input; Y /= input; Z /= input; W /= input; return *this; }	

	t_Vector4 operator*(const T& input) const { return t_Vector4(X * input, Y * input, Z * input, W * input); }
	t_Vector4 operator+(const T& input) const { return t_Vector4(X + input, Y + input, Z + input, W + input); }
	t_Vector4 operator/(const T& input) const { return t_Vector4(X / input, Y / input, Z / input, W / input); }
	t_Vector4 operator-(const T& input) const { return t_Vector4(X - input, Y - input, Z - input, W - input); }

	t_Vector4 operator&(const T& input) const { return t_Vector4(X & input, Y & input, Z & input, W & input); }
	t_Vector4 operator|(const T& input) const { return t_Vector4(X | input, Y | input, Z | input, W | input); }
	t_Vector4 operator >> (const int input) const { return t_Vector4(X >> input, Y >> input, Z >> input, W >> input); }
	t_Vector4 operator<<(const int input) const { return t_Vector4(X << input, Y << input, Z << input, W << input); }

	t_Vector4& operator&=(const T& input) { X &= input; Y &= input; Z &= input; W &= input; return *this; }
	t_Vector4& operator|=(const T& input) { X |= input; Y |= input; Z |= input; W |= input; return *this; }
	t_Vector4& operator>>=(const int& input) { X >>= input; Y >>= input; Z >>= input; W >>= input; return *this; }
	t_Vector4& operator<<=(const int& input) { X <<= input; Y <<= input; Z <<= input; W <<= input; return *this; }


	t_Vector4& operator-() { X *= -1; Y *= -1; Z *= -1; W *= -1; return *this; }

	template<typename _T = T>
	typename std::enable_if<std::is_integral<_T>::value, bool>::type operator==(const t_Vector4 &input) const { return (X == input.X && Y == input.Y && Z == input.Z && W == input.W); }
	
	template<typename _T = T>
	typename std::enable_if<std::is_floating_point<_T>::value, bool>::type operator==(const t_Vector4 &input) const { return FLTCMP(X, input.X) && FLTCMP(Y, input.Y) && FLTCMP(Z, input.Z) && FLTCMP(W, input.W); }

	ES_FORCEINLINE bool operator!=(const t_Vector4 &input) const { return !(*this == input); }
	ES_FORCEINLINE bool IsSymetrical() const { return (X == Y) && (X == Z) && (Z == W); }

	template<typename T2>ES_FORCEINLINE t_Vector4<T2> Convert() { return t_Vector4<T2>(static_cast<T2>(X), static_cast<T2>(Y), static_cast<T2>(Z), static_cast<T2>(W)); }

	ES_FORCEINLINE std::string ToString() const
	{
		return std::to_string(X) + ' ' + std::to_string(Y) + ' ' + std::to_string(Z) + ' ' + std::to_string(W);
	}
	ES_FORCEINLINE std::wstring ToStringW() const
	{
		return std::to_wstring(X) + L' ' + std::to_wstring(Y) + L' ' + std::to_wstring(Z) + L' ' + std::to_wstring(W);
	}
	ES_FORCEINLINE T& operator [](char pos) { return *(reinterpret_cast<T*>(this) + pos); }
	ES_FORCEINLINE const T& operator [](char pos) const { return *(reinterpret_cast<const T*>(this) + pos); }
	ES_FORCEINLINE int Sign()const { return X * Y * Z * W < 0 ? -1 : 1; }
	ES_FORCEINLINE T Length()const { return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2) + pow(W, 2))); }
	ES_FORCEINLINE T Dot(t_Vector4& input)const { return X*input.X + Y*input.Y + Z*input.Z + W*input.W; }
	t_Vector4 &Normalize()
	{
		T len = Length();
		if (!len) 
			return *this;
		X /=len;
		Y /=len;
		Z /=len;
		W /=len;
		return *this;
	}
	friend ES_INLINE std::ostream& operator<<(std::ostream &strm, const t_Vector4<T> &v) { return strm << v.X << " " << v.Y << " " << v.Z << " " << v.W; }
#ifdef ES_ENDIAN_DEFINED 
	ES_FORCEINLINE const void SwapEndian()
	{
		FByteswapper(X);
		FByteswapper(Y);
		FByteswapper(Z);
		FByteswapper(W);
	}

#endif
};
typedef t_Vector4<float> Vector4;
typedef Vector4 FVector4;
typedef t_Vector4<int> IVector4;
typedef t_Vector4<short> SVector4;
typedef t_Vector4<char> CVector4;
typedef t_Vector4<uint> UIVector4;
typedef t_Vector4<ushort> USVector4;
typedef t_Vector4<uchar> UCVector4;

#endif // ES_VECTORS_DEFINED

#ifdef ES_REFLECTOR_DEFINED
#ifndef ES_VECTORS_TEMPLATES_DEFINED
#define ES_VECTORS_TEMPLATES_DEFINED
template <class C> struct _getType<t_Vector2<C>> {
	static const char TYPE = 15; static const JenHash HASH = _getType<C>::TYPE; static const uchar SUBSIZE = sizeof(C);
};
template <class C> struct _getType<t_Vector<C>> {
	static const char TYPE = 16; static const JenHash HASH = _getType<C>::TYPE; static const uchar SUBSIZE = sizeof(C);
};
template <class C> struct _getType<t_Vector4<C>> {
	static const char TYPE = 17; static const JenHash HASH = _getType<C>::TYPE; static const uchar SUBSIZE = sizeof(C);
};
#endif
#endif // ES_REFLECTOR_DEFINED


