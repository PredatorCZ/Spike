/*  Vector, Vector2, Vector4 classes

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
#include "spike/reflect/reflector_fwd.hpp"
#include "spike/type/vectors_fwd.hpp"
#include "spike/util/endian_fwd.hpp"
#include <cfloat>
#include <cmath>

template <class C> static bool fltcmp(C a, C b) {
  return a <= (b + FLT_EPSILON) && a >= (b - FLT_EPSILON);
}

template <typename T> class t_Vector2 {
  using vector = t_Vector2;
  using vec_ref = vector &;
  using vec_cref = const vector &;

public:
  using value_type = T;
  using shift_value = int32;
  using shift_vec = const t_Vector2<shift_value> &;
  union {
    T _arr[2];
    struct {
      T X, Y;
    };
    struct {
      T x, y;
    };
  };

  t_Vector2() : X(0), Y(0) {}
  t_Vector2(T sc) : X(sc), Y(sc) {}
  t_Vector2(T inx, T iny) : X(inx), Y(iny) {}

  vector operator+(vec_cref input) const {
    return {T(X + input.X), T(Y + input.Y)};
  }
  vector operator-(vec_cref input) const {
    return {T(X - input.X), T(Y - input.Y)};
  }
  vector operator*(vec_cref input) const {
    return {T(X * input.X), T(Y * input.Y)};
  }
  vector operator/(vec_cref input) const {
    return {T(X / input.X), T(Y / input.Y)};
  }
  vector operator&(vec_cref input) const {
    return {T(X & input.X), T(Y & input.Y)};
  }
  vector operator|(vec_cref input) const {
    return {T(X | input.X), T(Y | input.Y)};
  }
  vector operator>>(shift_vec input) const {
    return {X >> input.X, Y >> input.Y};
  }
  vector operator<<(shift_vec input) const {
    return {X << input.X, Y << input.Y};
  }

  vector operator+(value_type input) const { return *this + vector(input); }
  vector operator-(value_type input) const { return *this - vector(input); }
  vector operator*(value_type input) const { return *this * vector(input); }
  vector operator/(value_type input) const { return *this / vector(input); }
  vector operator&(value_type input) const { return *this & vector(input); }
  vector operator|(value_type input) const { return *this | vector(input); }
  vector operator>>(shift_value input) const { return *this >> vector(input); }
  vector operator<<(shift_value input) const { return *this << vector(input); }

  vec_ref operator+=(vec_cref input) { return *this = *this + input; }
  vec_ref operator-=(vec_cref input) { return *this = *this - input; }
  vec_ref operator*=(vec_cref input) { return *this = *this * input; }
  vec_ref operator/=(vec_cref input) { return *this = *this / input; }
  vec_ref operator&=(vec_cref input) { return *this = *this & input; }
  vec_ref operator|=(vec_cref input) { return *this = *this | input; }
  vec_ref operator>>=(shift_vec input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_vec input) { return *this = *this << input; }

  vec_ref operator+=(value_type input) { return *this = *this + input; }
  vec_ref operator-=(value_type input) { return *this = *this - input; }
  vec_ref operator*=(value_type input) { return *this = *this * input; }
  vec_ref operator/=(value_type input) { return *this = *this / input; }
  vec_ref operator&=(value_type input) { return *this = *this & input; }
  vec_ref operator|=(value_type input) { return *this = *this | input; }
  vec_ref operator>>=(shift_value input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_value input) { return *this = *this << input; }

  vector operator-() const { return *this * -1; }

  // t_Vector<T> ToVector() const { return t_Vector<T>(this->X, this->Y, 0.0f);
  // } t_Vector4<T> ToVector4() const { return t_Vector4<T>(this->X, this->Y,
  // 0.0f, 0.0f); }

  value_type &operator[](size_t pos) { return _arr[pos]; }
  const value_type &operator[](size_t pos) const { return _arr[pos]; }

  template <typename _T = T>
  typename std::enable_if<std::is_integral<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return X == input.X && Y == input.Y;
  }

  template <typename _T = T>
  typename std::enable_if<std::is_floating_point<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return fltcmp(X, input.X) && fltcmp(Y, input.Y);
  }

  bool operator!=(vec_cref input) const { return !(*this == input); }

  bool IsSymetrical() const { return *this == vector{Y, X}; }

  template <typename T2> t_Vector2<T2> Convert() const {
    return {static_cast<T2>(X), static_cast<T2>(Y)};
  }

  value_type Length() const {
    return static_cast<value_type>(sqrt(pow(X, 2) + pow(Y, 2)));
  }
  int Sign() const { return (X | Y) < 0 ? -1 : 1; }
  value_type Dot(vec_cref input) const { return X * input.X + Y * input.Y; }

  vec_ref Normalize() {
    value_type len = Length();
    if (!len) {
      return *this;
    }
    return *this /= len;
  }

  void SwapEndian() {
    if constexpr (sizeof(X) > 1) {
      FByteswapper(X);
      FByteswapper(Y);
    }
  }
};

template <typename T> class t_Vector {
  using vector = t_Vector;
  using vec_ref = vector &;
  using vec_cref = const vector &;

public:
  using value_type = T;
  using shift_value = int32;
  using shift_vec = const t_Vector<shift_value> &;
  union {
    T _arr[3];
    struct {
      T X, Y, Z;
    };
    struct {
      T x, y, z;
    };
  };

  t_Vector() : X(0), Y(0), Z(0) {}
  t_Vector(T sc) : X(sc), Y(sc), Z(sc) {}
  t_Vector(T inx, T iny, T inz) : X(inx), Y(iny), Z(inz) {}

  vector operator+(vec_cref input) const {
    return {T(X + input.X), T(Y + input.Y), T(Z + input.Z)};
  }
  vector operator-(vec_cref input) const {
    return {T(X - input.X), T(Y - input.Y), T(Z - input.Z)};
  }
  vector operator*(vec_cref input) const {
    return {T(X * input.X), T(Y * input.Y), T(Z * input.Z)};
  }
  vector operator/(vec_cref input) const {
    return {T(X / input.X), T(Y / input.Y), T(Z / input.Z)};
  }
  vector operator&(vec_cref input) const {
    return {T(X & input.X), T(Y & input.Y), T(Z & input.Z)};
  }
  vector operator|(vec_cref input) const {
    return {T(X | input.X), T(Y | input.Y), T(Z | input.Z)};
  }
  vector operator>>(shift_vec input) const {
    return {X >> input.X, Y >> input.Y, Z >> input.Z};
  }
  vector operator<<(shift_vec input) const {
    return {X << input.X, Y << input.Y, Z << input.Z};
  }

  vector operator+(value_type input) const { return *this + vector(input); }
  vector operator-(value_type input) const { return *this - vector(input); }
  vector operator*(value_type input) const { return *this * vector(input); }
  vector operator/(value_type input) const { return *this / vector(input); }
  vector operator&(value_type input) const { return *this & vector(input); }
  vector operator|(value_type input) const { return *this | vector(input); }
  vector operator>>(shift_value input) const { return *this >> vector(input); }
  vector operator<<(shift_value input) const { return *this << vector(input); }

  vec_ref operator+=(vec_cref input) { return *this = *this + input; }
  vec_ref operator-=(vec_cref input) { return *this = *this - input; }
  vec_ref operator*=(vec_cref input) { return *this = *this * input; }
  vec_ref operator/=(vec_cref input) { return *this = *this / input; }
  vec_ref operator&=(vec_cref input) { return *this = *this & input; }
  vec_ref operator|=(vec_cref input) { return *this = *this | input; }
  vec_ref operator>>=(shift_vec input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_vec input) { return *this = *this << input; }

  vec_ref operator+=(value_type input) { return *this = *this + input; }
  vec_ref operator-=(value_type input) { return *this = *this - input; }
  vec_ref operator*=(value_type input) { return *this = *this * input; }
  vec_ref operator/=(value_type input) { return *this = *this / input; }
  vec_ref operator&=(value_type input) { return *this = *this & input; }
  vec_ref operator|=(value_type input) { return *this = *this | input; }
  vec_ref operator>>=(shift_value input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_value input) { return *this = *this << input; }

  template <typename R> operator t_Vector4_<R>() const;
  operator t_Vector2<T>() const { return t_Vector2<T>(this->X, this->Y); }

  vector operator-() const { return *this * -1; }

  template <typename _T = T>
  typename std::enable_if<std::is_integral<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return X == input.X && Y == input.Y && Z == input.Z;
  }

  template <typename _T = T>
  typename std::enable_if<std::is_floating_point<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return fltcmp(X, input.X) && fltcmp(Y, input.Y) && fltcmp(Z, input.Z);
  }

  bool operator!=(vec_cref input) const { return !(*this == input); }

  value_type &operator[](size_t pos) { return _arr[pos]; }
  const value_type &operator[](size_t pos) const { return _arr[pos]; }

  bool IsSymetrical() const { return (X == Y) && (X == Z); }

  template <typename T2> t_Vector<T2> Convert(void) const {
    return {static_cast<T2>(X), static_cast<T2>(Y), static_cast<T2>(Z)};
  }

  int Sign() const { return (X | Y | Z) < 0 ? -1 : 1; }
  value_type Length() const {
    return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2)));
  }
  value_type Dot(vec_cref input) const {
    return X * input.X + Y * input.Y + Z * input.Z;
  }
  vector Cross(vec_cref input) const {
    return {Y * input.Z - Z * input.Y, Z * input.X - X * input.Z,
            X * input.Y - Y * input.X};
  }

  vec_ref Normalize() {
    value_type len = Length();
    if (!len) {
      return *this;
    }
    return *this /= len;
  }

  void SwapEndian() {
    if constexpr (sizeof(X) > 1) {
      FByteswapper(X);
      FByteswapper(Y);
      FByteswapper(Z);
    }
  }
};

template <typename T> class V4ScalarType {
  using vector = V4ScalarType;
  using vec_ref = vector &;
  using vec_cref = const vector &;

public:
  using value_type = T;
  using shift_value = int32;
  using shift_vec = const V4ScalarType<shift_value> &;

  union {
    value_type _arr[4];
    struct {
      value_type X, Y, Z, W;
    };
    struct {
      value_type x, y, z, w;
    };
  };

  V4ScalarType() : X(0), Y(0), Z(0), W(0) {}
  V4ScalarType(T s) : X(s), Y(s), Z(s), W(s) {}
  V4ScalarType(T x, T y, T z, T w) : X(x), Y(y), Z(z), W(w) {}

  vector operator+(vec_cref input) const {
    return {T(X + input.X), T(Y + input.Y), T(Z + input.Z), T(W + input.W)};
  }
  vector operator-(vec_cref input) const {
    return {T(X - input.X), T(Y - input.Y), T(Z - input.Z), T(W - input.W)};
  }
  vector operator*(vec_cref input) const {
    return {T(X * input.X), T(Y * input.Y), T(Z * input.Z), T(W * input.W)};
  }
  vector operator/(vec_cref input) const {
    return {T(X / input.X), T(Y / input.Y), T(Z / input.Z), T(W / input.W)};
  }
  vector operator&(vec_cref input) const {
    return {T(X & input.X), T(Y & input.Y), T(Z & input.Z), T(W & input.W)};
  }
  vector operator|(vec_cref input) const {
    return {T(X | input.X), T(Y | input.Y), T(Z | input.Z), T(W | input.W)};
  }
  vector operator>>(shift_vec input) const {
    return {X >> input.X, Y >> input.Y, Z >> input.Z, W >> input.W};
  }
  vector operator<<(shift_vec input) const {
    return {X << input.X, Y << input.Y, Z << input.Z, W << input.W};
  }

  vector operator+(value_type input) const { return *this + vector(input); }
  vector operator-(value_type input) const { return *this - vector(input); }
  vector operator*(value_type input) const { return *this * vector(input); }
  vector operator/(value_type input) const { return *this / vector(input); }
  vector operator&(value_type input) const { return *this & vector(input); }
  vector operator|(value_type input) const { return *this | vector(input); }
  vector operator>>(shift_value input) const { return *this >> vector(input); }
  vector operator<<(shift_value input) const { return *this << vector(input); }

  vec_ref operator+=(vec_cref input) { return *this = *this + input; }
  vec_ref operator-=(vec_cref input) { return *this = *this - input; }
  vec_ref operator*=(vec_cref input) { return *this = *this * input; }
  vec_ref operator/=(vec_cref input) { return *this = *this / input; }
  vec_ref operator&=(vec_cref input) { return *this = *this & input; }
  vec_ref operator|=(vec_cref input) { return *this = *this & input; }
  vec_ref operator>>=(shift_vec input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_vec input) { return *this = *this << input; }

  vec_ref operator+=(value_type input) { return *this = *this + input; }
  vec_ref operator-=(value_type input) { return *this = *this - input; }
  vec_ref operator*=(value_type input) { return *this = *this * input; }
  vec_ref operator/=(value_type input) { return *this = *this / input; }
  vec_ref operator&=(value_type input) { return *this = *this & input; }
  vec_ref operator|=(value_type input) { return *this = *this & input; }
  vec_ref operator>>=(shift_value input) { return *this = *this >> input; }
  vec_ref operator<<=(shift_value input) { return *this = *this << input; }

  vector operator-() const { return *this * -1.f; }

  bool IsSymetrical() const { return (X == Y) && (X == Z) && (Z == W); }

  template <typename T2> V4ScalarType<T2> Convert() const {
    return V4ScalarType<T2>(static_cast<T2>(X), static_cast<T2>(Y),
                            static_cast<T2>(Z), static_cast<T2>(W));
  }

  int Sign() const { return (X | Y | Z | W) < 0 ? -1 : 1; }

  template <typename _T = T>
  typename std::enable_if<std::is_integral<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return X == input.X && Y == input.Y && Z == input.Z && W == input.W;
  }

  template <typename _T = T>
  typename std::enable_if<std::is_floating_point<_T>::value, bool>::type
  operator==(vec_cref input) const {
    return fltcmp(X, input.X) && fltcmp(Y, input.Y) && fltcmp(Z, input.Z) &&
           fltcmp(W, input.W);
  }

  bool operator!=(vec_cref input) const { return !(*this == input); }

  value_type Length() const {
    return static_cast<T>(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2) + pow(W, 2)));
  }
  value_type Dot(vec_cref input) const {
    return X * input.X + Y * input.Y + Z * input.Z + W * input.W;
  }
  vec_ref Normalize() {
    value_type len = Length();

    if (!len)
      return *this;

    return *this /= len;
  }
};

template <class C> class t_Vector4_ : public C {
public:
  using C::C;
  using value_type = typename C::value_type;
  t_Vector4_() = default;
  t_Vector4_(const C &input) : C(input) {}
  t_Vector4_(C &&input) : C(input) {}

  t_Vector4_ &operator=(const C &input) {
    static_cast<C &>(*this) = input;
    return *this;
  }
  t_Vector4_ &operator=(C &&input) {
    static_cast<C &>(*this) = input;
    return *this;
  }

  operator t_Vector<value_type>() const {
    return t_Vector<value_type>(this->X, this->Y, this->Z);
  }
  operator t_Vector2<value_type>() const {
    return t_Vector2<value_type>(this->X, this->Y);
  }

  value_type &operator[](size_t pos) { return this->_arr[pos]; }
  const value_type &operator[](size_t pos) const { return this->_arr[pos]; }

  void SwapEndian() {
    if constexpr (sizeof(this->X) > 1) {
      FByteswapper(this->X);
      FByteswapper(this->Y);
      FByteswapper(this->Z);
      FByteswapper(this->W);
    }
  }
};

template <typename T>
template <typename R>
t_Vector<T>::operator t_Vector4_<R>() const {
  return t_Vector4_<R>(this->X, this->Y, this->Z, 0.0f);
}

template <class C> struct _getType<t_Vector2<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr size_t SIZE = sizeof(C) * 2;
  static constexpr size_t SUBSIZE = sizeof(C);
  static constexpr uint16 NUMITEMS = 2;
};
template <class C> struct _getType<t_Vector<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<C>::TYPE;
  static constexpr size_t SIZE = sizeof(C) * 3;
  static constexpr size_t SUBSIZE = sizeof(C);
  static constexpr uint16 NUMITEMS = 3;
};
template <class C> struct _getType<t_Vector4_<C>> {
  static constexpr REFType TYPE = REFType::Vector;
  static constexpr REFType SUBTYPE = _getType<typename C::value_type>::TYPE;
  static constexpr size_t SIZE = sizeof(typename C::value_type) * 4;
  static constexpr size_t SUBSIZE = sizeof(typename C::value_type);
  static constexpr uint16 NUMITEMS = 4;
};
