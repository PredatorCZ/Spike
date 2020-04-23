/*  Storage class for uni::List wrapper
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
#include "list.hpp"
#include "element.hpp"
#include <memory>
#include <vector>

namespace uni {
template <class _interface_type, class _class_type>
class VectorList : public List<_interface_type> {
public:
  typedef _interface_type interface_type;
  typedef _class_type class_type;
  typedef Element<class_type> pointer_class_type;

  size_t Size() const override { return storage.size(); }
  pointer_type At(size_t id) const override {
    return dynamic_cast<pointer_type>(storage.at(id).get());
  }

  typedef VirtualIterator<List, &List::Size, pointer_type, &List::At>
      iterator_type;

  const iterator_type begin() const { return iterator_type(this, 0); }
  const iterator_type end() const { return iterator_type(this); }

  pointer_type operator[](size_t id) { return At(id); }

  std::vector<pointer_class_type> storage;
};
} // namespace uni