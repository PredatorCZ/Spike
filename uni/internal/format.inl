/*  uni format internal module
    part of uni module
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

#include "datas/float.hpp"

namespace _uni_ {
static constexpr size_t fmtStrides[]{0,  128, 96, 64, 64, 48, 32, 32, 32,
                                     32, 32,  32, 24, 16, 16, 16, 16, 8};
static constexpr size_t fmtNumElements[]{
    0, 4, 3, 4, 2, 3, 1, 2, 4, 3, 4, 2, 3, 2, 1, 3, 4, 1,
};

static constexpr size_t BM(size_t ID0, size_t ID1 = 0, size_t ID2 = 0,
                           size_t ID3 = 0) {
  return ID0 | (ID1 << 8) | (ID2 << 16) | ID3 << 24;
}

static constexpr size_t fmtBitmasks[]{
    BM(0),
    BM(32, 32, 32, 32),
    BM(32, 32, 32),
    BM(16, 16, 16, 16),
    BM(32, 32),
    BM(16, 16, 16),
    BM(32),
    BM(16, 16),
    BM(10, 10, 10, 2),
    BM(11, 11, 10),
    BM(8, 8, 8, 8),
    BM(24, 8),
    BM(8, 8, 8),
    BM(16),
    BM(5, 6, 5),
    BM(5, 5, 5, 1),
    BM(8),
};

} // namespace _uni_

namespace uni {

/***************************************/
/* UINT ********************************/
/***************************************/
template <>
class FormatCodec_t<FormatType::UINT, DataType::R32G32B32A32>
    : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    return IVector4A16(*reinterpret_cast<const IVector4 *>(input));
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R32G32B32>
    : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R32G32> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R32> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, 0, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R16G16B16A16>
    : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector4 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, inputRC.W);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R16G16B16>
    : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R16G16> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector2 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R16> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    return IVector4A16(inputRC, 0, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R8G8B8A8> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector4 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, inputRC.W);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R8G8B8> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R8G8> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector2 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R8> : public FormatCodec {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint8 *>(input);
    return IVector4A16(inputRC, 0, 0, 0);
  }
  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R10G10B10A2>
    : public FormatCodec {
  static constexpr size_t MASK0 = (1 << 10) - 1;
  static constexpr size_t MASK1 = 3;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint32 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK0, MASK0, MASK0, MASK1);

    return IVector4A16(inputRC, inputRC >> 10, inputRC >> 20, inputRC >> 30) &
           mask;
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R11G11B10>
    : public FormatCodec {
  static constexpr size_t MASK0 = (1 << 10) - 1;
  static constexpr size_t MASK1 = (1 << 11) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint32 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK1, MASK1, MASK0, 0);

    return IVector4A16(inputRC, inputRC >> 11, inputRC >> 22, 0) & mask;
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R24G8> : public FormatCodec {
  static constexpr size_t MASK0 = (1 << 24) - 1;
  static constexpr size_t MASK1 = (1 << 8) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint32 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK0, MASK1, 0, 0);

    return IVector4A16(inputRC, inputRC >> 24, 0, 0) & mask;
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R5G6B5> : public FormatCodec {
  static constexpr size_t MASK0 = (1 << 5) - 1;
  static constexpr size_t MASK1 = (1 << 6) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK0, MASK1, MASK0, 0);

    return IVector4A16(inputRC, inputRC >> 5, inputRC >> 11, 0) & mask;
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UINT, DataType::R5G5B5A1> : public FormatCodec {
  static constexpr size_t MASK0 = (1 << 5) - 1;
  static constexpr size_t MASK1 = 1;

public:
  static IVector4A16 GetValue(const char *input) {
    static const IVector4A16 mask(MASK0, MASK0, MASK0, MASK1);
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    const IVector4A16 mskInput(inputRC);

    return IVector4A16(inputRC, inputRC >> 5, inputRC >> 10, inputRC >> 15) &
           mask;
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

/***************************************/
/* INT *********************************/
/***************************************/

template <DataType cType>
class FormatCodec_t<FormatType::INT, cType>
    : public FormatCodec_t<FormatType::UINT, cType> {

  static const Vector4A16 _to_ps(IVector4A16 input) {
    return reinterpret_cast<const __m128 &>(input);
  }

  static const IVector4A16 _to_se(Vector4A16 input) {
    return reinterpret_cast<const __m128i &>(input);
  }

  void GetValue(IVector4A16 &out, const char *input) const override {
    using namespace _uni_;
    static const size_t nType = static_cast<size_t>(cType);
    static const IVector4A16 sBit(
        1 << ((fmtBitmasks[nType] & 0xff) - 1),
        1 << (((fmtBitmasks[nType] >> 8) & 0xff) - 1),
        1 << (((fmtBitmasks[nType] >> 16) & 0xff) - 1),
        1 << (((fmtBitmasks[nType] >> 24) & 0xff) - 1));
    static const IVector4A16 sMask((((sBit - 1) << 1) | 1) ^ 0xffffffff);

    out = FormatCodec_t<FormatType::UINT, cType>::GetValue(input);
    const auto negated = out | sMask;

    const auto cmpRes = _mm_cmpeq_epi32(out._data, sBit._data);
    out = _to_se(_mm_blendv_ps(_to_ps(out)._data, _to_ps(negated)._data,
                        _to_ps(cmpRes)._data));
  }
};

/***************************************/
/* UNORM  ******************************/
/***************************************/

template <>
class FormatCodec_t<FormatType::UNORM, DataType::R32G32B32A32>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32B32A32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input) >> 1) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

