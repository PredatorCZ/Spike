#include "vectors.hpp"
#include <pmmintrin.h>

class V4MMXShrtType;

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
  ES_FORCEINLINE __m128 CollectAdd(__m128 input) const {
    // input = _mm_add_ss(input, _mm_shuffle_ps(input, input, _MM_SHUFFLE(3, 2,
    // 0, 1)));
    //__m128 temp2 = _mm_movehl_ps(input, input);
    // input = _mm_add_ss(input, temp2);
    // input = _mm_add_ss(input, _mm_shuffle_ps(temp2, temp2, _MM_SHUFFLE(3, 2,
    // 0, 1))); return input;
    return _mm_hadd_ps(_mm_hadd_ps(input, input), input); // SSE3+ only
  }

  ES_FORCEINLINE static bool Compare(__m128 input1, __m128 input2) {
    static const __m128 _epsilon = _mm_set1_ps(FLT_EPSILON);
    const __m128 result =
        _mm_and_ps(_mm_cmple_ps(input2, _mm_add_ps(input1, _epsilon)),
                   _mm_cmpge_ps(input2, _mm_sub_ps(input1, _epsilon)));

    return reinterpret_cast<const t_Vector4<int> &>(result) !=
           t_Vector4<int>(0.0f, 0.0f, 0.0f, 0.0f);
  }

public:
  V4SimdFltType(const __m128 &input) { _data = input; }
  V4SimdFltType() { _data = _mm_setzero_ps(); }
  V4SimdFltType(eltype s) { _data = _mm_set1_ps(s); }
  V4SimdFltType(eltype x, eltype y, eltype z, eltype w) {
    _data = _mm_set_ps(x, y, z, w);
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
  V4SimdFltType operator~() const { return *this ^ V4SimdFltType(0xffff); }

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

  operator V4MMXShrtType();

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

  ES_FORCEINLINE bool IsSymetrical() const {
    const __m128 temp = _mm_shuffle_ps(_data, _data, _MM_SHUFFLE(3, 1, 0, 2));
    return Compare(_mm_hsub_ps(_data, _data), _mm_hsub_ps(temp, temp));
  }

  ES_FORCEINLINE int Sign() const { return _mm_movemask_ps(_data) ? -1 : 1; }

  ES_FORCEINLINE float Length() const {
    __m128 temp = CollectAdd(_mm_mul_ps(_data, _data));
    temp = _mm_sqrt_ss(temp);
    return _mm_cvtss_f32(temp);
  }

  ES_FORCEINLINE float Dot(const V4SimdFltType &input) const {
    return _mm_cvtss_f32(CollectAdd(_mm_mul_ps(_data, input._data)));
  }

  ES_FORCEINLINE V4SimdFltType &Normalize() {
    float len = Length();

    if (!len)
      return *this;

    return *this /= len;
  }
};

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

ES_INLINE V4SimdFltType::operator V4MMXShrtType() {
  return _mm_cvtps_pi16(_data);
}
#endif

typedef _t_Vector4<V4SimdFltType> Vector4A16;

