/*  Registering reflected classes
    more info in README for PreCore Project

    Copyright 2018-2020 Lukas Cone

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
#include "reflector_class_reg.hpp"
#include "reflector_enum_reg.hpp"

typedef std::unordered_map<uint32, ReflectedEnum> RefEnumMapper;
extern RefEnumMapper REFEnumStorage;
typedef std::unordered_map<uint32, const reflectorStatic *> RefSubClassMapper;
extern RefSubClassMapper REFSubClassStorage;

template <class C>
typename std::enable_if<std::is_class<C>::value>::type _RegisterRefClass() {
  REFSubClassStorage[ReflectorType<C>::Hash()] =
      ReflectorInterface<C>::GetReflector();
}

template <class C>
typename std::enable_if<std::is_enum<C>::value>::type _RegisterRefClass() {
  REFEnumStorage[_EnumWrap<C>::GetHash()] = GetReflectedEnum<C>();
}

#define _REFLECTOR_REGISTER(classname) _RegisterRefClass<classname>();


// Adding reflected classes/enums into global registry
// Required for enums/classes, if they are being used as a class member for
// another reflected class or for a run-time serialization
#define REFLECTOR_REGISTER(...) StaticFor(_REFLECTOR_REGISTER, __VA_ARGS__);
