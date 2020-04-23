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
#include "datas/Matrix44.hpp"
#include "element.hpp"
#include "list.hpp"

namespace uni {

struct RTSValue {
  Vector4A16 rotation;
  Vector4A16 translation;
  Vector4A16 scale;
};

class MotionCurve {
public:
  enum CurveType_e {
    Position,
    Rotation,
    Scale,
    Matrix,
    PositionRotationScale,
    SingleFloat
  };
  virtual CurveType_e CurveType() const = 0;
  virtual size_t BoneIndex() const = 0;
  virtual void GetValue(RTSValue &output, float time) const = 0;
  virtual void GetValue(esMatrix44 &output, float time) const = 0;
  virtual void GetValue(Vector4A16 &output, float time) const = 0;
  virtual void GetValue(float &output, float time) const = 0;
};

typedef Element<const List<MotionCurve>> MotionCurvesConst;
typedef Element<List<MotionCurve>> MotionCurves;

// Each track can contains multiple curves, one for Rotation, one for Position,
// etc.
class MotionTrack {
public:
  typedef MotionCurves::element_type::iterator_type iterator_type;

  virtual MotionCurvesConst Curves() const = 0;
  virtual size_t Index() const = 0;

  iterator_type begin() const { return Curves()->begin(); }
  iterator_type end() const { return Curves()->end(); }
};

typedef Element<const List<MotionTrack>> MotionTracksConst;
typedef Element<List<MotionTrack>> MotionTracks;

class Motion {
public:
  typedef MotionTracks::element_type::iterator_type iterator_type;
  enum MotionType_e { Absolute, Relative, Additive, Delta };

  virtual std::string Name() const = 0;
  virtual void FrameRate(uint32 fps) = 0;
  virtual uint32 FrameRate() const = 0;
  virtual float Duration() const = 0;
  virtual MotionTracksConst Tracks() const = 0;
  virtual MotionType_e MotionType() const = 0;

  iterator_type begin() const { return Tracks()->begin(); }
  iterator_type end() const { return Tracks()->end(); }
};
} // namespace uni