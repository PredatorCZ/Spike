/*  Variable floating point

    Copyright 2020 Lukas Cone

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
    const IVector4A16 vtored(value);
    const IVector4A16 mskEMSM(vtored & IVector4A16(EXPONENT_MASK, MANTISSA_MASK,
                                                   SIGN_MASK, MANTISSA_MASK));
    const IVector4A16 shEMS(mskEMSM * IVector4A16(1 << (23 - _mantissa),
                                                  1 << (23 - _mantissa),
                                                  1 << (32 - TOTAL_SIZE), 1));
    const size_t exponentI = mskEMSM.X;
    IVector4A16 finResult(shEMS);

    if (exponentI) {
      if (exponentI == EXPONENT_MASK) {
        finResult *= IVector4A16(0, 1, 1, 0);
        finResult += IVector4A16(0x7f800000, 0, 0, 0);
      } else {
        finResult *= IVector4A16(1, 1, 1, 0);
        finResult += IVector4A16(EXP_REF, 0, 0, 0);
      }
    } else {
      finResult *= IVector4A16(0, 0, 1, 1);
      finResult += IVector4A16(0x3f000000, 0, 0, 0);
    }

    const size_t result = _mm_extract_epi32(
        _mm_hadd_epi32(_mm_hadd_epi32(finResult._data, finResult._data),
                       finResult._data),
        0);
    const float resultFl = reinterpret_cast<const float &>(result);

    return exponentI ? resultFl : resultFl - 0.5f;
  }

  static size_t FromFloat(float input) {
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

  static const Vector4A16 _to_ps(IVector4A16 input) {
    return reinterpret_cast<const __m128 &>(input);
  }

  static const IVector4A16 _to_se(Vector4A16 input) {
    return reinterpret_cast<const __m128i &>(input);
  }

  static IVector4A16 FromVector4(Vector4A16 value) {
    const IVector4A16 cvted = _to_se(value);
    const IVector4A16 shtSign(((cvted & 0x80000000) >> (32 - TOTAL_SIZE)) &
                              SIGN_MASK);

    const IVector4A16 mskExp(cvted & 0x7f800000);
    const IVector4A16 shtMant(cvted >> (23 - _mantissa));

    value += 0.5f;
    const IVector4A16 cvtedDenorm = _to_se(value);
    const IVector4A16 mskExpDenorm(cvtedDenorm & 0x7f800000);

    const IVector4A16 denormMask(
        _mm_cmpeq_epi32(mskExpDenorm._data, IVector4A16(0x3f000000)._data));

    const Vector4A16 blendMant(_mm_blendv_ps(_to_ps(shtMant)._data,
                                             _to_ps(cvtedDenorm)._data,
                                             _to_ps(denormMask)._data));
    const IVector4A16 maskedMant(_to_se(blendMant) & MANTISSA_MASK);

    const IVector4A16 nanMask(
        _mm_cmpeq_epi32(mskExp._data, IVector4A16(0x7f800000)._data));

    const IVector4A16 unRef(((mskExp - EXP_REF) >> (23 - _mantissa)) &
                            EXPONENT_MASK);

    const Vector4A16 blendExp0(_mm_blendv_ps(_to_ps(unRef)._data,
                                             _to_ps(EXPONENT_MASK)._data,
                                             _to_ps(nanMask)._data));

    const Vector4A16 blendExp1(_mm_blendv_ps(blendExp0._data,
                                            Vector4A16()._data,
                                             _to_ps(denormMask)._data));

    return _to_se(blendExp1) | maskedMant | shtSign;
  }

  static Vector4A16 ToVector4(IVector4A16 value) {
    const IVector4A16 exponentI(value & EXPONENT_MASK);

    const IVector4A16 mantissa((value & MANTISSA_MASK) << (23 - _mantissa));
    const IVector4A16 sign((value & SIGN_MASK) << (32 - TOTAL_SIZE));

    const IVector4A16 exponentMask(
        _mm_cmpeq_epi32(exponentI._data, IVector4A16(EXPONENT_MASK)._data));

    const IVector4A16 nzResult(((exponentI << (23 - _mantissa)) + EXP_REF) |
                               mantissa);

    const auto imNANNZ = _mm_blendv_ps(_to_ps(nzResult)._data,
                                       _to_ps(IVector4A16(0x7f800000))._data,
                                       _to_ps(exponentMask)._data);

    const IVector4A16 denormMask(
        _mm_cmpeq_epi32(exponentI._data, IVector4A16()._data));

    const IVector4A16 denormResult0((value & MANTISSA_MASK) | 0x3f000000);
    const Vector4A16 denormResult1(_to_ps(denormResult0) - 0.5f);

    const IVector4A16 finalResult(_to_se(
        _mm_blendv_ps(imNANNZ, denormResult1._data, _to_ps(denormMask)._data)));

    return _to_ps(finalResult | sign);
  }
};

typedef esFloat<10, 5, true> float16;

struct VectorR11G11B10_UFLOAT {
  uint32 data;
  typedef esFloat<11, 5, false> codec_type;

  operator Vector4A16() const {
    const IVector4A16 input0(IVector4A16(data) *
                             IVector4A16(1 << 21, 1 << 10, 1, 0));
    const IVector4A16 input1(input0 >> 21);

    return codec_type::ToVector4(input1);
  }
};
