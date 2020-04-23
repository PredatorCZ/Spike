/*  Hybrid mode copyable unique pointer
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
#include "datas/deleter_hybrid.hpp"
#include <memory>

namespace uni {
template <class C>
class Element : public std::unique_ptr<C, es::deleter_hybrid> {
  typedef std::unique_ptr<C, es::deleter_hybrid> parent;

public:
  using parent::parent;
  using parent::operator=;

  Element(const Element &right) : Element(right.get(), false) {}

  Element &operator=(const Element &right) {
    if (&right != this) {
      Element rCopy(right.get(), false);
      operator=(std::move(rCopy));
    }

    return *this;
  }
};
} // namespace uni
