/*  Registering reflected classes
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

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
#include "../settings.hpp"
#include "reflector_class.hpp"
#include "reflector_enum.hpp"
#include <map>

typedef std::map<JenHash, const ReflectedEnum *> RefEnumMapper;
extern RefEnumMapper PC_EXTERN REFEnumStorage;
typedef std::map<JenHash, const reflectorStatic *> RefSubClassMapper;
extern RefSubClassMapper PC_EXTERN REFSubClassStorage;

template <class C> void RegisterReflectedType() {
  if constexpr (std::is_class_v<C>) {
    REFSubClassStorage[ClassHash<C>()] = GetReflectedClass<C>();
  } else {
    REFEnumStorage[EnumHash<C>()] = GetReflectedEnum<C>();
  }
}

// Adding reflected classes/enums into global registry
// Required for enums/classes, if they are being used as a class member for
// another reflected class or for a run-time serialization
template <class... C> void RegisterReflectedTypes() {
  (RegisterReflectedType<C>(), ...);
}
