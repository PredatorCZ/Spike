#include "gltf.hpp"
#include "uni/motion.hpp"
#include "uni/rts.hpp"

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
