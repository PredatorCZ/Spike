/*  Define reflected enumerations

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
#include "../jenkinshash.hpp"

struct EnumProxy {
  const char *name;
  uint64 value;
};

struct ReflectedEnum {
  JenHash enumHash;
  uint32 numMembers;
  const char *enumName;
  const char *const *names;
  const uint64 *values;

  template <class... C, size_t cs, class guard>
  ReflectedEnum(const guard *, const char (&enumName_)[cs], C... members)
      : enumHash(JenHash(enumName_)), numMembers(sizeof...(C)),
        enumName(enumName_) {
    static const char *names_[]{members.name...};
    names = names_;
    static const uint64 values_[]{members.value...};
    values = values_;
  }
};

template <class E> const ReflectedEnum *GetReflectedEnum();

#define DECL_EMEMBER(type) type,
#define DECL_EMEMBERVAL(type, value) type = value,
#define DECL_ENUM(type) enum type {
#define DECL_ENUMSCOPE(type, ...) DECL_ENUM(type)
#define DECL_END_ENUM(...)                                                     \
  }                                                                            \
  ;
#define DECL_END_ENUMSCOPE(...) DECL_END_ENUM()

#define DEF_EMEMBER(type)                                                      \
  EnumProxy{#type, static_cast<uint64>(enum_type::type)},
#define DEF_EMEMBERVAL(type, ...) DEF_EMEMBER(type)
#define DEF_ENUM(name)                                                         \
  template <> constexpr JenHash EnumHash<name>() { return #name; }             \
  template <> const ReflectedEnum *GetReflectedEnum<name>() {                  \
    using enum_type = name;                                                    \
    static const ReflectedEnum reflectedEnum {                                 \
      std::add_pointer_t<enum_type>{nullptr}, #name,

#define DEF_ENUMSCOPE(type, name) DEF_ENUM(name)
#define DEF_END_ENUM(...)                                                      \
  }                                                                            \
  ;                                                                            \
  return &reflectedEnum;                                                       \
  }
#define DEF_END_ENUMSCOPE(...) DEF_END_ENUM()

#define MAKE_DECL(x) DECL_##x
#define MAKE_DEF(x) DEF_##x

// Creates enumeration and it's reflection definition
// what: ENUM() or ENUMSCOPE()
// args: EMEMBER or EMEMBERVAL
#define MAKE_ENUM(what, ...)                                                   \
  DECL_##what StaticFor(MAKE_DECL, __VA_ARGS__)                                \
      DECL_END_##what DEF_##what StaticFor(MAKE_DEF, __VA_ARGS__)              \
          DEF_END_##what
