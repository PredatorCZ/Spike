/*  Abstraction for motion/animations
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

#pragma once
#include "common.hpp"
#include "datas/matrix44.hpp"
#include "list.hpp"

namespace uni {
class MotionTrack {
public:
  enum TrackType_e {
    Position,
    Rotation,
    Scale,
    Matrix,
    PositionRotationScale,
    SingleFloat
  };

  virtual TrackType_e TrackType() const = 0;
  virtual size_t BoneIndex() const = 0;
  virtual void GetValue(RTSValue &output, float time) const;
  virtual void GetValue(esMatrix44 &output, float time) const;
  virtual void GetValue(Vector4A16 &output, float time) const;
  virtual void GetValue(float &output, float time) const;
  virtual ~MotionTrack() {}
};

typedef Element<const List<MotionTrack>> MotionTracksConst;
typedef Element<List<MotionTrack>> MotionTracks;

class Motion {
public:
  typedef MotionTracks::element_type::iterator_type_const iterator_type_const;
  enum MotionType_e { Absolute, Relative, Additive, Delta };

  virtual std::string Name() const = 0;
  virtual void FrameRate(uint32 fps) const;
  virtual uint32 FrameRate() const = 0;
  virtual float Duration() const = 0;
  virtual MotionTracksConst Tracks() const = 0;
  virtual MotionType_e MotionType() const = 0;
  virtual ~Motion() {}

  iterator_type_const begin() const { return Tracks()->begin(); }
  iterator_type_const end() const { return Tracks()->end(); }
};

typedef Element<const List<Motion>> MotionsConst;
typedef Element<List<Motion>> Motions;

} // namespace uni

#include "internal/motion.inl"
