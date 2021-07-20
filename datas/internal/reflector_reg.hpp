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
#include "reflector_class_reg.hpp"
#include "reflector_enum_reg.hpp"
#include "../settings.hpp"
#include <map>

typedef std::map<JenHash, ReflectedEnum> RefEnumMapper;
extern RefEnumMapper PC_EXTERN REFEnumStorage;
typedef std::map<JenHash, const reflectorStatic *> RefSubClassMapper;
extern RefSubClassMapper PC_EXTERN REFSubClassStorage;

template <class C> struct RegisterReflectedType {
  template <class C_ = C>
  static typename std::enable_if<std::is_class<C_>::value>::type
  RegisterRefClass() {
    REFSubClassStorage[ReflectorType<C>::Hash()] =
        GetReflectedClass<C>();
  }

  template <class C_ = C>
  static typename std::enable_if<std::is_enum<C_>::value>::type
  RegisterRefClass() {
    REFEnumStorage[_EnumWrap<C>::GetHash()] = GetReflectedEnum<C>();
  }
  RegisterReflectedType() { RegisterRefClass(); }
};

// Adding reflected classes/enums into global registry
// Required for enums/classes, if they are being used as a class member for
// another reflected class or for a run-time serialization
template <class... C>
struct RegisterReflectedTypes : RegisterReflectedType<C>... {};

#define _REFLECTOR_REGISTER(classname)                                         \
  RegisterReflectedType<classname>::RegisterRefClass();

#define REFLECTOR_REGISTER(...)                                                \
  StaticFor(_REFLECTOR_REGISTER, __VA_ARGS__);                                 \
  ES_PRAGMA(message("REFLECTOR_REGISTER is deprecated, use "                   \
                    "RegisterReflectedTypes instead."))
