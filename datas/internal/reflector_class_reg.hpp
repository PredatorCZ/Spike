/*  Registering class reflectors
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
#include "../jenkinshash.hpp"
#include "../string_view.hpp"
#include "../supercore.hpp"

struct reflType;

struct _ReflDesc {
  es::string_view part1, part2;
};

template <class C> struct _RTag {};

struct ReflectorTypeBase {
  static constexpr JenHash Hash() { return {}; }
  static constexpr const char *ClassName() { return nullptr; }
  static constexpr size_t NumTypes() { return 0; }
  static const reflType *Types() { return nullptr; }
  static const char *const *TypeNames() { return nullptr; }
  static const char *const *TypeAliases() { return nullptr; }
  static const JenHash *TypeAliasHashes() { return nullptr; }
  static const _ReflDesc *TypeDescriptors() { return nullptr; }
};

template <class C> struct ReflectorType : ReflectorTypeBase {};

struct reflectorStatic_data {
  const JenHash classHash;
  const uint32 nTypes;
  const reflType *types;
  const char *const *typeNames;
  const char *className;
  const char *const *typeAliases;
  const JenHash *typeAliasHashes;
  const _ReflDesc *typeDescs;

  template <class C>
  reflectorStatic_data(_RTag<C>)
      : classHash(ReflectorType<C>::Hash()),
        nTypes(static_cast<uint32>(ReflectorType<C>::NumTypes())),
        types(ReflectorType<C>::Types()),
        typeNames(ReflectorType<C>::TypeNames()),
        className(ReflectorType<C>::ClassName()),
        typeAliases(ReflectorType<C>::TypeAliases()),
        typeAliasHashes(ReflectorType<C>::TypeAliasHashes()),
        typeDescs(ReflectorType<C>::TypeDescriptors()) {}
};

template <bool x64> struct reflectorStatic_t : reflectorStatic_data {
  using reflectorStatic_data::reflectorStatic_data;
};

template <> struct reflectorStatic_t<false> : reflectorStatic_data {
  using reflectorStatic_data::reflectorStatic_data;
  int _pad[6];
};

typedef reflectorStatic_t<ES_X64> reflectorStatic;
const int __sizeof_reflectorStatic = sizeof(reflectorStatic);

ES_STATIC_ASSERT(__sizeof_reflectorStatic == 56);

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

template <class C> struct ReflectorInterface {
  static const reflectorStatic ES_IMPORT *GetReflector();
};

template <class C> const reflectorStatic *GetReflectedClass() {
  return ReflectorInterface<C>::GetReflector();
}
