#include "gltf.hpp"
#include "datas/aabb.hpp"
#include "uni//model.hpp"
#include "uni/motion.hpp"
#include "uni/rts.hpp"
#include <span>

void GLTF::FinishAndSave(BinWritterRef wr, const std::string & docPath)
{
    size_t totalBufferSize = [&]
    {
        size_t retval = 0;

        for (auto & a : streams)
        {
            a.wr.ApplyPadding();
            retval += a.wr.Tell();
        }

        return retval;
    }();

    if (totalBufferSize)
    {
        size_t curOffset = 0;

        for (auto & a : streams)
        {
            a.buffer = 0;
            a.byteLength = a.wr.Tell();
            a.byteOffset = curOffset;
            curOffset += a.byteLength;
            bufferViews.at(a.index) = std::move(a);
        }

        auto state = gltf::StreamBinaryHeaders(*this, wr.BaseStream(), totalBufferSize);

        for (auto & a : streams)
        {
            char buffer[0x80000];
            const size_t numChunks = a.byteLength / sizeof(buffer);
            const size_t restBytes = a.byteLength % sizeof(buffer);

            for (size_t i = 0; i < numChunks; i++)
            {
                a.str.read(buffer, sizeof(buffer));
                wr.WriteBuffer(buffer, sizeof(buffer));
            }

            if (restBytes)
            {
                a.str.read(buffer, restBytes);
                wr.WriteBuffer(buffer, restBytes);
            }
        }

        gltf::StreamBinaryFinish(*this, state, wr.BaseStream(), docPath);
    }
    else
    {
        std::stringstream str;
        str << "empty";
        gltf::StreamBinaryFull(*this, str, 6, wr.BaseStream(), docPath);
    }
}

size_t GLTFModel::SaveIndices(const uni::IndexArray & idArray)
{
    auto & stream = GetIndexStream();
    auto [acc, index] = NewAccessor(stream, idArray.IndexSize());
    acc.type = gltf::Accessor::Type::Scalar;
    size_t indexCount = idArray.NumIndices();
    auto indicesRaw = idArray.RawIndexBuffer();

    auto Process = [&](auto indices, uint32 reset)
    {
        bool inverted = false;
        stream.wr.Write(indices[0]);
        stream.wr.Write(indices[1]);

        for (size_t i = 2; i < indexCount - 1; i++)
        {
            auto item = indices[i];

            if (item == reset)
            {
                stream.wr.Write(indices[i - 1]);
                if (inverted)
                {
                    stream.wr.Write(indices[i + 1]);
                    acc.count++;
                    inverted = false;
                }
                stream.wr.Write(indices[i + 1]);
                acc.count += 2;
            }
            else
            {
                stream.wr.Write(item);
                acc.count++;
                inverted = !inverted;
            }
        }

        if (indices.back() != reset)
        {
            stream.wr.Write(indices.back());
        }
    };

    if (idArray.IndexSize() == 4)
    {
        std::span<const uint32> indices(reinterpret_cast<const uint32 *>(indicesRaw), indexCount);
        bool as16bit = true;

        for (uint32 i : indices)
        {
            if (i > 0xfffe)
            {
                as16bit = false;
                break;
            }
        }

        acc.componentType = as16bit ? gltf::Accessor::ComponentType::UnsignedShort
                                    : gltf::Accessor::ComponentType::UnsignedInt;

        if (as16bit)
        {
            for (uint16 i : indices)
            {
                stream.wr.Write(i);
            }
        }
        else
        {
            std::span<const uint16> indices(reinterpret_cast<const uint16 *>(indicesRaw), indexCount);
            Process(indices, 0xffffffff);
        }
    }
    else
    {
        std::span<const uint16> indices(reinterpret_cast<const uint16 *>(indicesRaw), indexCount);
        acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
        Process(indices, 0xffff);
    }

    return index;
}

void GLTFModel::WritePositions(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    uni::FormatCodec::fvec basePosition;
    d.Codec().Sample(basePosition, d.RawBuffer(), numVertices, d.Stride());
    if (transform)
    {
        d.Resample(basePosition, *transform);
    }
    else
    {
        d.Resample(basePosition);
    }

    auto aabb = GetAABB(basePosition);
    auto & max = aabb.max;
    auto & min = aabb.min;
    const bool pos16 = max <= 1.f && min >= -1.f;
    auto & stream = pos16 ? GetVt8() : GetVt12();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.type = gltf::Accessor::Type::Vec3;
    acc.max.insert(acc.max.begin(), max._arr, max._arr + 3);
    acc.min.insert(acc.min.begin(), min._arr, min._arr + 3);
    attrs["POSITION"] = index;
    auto vertWr = stream.wr;

    if (pos16)
    {
        acc.normalized = true;

        if (min >= 0.f)
        {
            acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;

            for (auto & v : basePosition)
            {
                v *= 0xffff;
                v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
                vertWr.Write(v.Convert<uint16>());
            }
        }
        else
        {
            acc.componentType = gltf::Accessor::ComponentType::Short;

            for (auto & v : basePosition)
            {
                v *= 0x7fff;
                v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
                vertWr.Write(v.Convert<int16>());
            }
        }
    }
    else
    {
        acc.componentType = gltf::Accessor::ComponentType::Float;

        for (auto & v : basePosition)
        {
            vertWr.Write<Vector>(v);
        }
    }
}

