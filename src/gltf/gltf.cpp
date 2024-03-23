#include "spike/gltf.hpp"
#include "spike/uni//model.hpp"
#include "spike/uni/motion.hpp"
#include "spike/uni/rts.hpp"
#include "spike/util/aabb.hpp"

void GLTF::FinishAndSave(BinWritterRef wr, const std::string &docPath) {
  size_t totalBufferSize = [&] {
    size_t retval = 0;

    for (auto &a : streams) {
      a.byteLength = a.wr.Tell();
      a.wr.ApplyPadding();
      retval += a.wr.Tell();
    }

    return retval;
  }();

  size_t inBufferSize = [&] {
    size_t retval = 0;

    for (auto &b : buffers) {
      retval += b.byteLength;
    }

    return retval;
  }();

  if (totalBufferSize + inBufferSize) {
    size_t curOffset = inBufferSize;

    for (auto &a : streams) {
      a.buffer = 0;
      a.byteOffset = curOffset;
      curOffset += a.byteLength + GetPadding(a.byteLength, 16);
      bufferViews.at(a.index) = std::move(a);
    }

    auto state = gltf::StreamBinaryHeaders(*this, wr.BaseStream(),
                                           totalBufferSize + inBufferSize);

    for (auto &a : buffers) {
      wr.WriteContainer(a.data);
    }

    for (auto &a : streams) {
      char buffer[0x80000];
      const size_t numChunks = a.wr.Tell() / sizeof(buffer);
      const size_t restBytes = a.wr.Tell() % sizeof(buffer);

      for (size_t i = 0; i < numChunks; i++) {
        a.str.read(buffer, sizeof(buffer));
        wr.WriteBuffer(buffer, sizeof(buffer));
      }

      if (restBytes) {
        a.str.read(buffer, restBytes);
        wr.WriteBuffer(buffer, restBytes);
      }
    }

    gltf::StreamBinaryFinish(*this, state, wr.BaseStream(), docPath);
  } else {
    std::stringstream str;
    str << "empty";
    gltf::StreamBinaryFull(*this, str, 6, wr.BaseStream(), docPath);
  }
}

