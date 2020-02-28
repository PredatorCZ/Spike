/*  Registering class reflectors
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
#include "../jenkinshash.hpp"
#include "../string_view.hpp"
#include "../supercore.hpp"
#include <unordered_map>

constexpr size_t _GetReflDescPart(const char *value, int add,
                                  size_t curIndex = 0) {
  return (value[curIndex] == '%' || value[curIndex] == 0)
             ? curIndex + (value[curIndex] == '%' ? add : 0)
             : _GetReflDescPart(value, add, curIndex + 1);
}

struct reflType;

struct _ReflDesc {
  es::string_view part1, part2;
};

struct reflectorStatic_data {
  const JenHash classHash;
  const uint32 nTypes;
  const reflType *types;
  const char *const *typeNames;
  const char *className;
  const char *const *typeAliases;
  const JenHash *typeAliasHashes;
  const _ReflDesc *typeDescs;

  reflectorStatic_data(const uint32 _nTypes, const reflType *_types,
                       const char *const *_typeNames, const char *_className,
                       const JenHash _classHash)
      : classHash(_classHash), nTypes(_nTypes), types(_types),
        typeNames(_typeNames), className(_className), typeAliases(nullptr),
        typeAliasHashes(nullptr), typeDescs(nullptr) {}

  reflectorStatic_data(const uint32 _nTypes, const reflType *_types,
                       const JenHash _classHash)
      : classHash(_classHash), nTypes(_nTypes), types(_types),
        typeNames(nullptr), className(nullptr), typeAliases(nullptr),
        typeAliasHashes(nullptr), typeDescs(nullptr) {}

  reflectorStatic_data(const uint32 _nTypes, const reflType *_types,
                       const char *const *_typeNames, const char *_className,
                       const char *const *_typeAliases,
                       const JenHash *_typeAliasHashes,
                       const _ReflDesc *_typeDescs, const JenHash _classHash)
      : classHash(_classHash), nTypes(_nTypes), types(_types),
        typeNames(_typeNames), className(_className), typeAliases(_typeAliases),
        typeAliasHashes(_typeAliasHashes), typeDescs(_typeDescs) {}
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

struct reflectorInstance {
  const reflectorStatic *rfStatic;
  void *rfInstance;
};

struct reflectorInstanceConst {
  const reflectorStatic *rfStatic;
  const void *rfInstance;
};

typedef std::unordered_map<unsigned int, const reflectorStatic *>
    RefSubClassMapper;
extern RefSubClassMapper REFSubClassStorage;

#define REGISTER_CLASS(classname)                                              \
  if (_SubReflClassWrap<classname>::HASH)                                      \
    REFSubClassStorage[(const JenHash)_SubReflClassWrap<classname>::HASH] =    \
        classname::__rfPtrStatic;                                              \
  classname::_rfInit();

#define REGISTER_CLASSES(...) StaticFor(REGISTER_CLASS, __VA_ARGS__)