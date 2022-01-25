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
#include "gltf.h"
#include <sstream>

using namespace fx;

struct GLTFStream : gltf::BufferView
{
    std::stringstream str;
    BinWritterRef wr{ str };
    size_t slot;
    GLTFStream() = delete;
    GLTFStream(const GLTFStream &) = delete;
    GLTFStream(GLTFStream && o)
        : gltf::BufferView{ std::move(static_cast<gltf::BufferView &>(o)) }, str{ std::move(o.str) }, wr{ str }, slot(o.slot) {}
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
        acc.bufferView = where.slot;
        where.wr.ApplyPadding(alignment);
        acc.byteOffset = where.wr.Tell() + strideOffset;
        return std::make_pair(std::ref(acc), accessors.size() - 1);
    }

    void FinishAndSave(BinWritterRef wr, const std::string & docPath)
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
                bufferViews.emplace_back(std::move(a));
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

private:
    std::vector<GLTFStream> streams;
};
