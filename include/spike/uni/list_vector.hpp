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
#include "common.hpp"
#include "list.hpp"
#include <vector>

namespace uni {
template <class _interface_type, class _class_type,
          template <class itype> class _list_type = List>
class VectorList : public virtual _list_type<_interface_type> {
public:
  typedef _list_type<_interface_type> list_type;
  typedef _interface_type interface_type;
  typedef typename list_type::const_type const_type;
  typedef _class_type class_type;

  size_t Size() const override { return storage.size(); }
  const_type At(size_t id) const override { return storage.at(id); }

  typedef VirtualIterator<list_type, &list_type::Size, const_type,
                          &list_type::At>
      iterator_type;

  iterator_type begin() const { return iterator_type(this, 0); }
  iterator_type end() const { return iterator_type(this); }

  const_type operator[](size_t id) const { return At(id); }

  std::vector<class_type> storage;
};

template <class _interface_type, class _class_type,
          template <class ...itype> class _smart_type = std::unique_ptr>
class PolyVectorList : public virtual List<_interface_type> {
public:
  typedef List<_interface_type> list_type;
  typedef _interface_type interface_type;
  typedef typename list_type::const_type const_type;
  typedef _smart_type<_class_type> class_type;

  size_t Size() const override { return storage.size(); }
  const_type At(size_t id) const override { return {storage.at(id).get(), false}; }

  typedef VirtualIterator<list_type, &list_type::Size, const_type,
                          &list_type::At>
      iterator_type;

  iterator_type begin() const { return iterator_type(this, 0); }
  iterator_type end() const { return iterator_type(this); }

  const_type operator[](size_t id) const { return At(id); }

  std::vector<class_type> storage;
};
} // namespace uni
