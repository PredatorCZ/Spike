/*  uni format internal module
    part of uni module
    Copyright 2020-2024 Lukas Cone

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

#include "../format.hpp"
#include "spike/except.hpp"
#include "spike/type/float.hpp"
#include "spike/util/supercore.hpp"

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
    BM(8, 8),
    BM(16),
    BM(5, 6, 5),
    BM(5, 5, 5, 1),
    BM(8),
};

template <class codec, uni::DataType cType, class C>
static void _fmtSampler(C &out, const char *input, size_t count,
                        size_t stride) {
  static const size_t fmtStride = fmtStrides[static_cast<size_t>(cType)] / 8;
  if (stride && stride < fmtStride) {
    throw es::RuntimeError(
        "Provided stride for uni::format was less than format's stride!");
  }

  if (count) {
    out.resize(count);
  }

  if (!stride) {
    stride = fmtStride;
  }

  for (auto &a : out) {
    a = codec::GetValue(input);
    input += stride;
  }
}

} // namespace _uni_

namespace uni {
template <FormatType, DataType> class FormatCodec_t {};

/***************************************/
/* UINT ********************************/
/***************************************/
template <> class FormatCodec_t<FormatType::UINT, DataType::R32G32B32A32> {
public:
  static IVector4A16 GetValue(const char *input) {
    return IVector4A16(*reinterpret_cast<const IVector4 *>(input));
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R32G32B32> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R32G32> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R32> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const IVector *>(input);
    return IVector4A16(inputRC.X, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R16G16B16A16> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector4 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, inputRC.W);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R16G16B16> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R16G16> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const USVector2 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R16> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    return IVector4A16(inputRC, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R8G8B8A8> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector4 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, inputRC.W);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R8G8B8> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R8G8> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const UCVector2 *>(input);
    return IVector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R8> {
public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint8 *>(input);
    return IVector4A16(inputRC, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R10G10B10A2> {
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
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R11G11B10> {
  static constexpr size_t MASK0 = (1 << 10) - 1;
  static constexpr size_t MASK1 = (1 << 11) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint32 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK1, MASK1, MASK0, 0);

    return IVector4A16(inputRC, inputRC >> 11, inputRC >> 22, 0) & mask;
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R24G8> {
  static constexpr size_t MASK0 = (1 << 24) - 1;
  static constexpr size_t MASK1 = (1 << 8) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint32 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK0, MASK1, 0, 0);

    return IVector4A16(inputRC, inputRC >> 24, 0, 0) & mask;
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R5G6B5> {
  static constexpr size_t MASK0 = (1 << 5) - 1;
  static constexpr size_t MASK1 = (1 << 6) - 1;

public:
  static IVector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    const IVector4A16 mskInput(inputRC);
    static const IVector4A16 mask(MASK0, MASK1, MASK0, 0);

    return IVector4A16(inputRC, inputRC >> 5, inputRC >> 11, 0) & mask;
  }
};
template <> class FormatCodec_t<FormatType::UINT, DataType::R5G5B5A1> {
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

public:
  static IVector4A16 GetValue(const char *input) {
    using namespace _uni_;
    static constexpr size_t nType = static_cast<size_t>(cType);
    static constexpr size_t sBit00 = fmtBitmasks[nType] & 0xff;
    static constexpr size_t sBit10 = (fmtBitmasks[nType] >> 8) & 0xff;
    static constexpr size_t sBit20 = (fmtBitmasks[nType] >> 16) & 0xff;
    static constexpr size_t sBit30 = (fmtBitmasks[nType] >> 24) & 0xff;
    static constexpr size_t sBit01 = sBit00 ? 1 << (sBit00 - 1) : 0;
    static constexpr size_t sBit11 = sBit10 ? 1 << (sBit10 - 1) : 0;
    static constexpr size_t sBit21 = sBit20 ? 1 << (sBit20 - 1) : 0;
    static constexpr size_t sBit31 = sBit30 ? 1 << (sBit30 - 1) : 0;
    static const IVector4A16 sBit(sBit01, sBit11, sBit21, sBit31);
    static const IVector4A16 sMaskLow(sBit - 1);
    static const IVector4A16 sMask((((sMaskLow) << 1) | 1) ^ 0xffffffff);

    IVector4A16 out = FormatCodec_t<FormatType::UINT, cType>::GetValue(input);
    const auto negated = out | sMask;

    const auto cmpRes = _mm_cmpgt_epi32(out._data, sMaskLow._data);
    return _to_se(_mm_blendv_ps(_to_ps(out)._data, _to_ps(negated)._data,
                                _to_ps(cmpRes)._data));
  }
};

template <>
class FormatCodec_t<FormatType::INT, DataType::R32G32B32A32>
    : public FormatCodec_t<FormatType::UINT, DataType::R32G32B32A32> {};
template <>
class FormatCodec_t<FormatType::INT, DataType::R32G32B32>
    : public FormatCodec_t<FormatType::UINT, DataType::R32G32B32> {};
template <>
class FormatCodec_t<FormatType::INT, DataType::R32G32>
    : public FormatCodec_t<FormatType::UINT, DataType::R32G32> {};
template <>
class FormatCodec_t<FormatType::INT, DataType::R32>
    : public FormatCodec_t<FormatType::UINT, DataType::R32> {};

/***************************************/
/* UNORM  ******************************/
/***************************************/

template <> class FormatCodec_t<FormatType::UNORM, DataType::R32G32B32A32> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32B32A32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input) >> 1) * GetFraction(31);
  }
};

