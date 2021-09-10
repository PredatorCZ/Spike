/*  Class reflection definitions

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
#include "reflector_type.hpp"

struct ReflDesc {
  const char *part1 = nullptr;
  const char *part2 = nullptr;
  ReflDesc() = default;
  ReflDesc(const char *part1_) : part1(part1_) {}
  ReflDesc(const char *part1_, const char *part2_)
      : part1(part1_), part2(part2_) {}
};

struct NoName {};

struct MemberProxy {
  const char *typeName;
  const char *aliasName = nullptr;
  JenHash aliasHash;
  reflType type;
  ReflDesc description;

private:
  void setup(const char *aliasName_) {
    aliasName = aliasName_;
    aliasHash = JenHash(aliasName_);
  }

  void setup(const ReflDesc &desc) { description = desc; }

  void setup(NoName) { typeName = nullptr; }

public:
  template <class... Input, class type_>
  explicit MemberProxy(type_ getter, Input... inputs) {
    auto [offset, name, hash, rtfn, isbitmember] = getter();
    if constexpr (std::is_same_v<decltype(isbitmember), std::true_type>) {
      type = BuildBFReflType<std::remove_pointer_t<decltype(offset)>,
                             std::remove_pointer_t<decltype(rtfn)>>(hash);
    } else {
      type = BuildReflType<decltype(*rtfn)>(hash, 0, offset);
    }

    typeName = name;

    (setup(inputs), ...);
  }
};

struct reflectorStatic {
  const JenHash classHash;
  const uint32 nTypes;
  const reflType *types;
  const char *const *typeNames = nullptr;
  const char *className;
  const char *const *typeAliases = nullptr;
  const JenHash *typeAliasHashes = nullptr;
  const ReflDesc *typeDescs;

  template <class guard, class... C, size_t cs>
  reflectorStatic(const guard *, const char (&className_)[cs], C... members)
      : classHash(JenHash(className_)), nTypes(sizeof...(C)),
        className(className_) {
    if constexpr (sizeof...(C) > 0) {
      size_t index = 0;
      struct reflType2 : reflType {
        reflType2(const reflType &r, size_t index) : reflType(r) { ID = index; }
      };
      static const reflType types_[]{reflType2{members.type, index++}...};
      types = types_;
      union mutate {
        const char *h;
        uintptr_t i;
      };

      if ((mutate{members.typeName}.i | ...)) {
        static const char *typeNames_[]{members.typeName...};
        typeNames = typeNames_;
      }

      if ((mutate{members.aliasName}.i | ...)) {
        static const char *typeAliases_[]{members.aliasName...};
        typeAliases = typeAliases_;
        static JenHash typeAliasHashes_[]{members.aliasHash...};
        typeAliasHashes = typeAliasHashes_;
      }

      if ((mutate{members.description.part1}.i | ...) |
          (mutate{members.description.part2}.i | ...)) {
        static ReflDesc typeDescs_[]{members.description...};
        typeDescs = typeDescs_;
      }
    }
  }
};

static_assert(sizeof(reflectorStatic) == 56);

struct ReflectedInstance {
private:
  friend class Reflector;
  friend struct ReflectedInstanceFriend;

  const reflectorStatic *rfStatic = nullptr;
  union {
    const void *constInstance = nullptr;
    void *instance;
  };

public:
  ReflectedInstance() = default;
  ReflectedInstance(const reflectorStatic *rfStat, const void *inst)
      : rfStatic(rfStat), constInstance(inst) {}
};

template <class C> const reflectorStatic ES_IMPORT *GetReflectedClass();

#define MEMBERNAME(member, name, ...)                                          \
  MemberProxy {                                                                \
    [] {                                                                       \
      return std::make_tuple(                                                  \
          offsetof(class_type, member), name, JenHash(name),                   \
          std::add_pointer_t<decltype(class_type::member)>{nullptr},           \
          std::false_type{});                                                  \
    },                                                                         \
        __VA_ARGS__                                                            \
  }

#define MEMBER(member, ...) MEMBERNAME(member, #member, __VA_ARGS__)

#define BITMEMBERNAME(member, name, ...)                                       \
  MemberProxy {                                                                \
    [] {                                                                       \
      return std::make_tuple(                                                  \
          std::add_pointer_t<class_type>{nullptr}, name, JenHash(name),        \
          std::add_pointer_t<member>{nullptr}, std::true_type{});              \
    },                                                                         \
        __VA_ARGS__                                                            \
  }

#define BITMEMBER(member, ...) BITMEMBERNAME(member, #member, __VA_ARGS__)

#define CLASS(...)                                                             \
  template <> constexpr JenHash ClassHash<__VA_ARGS__>() {                     \
    return #__VA_ARGS__;                                                       \
  }                                                                            \
  template <>                                                                  \
  const reflectorStatic ES_EXPORT *GetReflectedClass<__VA_ARGS__>() {          \
    using class_type = __VA_ARGS__;                                            \
    static const reflectorStatic reflectedClass {                              \
      std::add_pointer_t<class_type>{nullptr}, #__VA_ARGS__,

// internal, do not use
#define END_CLASS(...)                                                         \
  }                                                                            \
  ;                                                                            \
  return &reflectedClass;                                                      \
  }

// Create reflection definition (use only in TU)
// what: always CLASS()
// args: MEMBER, MEMBERNAME or BITMEMBER, BITMEMBERNAME, invokes MemberProxy
// contructor
#define REFLECT(what, ...) what __VA_ARGS__ END_##what
