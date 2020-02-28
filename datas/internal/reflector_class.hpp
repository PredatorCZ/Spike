/*  Contains macros for reflection of classes

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

#define _REFLECTOR_EXTRACT_0(flags, item, ...) item
#define _REFLECTOR_EXTRACT_0S(flags, item, ...) #item

#define _REFLECTOR_EXTRACT_ALIASHASH(flags, ...)                               \
  VA_NARGS_EVAL(_REFLECTOR_EXTRACT_ALIASHASH_##flags(__VA_ARGS__))
#define _REFLECTOR_EXTRACT_ALIASHASH_A(x, al, ...) JenkinsHashC(al)
#define _REFLECTOR_EXTRACT_ALIASHASH_D(x, ds, ...) 0
#define _REFLECTOR_EXTRACT_ALIASHASH_AD(x, al, ...) JenkinsHashC(al)
#define _REFLECTOR_EXTRACT_ALIASHASH_(x, ...) 0
#define _REFLECTOR_EXTRACT_ALIASHASH_N(x, ...) 0

#define _REFLECTOR_EXTRACT_ALIAS(flags, ...)                                   \
  VA_NARGS_EVAL(_REFLECTOR_EXTRACT_ALIAS_##flags(__VA_ARGS__))
#define _REFLECTOR_EXTRACT_ALIAS_A(x, al, ...) al
#define _REFLECTOR_EXTRACT_ALIAS_D(x, ds, ...) nullptr
#define _REFLECTOR_EXTRACT_ALIAS_AD(x, al, ...) al
#define _REFLECTOR_EXTRACT_ALIAS_(x, ...) nullptr
#define _REFLECTOR_EXTRACT_ALIAS_N(x, ...) nullptr

#define _REFLECTOR_DESCBUILDER(item)                                           \
  { {item, _GetReflDescPart(item, 0)}, &item[_GetReflDescPart(item, 1)] }

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

#define _REFLECTOR_ADDN(classname, _id, mvalue)                                \
  BuildReflType<decltype(classname::mvalue)>(JenkinsHashC(#mvalue), _id),

#define _EXT_REFLECTOR_ADDN(classname, _id, value)                             \
  BuildReflType<decltype(classname::_REFLECTOR_EXTRACT_0 value)>(              \
      JenkinsHashC(_REFLECTOR_EXTRACT_0S value), _id),

#define _REFLECTOR_TYPES(classname, extType, ...)                              \
  static const reflType __##classname##_types[] = {                            \
      StaticForArgID(extType, classname, __VA_ARGS__)};

#define _REFLECTOR_MAIN_BODY(classname, namesbody, ...)                        \
  static const reflectorStatic __##classname##_statical = {                    \
      VA_NARGS(__VA_ARGS__), __##classname##_types,                            \
      namesbody JenkinsHashC(#classname)};

#define _REFLECTOR_NAMES_VARNAMES(classname, ...)                              \
  static const char *__##classname##_typeNames[] = {                           \
      StaticFor(_REFLECTOR_ADDN_ITEM, __VA_ARGS__)};

#define _REFLECTOR_NAMES_TEMPLATE(...)
#define _REFLECTOR_NAMES_SUBCLASS(...)
#define _REFLECTOR_NAMES_EXTENDED(classname, ...)                              \
  static const char *__##classname##_typeNames[] = {                           \
      StaticFor(_REFLECTOR_ADDN_ITEM_EXTNAME, __VA_ARGS__)};                   \
  static const char *__##classname##_typeAliases[] = {                         \
      StaticFor(_REFLECTOR_ADDN_ITEM_ALIAS, __VA_ARGS__)};                     \
  static const _ReflDesc __##classname##_typeDescs[] = {                       \
      StaticFor(_REFLECTOR_ADDN_ITEM_DESC, __VA_ARGS__)};                      \
  static const JenHash __##classname##_typeAliasHashes[] = {                   \
      StaticFor(_REFLECTOR_ADDN_ITEM_ALIASHASH, __VA_ARGS__)};

#define _REFLECTOR_NAMES_DEF_VARNAMES(classname)                               \
  __##classname##_typeNames, #classname,
#define _REFLECTOR_NAMES_DEF_TEMPLATE(classname)
#define _REFLECTOR_NAMES_DEF_SUBCLASS(classname)
#define _REFLECTOR_NAMES_DEF_EXTENDED(classname)                               \
  __##classname##_typeNames, #classname, __##classname##_typeAliases,          \
      __##classname##_typeAliasHashes, __##classname##_typeDescs,

#define _REFLECTOR_TEMPLATE_TEMPLATE template <>
#define _REFLECTOR_TEMPLATE_SUBCLASS
#define _REFLECTOR_TEMPLATE_VARNAMES
#define _REFLECTOR_TEMPLATE_EXTENDED

#define _REFLECTOR_EXTENDED_EXTENDED _EXT
#define _REFLECTOR_EXTENDED_TEMPLATE
#define _REFLECTOR_EXTENDED_SUBCLASS
#define _REFLECTOR_EXTENDED_VARNAMES

#define _REFLECTOR_SUBCLASS_SUBCLASS(classname)                                \
  template <> struct _SubReflClassWrap<classname> {                            \
    static const JenHash HASH = JenkinsHashC(#classname);                      \
  };
#define _REFLECTOR_SUBCLASS_TEMPLATE(classname)
#define _REFLECTOR_SUBCLASS_VARNAMES(classname)
#define _REFLECTOR_SUBCLASS_EXTENDED(classname)

#define _REFLECTOR_INIT_ELEMENT(classname, id, value)                          \
                                                                               \
  classname::__rfPtrStatic->types[id].offset = offsetof(classname, value);

#define _EXT_REFLECTOR_INIT_ELEMENT(classname, id, value)                      \
                                                                               \
  classname::__rfPtrStatic->types[id].offset =                                 \
      offsetof(classname, _REFLECTOR_EXTRACT_0 value);

#define _REFLECTOR_INIT_BODY(classname, filter, ...)                           \
  void classname::_rfInit() {                                                  \
    if (classname::__rfPtrStatic->types[0].offset != 0xffff)                   \
      return;                                                                  \
    StaticForArgID(filter, classname, __VA_ARGS__)                             \
  }

#define _REFLECTOR_START_VER0(classname, ...)                                  \
  _REFLECTOR_TYPES(classname, _REFLECTOR_ADDN, __VA_ARGS__)                    \
  _REFLECTOR_MAIN_BODY(classname, _REFLECTOR_NAMES_DEF_SUBCLASS(classname),    \
                       __VA_ARGS__)                                            \
  const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical; \
  _REFLECTOR_INIT_BODY(classname, _REFLECTOR_INIT_ELEMENT, __VA_ARGS__)

#define _REFLECTOR_START_VER1(classname, var01, ...)                           \
  _REFLECTOR_TYPES(classname,                                                  \
                   _LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01, _REFLECTOR_ADDN), \
                   __VA_ARGS__)                                                \
  _REFLECTOR_NAMES_##var01(classname, __VA_ARGS__);                            \
  _REFLECTOR_MAIN_BODY(classname, _REFLECTOR_NAMES_DEF_##var01(classname),     \
                       __VA_ARGS__)                                            \
  _REFLECTOR_TEMPLATE_##var01 const reflectorStatic                            \
      *classname::__rfPtrStatic = &__##classname##_statical;                   \
  _REFLECTOR_SUBCLASS_##var01(classname);                                      \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_INIT_BODY(                            \
      classname,                                                               \
      _LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01, _REFLECTOR_INIT_ELEMENT),      \
      __VA_ARGS__)

#define _REFLECTOR_START_VER2(classname, var01, var02, ...)                    \
  _REFLECTOR_TYPES(                                                            \
      classname,                                                               \
      _LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02,    \
                   _REFLECTOR_ADDN),                                           \
      __VA_ARGS__)                                                             \
  _REFLECTOR_NAMES_##var01(classname, __VA_ARGS__);                            \
  _REFLECTOR_NAMES_##var02(classname, __VA_ARGS__);                            \
  _REFLECTOR_MAIN_BODY(classname,                                              \
                       _REFLECTOR_NAMES_DEF_##var01(classname)                 \
                           _REFLECTOR_NAMES_DEF_##var02(classname),            \
                       __VA_ARGS__)                                            \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 const                \
      reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;   \
  _REFLECTOR_SUBCLASS_##var01(classname)                                       \
      _REFLECTOR_SUBCLASS_##var02(classname);                                  \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02                      \
      _REFLECTOR_INIT_BODY(                                                    \
          classname,                                                           \
          _LOOPER_CAT2(                                                        \
              _REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02,         \
              _REFLECTOR_INIT_ELEMENT),                                        \
          __VA_ARGS__)

#define _REFLECTOR_START_VER3(classname, var01, var02, var03, ...)             \
  _REFLECTOR_TYPES(                                                            \
      classname,                                                               \
      _LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02     \
                       _REFLECTOR_EXTENDED_##var03,                            \
                   _REFLECTOR_ADDN),                                           \
      __VA_ARGS__);                                                            \
  _REFLECTOR_NAMES_##var01(classname, __VA_ARGS__);                            \
  _REFLECTOR_NAMES_##var02(classname, __VA_ARGS__);                            \
  _REFLECTOR_NAMES_##var03(classname, __VA_ARGS__);                            \
  _REFLECTOR_MAIN_BODY(classname,                                              \
                       _REFLECTOR_NAMES_DEF_##var01(classname)                 \
                           _REFLECTOR_NAMES_DEF_##var02(classname)             \
                               _REFLECTOR_NAMES_DEF_##var03(classname),        \
                       __VA_ARGS__)                                            \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02                      \
      _REFLECTOR_TEMPLATE_##var03 const reflectorStatic                        \
          *classname::__rfPtrStatic = &__##classname##_statical;               \
  _REFLECTOR_SUBCLASS_##var01 _REFLECTOR_SUBCLASS_##var02                      \
      _REFLECTOR_SUBCLASS_##var03(classname);                                  \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02                      \
      _REFLECTOR_TEMPLATE_##var03 _REFLECTOR_INIT_BODY(                        \
          classname,                                                           \
          _LOOPER_CAT2(_REFLECTOR_EXTENDED_##var01 _REFLECTOR_EXTENDED_##var02 \
                           _REFLECTOR_EXTENDED_##var03,                        \
                       _REFLECTOR_INIT_ELEMENT) __VA_ARGS__)

template <class C> struct _SubReflClassWrap {
  static constexpr JenHash HASH = 0;
};