void GLTF::StripBuffers() {
  using BufferView = uint32;

  struct Gap {
    uint32 byteBegin;
    uint32 byteEnd;
  };

  struct AccRange {
    uint32 byteBegin;
    uint32 byteEnd;
    gltf::Accessor *acc;
  };

  std::map<BufferView, std::vector<AccRange>> rangesByView;

  auto NumElements = [](gltf::Accessor &acc) {
    switch (acc.type) {
    case gltf::Accessor::Type::Scalar:
      return 1;
    case gltf::Accessor::Type::Vec2:
      return 2;
    case gltf::Accessor::Type::Vec3:
      return 3;
    case gltf::Accessor::Type::Vec4:
      return 4;
    case gltf::Accessor::Type::Mat2:
      return 2 * 2;
    case gltf::Accessor::Type::Mat3:
      return 3 * 3;
    case gltf::Accessor::Type::Mat4:
      return 4 * 4;
    default:
      break;
    }

    return 0;
  };

  auto ElementSize = [](gltf::Accessor &acc) {
    switch (acc.componentType) {
    case gltf::Accessor::ComponentType::Byte:
    case gltf::Accessor::ComponentType::UnsignedByte:
      return 1;
    case gltf::Accessor::ComponentType::Short:
    case gltf::Accessor::ComponentType::UnsignedShort:
      return 2;
    case gltf::Accessor::ComponentType::Float:
    case gltf::Accessor::ComponentType::UnsignedInt:
      return 4;
    default:
      break;
    }

    return 0;
  };

  auto GetStride = [&](gltf::Accessor &acc) {
    return ElementSize(acc) * NumElements(acc);
  };

  for (auto &acc : accessors) {
    auto &ranges = rangesByView[acc.bufferView];
    const size_t bufferStride = Stream(acc.bufferView).byteStride;
    const uint32 realSize =
        (bufferStride ? bufferStride : GetStride(acc)) * acc.count;
    ranges.emplace_back(AccRange{
        .byteBegin = acc.byteOffset,
        .byteEnd = acc.byteOffset + realSize,
        .acc = &acc,
    });
  }

  for (auto &[bw, ranges] : rangesByView) {
    std::sort(ranges.begin(), ranges.end(), [](AccRange &i, AccRange &j) {
      return i.byteBegin < j.byteBegin;
    });

    std::vector<Gap> gaps;

    if (ranges.front().byteBegin > 0) {
      gaps.emplace_back(Gap{
          .byteBegin = 0,
          .byteEnd = ranges.front().byteBegin,
      });
    }

    for (size_t i = 1; i < ranges.size(); i++) {
      AccRange &prev = ranges.at(i - 1);
      AccRange &cur = ranges.at(i);

      if (prev.byteEnd < cur.byteBegin) {
        gaps.emplace_back(Gap{
            .byteBegin = prev.byteEnd,
            .byteEnd = cur.byteBegin,
        });
      }
    }
    GLTFStream *stream = [&]() -> GLTFStream * {
      for (auto &s : streams) {
        if (s.index == bw) {
          return &s;
        }
      }

      return nullptr;
    }();

    gaps.emplace_back(Gap{
        .byteBegin = ranges.back().byteEnd,
        .byteEnd = uint32(stream->wr.Tell()),
    });

    if (gaps.size() == 1 &&
        (uint32(stream->wr.Tell()) - ranges.back().byteEnd) < 0x2000) {
      continue;
    }

    std::string oldBuffer_(std::move(stream->str).str());
    std::string_view oldBuffer(oldBuffer_);

    uint32 currentOffset = 0;
    uint32 currentAccessor = 0;

    for (auto &m : gaps) {
      while (currentAccessor < ranges.size()) {
        auto &acc = ranges.at(currentAccessor);

        if (acc.byteBegin > m.byteBegin) {
          break;
        }

        acc.acc->byteOffset -= currentOffset;
        acc.acc->byteOffset += stream->str.tellp();

        if (acc.acc->max.size() > 0) {
          const size_t numElems = NumElements(*acc.acc);
          const size_t numItems = acc.acc->count * numElems;

          auto ComputeMinMax = [&](auto data) {
            using DataType = std::remove_pointer_t<decltype(data)>;
            std::span<DataType> dataSpan(data, numItems);
            const DataType MIN = std::numeric_limits<DataType>::lowest();
            const DataType MAX = std::numeric_limits<DataType>::max();
            DataType min[4]{MAX, MAX, MAX, MAX};
            DataType max[4]{MIN, MIN, MIN, MIN};

            for (size_t i = 0; i < numItems; i++) {
              const size_t idx = i % numElems;
              min[idx] = std::min(min[idx], dataSpan[i]);
              max[idx] = std::max(max[idx], dataSpan[i]);
            }

            acc.acc->max.clear();
            acc.acc->min.clear();

            for (size_t i = 0; i < numElems; i++) {
              acc.acc->max.push_back(max[i]);
              acc.acc->min.push_back(min[i]);
            }
          };

          char *bufData = oldBuffer_.data() + acc.byteBegin;

          switch (acc.acc->componentType) {
          case gltf::Accessor::ComponentType::Byte:
            ComputeMinMax(reinterpret_cast<int8 *>(bufData));
            break;
          case gltf::Accessor::ComponentType::UnsignedByte:
            ComputeMinMax(reinterpret_cast<uint8 *>(bufData));
            break;
          case gltf::Accessor::ComponentType::Short:
            ComputeMinMax(reinterpret_cast<int16 *>(bufData));
            break;
          case gltf::Accessor::ComponentType::UnsignedShort:
            ComputeMinMax(reinterpret_cast<uint16 *>(bufData));
            break;
          case gltf::Accessor::ComponentType::Float:
            ComputeMinMax(reinterpret_cast<float *>(bufData));
            break;
          case gltf::Accessor::ComponentType::UnsignedInt:
            ComputeMinMax(reinterpret_cast<uint32 *>(bufData));
            break;
          default:
            break;
          }
        }

        currentAccessor++;
      }

      stream->wr.WriteContainer(
          oldBuffer.substr(currentOffset, m.byteBegin - currentOffset));
      stream->wr.ApplyPadding();

      currentOffset = m.byteEnd;
    }

    /*while (currentAccessor < ranges.size()) {
      auto &acc = ranges.at(currentAccessor++);
      acc.acc->byteOffset -= currentOffset;
      acc.acc->byteOffset += stream->str.tellp();
    }

    stream->wr.WriteContainer(
        oldBuffer.substr(currentOffset, ranges.back().byteEnd -
    currentOffset));*/
  }
}

size_t GLTFModel::SaveIndices(const uni::IndexArray &idArray) {
  return SaveIndices(idArray.RawIndexBuffer(), idArray.NumIndices(),
                     idArray.IndexSize())
      .accessorIndex;
}

