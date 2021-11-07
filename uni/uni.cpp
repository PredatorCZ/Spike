/*  uni module common source
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

#include "datas/reflector.hpp"
#include "internal/exception.hpp"
#include "internal/motion.inl"
#include "internal/skeleton.inl"
#include "model.hpp"

namespace uni {

MetadataConst Base::Metadata() const { return {}; }

void Skin::GetTM(RTSValue &, size_t) const {
  throw _uni_::ThrowVoidCall<TransformType, TMTYPE_RTS>(this->TMType());
}
void Skin::GetTM(es::Matrix44 &, size_t) const {
  throw _uni_::ThrowVoidCall<TransformType, TMTYPE_MATRIX>(this->TMType());
}
} // namespace uni
