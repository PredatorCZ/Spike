/*  uni exception internal module
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
#include <stdexcept>

namespace _uni_ {

inline std::exception ThrowVoidCall(bool logical) {
  if (logical) {
    return std::logic_error("Unhandled call!");
  } else {
    return std::runtime_error("Unsupported call!");
  }
}
template <class C, C exValue> std::exception ThrowVoidCall(C expected) {
  return ThrowVoidCall(expected == exValue);
}
} // namespace _uni_

namespace uni {
class ModifyError : std::runtime_error {
  using parent = std::runtime_error;

public:
  explicit ModifyError(const std::string &modifyName)
      : parent(std::string("Setting ") + modifyName +
               "is forbidden for this class!") {}

  explicit ModifyError(const char *modifyName)
      : parent(std::string("Setting ") + modifyName +
               "is forbidden for this class!") {}
};
} // namespace uni