SavedIndices GLTFModel::SaveIndices(const void *data, size_t numIndices,
                                    size_t indexSize) {
  SavedIndices retVal{};
  auto &stream = GetIndexStream();
  auto [acc, index] = NewAccessor(stream, indexSize);
  acc.type = gltf::Accessor::Type::Scalar;
  retVal.accessorIndex = index;
  retVal.minIndex = -1;

  auto WriteIndex = [&retVal, &stream](auto value) {
    stream.wr.Write(value);
    retVal.maxIndex = std::max(retVal.maxIndex, uint32(value));
    retVal.minIndex = std::min(retVal.minIndex, uint32(value));
  };

  auto Process = [&, &acc = acc](auto indices, uint32 reset) {
    bool inverted = false;
    WriteIndex(indices[0]);
    WriteIndex(indices[1]);
    acc.count = 2;

    for (size_t i = 2; i < numIndices - 1; i++) {
      auto item = indices[i];

      if (item == reset) {
        WriteIndex(indices[i - 1]);

        while (i < numIndices - 1) {
          if (indices[i + 1] != reset) {
            break;
          }

          i++;
        }

        if (inverted) {
          WriteIndex(indices[i + 1]);
          acc.count++;
          inverted = false;
        }
        WriteIndex(indices[i + 1]);
        acc.count += 2;
      } else {
        WriteIndex(item);
        acc.count++;
        inverted = !inverted;
      }
    }

    if (indices.back() != reset) {
      WriteIndex(indices.back());
      acc.count++;
    }
  };

  if (indexSize == 4) {
    std::span<const uint32> indices(static_cast<const uint32 *>(data),
                                    numIndices);
    bool as16bit = true;

    for (uint32 i : indices) {
      if (i > 0xfffe) {
        as16bit = false;
        break;
      }
    }

    acc.componentType = as16bit ? gltf::Accessor::ComponentType::UnsignedShort
                                : gltf::Accessor::ComponentType::UnsignedInt;

    if (as16bit) {
      acc.count = numIndices;

      for (uint16 i : indices) {
        WriteIndex(i);
      }
    } else {
      std::span<const uint16> indices(static_cast<const uint16 *>(data),
                                      numIndices);
      Process(indices, 0xffffffff);
    }
  } else {
    std::span<const uint16> indices(static_cast<const uint16 *>(data),
                                    numIndices);
    acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
    Process(indices, 0xffff);
  }

  return retVal;
}

size_t WritePositions16s(GLTFModel &main,
                         uni::FormatCodec::fvec &basePosition) {
  auto &stream = main.GetVt8();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = basePosition.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::Short;
  auto vertWr = stream.wr;
  Vector4A16 min(FLT_MAX);
  Vector4A16 max(-FLT_MAX);

  for (auto &v : basePosition) {
    v *= 0x7fff;
    v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
    min = Vector4A16(_mm_min_ps(min._data, v._data));
    max = Vector4A16(_mm_max_ps(max._data, v._data));
    vertWr.Write(v.Convert<int16>());
  }

  acc.max.insert(acc.max.begin(), max._arr, max._arr + 3);
  acc.min.insert(acc.min.begin(), min._arr, min._arr + 3);

  return index;
}

size_t WritePositions16u(GLTFModel &main,
                         uni::FormatCodec::fvec &basePosition) {
  auto &stream = main.GetVt8();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = basePosition.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
  auto vertWr = stream.wr;
  Vector4A16 min(FLT_MAX);
  Vector4A16 max(-FLT_MAX);

  for (auto &v : basePosition) {
    v *= 0x7fff;
    v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
    min = Vector4A16(_mm_min_ps(min._data, v._data));
    max = Vector4A16(_mm_max_ps(max._data, v._data));
    vertWr.Write(v.Convert<int16>());
  }

  acc.max.insert(acc.max.begin(), max._arr, max._arr + 3);
  acc.min.insert(acc.min.begin(), min._arr, min._arr + 3);

  return index;
}

size_t WritePositions32(GLTFModel &main, uni::FormatCodec::fvec &basePosition) {
  auto &stream = main.GetVt12();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = basePosition.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  auto vertWr = stream.wr;
  Vector4A16 min(FLT_MAX);
  Vector4A16 max(-FLT_MAX);

  for (auto &v : basePosition) {
    min = Vector4A16(_mm_min_ps(min._data, v._data));
    max = Vector4A16(_mm_max_ps(max._data, v._data));
    vertWr.Write<Vector>(v);
  }

  acc.max.insert(acc.max.begin(), max._arr, max._arr + 3);
  acc.min.insert(acc.min.begin(), min._arr, min._arr + 3);

  return index;
}

size_t WriteNormals32(GLTFModel &main, uni::FormatCodec::fvec &normals) {
  auto &stream = main.GetVt12();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = normals.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.componentType = gltf::Accessor::ComponentType::Float;

  for (auto &v : normals) {
    Vector4A16 pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
    pure.Normalize();
    stream.wr.Write<Vector>(pure);
  }

  return index;
}

size_t WriteTangents32(GLTFModel &main, uni::FormatCodec::fvec &tangents) {
  auto &stream = main.GetVt12();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = tangents.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.componentType = gltf::Accessor::ComponentType::Float;

  for (auto &v : tangents) {
    Vector4A16 pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
    pure.Normalize();
    pure.w = -1 + 2 * (v.w > 0);
    stream.wr.Write(pure);
  }

  return index;
}

