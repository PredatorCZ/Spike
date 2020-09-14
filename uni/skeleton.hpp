/*  Abstraction for skeletons
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

class Bone {
public:
  enum TransformType { TMTYPE_RTS, TMTYPE_MATRIX };

  virtual TransformType TMType() const = 0;
  virtual void GetTM(RTSValue &out) const;
  virtual void GetTM(esMatrix44 &out) const;
  virtual const Bone *Parent() const = 0;
  // A special bone identicator, this is not a bone index within skeleton
  virtual size_t Index() const = 0;
  virtual std::string Name() const = 0;
  virtual ~Bone() {}
};

typedef Element<const List<Bone>> SkeletonBonesConst;
typedef Element<List<Bone>> SkeletonBones;

class Skeleton {
public:
  typedef SkeletonBonesConst::element_type::iterator_type_const
      iterator_type_const;

  virtual SkeletonBonesConst Bones() const = 0;
  virtual std::string Name() const = 0;
  virtual ~Skeleton() {}

  iterator_type_const begin() const { return Bones()->begin(); }
  iterator_type_const end() const { return Bones()->end(); }
};

typedef Element<const List<Skeleton>> SkeletonsConst;
typedef Element<List<Skeleton>> Skeletons;
} // namespace uni

#include "internal/skeleton.inl"
