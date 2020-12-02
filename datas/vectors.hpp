/*  Vector, Vector2, Vector4 classes
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
#include "reflector_fwd.hpp"
#include <ostream>
#include <float.h>
#include <cmath>

#define FLTCMP(a, b) (a <= (b + FLT_EPSILON) && a >= (b - FLT_EPSILON))

template <typename T> class t_Vector;
template <typename T> class _t_Vector4;
template<typename T> class V4ScalarType;

template<typename T>
void FByteswapper(T &);

template<typename T>
class t_Vector2
{
public:
	T X,Y;
	t_Vector2(void) :X(0), Y(0) {}
	t_Vector2(const T inx, const T iny) : X(inx), Y(iny) {}

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

	t_Vector2 operator-() const { return *this * -1; }

	//t_Vector<T> ToVector() const { return t_Vector<T>(this->X, this->Y, 0.0f); }
	//t_Vector4<T> ToVector4() const { return t_Vector4<T>(this->X, this->Y, 0.0f, 0.0f); }

	T& operator[](size_t pos) { return *(reinterpret_cast<T*>(this) + pos); }
	const T& operator[](size_t pos) const { return *(reinterpret_cast<const T*>(this) + pos); }

	template<typename _T = T>
	typename std::enable_if<std::is_integral<_T>::value, bool>::type operator==(const t_Vector2 &input) const { return (X == input.X && Y == input.Y); }

	template<typename _T = T>
	typename std::enable_if<std::is_floating_point<_T>::value, bool>::type operator==(const t_Vector2 &input) const { return FLTCMP(X, input.X) && FLTCMP(Y, input.Y); }

	bool operator!=(const t_Vector2 &input) const { return !(*this == input); }

	bool IsSymetrical() const { return (X == Y); }

	template<typename T2>t_Vector2<T2> Convert(void) const { return t_Vector2<T2>(static_cast<T2>(X), static_cast<T2>(Y)); }
	std::string ToString() const
	{
		return std::to_string(X) + ' ' + std::to_string(Y);
	}
	std::wstring ToWString() const
	{
		return std::to_wstring(X) + L' ' + std::to_wstring(Y);
	}

	T Length() const { return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2))); }
	int Sign() const { return X * Y < 0 ? -1 : 1; }
	T Dot(const t_Vector2& input) const { return X * input.X + Y * input.Y; }
	friend std::ostream& operator<<(std::ostream &strm, const t_Vector2<T> &v) { return strm << v.X << " " << v.Y; }

	t_Vector2 &Normalize()
	{
		T len = Length();
		if (!len) return *this;
		X /= len;
		Y /= len;
		return *this;
	}

	void SwapEndian()
	{
		FByteswapper(X);
		FByteswapper(Y);
	}
};
typedef t_Vector2<float> Vector2;
typedef Vector2 FVector2;
typedef t_Vector2<int32> IVector2;
typedef t_Vector2<int16> SVector2;
typedef t_Vector2<int8> CVector2;
typedef t_Vector2<uint32> UIVector2;
typedef t_Vector2<uint16> USVector2;
typedef t_Vector2<uint8> UCVector2;

template<typename T>
class t_Vector
{
public:
	T X,Y,Z;
	t_Vector(void) : X(0), Y(0), Z(0) {}
	t_Vector(const T inx, const T iny, const T inz) : X(inx), Y(iny), Z(inz) {}


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

	template<typename R> operator _t_Vector4<R>() const;
	operator t_Vector2<T>() const { return t_Vector2<T>(this->X, this->Y); }

	t_Vector operator-() const { return *this * -1; }

	template<typename _T = T>
	typename std::enable_if<std::is_integral<_T>::value, bool>::type operator==(const t_Vector &input) const { return (X == input.X && Y == input.Y && Z == input.Z); }

	template<typename _T = T>
	typename std::enable_if<std::is_floating_point<_T>::value, bool>::type operator==(const t_Vector &input) const { return FLTCMP(X, input.X) && FLTCMP(Y, input.Y) && FLTCMP(Z, input.Z); }

	bool operator!=(const t_Vector &input) const { return !(*this == input); }

	T& operator[](size_t pos) { return *(reinterpret_cast<T*>(this) + pos); }
	const T& operator[](size_t pos) const { return *(reinterpret_cast<const T*>(this) + pos); }

	bool IsSymetrical() const { return (X == Y) && (X == Z); }

	template<typename T2>t_Vector<T2> Convert(void) const { return t_Vector<T2>(static_cast<T2>(X), static_cast<T2>(Y), static_cast<T2>(Z)); }

	std::string ToString() const
	{
		return std::to_string(X)+ ' ' + std::to_string(Y)+ ' ' + std::to_string(Z);
	}
	std::wstring ToStringW() const
	{
		return std::to_wstring(X) + L' ' + std::to_wstring(Y) + L' ' + std::to_wstring(Z);
	}
	int Sign() const { return X * Y * Z < 0 ? -1 : 1; }
	T Length() const { return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2))); }
	T Dot(const t_Vector& input) const { return X*input.X + Y*input.Y + Z*input.Z; }
	t_Vector Cross(const t_Vector& input)const { return t_Vector((Y*input.Z - Z*input.Y), (Z*input.X - X*input.Z), (X*input.Y - Y*input.X)); }
	friend std::ostream& operator<<(std::ostream &strm, const t_Vector<T> &v) { return strm << v.X << " " << v.Y << " " << v.Z; }

	t_Vector &Normalize()
	{
		T len = Length();
		if (!len) return *this;
		X /=len;
		Y /=len;
		Z /=len;
		return *this;
	}

	void SwapEndian()
	{
		FByteswapper(X);
		FByteswapper(Y);
		FByteswapper(Z);
	}
};
typedef t_Vector<float> Vector;
typedef Vector FVector;
typedef t_Vector<int32> IVector;
typedef t_Vector<int16> SVector;
typedef t_Vector<int8> CVector;
typedef t_Vector<uint32> UIVector;
typedef t_Vector<uint16> USVector;
typedef t_Vector<uint8> UCVector;

template <typename T> class V4ScalarType {
public:
  typedef T eltype;

  union {
    eltype _arr[4];
    struct {
      eltype X, Y, Z, W;
    };
  };

  V4ScalarType() : X(0), Y(0), Z(0), W(0) {}
  V4ScalarType(T s) : X(s), Y(s), Z(s), W(s) {}
  V4ScalarType(T x, T y, T z, T w) : X(x), Y(y), Z(z), W(w) {}

  V4ScalarType &operator+=(const V4ScalarType &input) {
    return *this = *this + input;
  }
  V4ScalarType &operator-=(const V4ScalarType &input) {
    return *this = *this - input;
  }
  V4ScalarType &operator*=(const V4ScalarType &input) {
    return *this = *this * input;
  }
  V4ScalarType &operator/=(const V4ScalarType &input) {
    return *this = *this / input;
  }

  V4ScalarType operator+(const V4ScalarType &input) const {
    return {X + input.X, Y + input.Y, Z + input.Z, W + input.W};
  }
  V4ScalarType operator-(const V4ScalarType &input) const {
    return {X - input.X, Y - input.Y, Z - input.Z, W - input.W};
  }
  V4ScalarType operator*(const V4ScalarType &input) const {
    return {X * input.X, Y * input.Y, Z * input.Z, W * input.W};
  }
  V4ScalarType operator/(const V4ScalarType &input) const {
    return {X / input.X, Y / input.Y, Z / input.Z, W / input.W};
  }

  V4ScalarType &operator+=(const eltype &input) {
    return *this = *this + input;
  }
  V4ScalarType &operator-=(const eltype &input) {
    return *this = *this - input;
  }
  V4ScalarType &operator*=(const eltype &input) {
    return *this = *this * input;
  }
  V4ScalarType &operator/=(const eltype &input) {
    return *this = *this / input;
  }

  V4ScalarType operator+(const eltype &input) {
    return *this = *this + V4ScalarType(input);
  }
  V4ScalarType operator-(const eltype &input) {
    return *this = *this - V4ScalarType(input);
  }
  V4ScalarType operator*(const eltype &input) {
    return *this = *this * V4ScalarType(input);
  }
  V4ScalarType operator/(const eltype &input) {
    return *this = *this / V4ScalarType(input);
  }

  V4ScalarType operator&(const V4ScalarType &input) const {
    return {X & input.X, Y & input.Y, Z & input.Z, W & input.W};
  }
  V4ScalarType operator|(const V4ScalarType &input) const {
    return {X | input.X, Y | input.Y, Z | input.Z, W | input.W};
  }

  V4ScalarType operator&(const eltype &input) const {
    return *this = *this & V4ScalarType(input);
  }
  V4ScalarType operator|(const eltype &input) const {
    return *this = *this | V4ScalarType(input);
  }

  V4ScalarType operator>>(const int &input) const {
    return {X >> input, Y >> input, Z >> input, W >> input};
  }
  V4ScalarType operator<<(const int &input) const {
    return {X << input, Y << input, Z << input, W << input};
  }

  V4ScalarType &operator&=(const T &input) { return *this = *this & input; }
  V4ScalarType &operator|=(const T &input) { return *this = *this & input; }
  V4ScalarType &operator>>=(const int &input) { return *this = *this >> input; }
  V4ScalarType &operator<<=(const int &input) { return *this = *this << input; }

  V4ScalarType operator-() const { return *this * -1.f; }

  bool IsSymetrical() const {
    return (X == Y) && (X == Z) && (Z == W);
  }

  template <typename T2> V4ScalarType<T2> Convert() const {
    return V4ScalarType<T2>(static_cast<T2>(X), static_cast<T2>(Y),
                            static_cast<T2>(Z), static_cast<T2>(W));
  }

  int Sign() const { return X * Y * Z * W < 0 ? -1 : 1; }

  template <typename _T = T>
  typename std::enable_if<std::is_integral<_T>::value, bool>::type
  operator==(const V4ScalarType &input) const {
    return (this->X == input.X && this->Y == input.Y && this->Z == input.Z &&
            this->W == input.W);
  }

  template <typename _T = T>
  typename std::enable_if<std::is_floating_point<_T>::value, bool>::type
  operator==(const V4ScalarType &input) const {
    return FLTCMP(this->X, input.X) && FLTCMP(this->Y, input.Y) &&
           FLTCMP(this->Z, input.Z) && FLTCMP(this->W, input.W);
  }

  bool operator!=(const V4ScalarType &input) const {
    return !(*this == input);
  }

  T Length() const {
    return static_cast<T>(sqrt(pow(this->X, 2) + pow(this->Y, 2) +
                               pow(this->Z, 2) + pow(this->W, 2)));
  }
  T Dot(const V4ScalarType &input) const {
    return this->X * input.X + this->Y * input.Y + this->Z * input.Z +
           this->W * input.W;
  }
  V4ScalarType &Normalize() {
    T len = Length();

    if (!len)
      return *this;

    return *this /= len;
  }
};

template <class C> class _t_Vector4 : public C {
public:
  using C::C;
  typedef typename C::eltype eltype;
  _t_Vector4() = default;
  _t_Vector4(const C &input) : C(input) {}
  _t_Vector4(C &&input) : C(input) {}

  _t_Vector4 &operator=(const C &input) {
    static_cast<C &>(*this) = input;
    return *this;
  }
  _t_Vector4 &operator=(C &&input) {
    static_cast<C &>(*this) = input;
    return *this;
  }

  operator t_Vector<eltype>() const {
    return t_Vector<eltype>(this->X, this->Y, this->Z);
  }
  operator t_Vector2<eltype>() const {
    return t_Vector2<eltype>(this->X, this->Y);
  }

  std::string ToString() const {
    return std::to_string(this->X) + ' ' + std::to_string(this->Y) + ' ' +
           std::to_string(this->Z) + ' ' + std::to_string(this->W);
  }

  std::wstring ToStringW() const {
    return std::to_wstring(this->X) + ' ' + std::to_wstring(this->Y) + ' ' +
           std::to_wstring(this->Z) + ' ' + std::to_wstring(this->W);
  }

  eltype &operator[](size_t pos) { return this->_arr[pos]; }
  const eltype &operator[](size_t pos) const {
    return this->_arr[pos];
  }

  friend std::ostream &operator<<(std::ostream &strm,
                                            const _t_Vector4 &v) {
    return strm << v.X << " " << v.Y << " " << v.Z << " " << v.W;
  }

  void SwapEndian() {
    FByteswapper(this->X);
    FByteswapper(this->Y);
    FByteswapper(this->Z);
    FByteswapper(this->W);
  }
};

template <typename T> using t_Vector4 = _t_Vector4<V4ScalarType<T>>;

template <typename T>
template <typename R>
t_Vector<T>::operator _t_Vector4<R>() const {
  return _t_Vector4<R>(this->X, this->Y, this->Z, 0.0f);
}

typedef t_Vector4<float> Vector4;
typedef Vector4 FVector4;
typedef t_Vector4<int32> IVector4;
typedef t_Vector4<int16> SVector4;
typedef t_Vector4<int8> CVector4;
typedef t_Vector4<uint32> UIVector4;
typedef t_Vector4<uint16> USVector4;
typedef t_Vector4<uint8> UCVector4;

constexpr JenHash CompileVectorHash_(REFType type, uint8 size,
                                     uint16 numItems) {
  return JenHash(static_cast<uint32>(type) | static_cast<uint32>(size) << 8 |
                 static_cast<uint32>(numItems) << 16);
}

template <class C> struct _getType<t_Vector2<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr uint8 SUBSIZE = sizeof(C);
  static constexpr uint16 NUMITEMS = 2;
  static constexpr JenHash Hash() { return  CompileVectorHash_(SUBTYPE, SUBSIZE, NUMITEMS); }
};
template <class C> struct _getType<t_Vector<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr uint8 SUBSIZE = sizeof(C);
  static constexpr uint16 NUMITEMS = 3;
  static constexpr JenHash Hash() { return  CompileVectorHash_(SUBTYPE, SUBSIZE, NUMITEMS); }
};
template <class C> struct _getType<_t_Vector4<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<typename C::eltype>::TYPE;
  static constexpr uint8 SUBSIZE = sizeof(typename C::eltype);
  static constexpr uint16 NUMITEMS = 4;
  static constexpr JenHash Hash() { return  CompileVectorHash_(SUBTYPE, SUBSIZE, NUMITEMS); }
};