size_t WriteNormals16(GLTFModel &main, uni::FormatCodec::fvec &normals) {
  auto &stream = main.GetVt8();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = normals.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::Short;

  for (auto &v : normals) {
    auto pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
    pure.Normalize() *= 0x7fff;
    pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
    auto comp = pure.Convert<int16>();
    comp.w = 0x7fff + (v.w < 0);
    stream.wr.Write(comp);
  }

  return index;
}

size_t WriteNormals8(GLTFModel &main, uni::FormatCodec::fvec &normals) {
  auto &stream = main.GetVt4();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = normals.size();
  acc.type = gltf::Accessor::Type::Vec3;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::Byte;

  for (auto &v : normals) {
    auto pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
    pure.Normalize() *= 0x7f;
    pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
    auto comp = pure.Convert<int8>();
    comp.w = 0x7f + (v.w < 0);
    stream.wr.Write(comp);
  }

  return index;
}

size_t WriteTexcoord16s(GLTFModel &main, uni::FormatCodec::fvec &coords) {
  auto &stream = main.GetVt4();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = coords.size();
  acc.type = gltf::Accessor::Type::Vec2;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::Short;
  auto vertWr = stream.wr;

  for (auto &v : coords) {
    v *= 0x7fff;
    v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
    SVector4 comp = v.Convert<int16>();
    vertWr.Write(SVector2(comp));
  }

  return index;
}

size_t WriteTexcoord16u(GLTFModel &main, uni::FormatCodec::fvec &coords) {
  auto &stream = main.GetVt4();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = coords.size();
  acc.type = gltf::Accessor::Type::Vec2;
  acc.normalized = true;
  acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
  auto vertWr = stream.wr;

  for (auto &v : coords) {
    v *= 0xffff;
    v = Vector4A16(_mm_round_ps(v._data, _MM_ROUND_NEAREST));
    USVector4 comp = v.Convert<uint16>();
    vertWr.Write(USVector2(comp));
  }

  return index;
}

size_t WriteTexcoord32(GLTFModel &main, uni::FormatCodec::fvec &coords) {
  auto &stream = main.GetVt8();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = coords.size();
  acc.type = gltf::Accessor::Type::Vec2;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  auto vertWr = stream.wr;

  for (auto &v : coords) {
    vertWr.Write<Vector2>(v);
  }

  return index;
}

size_t WriteColor(GLTFModel &main, uni::FormatCodec::fvec &color) {
  auto &stream = main.GetVt4();
  auto [acc, index] = main.NewAccessor(stream, 4);
  acc.count = color.size();
  acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
  acc.normalized = true;
  acc.type = gltf::Accessor::Type::Vec4;

  for (auto &v : color) {
    stream.wr.Write((v * 0xff).Convert<uint8>());
  }

  return index;
}

uni::FormatCodec::fvec SampleAttribute(const char *data, size_t numVertices,
                                       Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec retVal;
  if (attribute.customCodec && attribute.customCodec->CanSample()) {
    retVal.resize(numVertices);
    attribute.customCodec->Sample(retVal, data, stride);
  } else {
    auto &codec = uni::FormatCodec::Get({attribute.format, attribute.type});
    codec.Sample(retVal, data, numVertices, stride);
  }

  if (attribute.customCodec && attribute.customCodec->CanTransform()) {
    attribute.customCodec->Transform(retVal);
  }

  return retVal;
}

size_t SavePositions(GLTFModel &main, const char *data, size_t numVertices,
                     Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec basePosition =
      SampleAttribute(data, numVertices, attribute, stride);

  if (!main.quantizeMesh ||
      (attribute.customCodec && !attribute.customCodec->IsNormalized())) {
    return WritePositions32(main, basePosition);
  }

  switch (attribute.format) {
  case uni::FormatType::NORM:
    return WritePositions16s(main, basePosition);
  case uni::FormatType::UNORM:
    return WritePositions16u(main, basePosition);
  default:
    return WritePositions32(main, basePosition);
  }
}

size_t SaveNormals(GLTFModel &main, const char *data, size_t numVertices,
                   Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec normals =
      SampleAttribute(data, numVertices, attribute, stride);

  if (!main.quantizeMesh) {
    return WriteNormals32(main, normals);
  }

  switch (attribute.type) {
  case uni::DataType::R8G8B8:
  case uni::DataType::R8G8B8A8:
  case uni::DataType::R5G6B5:
    return WriteNormals8(main, normals);
  default:
    return WriteNormals16(main, normals);
  }
}

