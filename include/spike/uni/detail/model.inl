/*  uni skeleton internal module
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

namespace uni {
inline void PrimitiveDescriptor::Resample(FormatCodec::fvec &data) const {
  switch (UnpackDataType()) {
  case UnpackDataType_e::None:
    break;

  case UnpackDataType_e::Mul: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d *= udata.min;
    }
    break;
  }

  case UnpackDataType_e::Madd: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d = udata.max + d * udata.min;
    }
    break;
  }

  case UnpackDataType_e::Add: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d += udata.min;
    }
    break;
  }
  }
}

inline void PrimitiveDescriptor::Resample(FormatCodec::fvec &data,
                                   const es::Matrix44 &transform) const {
  switch (UnpackDataType()) {
  case UnpackDataType_e::None:
    for (auto &d : data) {
      d = d * transform;
    }
    break;

  case UnpackDataType_e::Mul: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d = (d * udata.min) * transform;
    }
    break;
  }

  case UnpackDataType_e::Madd: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d = (udata.max + d * udata.min) * transform;
    }
    break;
  }

  case UnpackDataType_e::Add: {
    auto udata = UnpackData();

    for (auto &d : data) {
      d = (d + udata.min) * transform;
    }
    break;
  }
  }
}

} // namespace uni
