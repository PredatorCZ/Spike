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

namespace _uni_ {
template <class C, class _const_type> class List {
public:
  typedef C value_type;
  typedef _const_type const_type;

  virtual size_t Size() const = 0;
  virtual const_type At(size_t id) const = 0;
  virtual ~List() {}

  typedef uni::VirtualIterator<List, &List::Size, const_type, &List::At>
      iterator_type_const;

  iterator_type_const begin() const { return iterator_type_const(this, 0); }
  iterator_type_const end() const { return iterator_type_const(this); }

  const_type operator[](size_t id) const { return At(id); }
};
}; // namespace _uni_

namespace uni {
template <class C> using List = _uni_::List<Element<C>, Element<const C>>;
template <class C> using Vector = _uni_::List<C, const C>;
} // namespace uni