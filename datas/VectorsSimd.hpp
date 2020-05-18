/*      SIMD Vector Classes with Intel intrinsics
        more info in README for PreCore Project

        Copyright 2019 Lukas Cone

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

class V4MMXShrtType;
template <class eType> class V4SimdIntType_t;
typedef V4SimdIntType_t<int32> V4SimdIntType;

#ifdef _MSC_VER
thread_local static float __V4SimdFltType_EPSILON = FLT_EPSILON;
#else
thread_local static __m128 __V4SimdFltType_EPSILON = _mm_set1_ps(FLT_EPSILON);
#endif

class alignas(16) V4SimdFltType {
public:
  typedef float eltype;

  union {
    __m128 _data;
    eltype _arr[4];
    struct {
      eltype X, Y, Z, W;
    };
  };

private:
  __m128 CollectAdd(__m128 input) const {
    return _mm_hadd_ps(_mm_hadd_ps(input, input), input);
  }

  static __m128 GetEpsilon() {
    return
#ifdef _MSC_VER
        _mm_set1_ps(__V4SimdFltType_EPSILON);
#else
        __V4SimdFltType_EPSILON;
#endif
  }

  static bool Compare(__m128 input1, __m128 input2) {
    const __m128 result =
        _mm_and_ps(_mm_cmple_ps(input2, _mm_add_ps(input1, GetEpsilon())),
                   _mm_cmpge_ps(input2, _mm_sub_ps(input1, GetEpsilon())));

    return _mm_movemask_ps(result) == 0xF;
  }

public:
  V4SimdFltType(const __m128 &input) { _data = input; }
  V4SimdFltType() { _data = _mm_setzero_ps(); }
  V4SimdFltType(eltype s) { _data = _mm_set1_ps(s); }
  V4SimdFltType(eltype x, eltype y, eltype z, eltype w) {
    _data = _mm_set_ps(w, z, y, x);
  }
  V4SimdFltType(const Vector &input, float w) {
    _data = _mm_set_ps(w, input.Z, input.Y, input.X);
  }
  V4SimdFltType(const Vector4 &input) {
    _data = _mm_set_ps(input.W, input.Z, input.Y, input.X);
  }

  static void SetEpsilon(float newEpsilon) {
    __V4SimdFltType_EPSILON =
#ifdef _MSC_VER
        newEpsilon;
#else
        _mm_set1_ps(newEpsilon);
#endif
  }

  V4SimdFltType &operator+=(const V4SimdFltType &input) {
    return *this = *this + input;
  }
  V4SimdFltType &operator-=(const V4SimdFltType &input) {
    return *this = *this - input;
  }
  V4SimdFltType &operator*=(const V4SimdFltType &input) {
    return *this = *this * input;
  }
  V4SimdFltType &operator/=(const V4SimdFltType &input) {
    return *this = *this / input;
  }

  V4SimdFltType operator+(const V4SimdFltType &input) const {
    return _mm_add_ps(_data, input._data);
  }
  V4SimdFltType operator-(const V4SimdFltType &input) const {
    return _mm_sub_ps(_data, input._data);
  }
  V4SimdFltType operator*(const V4SimdFltType &input) const {
    return _mm_mul_ps(_data, input._data);
  }
  V4SimdFltType operator/(const V4SimdFltType &input) const {
    return _mm_div_ps(_data, input._data);
  }

  V4SimdFltType &operator+=(const eltype &input) {
    return *this = *this + input;
  }
  V4SimdFltType &operator-=(const eltype &input) {
    return *this = *this - input;
  }
  V4SimdFltType &operator*=(const eltype &input) {
    return *this = *this * input;
  }
  V4SimdFltType &operator/=(const eltype &input) {
    return *this = *this / input;
  }

  V4SimdFltType operator*(const eltype &input) const {
    return *this * V4SimdFltType(input);
  }
  V4SimdFltType operator+(const eltype &input) const {
    return *this + V4SimdFltType(input);
  }
  V4SimdFltType operator-(const eltype &input) const {
    return *this - V4SimdFltType(input);
  }
  V4SimdFltType operator/(const eltype &input) const {
    return *this / V4SimdFltType(input);
  }

  V4SimdFltType operator&(const eltype &input) const {
    return *this & V4SimdFltType(input);
  }
  V4SimdFltType operator|(const eltype &input) const {
    return *this | V4SimdFltType(input);
  }

  V4SimdFltType operator&(const V4SimdFltType &input) const {
    return _mm_and_ps(_data, input._data);
  }
  V4SimdFltType operator|(const V4SimdFltType &input) const {
    return _mm_or_ps(_data, input._data);
  }
  V4SimdFltType operator^(const V4SimdFltType &input) const {
    return _mm_xor_ps(_data, input._data);
  }
  V4SimdFltType operator~() const {
    return *this ^ V4SimdFltType(_mm_castsi128_ps(_mm_set1_epi32(-1)));
  }

  V4SimdFltType &operator&=(const V4SimdFltType &input) {
    return *this = *this & input;
  }
  V4SimdFltType &operator|=(const V4SimdFltType &input) {
    return *this = *this | input;
  }
  V4SimdFltType &operator^=(const V4SimdFltType &input) {
    return *this = *this ^ input;
  }

  V4SimdFltType &operator&=(const eltype &input) {
    return *this = *this & input;
  }
  V4SimdFltType &operator|=(const eltype &input) {
    return *this = *this | input;
  }
  V4SimdFltType &operator^=(const eltype &input) {
    return *this = *this ^ input;
  }

  V4SimdFltType operator-() const { return *this * -1.f; }

  operator V4MMXShrtType() const;
  operator V4SimdIntType() const;

  template <typename T> V4ScalarType<T> Convert() const {
    return V4ScalarType<T>(static_cast<T>(X), static_cast<T>(Y),
                           static_cast<T>(Z), static_cast<T>(W));
  }

  bool operator==(const V4SimdFltType &input) const {
    return Compare(input._data, _data);
  }

  bool operator!=(const V4SimdFltType &input) const {
    return !(*this == input);
  }

  bool IsSymetrical() const {
    const __m128 temp = _mm_shuffle_ps(_data, _data, _MM_SHUFFLE(3, 1, 0, 2));
    return Compare(_mm_hsub_ps(_data, _data), _mm_hsub_ps(temp, temp));
  }

  int Sign() const { return _mm_movemask_ps(_data) ? -1 : 1; }

  float Length() const {
    __m128 temp = CollectAdd(_mm_mul_ps(_data, _data));
    temp = _mm_sqrt_ss(temp);
    return _mm_cvtss_f32(temp);
  }

  float Dot(const V4SimdFltType &input) const { // use _mm_dp_ps
    return _mm_cvtss_f32(CollectAdd(_mm_mul_ps(_data, input._data)));
  }

  V4SimdFltType &Normalize() {
    float len = Length();

    if (!len)
      return *this;

    return *this /= len;
  }

  V4SimdFltType QConjugate() const {
    return *this * V4SimdFltType(-1.0f, -1.0f, -1.0f, 1.0f);
  }

  V4SimdFltType &QComputeElement(int elementIndex = 3) {
    _arr[elementIndex] =
        sqrtf(1.0f - _mm_cvtss_f32(CollectAdd(_mm_mul_ps(_data, _data))));

    return *this;
  }
};

typedef _t_Vector4<V4SimdFltType> Vector4A16;

template <class eType> class alignas(16) V4SimdIntType_t {
  static const bool IS_UNSIGNED = std::is_unsigned<eType>::value;

public:
  typedef eType eltype;

  union {
    __m128i _data;
    eltype _arr[4];
    struct {
      eltype X, Y, Z, W;
    };
  };

  V4SimdIntType_t(const __m128i &input) { _data = input; }
  V4SimdIntType_t() { _data = _mm_setzero_si128(); }
  template <class _other0>
  V4SimdIntType_t(const V4SimdIntType_t<_other0> &input) {
    _data = input._data;
  }
  V4SimdIntType_t(eltype s) { _data = _mm_set1_epi32(s); }
  V4SimdIntType_t(eltype x, eltype y, eltype z, eltype w) {
    _data = _mm_set_epi32(w, z, y, x);
  }
  V4SimdIntType_t(const t_Vector4<eltype> &input) {
    _data = _mm_set_epi32(input.W, input.Z, input.Y, input.X);
  }

  operator V4SimdFltType() const { return _mm_cvtepi32_ps(_data); }

  V4SimdIntType_t &operator+=(const V4SimdIntType_t &input) {
    return *this = *this + input;
  }
  V4SimdIntType_t &operator-=(const V4SimdIntType_t &input) {
    return *this = *this - input;
  }
  V4SimdIntType_t &operator*=(const V4SimdIntType_t &input) {
    return *this = *this * input;
  }

  V4SimdIntType_t operator+(const V4SimdIntType_t &input) const {
    return _mm_add_epi32(_data, input._data);
  }
  V4SimdIntType_t operator-(const V4SimdIntType_t &input) const {
    return _mm_sub_epi32(_data, input._data);
  }
  V4SimdIntType_t operator*(const V4SimdIntType_t &input) const {
    return _mm_mullo_epi32(_data, input._data);
  }

  V4SimdIntType_t &operator+=(const eltype &input) {
    return *this = *this + input;
  }
  V4SimdIntType_t &operator-=(const eltype &input) {
    return *this = *this - input;
  }

  V4SimdIntType_t operator+(const eltype &input) const {
    return *this + V4SimdIntType_t(input);
  }
  V4SimdIntType_t operator-(const eltype &input) const {
    return *this - V4SimdIntType_t(input);
  }

  V4SimdIntType_t operator&(const eltype &input) const {
    return *this & V4SimdIntType_t(input);
  }
  V4SimdIntType_t operator|(const eltype &input) const {
    return *this | V4SimdIntType_t(input);
  }

  V4SimdIntType_t operator&(const V4SimdIntType_t &input) const {
    return _mm_and_si128(_data, input._data);
  }
  V4SimdIntType_t operator|(const V4SimdIntType_t &input) const {
    return _mm_or_si128(_data, input._data);
  }
  V4SimdIntType_t operator^(const V4SimdIntType_t &input) const {
    return _mm_xor_si128(_data, input._data);
  }

  V4SimdIntType_t operator<<(const eltype &input) const {
    return _mm_slli_epi32(_data, input);
  }

  // Logical shift
  template<class C = V4SimdIntType_t>
  typename std::enable_if<IS_UNSIGNED, C>::type
  operator>>(const eltype &input) const {
    return _mm_srli_epi32(_data, input);
  }

  // Arithmetic shift
  template<class C = V4SimdIntType_t>
  typename std::enable_if<!IS_UNSIGNED, C>::type
  operator>>(const eltype &input) const {
    return _mm_srai_epi32(_data, input);
  }

  V4SimdIntType_t operator~() const {
    return *this ^ V4SimdIntType_t(0xffffffff);
  }

  V4SimdIntType_t &operator&=(const V4SimdIntType_t &input) {
    return *this = *this & input;
  }
  V4SimdIntType_t &operator|=(const V4SimdIntType_t &input) {
    return *this = *this | input;
  }
  V4SimdIntType_t &operator^=(const V4SimdIntType_t &input) {
    return *this = *this ^ input;
  }

  V4SimdIntType_t &operator&=(const eltype &input) {
    return *this = *this & input;
  }
  V4SimdIntType_t &operator|=(const eltype &input) {
    return *this = *this | input;
  }
  V4SimdIntType_t &operator^=(const eltype &input) {
    return *this = *this ^ input;
  }
  V4SimdIntType_t &operator<<=(const eltype &input) {
    return *this = *this << input;
  }
  V4SimdIntType_t &operator>>=(const eltype &input) {
    return *this = *this >> input;
  }

  template <typename T> V4ScalarType<T> Convert() const {
    return V4ScalarType<T>(static_cast<T>(X), static_cast<T>(Y),
                           static_cast<T>(Z), static_cast<T>(W));
  }

  bool operator==(const V4SimdIntType_t &input) const {
    const __m128i rsInt = _mm_cmpeq_epi32(input._data, _data);
    const __m128 result = reinterpret_cast<const __m128 &>(rsInt);
    return _mm_movemask_ps(result) == 0xF;
  }

  bool operator!=(const V4SimdIntType_t &input) const {
    return !(*this == input);
  }
};

inline V4SimdFltType::operator V4SimdIntType() const {
  return _mm_cvtps_epi32(_data);
}

typedef _t_Vector4<V4SimdIntType> IVector4A16;
typedef _t_Vector4<V4SimdIntType_t<uint32>> UIVector4A16;

#ifdef ES_USE_MMX

class alignas(8) V4MMXShrtType {
public:
  typedef short eltype;

  union {
    __m64 _data;
    eltype _arr[4];
    struct {
      eltype X, Y, Z, W;
    };
  };

public:
  V4MMXShrtType(const __m64 &input) { _data = input; }
  V4MMXShrtType() { _data = _mm_setzero_si64(); }
  V4MMXShrtType(eltype s) { _data = _mm_set1_pi16(s); }
  V4MMXShrtType(eltype x, eltype y, eltype z, eltype w) {
    _data = _mm_set_pi16(x, y, z, w);
  }

  V4MMXShrtType &operator+=(const V4MMXShrtType &input) {
    return *this = *this + input;
  }
  V4MMXShrtType &operator-=(const V4MMXShrtType &input) {
    return *this = *this - input;
  }
  V4MMXShrtType &operator*=(const V4MMXShrtType &input) {
    return *this = *this * input;
  }

  V4MMXShrtType operator+(const V4MMXShrtType &input) const {
    return _mm_adds_pi16(_data, input._data);
  }
  V4MMXShrtType operator-(const V4MMXShrtType &input) const {
    return _mm_subs_pi16(_data, input._data);
  }
  V4MMXShrtType operator*(const V4MMXShrtType &input) const {
    return _mm_adds_pi16(_mm_mulhi_pi16(_data, input._data),
                         _mm_mullo_pi16(_data, input._data));
  }

  V4MMXShrtType &operator+=(const eltype &input) {
    return *this = *this + input;
  }
  V4MMXShrtType &operator-=(const eltype &input) {
    return *this = *this - input;
  }
  V4MMXShrtType &operator*=(const eltype &input) {
    return *this = *this * input;
  }

  V4MMXShrtType operator*(const eltype &input) const {
    return *this * V4MMXShrtType(input);
  }
  V4MMXShrtType operator+(const eltype &input) const {
    return *this + V4MMXShrtType(input);
  }
  V4MMXShrtType operator-(const eltype &input) const {
    return *this - V4MMXShrtType(input);
  }

  V4MMXShrtType operator&(const eltype &input) const {
    return *this & V4MMXShrtType(input);
  }
  V4MMXShrtType operator|(const eltype &input) const {
    return *this | V4MMXShrtType(input);
  }
  V4MMXShrtType operator<<(const eltype &input) const {
    return *this << V4MMXShrtType(input);
  }
  V4MMXShrtType operator>>(const eltype &input) const {
    return *this >> V4MMXShrtType(input);
  }

  V4MMXShrtType operator&(const V4MMXShrtType &input) const {
    return _mm_and_si64(_data, input._data);
  }
  V4MMXShrtType operator|(const V4MMXShrtType &input) const {
    return _mm_or_si64(_data, input._data);
  }
  V4MMXShrtType operator<<(const V4MMXShrtType &input) const {
    return _mm_sll_pi16(_data, input._data);
  }
  V4MMXShrtType operator>>(const V4MMXShrtType &input) const {
    return _mm_srl_pi16(_data, input._data);
  }
  V4MMXShrtType operator^(const V4MMXShrtType &input) const {
    return _mm_xor_si64(_data, input._data);
  }
  V4MMXShrtType operator~() const { return *this ^ V4MMXShrtType(0xffff); }

  V4MMXShrtType &operator&=(const V4MMXShrtType &input) {
    return *this = *this & input;
  }
  V4MMXShrtType &operator|=(const V4MMXShrtType &input) {
    return *this = *this | input;
  }
  V4MMXShrtType &operator<<=(const V4MMXShrtType &input) {
    return *this = *this << input;
  }
  V4MMXShrtType &operator>>=(const V4MMXShrtType &input) {
    return *this = *this >> input;
  }
  V4MMXShrtType &operator^=(const V4MMXShrtType &input) {
    return *this = *this ^ input;
  }

  V4MMXShrtType &operator&=(const eltype &input) {
    return *this = *this & input;
  }
  V4MMXShrtType &operator|=(const eltype &input) {
    return *this = *this | input;
  }
  V4MMXShrtType &operator<<=(const eltype &input) {
    return *this = *this << input;
  }
  V4MMXShrtType &operator>>=(const eltype &input) {
    return *this = *this >> input;
  }
  V4MMXShrtType &operator^=(const eltype &input) {
    return *this = *this ^ input;
  }

  V4MMXShrtType operator-() const { return *this * -1.f; }

  template <typename T> V4ScalarType<T> Convert() const {
    return V4ScalarType<T>(static_cast<T>(X), static_cast<T>(Y),
                           static_cast<T>(Z), static_cast<T>(W));
  }

  operator V4SimdFltType() { return _mm_cvtpi16_ps(_data); }

  bool operator==(const V4MMXShrtType &input) const {
    return !_mm_cvtm64_si64((*this ^ input)._data);
  }

  bool operator!=(const V4MMXShrtType &input) const {
    return !(*this == input);
  }

  bool IsSymetrical() const {
    return V4MMXShrtType(_mm_unpacklo_pi32(_data, _data)) ==
           V4MMXShrtType(_mm_unpackhi_pi32(_data, _data));
  }

  // Call this after mmx instruction set and before floating point operations
  void empty() { _mm_empty(); }
};

typedef _t_Vector4<V4MMXShrtType> SVector4A8;

V4SimdFltType::operator V4MMXShrtType() const { return _mm_cvtps_pi16(_data); }
#endif