size_t SaveTangents(GLTFModel &main, const char *data, size_t numVertices,
                    Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec tangents =
      SampleAttribute(data, numVertices, attribute, stride);

  if (!main.quantizeMesh) {
    return WriteTangents32(main, tangents);
  }

  switch (attribute.type) {
  case uni::DataType::R8G8B8A8:
    return WriteNormals8(main, tangents);
  default:
    return WriteNormals16(main, tangents);
  }
}

size_t SaveTexcoords(GLTFModel &main, const char *data, size_t numVertices,
                     Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec coords =
      SampleAttribute(data, numVertices, attribute, stride);

  if (!main.quantizeMesh ||
      (attribute.customCodec && !attribute.customCodec->IsNormalized())) {
    return WriteTexcoord32(main, coords);
  }

  switch (attribute.format) {
  case uni::FormatType::NORM:
    return WriteTexcoord16s(main, coords);
  case uni::FormatType::UNORM:
    return WriteTexcoord16u(main, coords);
  default:
    return WriteTexcoord32(main, coords);
  }
}

size_t SaveColor(GLTFModel &main, const char *data, size_t numVertices,
                 Attribute attribute, size_t stride) {
  uni::FormatCodec::fvec color =
      SampleAttribute(data, numVertices, attribute, stride);

  return WriteColor(main, color);
}

static constexpr size_t fmtNumElements[]{
    0, 4, 3, 4, 2, 3, 1, 2, 4, 3, 4, 2, 3, 2, 1, 3, 4, 1,
};

struct BWBuffer {
  uint8 data[8];
};

void SaveWeights(const char *data, size_t numVertices, Attribute attribute,
                 size_t stride, std::vector<BWBuffer> &buffer,
                 uint8 &usedBufferElements) {
  const size_t numElements = fmtNumElements[uint32(attribute.type)];

  if (numElements + usedBufferElements > 8) {
    throw std::runtime_error("Too many bone weights for vertex, max is 8");
  }

  uni::FormatCodec::fvec weights =
      SampleAttribute(data, numVertices, attribute, stride);

  for (size_t index = 0; auto w : weights) {
    UCVector4 wt((w * 0xff).Convert<uint8>());
    memcpy(buffer.at(index++).data + usedBufferElements, &wt, numElements);
  }

  usedBufferElements += numElements;
}

void SaveBones(const char *data, size_t numVertices, Attribute attribute,
               size_t stride, std::vector<BWBuffer> &buffer,
               uint8 &usedBufferElements) {
  const size_t numElements = fmtNumElements[uint32(attribute.type)];

  if (numElements + usedBufferElements > 8) {
    throw std::runtime_error("Too many bone weights for vertex, max is 8");
  }

  if (attribute.format == uni::FormatType::UINT) {
    uni::FormatCodec::ivec bones;
    auto &codec = uni::FormatCodec::Get({attribute.format, attribute.type});
    codec.Sample(bones, data, numVertices, stride);

    for (size_t index = 0; auto b : bones) {
      UCVector4 bn(b.Convert<uint8>());
      memcpy(buffer.at(index++).data + usedBufferElements, &bn, numElements);
    }
  } else {
    uni::FormatCodec::fvec bones =
        SampleAttribute(data, numVertices, attribute, stride);

    for (size_t index = 0; auto b : bones) {
      UCVector4 bn(b.Convert<uint8>());
      memcpy(buffer.at(index++).data + usedBufferElements, &bn, numElements);
    }
  }

  usedBufferElements += numElements;
}

void GLTFModel::WritePositions(gltf::Attributes &attrs,
                               const uni::PrimitiveDescriptor &d,
                               size_t numVertices) {
  uni::FormatCodec::fvec basePosition;
  d.Codec().Sample(basePosition, d.RawBuffer(), numVertices, d.Stride());
  if (transform) {
    d.Resample(basePosition, *transform);
  } else {
    d.Resample(basePosition);
  }

  if (!quantizeMesh) {
    attrs["POSITION"] = WritePositions32(*this, basePosition);
    return;
  }

  auto aabb = GetAABB(basePosition);
  auto &max = aabb.max;
  auto &min = aabb.min;
  const bool pos16 = max <= 1.f && min >= -1.f;

  attrs["POSITION"] = [&] {
    if (pos16) {
      if (min >= 0.f) {
        return WritePositions16u(*this, basePosition);
      } else {
        return WritePositions16s(*this, basePosition);
      }
    }
    return WritePositions32(*this, basePosition);
  }();
}

