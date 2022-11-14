/*  GLTF stream wrapper

    Copyright 2022 Lukas Cone

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
#include "datas/binwritter_stream.hpp"
#include "datas/vectors_simd.hpp"
#include "gltf.h"
#include <datas/matrix44.hpp>
#include <optional>
#include <sstream>

using namespace fx;

namespace uni
{
class MotionTrack;
class IndexArray;
class PrimitiveDescriptor;
} // namespace uni

struct GLTFStream : gltf::BufferView
{
    std::stringstream str;
    BinWritterRef wr{ str };
    size_t slot;
    size_t index;
    GLTFStream() = delete;
    GLTFStream(const GLTFStream &) = delete;
    GLTFStream(GLTFStream && o)
        : gltf::BufferView{ std::move(static_cast<gltf::BufferView &>(o)) }, str{ std::move(o.str) }, wr{ str }, slot(o.slot), index(o.index) {}
    GLTFStream & operator=(GLTFStream &&) = delete;
    GLTFStream & operator=(const GLTFStream &) = delete;
    GLTFStream(size_t slot_)
        : slot(slot_) {}
    GLTFStream(size_t slot_, size_t stride)
        : slot(slot_)
    {
        byteStride = stride;
    }
};

struct GLTF : gltf::Document
{
    GLTF()
    {
        scenes.emplace_back();
    }

    GLTFStream & NewStream(const std::string & name, size_t stride = 0)
    {
        auto & stream = streams.emplace_back(streams.size(), stride);
        stream.name = name;
        stream.index = bufferViews.size();
        bufferViews.emplace_back();
        return stream;
    }

    GLTFStream & Stream(size_t at)
    {
        return streams.at(at);
    }

    GLTFStream & LastStream()
    {
        return streams.back();
    }

    size_t NumStreams() const
    {
        return streams.size();
    }

    auto NewAccessor(GLTFStream & where, size_t alignment, size_t strideOffset = 0, const gltf::Accessor * copyFrom = nullptr)
    {
        if (copyFrom)
        {
            accessors.emplace_back(*copyFrom);
        }
        else
        {
            accessors.emplace_back();
        }
        auto & acc = accessors.back();
        acc.bufferView = where.index;
        where.wr.ApplyPadding(alignment);
        acc.byteOffset = where.wr.Tell() + strideOffset;
        return std::make_pair(std::ref(acc), accessors.size() - 1);
    }

    void GLTF_EXTERN FinishAndSave(BinWritterRef wr, const std::string & docPath);

private:
    std::vector<GLTFStream> streams;
};

struct GLTFModel : GLTF
{
    std::optional<es::Matrix44> transform;
    size_t GLTF_EXTERN SaveIndices(const uni::IndexArray & idArray);
    void GLTF_EXTERN WritePositions(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices);
    void GLTF_EXTERN WriteTexCoord(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices);
    void GLTF_EXTERN WriteVertexColor(gltf::Attributes & attrs, const uni::PrimitiveDescriptor & d, size_t numVertices);
    size_t GLTF_EXTERN WriteNormals8(const uni::PrimitiveDescriptor & d, size_t numVertices);
    size_t GLTF_EXTERN WriteNormals16(const uni::PrimitiveDescriptor & d, size_t numVertices);

    GLTFStream & SkinStream()
    {
        if (ibmStream < 0)
        {
            auto & newStream = NewStream("ibms");
            ibmStream = newStream.slot;
            return newStream;
        }
        return Stream(ibmStream);
    }

    GLTFStream & GetIndexStream()
    {
        if (indexStream < 0)
        {
            auto & str = NewStream("indices");
            str.target = gltf::BufferView::TargetType::ElementArrayBuffer;
            indexStream = str.slot;
            return str;
        }

        return Stream(indexStream);
    }

    GLTFStream & GetVt12()
    {
        if (vt12Stream < 0)
        {
            auto & str = NewStream("vtStride12", 12);
            str.target = gltf::BufferView::TargetType::ArrayBuffer;
            vt12Stream = str.slot;
            return str;
        }

        return Stream(vt12Stream);
    }

    GLTFStream & GetVt8()
    {
        if (vt8Stream < 0)
        {
            auto & str = NewStream("vtStride8", 8);
            str.target = gltf::BufferView::TargetType::ArrayBuffer;
            vt8Stream = str.slot;
            return str;
        }

        return Stream(vt8Stream);
    }

    GLTFStream & GetVt4()
    {
        if (vt4Stream < 0)
        {
            auto & str = NewStream("vtStride4", 4);
            str.target = gltf::BufferView::TargetType::ArrayBuffer;
            vt4Stream = str.slot;
            return str;
        }

        return Stream(vt4Stream);
    }

private:
    int32 ibmStream = -1;
    int32 indexStream = -1;
    int32 vt12Stream = -1;
    int32 vt8Stream = -1;
    int32 vt4Stream = -1;
};

namespace gltfutils
{
std::vector<float> GLTF_EXTERN MakeSamples(float sampleRate, float duration);

struct StripResult
{
    std::vector<uint16> timeIndices;
    std::vector<Vector4A16> values;
};

StripResult GLTF_EXTERN StripValues(const std::vector<float> & times, size_t upperLimit, const uni::MotionTrack * tck);
std::array<StripResult, 3> GLTF_EXTERN StripValuesBlock(const std::vector<float> & times, size_t upperLimit, const uni::MotionTrack * tck);
size_t GLTF_EXTERN FindTimeEndIndex(const std::vector<float> & times, float duration);

inline bool fltcmp(float f0, float f1, float epsilon = FLT_EPSILON)
{
    return (f1 <= f0 + epsilon) && (f1 >= f0 - epsilon);
}
} // namespace gltfutils
