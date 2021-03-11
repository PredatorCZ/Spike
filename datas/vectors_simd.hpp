/*  SIMD Vector Classes with Intel intrinsics
    more info in README for PreCore Project

    Copyright 2019-2021 Lukas Cone

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
#include <smmintrin.h>

#ifdef _MSC_VER
thread_local static float __V4SimdFltType_EPSILON = FLT_EPSILON;
#else
thread_local static __m128 __V4SimdFltType_EPSILON = _mm_set1_ps(FLT_EPSILON);
#endif

class alignas(16) V4SimdFltType {
  using vector = V4SimdFltType;
  using vec_ref = vector &;
  using vec_cref = const vector &;

public:
  using value_type = float;
  using store_type = __m128;

  union {
    __m128 _data;
    value_type _arr[4];
    struct {
      value_type X, Y, Z, W;
    };
  };

private:
  static store_type GetEpsilon() {
    return
#ifdef _MSC_VER
        _mm_set1_ps(__V4SimdFltType_EPSILON);
#else
        __V4SimdFltType_EPSILON;
#endif
  }

  static bool Compare(store_type input1, store_type input2) {
    const store_type result =
        _mm_and_ps(_mm_cmple_ps(input2, _mm_add_ps(input1, GetEpsilon())),
                   _mm_cmpge_ps(input2, _mm_sub_ps(input1, GetEpsilon())));

    return _mm_movemask_ps(result) == 0xF;
  }

public:
  V4SimdFltType(store_type input) { _data = input; }
  V4SimdFltType() { _data = _mm_setzero_ps(); }
  V4SimdFltType(value_type s) { _data = _mm_set1_ps(s); }
  V4SimdFltType(value_type x, value_type y, value_type z, value_type w) {
    _data = _mm_set_ps(w, z, y, x);
  }
  V4SimdFltType(const Vector &input, float w) {
    _data = _mm_set_ps(w, input.Z, input.Y, input.X);
  }
  V4SimdFltType(const Vector4 &input) {
    _data = _mm_set_ps(input.W, input.Z, input.Y, input.X);
  }

  // Set thread safe TU local expsilon for comparing
  // Default: FLT_EPSILON
  static void SetEpsilon(value_type newEpsilon) {
    __V4SimdFltType_EPSILON =
#ifdef _MSC_VER
        newEpsilon;
#else
        _mm_set1_ps(newEpsilon);
#endif
  }

  vector operator+(vec_cref input) const {
    return _mm_add_ps(_data, input._data);
  }
  vector operator-(vec_cref input) const {
    return _mm_sub_ps(_data, input._data);
  }
  vector operator*(vec_cref input) const {
    return _mm_mul_ps(_data, input._data);
  }
  vector operator/(vec_cref input) const {
    return _mm_div_ps(_data, input._data);
  }
  vector operator&(vec_cref input) const {
    return _mm_and_ps(_data, input._data);
  }
  vector operator|(vec_cref input) const {
    return _mm_or_ps(_data, input._data);
  }
  vector operator^(vec_cref input) const {
    return _mm_xor_ps(_data, input._data);
  }
  vector operator~() const {
    return *this ^ vector(_mm_castsi128_ps(_mm_set1_epi32(-1)));
  }

  vector operator*(value_type input) const { return *this * vector(input); }
  vector operator+(value_type input) const { return *this + vector(input); }
  vector operator-(value_type input) const { return *this - vector(input); }
  vector operator/(value_type input) const { return *this / vector(input); }
  vector operator&(value_type input) const { return *this & vector(input); }
  vector operator|(value_type input) const { return *this | vector(input); }
  vector operator^(value_type input) const { return *this ^ vector(input); }

  vec_ref operator+=(vec_cref input) { return *this = *this + input; }
  vec_ref operator-=(vec_cref input) { return *this = *this - input; }
  vec_ref operator*=(vec_cref input) { return *this = *this * input; }
  vec_ref operator/=(vec_cref input) { return *this = *this / input; }
  vec_ref operator&=(vec_cref input) { return *this = *this & input; }
  vec_ref operator|=(vec_cref input) { return *this = *this | input; }
  vec_ref operator^=(vec_cref input) { return *this = *this ^ input; }

  vec_ref operator+=(value_type input) { return *this = *this + input; }
  vec_ref operator-=(value_type input) { return *this = *this - input; }
  vec_ref operator*=(value_type input) { return *this = *this * input; }
  vec_ref operator/=(value_type input) { return *this = *this / input; }
  vec_ref operator&=(value_type input) { return *this = *this & input; }
  vec_ref operator|=(value_type input) { return *this = *this | input; }
  vec_ref operator^=(value_type input) { return *this = *this ^ input; }

  vector operator-() const { return *this * -1.f; }

  operator V4SimdIntType() const;

  template <typename T> V4ScalarType<T> Convert() const {
    return V4ScalarType<T>(static_cast<T>(X), static_cast<T>(Y),
                           static_cast<T>(Z), static_cast<T>(W));
  }

  bool operator==(vec_cref input) const { return Compare(input._data, _data); }

  bool operator!=(vec_cref input) const { return !(*this == input); }

  // Check if X == Y == Z == W
  bool IsSymetrical() const {
    const store_type temp =
        _mm_shuffle_ps(_data, _data, _MM_SHUFFLE(3, 1, 0, 2));
    return Compare(_mm_hsub_ps(_data, _data), _mm_hsub_ps(temp, temp));
  }

  // Return -1 if [X | Y | Z | W] < 0
  int Sign() const { return _mm_movemask_ps(_data) ? -1 : 1; }

  value_type Length() const {
    const auto temp = _mm_sqrt_ss(DotV(*this)._data);
    return _mm_cvtss_f32(temp);
  }

  value_type Dot(vec_cref input) const {
    return _mm_cvtss_f32(_mm_dp_ps(_data, input._data, 0xf1));
  }

  vector DotV(vec_cref input) const {
    return _mm_dp_ps(_data, input._data, 0xff);
  }

  vector Cross(vec_cref input) const {
    const auto temp0 =
        vector(_mm_shuffle_ps(_data, _data, _MM_SHUFFLE(3, 0, 2, 1)));
    const auto temp1 = vector(
        _mm_shuffle_ps(input._data, input._data, _MM_SHUFFLE(3, 1, 0, 2)));
    const auto temp2 =
        vector(_mm_shuffle_ps(_data, _data, _MM_SHUFFLE(3, 1, 0, 2)));
    const auto temp3 = vector(
        _mm_shuffle_ps(input._data, input._data, _MM_SHUFFLE(3, 0, 2, 1)));
    return (temp0 * temp1) - (temp2 * temp3);
  }

  vec_ref Normalize() {
    value_type len = Length();

    if (!len) {
      return *this;
    }

    return *this /= len;
  }

  vector QConjugate() const {
    return *this * vector(-1.0f, -1.0f, -1.0f, 1.0f);
  }

  template <size_t elementIndex = 3> vec_ref QComputeElement() {
    const auto res0 = vector(1.f) - DotV(*this);
    const auto res1 = _mm_sqrt_ss(res0._data);

    _data = _mm_insert_ps(_data, res1, _MM_MK_INSERTPS_NDX(0, elementIndex, 0));

    return *this;
  }

  vec_ref QComputeElementVar(int elementIndex = 3) {
    _arr[elementIndex] = sqrtf(1.0f - Dot(*this));

    return *this;
  }
};

template <class eType> class alignas(16) V4SimdIntType_t {
  static constexpr bool IsUnsigned() { return std::is_unsigned<eType>::value; }
  using vector = V4SimdIntType_t;
  using vec_ref = vector &;
  using vec_cref = const vector &;

public:
  using value_type = eType;
  using store_type = __m128i;

  union {
    store_type _data;
    value_type _arr[4];
    struct {
      value_type X, Y, Z, W;
    };
  };

  V4SimdIntType_t(store_type input) { _data = input; }
  V4SimdIntType_t() { _data = _mm_setzero_si128(); }
  template <class _other0>
  V4SimdIntType_t(const V4SimdIntType_t<_other0> &input) {
    _data = input._data;
  }
  V4SimdIntType_t(value_type s) { _data = _mm_set1_epi32(s); }
  V4SimdIntType_t(value_type x, value_type y, value_type z, value_type w) {
    _data = _mm_set_epi32(w, z, y, x);
  }
  V4SimdIntType_t(const t_Vector4<value_type> &input) {
    _data = _mm_set_epi32(input.W, input.Z, input.Y, input.X);
  }

  operator V4SimdFltType() const { return _mm_cvtepi32_ps(_data); }

  vector operator+(vec_cref input) const {
    return _mm_add_epi32(_data, input._data);
  }
  vector operator-(vec_cref input) const {
    return _mm_sub_epi32(_data, input._data);
  }
  vector operator*(vec_cref input) const {
    return _mm_mullo_epi32(_data, input._data);
  }
  vector operator&(vec_cref input) const {
    return _mm_and_si128(_data, input._data);
  }
  vector operator|(vec_cref input) const {
    return _mm_or_si128(_data, input._data);
  }
  vector operator^(vec_cref input) const {
    return _mm_xor_si128(_data, input._data);
  }

  vector operator~() const { return *this ^ vector(0xffffffff); }

  vector operator+(value_type input) const { return *this + vector(input); }
  vector operator-(value_type input) const { return *this - vector(input); }
  vector operator*(value_type input) const { return *this * vector(input); }
  vector operator&(value_type input) const { return *this & vector(input); }
  vector operator|(value_type input) const { return *this | vector(input); }
  vector operator<<(value_type input) const {
    return _mm_slli_epi32(_data, input);
  }

  // Logical shift
  template <class C = vector>
  typename std::enable_if<IsUnsigned(), C>::type
  operator>>(value_type input) const {
    return _mm_srli_epi32(_data, input);
  }

  // Arithmetic shift
  template <class C = vector>
  typename std::enable_if<!IsUnsigned(), C>::type
  operator>>(value_type input) const {
    return _mm_srai_epi32(_data, input);
  }

  vec_ref operator+=(vec_cref input) { return *this = *this + input; }
  vec_ref operator-=(vec_cref input) { return *this = *this - input; }
  vec_ref operator*=(vec_cref input) { return *this = *this * input; }
  vec_ref operator&=(vec_cref input) { return *this = *this & input; }
  vec_ref operator|=(vec_cref input) { return *this = *this | input; }
  vec_ref operator^=(vec_cref input) { return *this = *this ^ input; }

  vec_ref operator+=(value_type input) { return *this = *this + input; }
  vec_ref operator-=(value_type input) { return *this = *this - input; }
  vec_ref operator*=(value_type input) { return *this = *this * input; }
  vec_ref operator&=(value_type input) { return *this = *this & input; }
  vec_ref operator|=(value_type input) { return *this = *this | input; }
  vec_ref operator^=(value_type input) { return *this = *this ^ input; }
  vec_ref operator<<=(value_type input) { return *this = *this << input; }
  vec_ref operator>>=(value_type input) { return *this = *this >> input; }

  template <typename T> V4ScalarType<T> Convert() const {
    return V4ScalarType<T>(static_cast<T>(X), static_cast<T>(Y),
                           static_cast<T>(Z), static_cast<T>(W));
  }

  bool operator==(vec_cref input) const {
    const store_type rsInt = _mm_cmpeq_epi32(input._data, _data);
    const __m128 result = reinterpret_cast<const __m128 &>(rsInt);
    return _mm_movemask_ps(result) == 0xF;
  }

  bool operator!=(vec_cref input) const { return !(*this == input); }
};

inline V4SimdFltType::operator V4SimdIntType() const {
  return _mm_cvtps_epi32(_data);
}