void GLTFModel::WriteTexCoord(gltf::Attributes &attrs,
                              const uni::PrimitiveDescriptor &d,
                              size_t numVertices) {
  uni::FormatCodec::fvec sampled;
  d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
  d.Resample(sampled);

  const std::string coordName = "TEXCOORD_" + std::to_string(d.Index());

  if (!quantizeMesh) {
    attrs[coordName] = WriteTexcoord32(*this, sampled);
    return;
  }

  auto aabb = GetAABB(sampled);
  auto &max = aabb.max;
  auto &min = aabb.min;
  const bool uv16 = max <= 1.f && min >= -1.f;

  attrs[coordName] = [&] {
    if (uv16) {
      if (min >= 0.f) {
        return WriteTexcoord16u(*this, sampled);
      } else {
        return WriteTexcoord16s(*this, sampled);
      }
    }
    return WriteTexcoord32(*this, sampled);
  }();
}

void GLTFModel::WriteVertexColor(fx::gltf::Attributes &attrs,
                                 const uni::PrimitiveDescriptor &d,
                                 size_t numVertices) {
  uni::FormatCodec::fvec sampled;
  d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
  auto coordName = "COLOR_" + std::to_string(d.Index());
  attrs[coordName] = WriteColor(*this, sampled);
}

size_t GLTFModel::WriteNormals8(const uni::PrimitiveDescriptor &d,
                                size_t numVertices) {
  uni::FormatCodec::fvec sampled;
  d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
  if (transform) {
    d.Resample(sampled, *transform);
  } else {
    d.Resample(sampled);
  }
  return ::WriteNormals8(*this, sampled);
}

size_t GLTFModel::WriteNormals16(const uni::PrimitiveDescriptor &d,
                                 size_t numVertices) {
  uni::FormatCodec::fvec sampled;
  d.Codec().Sample(sampled, d.RawBuffer(), numVertices, d.Stride());
  if (transform) {
    d.Resample(sampled, *transform);
  } else {
    d.Resample(sampled);
  }

  return ::WriteNormals16(*this, sampled);
}

gltf::Attributes GLTFModel::SaveVertices(const void *data, size_t numVertices,
                                         std::span<const Attribute> attributes,
                                         size_t stride) {
  static constexpr int fmtStrides[]{0,  128, 96, 64, 64, 48, 32, 32, 32,
                                    32, 32,  32, 24, 16, 16, 16, 16, 8};
  int8 currentOffset = 0;
  uint32 coordIndex = 0;
  uint32 colorIndex = 0;
  gltf::Attributes attrs;
  std::vector<BWBuffer> bonesBuffer;
  std::vector<BWBuffer> weightsBuffer;
  uint8 boneElement = 0;
  uint8 weightElement = 0;

  for (auto a : attributes) {
    if (a.offset > -1) {
      currentOffset = a.offset;
    }

    switch (a.usage) {
    case AttributeType::Undefined:
      break;
    case AttributeType::Position:
      attrs["POSITION"] =
          SaveVertices(static_cast<const char *>(data) + currentOffset,
                       numVertices, a, stride);
      break;
    case AttributeType::Normal:
      attrs["NORMAL"] =
          SaveVertices(static_cast<const char *>(data) + currentOffset,
                       numVertices, a, stride);
      break;
    case AttributeType::Tangent:
      attrs["TANGENT"] =
          SaveVertices(static_cast<const char *>(data) + currentOffset,
                       numVertices, a, stride);
      break;
    case AttributeType::TextureCoordiante: {
      const std::string coordName = "TEXCOORD_" + std::to_string(coordIndex++);
      attrs[coordName] =
          SaveVertices(static_cast<const char *>(data) + currentOffset,
                       numVertices, a, stride);
      break;
    }
    case AttributeType::VertexColor: {
      const std::string coordName = "COLOR_" + std::to_string(colorIndex++);
      attrs[coordName] =
          SaveVertices(static_cast<const char *>(data) + currentOffset,
                       numVertices, a, stride);
      break;
    }

    case AttributeType::BoneIndices:
      if (bonesBuffer.empty()) {
        bonesBuffer.resize(numVertices);
      }

      SaveBones(static_cast<const char *>(data) + currentOffset, numVertices, a,
                stride, bonesBuffer, boneElement);
      break;

    case AttributeType::BoneWeights:
      if (weightsBuffer.empty()) {
        weightsBuffer.resize(numVertices);
      }

      SaveWeights(static_cast<const char *>(data) + currentOffset, numVertices,
                  a, stride, weightsBuffer, weightElement);
      break;
    }

    currentOffset += fmtStrides[uint32(a.type)] / 8;
  }

  if (bonesBuffer.empty()) {
    return attrs;
  }

  if (weightElement != boneElement) {
    for (auto &bw : weightsBuffer) {
      int32 restWeight = bw.data[0] + bw.data[1] + bw.data[2] + bw.data[3] +
                         bw.data[4] + bw.data[5] + bw.data[6];
      if (restWeight < 0xff) {
        bw.data[weightElement] = 0xff - restWeight;
      }
    }
  }

  if (boneElement > 4) {
    auto &stream = GetVt8();
    auto [acc0, index0] = NewAccessor(stream, 4);
    acc0.count = numVertices;
    acc0.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc0.type = gltf::Accessor::Type::Vec4;

    auto [acc1, index1] = NewAccessor(stream, 4, 4, &acc0);
    stream.wr.WriteContainer(bonesBuffer);

    attrs["JOINTS_0"] = index0;
    attrs["JOINTS_1"] = index1;

    auto [acc2, index2] = NewAccessor(stream, 4);
    acc2.count = numVertices;
    acc2.normalized = true;
    acc2.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc2.type = gltf::Accessor::Type::Vec4;

    auto [acc3, index3] = NewAccessor(stream, 4, 4, &acc2);
    stream.wr.WriteContainer(weightsBuffer);

    attrs["WEIGHTS_0"] = index2;
    attrs["WEIGHTS_1"] = index3;
  } else {
    auto &stream = GetVt4();
    auto [acc, index] = NewAccessor(stream, 4);
    acc.count = numVertices;
    acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc.type = gltf::Accessor::Type::Vec4;

    for (auto &b : bonesBuffer) {
      stream.wr.WriteBuffer(reinterpret_cast<const char *>(b.data), 4);
    }

    attrs["JOINTS_0"] = index;

    auto [acc1, index1] = NewAccessor(stream, 4);
    acc1.count = numVertices;
    acc1.normalized = true;
    acc1.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc1.type = gltf::Accessor::Type::Vec4;

    for (auto &b : weightsBuffer) {
      stream.wr.WriteBuffer(reinterpret_cast<const char *>(b.data), 4);
    }

    attrs["WEIGHTS_0"] = index1;
  }

  return attrs;
}

