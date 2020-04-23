/*  List abstraction
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
#include "virtual_iterator.hpp"

namespace uni {
template <class C> class List {
public:
  typedef C *pointer_type;
  typedef C value_type;

  virtual size_t Size() const = 0;
  virtual pointer_type At(size_t id) const = 0;

  typedef VirtualIterator<List, &List::Size, pointer_type, &List::At>
      iterator_type;

  iterator_type begin() const { return iterator_type(this, 0); }
  iterator_type end() const { return iterator_type(this); }

  pointer_type operator[](size_t id) { return At(id); }
};
} // namespace uni