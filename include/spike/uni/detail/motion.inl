/*  uni motion internal module
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

#include "../motion.hpp"

namespace uni {
void MotionTrack::GetValue(RTSValue &, float) const {
  throw _uni_::ThrowVoidCall<MotionTrack::TrackType_e,
                             MotionTrack::PositionRotationScale>(
      this->TrackType());
}
void MotionTrack::GetValue(es::Matrix44 &, float) const {
  throw _uni_::ThrowVoidCall<MotionTrack::TrackType_e, MotionTrack::Matrix>(
      this->TrackType());
}
void MotionTrack::GetValue(Vector4A16 &, float) const {
  const auto ttype = this->TrackType();
  throw _uni_::ThrowVoidCall(ttype == MotionTrack::Position ||
                             ttype == MotionTrack::Rotation ||
                             ttype == MotionTrack::Scale);
}
void MotionTrack::GetValue(float &, float) const {
  throw _uni_::ThrowVoidCall<MotionTrack::TrackType_e,
                             MotionTrack::SingleFloat>(this->TrackType());
}

void Motion::FrameRate(uint32) const { throw ModifyError("FrameRate"); }

} // namespace uni