template <> class FormatCodec_t<FormatType::UNORM, DataType::R32G32B32> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32B32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input) >> 1) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R32G32> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    typedef FormatCodec_t<FormatType::UINT, DataType::R32G32> parent;
    return parent::GetValue(input);
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input) >> 1) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R32> {
public:
  static uint32 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint32 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    const float p0 = static_cast<float>(GetValueNoFrac(input) >> 1);
    return Vector4A16(p0, 0, 0, 0) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint64 rtVal = reinterpret_cast<const uint64 &>(*input);
    const UIVector4A16 vctr(_mm_set1_epi64x(rtVal)); // XY, ZW, XY, ZW
    const UIVector4A16 result((vctr * UIVector4A16(1 << 16, 1 << 16, 1, 1)) >>
                              16); // X, Z, Y, W
    return _mm_shuffle_epi32(result._data, _MM_SHUFFLE(3, 1, 2, 0));
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(16);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R16G16B16> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint64 rtVal = reinterpret_cast<const uint64 &>(*input);
    UIVector4A16 vctr(_mm_set1_epi64x(rtVal)); // XY, ZW, XY, ZW
    const UIVector4A16 result((vctr * UIVector4A16(1 << 16, 1 << 16, 1, 0)) >>
                              16); // X, Z, Y, W
    return _mm_shuffle_epi32(result._data, _MM_SHUFFLE(3, 1, 2, 0));
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(16);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R16G16> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 16, 1, 0, 0)) >> 16;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(16);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R16> {
public:
  static uint16 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint16 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    const float result =
        static_cast<float>(GetValueNoFrac(input)) * GetFraction(16);
    return Vector4A16(result, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R8G8B8A8> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 24, 1 << 16, 1 << 8, 1)) >> 24;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(8);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R8G8B8> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 24, 1 << 16, 1 << 8, 0)) >> 24;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(8);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R8G8> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint16 rtVal = reinterpret_cast<const uint16 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 24, 1 << 16, 0, 0)) >> 24;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(8);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R8> {
public:
  static uint8 GetValueNoFrac(const char *input) {
    return reinterpret_cast<const uint8 &>(*input);
  }

  static Vector4A16 GetValue(const char *input) {
    const float result = static_cast<float>(GetValueNoFrac(input));
    return Vector4A16(result, 0, 0, 0) * GetFraction(8);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R10G10B10A2> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    vctr *= UIVector4A16(1 << 22, 1 << 12, 1 << 2, 0);

    return _mm_insert_epi32((vctr >> 22)._data, rtVal >> 30, 3);
  }

  static Vector4A16 GetValue(const char *input) {
    static const Vector4A16 frac(GetFraction(10), GetFraction(10),
                                 GetFraction(10), GetFraction(2));

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    UIVector4A16 vctr(rtVal);
    return (vctr * UIVector4A16(1 << 21, 1 << 10, 1, 0)) >> 21;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(11);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R24G8> {
  static constexpr size_t MASK0 = (1 << 24) - 1;

public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint32 rtVal = reinterpret_cast<const uint32 &>(*input);
    return UIVector4A16(rtVal & MASK0, rtVal >> 24, 0, 0);
  }

  static Vector4A16 GetValue(const char *input) {
    static const Vector4A16 frac(GetFraction(24), GetFraction(8), 0, 0);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R5G5B5A1> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint16 rtVal = reinterpret_cast<const uint16 &>(*input);
    UIVector4A16 vctr(rtVal);
    vctr *= UIVector4A16(1 << 27, 1 << 22, 1 << 17, 1);
    const UIVector4A16 cmpMask = _mm_cmpeq_epi32(
        UIVector4A16(1 << 15)._data, (vctr & UIVector4A16(1 << 15))._data);
    vctr |= cmpMask;

    return vctr >> 27;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(5);
  }
};
template <> class FormatCodec_t<FormatType::UNORM, DataType::R5G6B5> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const uint16 rtVal = reinterpret_cast<const uint16 &>(*input);
    UIVector4A16 vctr(rtVal);
    vctr *= UIVector4A16(1 << 27, 1 << 21, 1 << 16, 0);

    return vctr >> 26;
  }

  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(GetValueNoFrac(input)) * GetFraction(6);
  }
};