size_t GLTFModel::SaveVertices(const void *data, size_t numVertices,
                               Attribute attribute, size_t stride) {
  switch (attribute.usage) {
  case AttributeType::Position:
    return SavePositions(*this, static_cast<const char *>(data), numVertices,
                         attribute, stride);
  case AttributeType::Normal:
    return SaveNormals(*this, static_cast<const char *>(data), numVertices,
                       attribute, stride);
  case AttributeType::Tangent:
    return SaveTangents(*this, static_cast<const char *>(data), numVertices,
                        attribute, stride);
  case AttributeType::TextureCoordiante:
    return SaveTexcoords(*this, static_cast<const char *>(data), numVertices,
                         attribute, stride);
  case AttributeType::VertexColor:
    return SaveColor(*this, static_cast<const char *>(data), numVertices,
                     attribute, stride);
  default:
    break;
  }

  return 0;
}

namespace gltfutils {
std::vector<float> MakeSamples(float sampleRate, float duration) {
  std::vector<float> times;
  float cdur = 0;
  const float fraction = 1.f / sampleRate;

  while (cdur < duration) {
    times.push_back(cdur);
    cdur += fraction;
  }

  times.back() = duration;
  return times;
}

StripResult StripValues(std::span<float> times, size_t upperLimit,
                        const uni::MotionTrack *tck) {
  StripResult retval;
  retval.timeIndices.push_back(0);
  Vector4A16 low, middle, high;
  tck->GetValue(low, times[0]);
  retval.values.push_back(low);

  if (upperLimit == 1) {
    return retval;
  }

  tck->GetValue(middle, times[1]);

  for (size_t i = 2; i < upperLimit; i++) {
    tck->GetValue(high, times[i]);

    for (size_t p = 0; p < 4; p++) {
      if (!fltcmp(low[p], high[p], 0.00001f)) {
        auto ratio = (low[p] - middle[p]) / (low[p] - high[p]);
        if (!fltcmp(ratio, 0.5f, 0.00001f)) {
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

  if (middle != low) {
    retval.timeIndices.push_back(upperLimit - 1);
    retval.values.push_back(middle);
  }

  return retval;
}

class RTSHelper : public uni::MotionTrack {
public:
  const uni::MotionTrack *base;
  TrackType_e mode;
  TrackType_e TrackType() const override { return {}; }
  size_t BoneIndex() const override { return {}; }
  void GetValue(Vector4A16 &output, float time) const override {
    uni::RTSValue value;
    base->GetValue(value, time);

    switch (mode) {
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

std::array<StripResult, 3> StripValuesBlock(std::span<float> times,
                                            size_t upperLimit,
                                            const uni::MotionTrack *tck) {
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

size_t FindTimeEndIndex(std::span<float> times, float duration) {
  size_t upperLimit = -1U;

  for (size_t i = 0; i < times.size(); i++) {
    if (fltcmp(times[i], duration, 0.0001f)) {
      upperLimit = i + 1;
      break;
    }
  }

  if (upperLimit == -1U) {
    throw std::logic_error("Floating point mismatch");
  }

  return upperLimit;
}

void BoneInfo::Add(size_t index, Vector4A16 translation, int32 parentIndex) {
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

  if (lookupMtx.r3().Length() < 0.00001f) {
    lookupMtx.r2() =
        lookupMtx.r1() * es::Matrix44(Vector4A16(0.7007, 0, 0, 0.7007));
    lookupMtx.r3() = lookupMtx.r1().Cross(lookupMtx.r2());
  }

  lookupMtx.r1() *= boneLen;
  lookupMtx.r2() *= boneLen;
  lookupMtx.r3() *= boneLen;

  boneLookupTMs.emplace_back(parentIndex, lookupMtx);
}

void VisualizeSkeleton(GLTF &main, const BoneInfo &infos) {
  size_t idStreamSlot = 0;
  {
    static const Vector octa[]{
        {0, 0, 0}, {1, 1, -1}, {1, 1, 1}, {1, -1, -1}, {1, -1, 1}, {10, 0, 0},
    };
    static const uint16 octaIndices[]{
        0, 2, 1, 0, 4, 2, 0, 3, 4, 0, 1, 3, 5, 1, 2, 5, 2, 4, 5, 4, 3, 5, 3, 1,
    };

    auto &vtStream = main.NewStream("visual-vertices", 20);
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

    auto &idStream = main.NewStream("visual-indices");
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

    for (auto [id, _] : infos.boneLookupTMs) {
      if (remaps.contains(id)) {
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

    auto &vtnStream = main.Stream(vtStreamSlot);

    for (auto [id, tm] : infos.boneLookupTMs) {
      for (auto t : octa) {
        Vector correctedPos = (t * 0.1f) * tm;
        vtnStream.wr.Write(correctedPos);
        uint8 boneId[]{uint8(remaps.at(id)), 0, 0, 0};
        vtnStream.wr.Write(boneId);
        uint8 boneWt[]{0xff, 0, 0, 0};
        vtnStream.wr.Write(boneWt);
      }

      for (auto t : octaIndices) {
        uint16 correctedIndex = t + (localId * 6);
        idStream.wr.Write(correctedIndex);
      }
      localId++;
    }
  }
  {

#include "icosphere.hpp"
    auto &idStream = main.Stream(idStreamSlot);

    auto [idAcc, idId] = main.NewAccessor(idStream, 2);
    idAcc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
    idAcc.type = gltf::Accessor::Type::Scalar;
    idAcc.count = 960;
    idStream.wr.Write(icoSphereIndices);

    auto &vtStream = main.NewStream("icovisual-basevertices", 16);
    vtStream.target = gltf::BufferView::TargetType::ArrayBuffer;
    auto [vpAcc, vpId] = main.NewAccessor(vtStream, 4);
    vpAcc.componentType = gltf::Accessor::ComponentType::Float;
    vpAcc.type = gltf::Accessor::Type::Vec3;
    vpAcc.count = 162;
    vpAcc.min = {-1, -1, -1};
    vpAcc.max = {1, 1, 1};
    auto [bwAcc, bwId] = main.NewAccessor(vtStream, 4, 12);
    bwAcc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    bwAcc.type = gltf::Accessor::Type::Vec4;
    bwAcc.count = 162;
    bwAcc.normalized = true;

    for (auto t : icoSphereX3) {
      vtStream.wr.Write(t);
      uint8 boneWt[]{0xff, 0, 0, 0};
      vtStream.wr.Write(boneWt);
    }

    gltf::Mesh mesh;
    gltf::Skin skin;
    uint8 localId = 0;

    auto vbStreamSlot = main.NewStream("icovisual-vertices", 4).slot;
    auto &viStream = main.NewStream("icovisual-ibms");
    auto [viAcc, viId] = main.NewAccessor(viStream, 16);
    skin.inverseBindMatrices = viId;
    viAcc.componentType = gltf::Accessor::ComponentType::Float;
    viAcc.type = gltf::Accessor::Type::Mat4;
    viAcc.count = infos.boneLens.size();

    auto &vbStream = main.Stream(vbStreamSlot);
    vbStream.target = gltf::BufferView::TargetType::ArrayBuffer;

    const float avgLen = [&] {
      float totalLen = 0;
      int32 totalItems = 0;

      for (auto [id, len] : infos.boneLens) {
        if (len < 0.00001) {
          continue;
        }

        totalLen += len;
        totalItems++;
      }

      return totalLen / totalItems;
    }();

    for (auto [id, len] : infos.boneLens) {
      if (len < 0.00001) {
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
      uint8 boneId[]{localId++, 0, 0, 0};

      for (size_t i = 0; i < biAcc.count; i++) {
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
