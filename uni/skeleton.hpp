/*  Abstraction for skeletons
    part of uni module
    Copyright 2020-2021 Lukas Cone

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
#include "list.hpp"
#include <string>

namespace es {
class Matrix44;
}

namespace uni {
struct RTSValue;

class PC_EXTERN Bone : public Base {
public:
  virtual TransformType TMType() const = 0;
  virtual void GetTM(RTSValue &out) const;
  virtual void GetTM(es::Matrix44 &out) const;
  virtual const Bone *Parent() const = 0;
  // A special bone identifier, this is not a bone index within skeleton
  virtual size_t Index() const = 0;
  virtual std::string Name() const = 0;
};

using SkeletonBonesConst = Element<const List<Bone>>;
using SkeletonBones = Element<List<Bone>>;

class Skeleton : public Base {
public:
  using iterator_type_const =
      SkeletonBonesConst::element_type::iterator_type_const;

  virtual SkeletonBonesConst Bones() const = 0;
  virtual std::string Name() const = 0;

  iterator_type_const begin() const { return Bones()->begin(); }
  iterator_type_const end() const { return Bones()->end(); }
};

using SkeletonsConst = Element<const List<Skeleton>>;
using Skeletons = Element<List<Skeleton>>;
} // namespace uni
