/*      std::deleter_hybrid for pointer classes, allows to link stack value.
        more info in README for PreCore Project

        Copyright 2019 Lukas Cone

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

namespace std {

class deleter_hybrid {
  bool canDelete; // Ouchi ouch, doubles the size of pointer class
public:
  deleter_hybrid() : canDelete(true) {}
  deleter_hybrid(bool shoudDelete) : canDelete(shoudDelete) {}

  template <class _Ty> void operator()(_Ty *ptr) const {
    if (canDelete)
      delete ptr;
  }
};

class deleter_hybrid_c {
  bool canDelete; // Ouchi ouch, doubles the size of pointer class
public:
  deleter_hybrid_c() : canDelete(true) {}
  deleter_hybrid_c(bool shoudDelete) : canDelete(shoudDelete) {}

  template <class _Ty> void operator()(_Ty *ptr) const {
    if (canDelete)
      free(ptr);
  }
};
} // namespace std