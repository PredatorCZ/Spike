/*  Contains macros for reflection of classes

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
#include <cstddef>

template <size_t n>
constexpr size_t _GetReflDescPart(const char (&value)[n], bool part2 = false) {
  size_t cutter = 0;

  for (auto c : value) {
    if (!c || c == '%') {
      if (c && part2) {
        cutter++;
      }
      break;
    }

    cutter++;
  }

  return cutter;
}

#define _REFLECTOR_EXTRACT_0(flags, item, ...) item
#define _REFLECTOR_EXTRACT_0S(flags, item, ...) #item

#define _REFLECTOR_EXTRACT_ALIASHASH(flags, ...)                               \
  VA_NARGS_EVAL(_REFLECTOR_EXTRACT_ALIASHASH_##flags(__VA_ARGS__))
#define _REFLECTOR_EXTRACT_ALIASHASH_A(x, al, ...)                             \
  JenHash { al }
#define _REFLECTOR_EXTRACT_ALIASHASH_D(x, ds, ...)                             \
  {}
#define _REFLECTOR_EXTRACT_ALIASHASH_AD(x, al, ...)                            \
  JenHash { al }
#define _REFLECTOR_EXTRACT_ALIASHASH_(x, ...)                                  \
  {}
#define _REFLECTOR_EXTRACT_ALIASHASH_N(x, ...)                                 \
  {}

#define _REFLECTOR_EXTRACT_ALIAS(flags, ...)                                   \
  VA_NARGS_EVAL(_REFLECTOR_EXTRACT_ALIAS_##flags(__VA_ARGS__))
#define _REFLECTOR_EXTRACT_ALIAS_A(x, al, ...) al
#define _REFLECTOR_EXTRACT_ALIAS_D(x, ds, ...) nullptr
#define _REFLECTOR_EXTRACT_ALIAS_AD(x, al, ...) al
#define _REFLECTOR_EXTRACT_ALIAS_(x, ...) nullptr
#define _REFLECTOR_EXTRACT_ALIAS_N(x, ...) nullptr

#define _REFLECTOR_DESCBUILDER(item)                                           \
  { {item, _GetReflDescPart(item)}, &item[_GetReflDescPart(item, true)] }

#define _REFLECTOR_EXTRACT_DESC(flags, ...)                                    \
  VA_NARGS_EVAL(_REFLECTOR_EXTRACT_DESC_##flags(__VA_ARGS__))

#define _REFLECTOR_EXTRACT_DESC_A(...)                                         \
  {                                                                            \
    {nullptr, (size_t)0}, { nullptr, (size_t)0 }                               \
  }
#define _REFLECTOR_EXTRACT_DESC_D(x, ds, ...) _REFLECTOR_DESCBUILDER(ds)
#define _REFLECTOR_EXTRACT_DESC_AD(x, al, ds, ...) _REFLECTOR_DESCBUILDER(ds)
#define _REFLECTOR_EXTRACT_DESC_(...) _REFLECTOR_EXTRACT_DESC_A(__VA_ARGS__)
#define _REFLECTOR_EXTRACT_DESC_N(...) _REFLECTOR_EXTRACT_DESC_A(__VA_ARGS__)

#define _REFLECTOR_ADDN_ITEM_DESC(value) _REFLECTOR_EXTRACT_DESC value,
#define _REFLECTOR_ADDN_ITEM_ALIAS(value) _REFLECTOR_EXTRACT_ALIAS value,
#define _REFLECTOR_ADDN_ITEM_ALIASHASH(value)                                  \
  _REFLECTOR_EXTRACT_ALIASHASH value,
#define _REFLECTOR_ADDN_ITEM_EXTNAME(value) _REFLECTOR_EXTRACT_0S value,
#define _REFLECTOR_ADDN_ITEM(value) #value,
#define _REFLECTOR_GET_CNAME(...) #__VA_ARGS__
#define _REFLECTOR_GET_CLASS(...) __VA_ARGS__

#define _REFLECTOR_ADDN(classname, _id, mvalue)                                \
  BuildReflType<decltype(classname::mvalue), classname>(                       \
      JenHash(#mvalue), _id, offsetof(classname, mvalue)),

#define _EXT_REFLECTOR_ADDN(classname, _id, value)                             \
  BuildReflType<decltype(classname::_REFLECTOR_EXTRACT_0 value), classname>(   \
      JenHash(_REFLECTOR_EXTRACT_0S value), _id,                               \
      offsetof(classname, _REFLECTOR_EXTRACT_0 value)),

#define _REFLECTOR_MAIN_BODY(extType, ...)                                     \
  static const reflType *Types() {                                             \
    static const reflType types[] = {                                          \
        StaticForArgID(extType, value_type, __VA_ARGS__)};                     \
    return types;                                                              \
  }                                                                            \
  static constexpr size_t NumTypes() { return VA_NARGS(__VA_ARGS__); }

#define _REFLECTOR_MAIN_CLASSHASH(clseval, classname)                          \
  static constexpr JenHash Hash() { return JenHash(clseval(classname)); }

#define _REFLECTOR_CNAME_VARNAMES(clseval, classname)                          \
  static const char *ClassName() { return clseval(classname); }
#define _REFLECTOR_CNAME_TEMPLATE(...)
#define _REFLECTOR_CNAME_EXTENDED(clseval, classname)                          \
  static const char *ClassName() { return clseval(classname); }

#define _REFLECTOR_NAMES_VARNAMES(...)                                         \
  static const char *const *TypeNames() {                                      \
    static const char *typeNames[] = {                                         \
        StaticFor(_REFLECTOR_ADDN_ITEM, __VA_ARGS__)};                         \
    return typeNames;                                                          \
  }

#define _REFLECTOR_NAMES_TEMPLATE(...)
#define _REFLECTOR_NAMES_EXTENDED(...)                                         \
  static const char *const *TypeNames() {                                      \
    static const char *typeNames[] = {                                         \
        StaticFor(_REFLECTOR_ADDN_ITEM_EXTNAME, __VA_ARGS__)};                 \
    return typeNames;                                                          \
  }                                                                            \
  static const char *const *TypeAliases() {                                    \
    static const char *typeAliases[] = {                                       \
        StaticFor(_REFLECTOR_ADDN_ITEM_ALIAS, __VA_ARGS__)};                   \
    return typeAliases;                                                        \
  }                                                                            \
  static const JenHash *TypeAliasHashes() {                                    \
    static const JenHash typeAliasHashes[] = {                                 \
        StaticFor(_REFLECTOR_ADDN_ITEM_ALIASHASH, __VA_ARGS__)};               \
    return typeAliasHashes;                                                    \
  }                                                                            \
  static const _ReflDesc *TypeDescriptors() {                                  \
    static const _ReflDesc typeDescriptors[] = {                               \
        StaticFor(_REFLECTOR_ADDN_ITEM_DESC, __VA_ARGS__)};                    \
    return typeDescriptors;                                                    \
  }

#define _REFLECTOR_CNAME(classname) #classname
#define _TEMPLATE_REFLECTOR_CNAME(classname) _REFLECTOR_GET_CNAME classname

#define _REFLECTOR_CLASS(classname) classname
#define _TEMPLATE_REFLECTOR_CLASS(classname) _REFLECTOR_GET_CLASS classname

#define _REFLECTOR_TEMPLATE_TEMPLATE _TEMPLATE
#define _REFLECTOR_TEMPLATE_VARNAMES
#define _REFLECTOR_TEMPLATE_EXTENDED

#define _REFLECTOR_EXTENDED_EXTENDED _EXT
#define _REFLECTOR_EXTENDED_TEMPLATE
#define _REFLECTOR_EXTENDED_VARNAMES

#define _REFLECTOR_INTERFACE(clseval, classname)                               \
  template <>                                                                  \
  const reflectorStatic ES_EXPORT *                                                      \
  ReflectorInterface<clseval(classname)>::GetReflector() {                     \
    static const reflectorStatic rclass(_RTag<clseval(classname)>{});          \
    return &rclass;                                                            \
  }

#define _REFLECTOR_START_VER0_(adtype, classname, ...)                         \
  template <> struct ReflectorType<classname> : ReflectorTypeBase {            \
    using value_type = classname;                                              \
    _REFLECTOR_MAIN_BODY(adtype, __VA_ARGS__);                                 \
  };                                                                           \
  _REFLECTOR_INTERFACE(_REFLECTOR_CLASS, classname)

// clang-format off

#define _REFLECTOR_START_VER1_(adtype, classname, var01, ...)                                                 \
  template <> struct ReflectorType<_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01, _REFLECTOR_CLASS)(classname)>    \
  : ReflectorTypeBase {                                                                                       \
    using value_type = _LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01, _REFLECTOR_CLASS)(classname);                \
    _REFLECTOR_MAIN_BODY(_LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01, adtype), __VA_ARGS__)                      \
    _REFLECTOR_NAMES_##var01(__VA_ARGS__);                                                                    \
    _REFLECTOR_CNAME_##var01(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01, _REFLECTOR_CNAME), classname)          \
    _REFLECTOR_MAIN_CLASSHASH(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01, _REFLECTOR_CNAME), classname)         \
  };                                                                                                          \
  _REFLECTOR_INTERFACE(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01, _REFLECTOR_CLASS), classname)                \

#define _REFLECTOR_START_VER2_(adtype, classname, var01, var02, ...)                                                                   \
  template <> struct ReflectorType<_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CLASS)(classname)> \
  : ReflectorTypeBase {                                                                                                                \
    using value_type = _LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CLASS)(classname);             \
    _REFLECTOR_MAIN_BODY(_LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02, adtype), __VA_ARGS__)                   \
    _REFLECTOR_NAMES_##var01(__VA_ARGS__);                                                                                             \
    _REFLECTOR_NAMES_##var02(__VA_ARGS__);                                                                                             \
    _REFLECTOR_CNAME_##var01(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CNAME), classname)       \
    _REFLECTOR_CNAME_##var02(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CNAME), classname)       \
    _REFLECTOR_MAIN_CLASSHASH(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CNAME), classname)      \
  };                                                                                                                                   \
  _REFLECTOR_INTERFACE(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02, _REFLECTOR_CLASS), classname)

#define _REFLECTOR_START_VER3_(adtype, classname, var01, var02, var03, ...)                                                                                        \
  template <> struct ReflectorType<_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CLASS)(classname)> \
  : ReflectorTypeBase {                                                                                                                                            \
    using value_type = _LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CLASS)(classname);             \
    _REFLECTOR_MAIN_BODY(_LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02 _REFLECTOR_EXTENDED_##var03, adtype), __VA_ARGS__)                   \
    _REFLECTOR_NAMES_##var01(__VA_ARGS__);                                                                                                                         \
    _REFLECTOR_NAMES_##var02(__VA_ARGS__);                                                                                                                         \
    _REFLECTOR_NAMES_##var03(__VA_ARGS__);                                                                                                                         \
    _REFLECTOR_CNAME_##var01(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CNAME), classname)       \
    _REFLECTOR_CNAME_##var02(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CNAME), classname)       \
    _REFLECTOR_CNAME_##var03(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CNAME), classname)       \
    _REFLECTOR_MAIN_CLASSHASH(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CNAME), classname)      \
  };                                                                                                                                                               \
  _REFLECTOR_INTERFACE(_LOOPER_CAT2(_REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 _REFLECTOR_TEMPLATE_##var03, _REFLECTOR_CLASS), classname)

// clang-format on

#define _REFLECTOR_START_VER0(...)                                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER0_(_REFLECTOR_ADDN, __VA_ARGS__))
#define _REFLECTOR_START_VER1(...)                                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER1_(_REFLECTOR_ADDN, __VA_ARGS__))
#define _REFLECTOR_START_VER2(...)                                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER2_(_REFLECTOR_ADDN, __VA_ARGS__))
#define _REFLECTOR_START_VER3(...)                                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER3_(_REFLECTOR_ADDN, __VA_ARGS__))

#define _REFLECTOR_ADDB(classname, _id, mvalue)                                \
  BuildBFReflType<classname, mvalue>(JenHash(#mvalue)),

#define _EXT_REFLECTOR_ADDB(classname, _id, value)                             \
  BuildBFReflType<classname, _REFLECTOR_EXTRACT_0 value>(                      \
      JenHash(_REFLECTOR_EXTRACT_0S value)),

#define _REFLECTOR_START_VERBITFIELD(classname, numFlags, ...)                 \
  VA_NARGS_EVAL(_REFLECTOR_START_VER##numFlags##_(_REFLECTOR_ADDB, classname,  \
                                                  __VA_ARGS__))
