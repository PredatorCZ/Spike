/*  Variable floating point

    Copyright 2020-2023 Lukas Cone

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
#include "vectors_simd.hpp"

namespace esFloatDetail {
static float ToFloat(size_t value, size_t _mantissa, size_t EXPONENT_MASK,
                     size_t MANTISSA_MASK, size_t SIGN_MASK, size_t TOTAL_SIZE,
                     size_t EXP_REF) {
  int32x4a16 vtored(static_cast<int32>(value));
  int32x4a16 mskEMSM(vtored & int32x4a16(EXPONENT_MASK, MANTISSA_MASK,
                                                 SIGN_MASK, MANTISSA_MASK));
  int32x4a16 shEMS(mskEMSM * int32x4a16(1 << (23 - _mantissa),
                                                1 << (23 - _mantissa),
                                                1 << (32 - TOTAL_SIZE), 1));
  const size_t exponentI = mskEMSM.X;
  mint32x4a16 finResult(shEMS);

  if (exponentI) {
    if (exponentI == EXPONENT_MASK) {
      finResult *= int32x4a16(0, 1, 1, 0);
      finResult += int32x4a16(0x7f800000, 0, 0, 0);
    } else {
      finResult *= int32x4a16(1, 1, 1, 0);
      finResult += int32x4a16(EXP_REF, 0, 0, 0);
    }
  } else {
    finResult *= int32x4a16(0, 0, 1, 1);
    finResult += int32x4a16(0x3f000000, 0, 0, 0);
  }

  const size_t result = _mm_extract_epi32(
      _mm_hadd_epi32(_mm_hadd_epi32(finResult._data, finResult._data),
                     finResult._data),
      0);
  const float resultFl = reinterpret_cast<const float &>(result);

  return exponentI ? resultFl : resultFl - 0.5f;
}
[[maybe_unused]] static float ToFloat(size_t value, size_t _mantissa,
                                      size_t _exponent, size_t _sign) {
  const size_t TOTAL_SIZE = _mantissa + _exponent + _sign;
  const size_t MANTISSA_MASK = (1 << _mantissa) - 1;
  const size_t EXPONENT_MASK_NS = (1 << _exponent) - 1;
  const size_t EXPONENT_MASK = EXPONENT_MASK_NS << _mantissa;
  const size_t SIGN_MASK = _sign ? 1 << (TOTAL_SIZE - 1) : 0;
  const size_t EXP_REF = (0x3f800000 - ((EXPONENT_MASK_NS >> 1) << 23));

  return ToFloat(value, _mantissa, EXPONENT_MASK, MANTISSA_MASK, SIGN_MASK,
                 TOTAL_SIZE, EXP_REF);
}

static size_t FromFloat(float input, size_t _mantissa, size_t _sign,
                        size_t MANTISSA_MASK, size_t SIGN_MASK,
                        size_t EXPONENT_MASK, size_t EXP_REF) {
  const size_t cvted = reinterpret_cast<uint32 &>(input);
  const float catchDenormal = input + 0.5f;
  const size_t cvtedDenorm = reinterpret_cast<const uint32 &>(catchDenormal);
  const size_t mskExp = cvted & 0x7f800000;
  const bool isSubNorm = (cvtedDenorm & 0x3f000000) == 0x3f000000;

  size_t result =
      (isSubNorm ? cvtedDenorm : (cvted >> (23 - _mantissa))) & MANTISSA_MASK;

  if (_sign && cvted & 0x80000000) {
    result |= SIGN_MASK;
  }

  if (mskExp == 0x7f800000) {
    result |= EXPONENT_MASK;
  } else if (!isSubNorm) {
    const size_t unRef = mskExp - EXP_REF;
    result |= (unRef >> (23 - _mantissa)) & EXPONENT_MASK;
  }

  return result;
}
[[maybe_unused]] static size_t FromFloat(float value, size_t _mantissa,
                                         size_t _exponent, size_t _sign) {
  const size_t TOTAL_SIZE = _mantissa + _exponent + _sign;
  const size_t MANTISSA_MASK = (1 << _mantissa) - 1;
  const size_t EXPONENT_MASK_NS = (1 << _exponent) - 1;
  const size_t EXPONENT_MASK = EXPONENT_MASK_NS << _mantissa;
  const size_t SIGN_MASK = _sign ? 1 << (TOTAL_SIZE - 1) : 0;
  const size_t EXP_REF = (0x3f800000 - ((EXPONENT_MASK_NS >> 1) << 23));

  return FromFloat(value, _mantissa, _sign, MANTISSA_MASK, SIGN_MASK,
                   EXPONENT_MASK, EXP_REF);
}

} // namespace esFloatDetail

template <size_t _mantissa, size_t _exponent, bool _sign> class esFloat {
public:
  static constexpr size_t TOTAL_SIZE = _mantissa + _exponent + _sign;
  static constexpr size_t MANTISSA_MASK = (1 << _mantissa) - 1;
  static constexpr size_t EXPONENT_MASK_NS = (1 << _exponent) - 1;
  static constexpr size_t EXPONENT_MASK = EXPONENT_MASK_NS << _mantissa;
  static constexpr size_t SIGN_MASK = _sign ? 1 << (TOTAL_SIZE - 1) : 0;
  static constexpr size_t EXP_REF =
      (0x3f800000 - ((EXPONENT_MASK_NS >> 1) << 23));

  static float ToFloat(size_t value) {
    return esFloatDetail::ToFloat(value, _mantissa, EXPONENT_MASK,
                                  MANTISSA_MASK, SIGN_MASK, TOTAL_SIZE,
                                  EXP_REF);
  }

  static size_t FromFloat(float input) {
    return esFloatDetail::FromFloat(input, _mantissa, _sign, MANTISSA_MASK,
                                    SIGN_MASK, EXPONENT_MASK, EXP_REF);
  }

  static real32x4a16 _to_ps(mint32x4a16 input) {
    return reinterpret_cast<const __m128 &>(input);
  }

  static int32x4a16 _to_se(mreal32x4a16 input) {
    return reinterpret_cast<const __m128i &>(input);
  }

  static int32x4a16 FromVector4(mreal32x4a16 value) {
    int32x4a16 cvted = _to_se(value);
    int32x4a16 shtSign(((cvted & 0x80000000) >> (32 - TOTAL_SIZE)) &
                              SIGN_MASK);

    int32x4a16 mskExp(cvted & 0x7f800000);
    int32x4a16 shtMant(cvted >> (23 - _mantissa));

    value += 0.5f;
    int32x4a16 cvtedDenorm = _to_se(value);
    int32x4a16 mskExpDenorm(cvtedDenorm & 0x7f800000);

    int32x4a16 denormMask(
        _mm_cmpeq_epi32(mskExpDenorm._data, int32x4a16(0x3f000000)._data));

    real32x4a16 blendMant(_mm_blendv_ps(_to_ps(shtMant)._data,
                                             _to_ps(cvtedDenorm)._data,
                                             _to_ps(denormMask)._data));
    int32x4a16 maskedMant(_to_se(blendMant) & MANTISSA_MASK);

    int32x4a16 nanMask(
        _mm_cmpeq_epi32(mskExp._data, int32x4a16(0x7f800000)._data));

    int32x4a16 unRef(((mskExp - EXP_REF) >> (23 - _mantissa)) &
                            EXPONENT_MASK);

    real32x4a16 blendExp0(_mm_blendv_ps(_to_ps(unRef)._data,
                                             _to_ps(EXPONENT_MASK)._data,
                                             _to_ps(nanMask)._data));

    real32x4a16 blendExp1(_mm_blendv_ps(
        blendExp0._data, real32x4a16()._data, _to_ps(denormMask)._data));

    return _to_se(blendExp1) | maskedMant | shtSign;
  }

  static real32x4a16 ToVector4(int32x4a16 value) {
    int32x4a16 exponentI(value & EXPONENT_MASK);

    int32x4a16 mantissa((value & MANTISSA_MASK) << (23 - _mantissa));
    int32x4a16 sign((value & SIGN_MASK) << (32 - TOTAL_SIZE));

    int32x4a16 exponentMask(
        _mm_cmpeq_epi32(exponentI._data, int32x4a16(EXPONENT_MASK)._data));

    int32x4a16 nzResult(((exponentI << (23 - _mantissa)) + EXP_REF) |
                               mantissa);

    const auto imNANNZ = _mm_blendv_ps(_to_ps(nzResult)._data,
                                       _to_ps(int32x4a16(0x7f800000))._data,
                                       _to_ps(exponentMask)._data);

    int32x4a16 denormMask(
        _mm_cmpeq_epi32(exponentI._data, int32x4a16()._data));

    int32x4a16 denormResult0((value & MANTISSA_MASK) | 0x3f000000);
    real32x4a16 denormResult1(_to_ps(denormResult0) - 0.5f);

    int32x4a16 finalResult(_to_se(
        _mm_blendv_ps(imNANNZ, denormResult1._data, _to_ps(denormMask)._data)));

    return _to_ps(finalResult | sign);
  }
};

template <class value_type_, size_t mantissa, size_t exponent, bool sign>
struct esFloatValue {
  value_type_ value = 0;
  using value_type = value_type_;
  using float_codec = esFloat<mantissa, exponent, sign>;

  esFloatValue() = default;
  esFloatValue(float input) : value(float_codec::FromFloat(input)) {}
  esFloatValue(const esFloatValue &) = default;
  esFloatValue(esFloatValue &&) = default;
  esFloatValue &operator=(const esFloatValue &) = default;
  esFloatValue &operator=(esFloatValue &&) = default;
  operator float() const { return float_codec::ToFloat(value); }
  void Swap();
};

using float16 = esFloatValue<uint16, 10, 5, true>;
using float16_codec = esFloat<10, 5, true>;

struct VectorR11G11B10_UFLOAT {
  uint32 data;
  typedef esFloat<6, 5, false> codec_type;

  operator real32x4a16() const {
    int32x4a16 input0(int32x4a16(data) *
                             int32x4a16(1 << 21, 1 << 10, 1, 0));
    int32x4a16 input1(input0 >> 21);

    return codec_type::ToVector4(input1);
  }

  void Swap();
};

template <size_t _mantissa, size_t _exponent, bool _sign>
struct _getType<esFloat<_mantissa, _exponent, _sign>> : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::FloatingPoint;
  static constexpr JenHash Hash() { return JenHash{1}; }
  static constexpr size_t SIZE =
      esFloat<_mantissa, _exponent, _sign>::TOTAL_SIZE;
  static constexpr uint8 MANTISSA = _mantissa;
  static constexpr uint8 EXPONENT = _exponent;
  static constexpr bool SIGN = _sign;
};

template <class type, size_t _mantissa, size_t _exponent, bool _sign>
struct _getType<esFloatValue<type, _mantissa, _exponent, _sign>>
    : reflTypeDefault_ {
  static constexpr REFType TYPE = REFType::FloatingPoint;
  static constexpr JenHash Hash() { return JenHash{1}; }
  static constexpr size_t SIZE = sizeof(type);
  static constexpr uint8 MANTISSA = _mantissa;
  static constexpr uint8 EXPONENT = _exponent;
  static constexpr bool SIGN = _sign;
};
