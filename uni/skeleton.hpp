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
#include "datas/Matrix44.hpp"
#include "element.hpp"
#include "list.hpp"

namespace uni {

class Bone {
public:
  virtual esMatrix44 Transform() const = 0;
  virtual const Bone *Parent() const = 0;
  // A special bone identicator, this is not a bone index within skeleton
  virtual size_t Index() const = 0;
  virtual std::string Name() const = 0;
};

typedef Element<const List<Bone>> BonesConst;
typedef Element<List<Bone>> Bones;

class Skeleton {
public:
  typedef Bones::element_type::iterator_type iterator_type;

  virtual BonesConst Bones() const = 0;
  virtual std::string Name() const = 0;

  iterator_type begin() const { return Bones()->begin(); }
  iterator_type end() const { return Bones()->end(); }
};
} // namespace uni