void GLTFModel::WriteTexCoord(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    uni::FormatCodec::fvec sampled;
    d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
    d.Resample(sampled);
    auto aabb = GetAABB(sampled);
    auto & max = aabb.max;
    auto & min = aabb.min;
    const bool uv16 = max <= 1.f && min >= -1.f;
    auto & stream = uv16 ? GetVt4() : GetVt8();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.type = gltf::Accessor::Type::Vec2;
    auto coordName = "TEXCOORD_" + std::to_string(d.Index());
    attrs[coordName] = index;
    auto vertWr = stream.wr;

    if (uv16)
    {
        acc.normalized = true;
        if (min >= 0.f)
        {
            acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;

            for (auto & v : sampled)
            {
                v *= 0xffff;
                v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
                USVector4 comp = v.Convert<uint16>();
                vertWr.Write(USVector2(comp));
            }
        }
        else
        {
            acc.componentType = gltf::Accessor::ComponentType::Short;

            for (auto & v : sampled)
            {
                v *= 0x7fff;
                v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
                SVector4 comp = v.Convert<int16>();
                vertWr.Write(SVector2(comp));
            }
        }
    }
    else
    {
        acc.componentType = gltf::Accessor::ComponentType::Float;

        for (auto & v : sampled)
        {
            vertWr.Write<Vector2>(v);
        }
    }
}

void GLTFModel::WriteVertexColor(fx::gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    auto & stream = GetVt4();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc.normalized = true;
    acc.type = gltf::Accessor::Type::Vec4;
    auto coordName = "COLOR_" + std::to_string(d.Index());
    attrs[coordName] = index;
    uni::FormatCodec::fvec sampled;
    d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());

    for (auto & v : sampled)
    {
        stream.wr.Write((v * 0xff).Convert<uint8>());
    }
}

size_t GLTFModel::WriteNormals8(const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    auto & stream = GetVt4();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.componentType = gltf::Accessor::ComponentType::Byte;
    acc.normalized = true;
    acc.type = gltf::Accessor::Type::Vec3;

    uni::FormatCodec::fvec sampled;
    d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
    d.Resample(sampled);

    for (auto & v : sampled)
    {
        auto pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
        pure.Normalize() *= 0x7f;
        pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
        auto comp = pure.Convert<int8>();
        stream.wr.Write(comp);
    }

    return index;
}

size_t GLTFModel::WriteNormals16(const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    auto & stream = GetVt8();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.componentType = gltf::Accessor::ComponentType::Short;
    acc.normalized = true;
    acc.type = gltf::Accessor::Type::Vec3;

    uni::FormatCodec::fvec sampled;
    d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
    d.Resample(sampled);

    for (auto & v : sampled)
    {
        auto pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
        pure.Normalize() *= 0x7fff;
        pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
        auto comp = pure.Convert<int16>();
        stream.wr.Write(comp);
    }

    return index;
}

namespace gltfutils
{
std::vector<float> MakeSamples(float sampleRate, float duration)
{
    std::vector<float> times;
    float cdur = 0;
    const float fraction = 1.f / sampleRate;

    while (cdur < duration)
    {
        times.push_back(cdur);
        cdur += fraction;
    }

    times.back() = duration;
    return times;
}

StripResult StripValues(const std::vector<float> & times, size_t upperLimit, const uni::MotionTrack * tck)
{
    StripResult retval;
    retval.timeIndices.push_back(0);
    Vector4A16 low, middle, high;
    tck->GetValue(low, times[0]);
    retval.values.push_back(low);

    if (upperLimit == 1)
    {
        return retval;
    }

    tck->GetValue(middle, times[1]);

    for (size_t i = 2; i < upperLimit; i++)
    {
        tck->GetValue(high, times[i]);

        for (size_t p = 0; p < 4; p++)
        {
            if (!fltcmp(low[p], high[p], 0.00001f))
            {
                auto ratio = (low[p] - middle[p]) / (low[p] - high[p]);
                if (!fltcmp(ratio, 0.5f, 0.00001f))
                {
                    retval.timeIndices.push_back(i - 1);
                    retval.values.push_back(middle);
                    break;
                }
            }
        }

        auto tmp = middle;
        middle = high;
        low = tmp;
    }

    if (middle != low)
    {
        retval.timeIndices.push_back(upperLimit - 1);
        retval.values.push_back(middle);
    }

    return retval;
}

class RTSHelper : public uni::MotionTrack
{
public:
    const uni::MotionTrack * base;
    TrackType_e mode;
    TrackType_e TrackType() const override
    {
        return {};
    }
    size_t BoneIndex() const override
    {
        return {};
    }
    void GetValue(Vector4A16 & output, float time) const override
    {
        uni::RTSValue value;
        base->GetValue(value, time);

        switch (mode)
        {
        case Position:
            output = value.translation;
            break;
        case Rotation:
            output = value.rotation;
            break;
        case Scale:
            output = value.scale;
            break;
        default:
            break;
        }
    }
};

std::array<StripResult, 3> GLTF_EXTERN StripValuesBlock(const std::vector<float> & times, size_t upperLimit, const uni::MotionTrack * tck)
{
    std::array<StripResult, 3> retVal;
    RTSHelper helper;
    helper.base = tck;
    helper.mode = uni::MotionTrack::Position;
    retVal[0] = StripValues(times, upperLimit, &helper);
    helper.mode = uni::MotionTrack::Rotation;
    retVal[1] = StripValues(times, upperLimit, &helper);
    helper.mode = uni::MotionTrack::Scale;
    retVal[2] = StripValues(times, upperLimit, &helper);

    return retVal;
}

size_t FindTimeEndIndex(const std::vector<float> & times, float duration)
{
    size_t upperLimit = -1U;

    for (size_t i = 0; i < times.size(); i++)
    {
        if (fltcmp(times[i], duration, 0.0001f))
        {
            upperLimit = i + 1;
            break;
        }
    }

    if (upperLimit == -1U)
    {
        throw std::logic_error("Floating point mismatch");
    }

    return upperLimit;
}

} // namespace gltfutils