/***************************************/
/* NORM  *******************************/
/***************************************/

template <> class FormatCodec_t<FormatType::NORM, DataType::R32G32B32A32> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32B32A32> parent;
    return Vector4A16(parent::GetValueNoFrac(input)) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R32G32B32> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32B32> parent;
    return Vector4A16(parent::GetValueNoFrac(input)) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R32G32> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32G32> parent;
    return Vector4A16(parent::GetValueNoFrac(input)) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R32> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R32> parent;
    const float p0 =
        static_cast<float>(static_cast<int32>(parent::GetValueNoFrac(input)));
    return Vector4A16(p0, 0, 0, 0) * GetFraction(31);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R16G16B16A16> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * GetFraction(15);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R16G16B16> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * GetFraction(15);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R16G16> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 16) >> 16) * GetFraction(15);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R16> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16> parent;
    const int32 result0 = static_cast<int16>(parent::GetValueNoFrac(input));
    const float result = static_cast<float>(result0) * GetFraction(15);
    return Vector4A16(result, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R8G8B8A8> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8B8A8> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * GetFraction(7);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R8G8B8> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8B8> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * GetFraction(7);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R8G8> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8G8> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));

    return Vector4A16((result << 24) >> 24) * GetFraction(7);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R8> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R8> parent;
    const int32 result0 = static_cast<int8>(parent::GetValueNoFrac(input));
    const float result = static_cast<float>(result0) * GetFraction(7);
    return Vector4A16(result, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R10G10B10A2> {
public:
  static IVector4A16 GetValueNoFrac(const char *input) {
    const int32 rtVal = reinterpret_cast<const int32 &>(*input);
    IVector4A16 vctr(rtVal);
    vctr *= IVector4A16(1 << 22, 1 << 12, 1 << 2, 0);

    return _mm_insert_epi32((vctr >> 22)._data, rtVal >> 30, 3);
  }

  static Vector4A16 GetValue(const char *input) {
    static const Vector4A16 frac(GetFraction(9), GetFraction(9), GetFraction(9),
                                 GetFraction(1));
    const Vector4A16 result(GetValueNoFrac(input));

    return result * frac;
  }
};

template <> class FormatCodec_t<FormatType::NORM, DataType::R11G11B10> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 21) >> 21) * GetFraction(10);
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R24G8> {
public:
  static UIVector4A16 GetValueNoFrac(const char *input) {
    const int32 rtVal0 = reinterpret_cast<const int32 &>(*input) << 8;
    const int8 rtVal1 =
        static_cast<int8>(reinterpret_cast<const int32 &>(*input) >> 24);

    return UIVector4A16(rtVal0 >> 8, rtVal1, 0, 0);
  }

  static Vector4A16 GetValue(const char *input) {
    static const Vector4A16 frac(GetFraction(23), GetFraction(7), 0, 0);

    return Vector4A16(GetValueNoFrac(input)) * frac;
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R5G5B5A1> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R5G5B5A1> parent;
    static const Vector4A16 frac(GetFraction(4), GetFraction(4), GetFraction(4),
                                 1.f);
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 27) >> 27) * frac;
  }
};
template <> class FormatCodec_t<FormatType::NORM, DataType::R5G6B5> {
public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R5G6B5> parent;
    const IVector4A16 result(parent::GetValueNoFrac(input));
    return Vector4A16((result << 26) >> 26) * GetFraction(5);
  }
};

