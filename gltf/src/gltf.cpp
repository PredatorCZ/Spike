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

    size_t inBufferSize = [&]
    {
        size_t retval = 0;

        for (auto & b : buffers)
        {
            retval += b.byteLength;
        }

        return retval;
    }();

    if (totalBufferSize + inBufferSize)
    {
        size_t curOffset = inBufferSize;

        for (auto & a : streams)
        {
            a.buffer = 0;
            a.byteLength = a.wr.Tell();
            a.byteOffset = curOffset;
            curOffset += a.byteLength;
            bufferViews.at(a.index) = std::move(a);
        }

        auto state = gltf::StreamBinaryHeaders(*this, wr.BaseStream(), totalBufferSize + inBufferSize);

        for (auto & a : buffers)
        {
            wr.WriteContainer(a.data);
        }

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

    auto Process = [&, &acc = acc](auto indices, uint32 reset)
    {
        bool inverted = false;
        stream.wr.Write(indices[0]);
        stream.wr.Write(indices[1]);
        acc.count = 2;

        for (size_t i = 2; i < indexCount - 1; i++)
        {
            auto item = indices[i];

            if (item == reset)
            {
                stream.wr.Write(indices[i - 1]);

                while (i < indexCount - 1)
                {
                    if (indices[i + 1] != reset)
                    {
                        break;
                    }

                    i++;
                }

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
            acc.count++;
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
            acc.count = indexCount;

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
    attrs["POSITION"] = index;
    auto vertWr = stream.wr;

    if (pos16)
    {
        acc.normalized = true;

        if (min >= 0.f)
        {
            acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
            min *= 0xffff;
            min = Vector4A16(_mm_round_ps(min._data, _MM_ROUND_NEAREST));
            max *= 0xffff;
            max = Vector4A16(_mm_round_ps(max._data, _MM_ROUND_NEAREST));

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
            min *= 0x7fff;
            min = Vector4A16(_mm_round_ps(min._data, _MM_ROUND_NEAREST));
            max *= 0x7fff;
            max = Vector4A16(_mm_round_ps(max._data, _MM_ROUND_NEAREST));

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

    acc.max.insert(acc.max.begin(), max._arr, max._arr + 3);
    acc.min.insert(acc.min.begin(), min._arr, min._arr + 3);
}

void GLTFModel::WriteTexCoord(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices)
{
    uni::FormatCodec::fvec sampled;

    if (d.Type().outType == uni::FormatType::INT || d.Type().outType == uni::FormatType::UINT)
    {
        uni::FormatCodec::ivec sampled_;
        d.Codec().Sample(sampled_, d.RawBuffer(), numVertices, d.Stride());

        for (auto & s : sampled_)
        {
            sampled.emplace_back(s);
        }
    }
    else
    {
        d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
    }

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

StripResult StripValues(std::span<float> times, size_t upperLimit, const uni::MotionTrack * tck)
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

std::array<StripResult, 3> StripValuesBlock(std::span<float> times, size_t upperLimit, const uni::MotionTrack * tck)
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

size_t FindTimeEndIndex(std::span<float> times, float duration)
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

void BoneInfo::Add(size_t index, Vector4A16 translation, int32 parentIndex)
{
    auto boneLen = translation.Length();
    boneLens.emplace(index, boneLen);

    if (parentIndex < 0 || boneLen < 0.00001f)
        return;

    // Generate octahedron transforms for bone visualization
    // This might be still finicky
    es::Matrix44 lookupMtx;
    lookupMtx.r1() = translation.Normalized();
    lookupMtx.r2() =
        lookupMtx.r1() * es::Matrix44(Vector4A16(0, 0, 0.7007, 0.7007));
    lookupMtx.r3() = lookupMtx.r1().Cross(lookupMtx.r2());

    if (lookupMtx.r3().Length() < 0.00001f)
    {
        lookupMtx.r2() =
            lookupMtx.r1() * es::Matrix44(Vector4A16(0.7007, 0, 0, 0.7007));
        lookupMtx.r3() = lookupMtx.r1().Cross(lookupMtx.r2());
    }

    lookupMtx.r1() *= boneLen;
    lookupMtx.r2() *= boneLen;
    lookupMtx.r3() *= boneLen;

    boneLookupTMs.emplace_back(parentIndex, lookupMtx);
}

void VisualizeSkeleton(GLTF & main, const BoneInfo & infos)
{
    size_t idStreamSlot = 0;
    {
        static const Vector octa[]{
            { 0, 0, 0 },
            { 1, 1, -1 },
            { 1, 1, 1 },
            { 1, -1, -1 },
            { 1, -1, 1 },
            { 10, 0, 0 },
        };
        static const uint16 octaIndices[]{
            0,
            2,
            1,
            0,
            4,
            2,
            0,
            3,
            4,
            0,
            1,
            3,
            5,
            1,
            2,
            5,
            2,
            4,
            5,
            4,
            3,
            5,
            3,
            1,
        };

        auto & vtStream = main.NewStream("visual-vertices", 20);
        vtStream.target = gltf::BufferView::TargetType::ArrayBuffer;
        size_t vtStreamSlot = vtStream.slot;
        size_t numVerts = 6 * infos.boneLookupTMs.size();
        auto [vpAcc, vpId] = main.NewAccessor(vtStream, 4);
        vpAcc.componentType = gltf::Accessor::ComponentType::Float;
        vpAcc.type = gltf::Accessor::Type::Vec3;
        vpAcc.count = numVerts;
        auto [biAcc, biId] = main.NewAccessor(vtStream, 4, 12);
        biAcc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        biAcc.type = gltf::Accessor::Type::Vec4;
        biAcc.count = numVerts;
        auto [bwAcc, bwId] = main.NewAccessor(vtStream, 4, 16, &biAcc);
        bwAcc.normalized = true;

        auto & idStream = main.NewStream("visual-indices");
        idStreamSlot = idStream.slot;
        idStream.target = gltf::BufferView::TargetType::ElementArrayBuffer;
        auto [idAcc, idId] = main.NewAccessor(idStream, 2);
        idAcc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
        idAcc.type = gltf::Accessor::Type::Scalar;
        idAcc.count = 24 * infos.boneLookupTMs.size();

        gltf::Primitive prim;
        prim.mode = gltf::Primitive::Mode::Triangles;
        prim.indices = idId;
        prim.attributes["POSITION"] = vpId;
        prim.attributes["JOINTS_0"] = biId;
        prim.attributes["WEIGHTS_0"] = bwId;

        gltf::Mesh mesh;
        mesh.primitives.emplace_back(std::move(prim));

        gltf::Skin skin;
        std::map<size_t, size_t> remaps;

        for (auto [id, _] : infos.boneLookupTMs)
        {
            if (remaps.contains(id))
            {
                continue;
            }
            skin.joints.push_back(id);
            remaps.emplace(id, remaps.size());
        };

        gltf::Node meshNode;
        meshNode.mesh = main.meshes.size();
        meshNode.skin = main.skins.size();
        main.scenes.back().nodes.push_back(main.nodes.size());
        main.nodes.push_back(meshNode);

        main.meshes.emplace_back(std::move(mesh));
        main.skins.emplace_back(std::move(skin));

        size_t localId = 0;

        auto & vtnStream = main.Stream(vtStreamSlot);

        for (auto [id, tm] : infos.boneLookupTMs)
        {
            for (auto t : octa)
            {
                Vector correctedPos = (t * 0.1f) * tm;
                vtnStream.wr.Write(correctedPos);
                uint8 boneId[]{ uint8(remaps.at(id)), 0, 0, 0 };
                vtnStream.wr.Write(boneId);
                uint8 boneWt[]{ 0xff, 0, 0, 0 };
                vtnStream.wr.Write(boneWt);
            }

            for (auto t : octaIndices)
            {
                uint16 correctedIndex = t + (localId * 6);
                idStream.wr.Write(correctedIndex);
            }
            localId++;
        }
    }
    {

#include "icosphere.hpp"
        auto & idStream = main.Stream(idStreamSlot);

        auto [idAcc, idId] = main.NewAccessor(idStream, 2);
        idAcc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
        idAcc.type = gltf::Accessor::Type::Scalar;
        idAcc.count = 960;
        idStream.wr.Write(icoSphereIndices);

        auto & vtStream = main.NewStream("icovisual-basevertices", 16);
        vtStream.target = gltf::BufferView::TargetType::ArrayBuffer;
        auto [vpAcc, vpId] = main.NewAccessor(vtStream, 4);
        vpAcc.componentType = gltf::Accessor::ComponentType::Float;
        vpAcc.type = gltf::Accessor::Type::Vec3;
        vpAcc.count = 162;
        vpAcc.min = { -1, -1, -1 };
        vpAcc.max = { 1, 1, 1 };
        auto [bwAcc, bwId] = main.NewAccessor(vtStream, 4, 12);
        bwAcc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        bwAcc.type = gltf::Accessor::Type::Vec4;
        bwAcc.count = 162;
        bwAcc.normalized = true;

        for (auto t : icoSphereX3)
        {
            vtStream.wr.Write(t);
            uint8 boneWt[]{ 0xff, 0, 0, 0 };
            vtStream.wr.Write(boneWt);
        }

        gltf::Mesh mesh;
        gltf::Skin skin;
        uint8 localId = 0;

        auto vbStreamSlot = main.NewStream("icovisual-vertices", 4).slot;
        auto & viStream = main.NewStream("icovisual-ibms");
        auto [viAcc, viId] = main.NewAccessor(viStream, 16);
        skin.inverseBindMatrices = viId;
        viAcc.componentType = gltf::Accessor::ComponentType::Float;
        viAcc.type = gltf::Accessor::Type::Mat4;
        viAcc.count = infos.boneLens.size();

        auto & vbStream = main.Stream(vbStreamSlot);
        vbStream.target = gltf::BufferView::TargetType::ArrayBuffer;

        const float avgLen = [&]
        {
            float totalLen = 0;
            int32 totalItems = 0;

            for (auto [id, len] : infos.boneLens)
            {
                if (len < 0.00001)
                {
                    continue;
                }

                totalLen += len;
                totalItems++;
            }

            return totalLen / totalItems;
        }();

        for (auto [id, len] : infos.boneLens)
        {
            if (len < 0.00001)
            {
                len = avgLen;
            }
            es::Matrix44 ibm;
            len *= 0.05f;
            ibm.r1() *= len;
            ibm.r2() *= len;
            ibm.r3() *= len;
            viStream.wr.Write(ibm);
            skin.joints.push_back(id);
            auto [biAcc, biId] = main.NewAccessor(vbStream, 4);
            biAcc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
            biAcc.type = gltf::Accessor::Type::Vec4;
            biAcc.count = 162;

            gltf::Primitive prim;
            prim.mode = gltf::Primitive::Mode::Triangles;
            prim.indices = idId;
            prim.attributes["POSITION"] = vpId;
            prim.attributes["NORMAL"] = vpId;
            prim.attributes["JOINTS_0"] = biId;
            prim.attributes["WEIGHTS_0"] = bwId;

            mesh.primitives.emplace_back(std::move(prim));
            uint8 boneId[]{ localId++, 0, 0, 0 };

            for (size_t i = 0; i < biAcc.count; i++)
            {
                vbStream.wr.Write(boneId);
            }
        }

        gltf::Node meshNode;
        meshNode.mesh = main.meshes.size();
        meshNode.skin = main.skins.size();
        main.scenes.back().nodes.push_back(main.nodes.size());
        main.nodes.push_back(meshNode);

        main.meshes.emplace_back(std::move(mesh));
        main.skins.emplace_back(std::move(skin));
    }
}

} // namespace gltfutils
