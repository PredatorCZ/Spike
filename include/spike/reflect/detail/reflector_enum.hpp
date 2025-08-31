/*  Define reflected enumerations

    Copyright 2018-2024 Lukas Cone

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
#include "spike/crypto/jenkinshash.hpp"
#include "spike/util/settings.hpp"
#include <map>

struct EnumProxy {
  const char *name;
  uint64 value;
  const char *description = nullptr;
};

struct ReflectedEnum {
  using RegistryType = std::map<JenHash, const ReflectedEnum *>;
  JenHash enumHash;
  uint32 numMembers;
  uint32 size;
  const char *enumName;
  const char *const *names;
  const uint64 *values;
  const char *const *descriptions = nullptr;

  template <class... C, size_t cs, class guard>
  ReflectedEnum(const guard *, const char (&enumName_)[cs], C... members)
      : enumHash(JenHash(enumName_)), numMembers(sizeof...(C)),
        size(sizeof(guard)), enumName(enumName_) {
    static const char *names_[]{members.name...};
    names = names_;
    static const uint64 values_[]{members.value...};
    values = values_;

    union mutate {
      const char *h;
      uintptr i;
    };

    if ((mutate{members.description}.i | ...)) {
      static const char *descriptions_[]{members.description...};
      descriptions = descriptions_;
    }

    Registry()[enumHash] = this;
  }

  static RegistryType PC_EXTERN &Registry();
};

template <class E> const ReflectedEnum *GetReflectedEnum();

#define DECL_EMEMBER(type, ...) type,
#define DECL_EMEMBERVAL(type, value, ...) type = value,
#define DECL_EMEMBERNAME(type, ...) type,
#define DECL_ENUM(type) enum type {
#define DECL_ENUMSCOPE(type, ...) DECL_ENUM(type)
#define DECL_END_ENUM(...)                                                     \
  }                                                                            \
  ;
#define DECL_END_ENUMSCOPE(...) DECL_END_ENUM()

#define DEF_EMEMBER(type, ...)                                                 \
  EnumProxy{#type, static_cast<uint64>(enum_type::type), __VA_ARGS__},
#define DEF_EMEMBERVAL(type, value, ...) DEF_EMEMBER(type, __VA_ARGS__)
#define DEF_EMEMBERNAME(type, name, ...)                                       \
  EnumProxy{name, static_cast<uint64>(enum_type::type), __VA_ARGS__},
#define DEF_ENUM(name)                                                         \
  template <> constexpr JenHash EnumHash<name>() { return #name; }             \
  template <> inline const ReflectedEnum *GetReflectedEnum<name>() {           \
    using enum_type = name;                                                    \
    static const ReflectedEnum reflectedEnum {                                 \
      std::add_pointer_t<enum_type>{nullptr}, #name,

#define DEF_ENUMSCOPE(type, name) DEF_ENUM(name)
#define DEF_END_ENUM(...)                                                      \
  }                                                                            \
  ;                                                                            \
  return &reflectedEnum;                                                       \
  }                                                                            \
  template <> class es::reflector::detail::InvokeGuard<__VA_ARGS__> {          \
    static inline const ReflectedEnum *data = GetReflectedEnum<__VA_ARGS__>(); \
  };
#define DEF_END_ENUMSCOPE(type, name) DEF_END_ENUM(name)

#define MAKE_DECL(x) DECL_##x
#define MAKE_DEF(x) DEF_##x

// clang-format off
// Creates enumeration and it's reflection definition
// what: ENUM(name) or ENUMSCOPE(decl, name)
// args: EMEMBER(name [, description]), EMEMBERVAL(name, value [, description]), EMEMBERNAME(name, refname [, description])
// clang-format on
#define MAKE_ENUM(what, ...)                                                   \
  DECL_##what StaticFor(MAKE_DECL, __VA_ARGS__)                                \
      DECL_END_##what DEF_##what StaticFor(MAKE_DEF, __VA_ARGS__)              \
          DEF_END_##what