/***************************************/
/* FLOAT  ******************************/
/***************************************/

template <> class FormatCodec_t<FormatType::FLOAT, DataType::R32G32B32A32> {
public:
  static Vector4A16 GetValue(const char *input) {
    return Vector4A16(*reinterpret_cast<const Vector4 *>(input));
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R32G32B32> {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const ::Vector *>(input);
    return Vector4A16(inputRC.X, inputRC.Y, inputRC.Z, 0);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R32G32> {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const Vector2 *>(input);
    return Vector4A16(inputRC.X, inputRC.Y, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R32> {
public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const float *>(input);
    return Vector4A16(inputRC, 0, 0, 0);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R16G16B16A16> {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16A16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R16G16B16> {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16B16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R16G16> {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R16G16> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
};
template <> class FormatCodec_t<FormatType::FLOAT, DataType::R16> {
  typedef esFloat<10, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    const auto inputRC = *reinterpret_cast<const uint16 *>(input);
    return Vector4A16(codec_type::ToFloat(inputRC), 0, 0, 0);
  }
};

template <> class FormatCodec_t<FormatType::FLOAT, DataType::R11G11B10> {
  typedef esFloat<5, 5, true> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
};

template <> class FormatCodec_t<FormatType::UFLOAT, DataType::R11G11B10> {
  typedef esFloat<6, 5, false> codec_type;

public:
  static Vector4A16 GetValue(const char *input) {
    typedef FormatCodec_t<FormatType::UNORM, DataType::R11G11B10> parent;
    const UIVector4A16 result(parent::GetValueNoFrac(input));

    return codec_type::ToVector4(result);
  }
};

template <FormatType fType, DataType cType>
class _FormatCodecImpl_t : public FormatCodec {
public:
  typedef FormatCodec_t<fType, cType> codec;

  void GetValue(Vector4A16 &out, const char *input) const override {
    out = codec::GetValue(input);
  }

  void Sample(fvec &out, const char *input, size_t count,
              size_t stride) const override {
    _uni_::_fmtSampler<codec, cType>(out, input, count, stride);
  }
};

template <DataType cType>
class _FormatCodecImpl_t<FormatType::INT, cType> : public FormatCodec {
public:
  typedef FormatCodec_t<FormatType::INT, cType> codec;

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = codec::GetValue(input);
  }

  void Sample(ivec &out, const char *input, size_t count,
              size_t stride) const override {
    _uni_::_fmtSampler<codec, cType>(out, input, count, stride);
  }
};

template <DataType cType>
class _FormatCodecImpl_t<FormatType::UINT, cType> : public FormatCodec {
public:
  typedef FormatCodec_t<FormatType::UINT, cType> codec;

  void GetValue(IVector4A16 &out, const char *input) const override {
    out = codec::GetValue(input);
  }

  void Sample(ivec &out, const char *input, size_t count,
              size_t stride) const override {
    _uni_::_fmtSampler<codec, cType>(out, input, count, stride);
  }
};

} // namespace uni