template <>
class FormatCodec_t<FormatType::UNORM, DataType::R32G32B32>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32B32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input) >> 1) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R32G32> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input) >> 1) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R32> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static uint32 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint32 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const float p0 = static_cast<float>(GetValueNoFrac(input) >> 1);
    return Vector4A16(p0) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<16>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint64 rtVal = reinterpret_cast<const uint64 &>(*input);
    const UIVector4A16 vctr(_mm_set1_epi64x(rtVal)); // XY, ZW, XY, ZW
    return (vctr * UIVector4A16(1 << 16, 1, 1 << 16, 1)) >> 16; // X, Y, Z, W
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R16G16B16>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<16>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint64 rtVal = reinterpret_cast<const uint64 &>(*input);
    UIVector4A16 vctr(_mm_set1_epi64x(rtVal)); // XY, ZW, XY, ZW
    return (vctr * UIVector4A16(1 << 16, 1, 1 << 16, 0)) >> 16; // X, Y, Z, W
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R16G16> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<16>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 16, 1, 0, 0)) >> 16;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R16> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<16>::VALUE;

public:
  static uint16 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint16 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const float result = static_cast<float>(GetValueNoFrac(input)) * rtFrac;
    return Vector4A16(result, 0, 0, 0);
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R8G8B8A8>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<8>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 24, 1 << 16, 1 << 8, 1)) >> 24;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R8G8B8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<8>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 24, 1 << 16, 1 << 8, 0)) >> 24;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R8G8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<8>::VALUE;

public:
  static IVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    IVector4A16 vctr(rtVal);
    return (vctr * IVector4A16(1 << 8, 1, 0, 0)) >> 8;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<8>::VALUE;

public:
  static uint8 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint8 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const float result = static_cast<float>(GetValueNoFrac(input)) * rtFrac;
    return Vector4A16(result, 0, 0, 0);
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R10G10B10A2>
    : public FormatCodec {
  static constexpr size_t FRAC0 = GetFraction<10>::VALUE;
  static constexpr size_t FRAC1 = GetFraction<2>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    vctr *= UIVector4A16(1 << 22, 1 << 12, 1 << 2, 0);

    return _mm_insert_epi32((vctr >> 22)._data, rtVal >> 30, 3);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac0 = reinterpret_cast<const float &>(FRAC0);
    static const float rtFrac1 = reinterpret_cast<const float &>(FRAC1);
    static const Vector4A16 frac(rtFrac0, rtFrac0, rtFrac0, rtFrac1);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R11G11B10>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<11>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 21, 1 << 10, 1, 0)) >> 21;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R24G8> : public FormatCodec {
  static constexpr size_t FRAC0 = GetFraction<24>::VALUE;
  static constexpr size_t MASK0 = (1 << 24) - 1;
  static constexpr size_t FRAC1 = GetFraction<8>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    return UIVector4A16(rtVal & MASK0, rtVal >> 24, 0, 0);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac0 = reinterpret_cast<const float &>(FRAC0);
    static const float rtFrac1 = reinterpret_cast<const float &>(FRAC1);
    static const Vector4A16 frac(rtFrac0, rtFrac1, 0, 0);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R5G5B5A1>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<5>::VALUE;

public:
  static IVector4A16 GetValueNoFrac(const char *input) {
    const uint16 rtVal = reinterpret_cast<const uint16 &>(*input);
    IVector4A16 vctr(rtVal);
    vctr *= IVector4A16(1 << 11, 1 << 6, 1 << 1, 1 << 8);
    const IVector4A16 cmpMask =
        _mm_cmpeq_epi32(vctr._data, IVector4A16(1 << 23)._data);
    vctr |= cmpMask >> 16;

    return vctr >> 11;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::UNORM, DataType::R5G6B5> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<6>::VALUE;

public:
  static IVector4A16 GetValueNoFrac(const char *input) {
    const uint16 rtVal = reinterpret_cast<const uint16 &>(*input);
    IVector4A16 vctr(rtVal);
    vctr *= IVector4A16(1 << 11, 1 << 5, 1, 0);

    return vctr >> 10;
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

/***************************************/
/* NORM  *******************************/
/***************************************/

template <>
class FormatCodec_t<FormatType::NORM, DataType::R32G32B32A32>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32B32A32> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(parent::GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R32G32B32>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32B32> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(parent::GetValueNoFrac(input)) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R32G32> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    return Vector4A16(parent::GetValueNoFrac(input) >> 1) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R32> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<31>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const float p0 =
        static_cast<float>(static_cast<int32>(parent::GetValueNoFrac(input)));
    return Vector4A16(p0) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R16G16B16A16>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<15>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R16G16B16>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<15>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R16G16> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<15>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R16> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<15>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const int32 result0 = static_cast<int16>(parent::GetValueNoFrac(input));
    const float result = static_cast<float>(result0) * rtFrac;
    return Vector4A16(result, 0, 0, 0);
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R8G8B8A8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<7>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8B8A8> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R8G8B8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<7>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8B8> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R8G8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<7>::VALUE;

  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R8> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<7>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const int32 result0 = static_cast<int8>(parent::GetValueNoFrac(input));
    const float result = static_cast<float>(result0) * rtFrac;
    return Vector4A16(result, 0, 0, 0);
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R10G10B10A2>
    : public FormatCodec {
  static constexpr size_t FRAC0 = GetFraction<9>::VALUE;
  static constexpr size_t FRAC1 = GetFraction<1>::VALUE;

public:
  static IVector4A16 GetValueNoFrac(const char *input) {
    const int32 rtVal = reinterpret_cast<const int32 &>(*input);
    IVector4A16 vctr(rtVal);
    vctr *= IVector4A16(1 << 22, 1 << 12, 1 << 2, 0);

    return _mm_insert_epi32((vctr >> 22)._data, rtVal >> 30, 3);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac0 = reinterpret_cast<const float &>(FRAC0);
    static const float rtFrac1 = reinterpret_cast<const float &>(FRAC1);
    static const Vector4A16 frac(rtFrac0, rtFrac0, rtFrac0, rtFrac1);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

template <>
class FormatCodec_t<FormatType::NORM, DataType::R11G11B10>
    : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<10>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 21) >> 21) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R24G8> : public FormatCodec {
  static constexpr size_t FRAC0 = GetFraction<23>::VALUE;
  static constexpr size_t FRAC1 = GetFraction<7>::VALUE;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const int32 rtVal0 = reinterpret_cast<const int32 &>(*input) << 8;
    const int8 rtVal1 =
        static_cast<int8>(reinterpret_cast<const int32 &>(*input) >> 24);

    return UIVector4A16(rtVal0 >> 8, rtVal1, 0, 0);
  }

  static Vector4A16 GetValue(const char *input) {
    static const float rtFrac0 = reinterpret_cast<const float &>(FRAC0);
    static const float rtFrac1 = reinterpret_cast<const float &>(FRAC1);
    static const Vector4A16 frac(rtFrac0, rtFrac1, 0, 0);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R5G5B5A1> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<4>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R5G5B5A1> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 27) >> 27) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::NORM, DataType::R5G6B5> : public FormatCodec {
  static constexpr size_t FRAC = GetFraction<5>::VALUE;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R5G6B5> parent;
    static const float rtFrac = reinterpret_cast<const float &>(FRAC);
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 26) >> 26) * rtFrac;
  }

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

/***************************************/
/* FLOAT  ******************************/
/***************************************/

template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R32G32B32A32>
    : public FormatCodec {
public:
  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(*reinterpret_cast<const Vector4 *>(input));
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R32G32B32>
    : public FormatCodec {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const Vector *>(input);
    return Vector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R32G32> : public FormatCodec {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const Vector2 *>(input);
    return Vector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R32> : public FormatCodec {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const float *>(input);
    return Vector4A16(inputRC, 0, 0, 0);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R16G16B16A16>
    : public FormatCodec {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R16G16B16>
    : public FormatCodec {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R16G16> : public FormatCodec {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R16> : public FormatCodec {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    return codec_type::ToFloat(inputRC);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

template <>
class FormatCodec_t<FormatType::FLOAT, DataType::R11G11B10>
    : public FormatCodec {
  typedef esFloat<5, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};

template <>
class FormatCodec_t<FormatType::UFLOAT, DataType::R11G11B10>
    : public FormatCodec {
  typedef esFloat<6, 5, false> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
  void GetValue(Vector4A16 &out, const char *input) const override {
    out = GetValue(input);
  }
};
} // namespace uni
