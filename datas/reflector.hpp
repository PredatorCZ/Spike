/*      Contains macros/classes for reflection of classes/enums
        more info in README for PreCore Project

        Copyright 2018-2019 Lukas Cone

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        -- April, 2019
            - REFLECTOR_ENUM macros no longer require eplicit hash
            - removed all DECLARE_REFLECTOR macros, exept this one
*/

#ifndef ES_REFLECTOR_DEFINED
#define ES_REFLECTOR_DEFINED
#include "jenkinshash.hpp"
#include "macroLoop.hpp"
#include "supercore.hpp"
#include <cstring>
#include <string>

constexpr size_t _GetReflEnumItemSize(const char *value, size_t curIndex = 0) {
  return (value[curIndex] == '=' || value[curIndex] == 0)
             ? curIndex - (value[curIndex] == '=' ? 1 : 0)
             : _GetReflEnumItemSize(value, curIndex + 1);
}

#define _REFLECTOR_ADDN_ENUM(value) #value,
#define _REFLECTOR_ADDN_ENUMSIZE(value)                                        \
  static_cast<uchar>(_GetReflEnumItemSize(#value)),
#define _REFLECTOR_ADDN_ENUMVAL(value) value,
#define _REFLECTOR_ADDN_ENUMDUMMY(value) 0,

#define _REFLECTOR_ENUM_CLASS_CLASS class
#define _REFLECTOR_ENUM_CLASS_EXTERN
#define _REFLECTOR_ENUM_CLASS_64
#define _REFLECTOR_ENUM_CLASS_32
#define _REFLECTOR_ENUM_CLASS_16
#define _REFLECTOR_ENUM_CLASS_8

#define _REFLECTOR_ENUM_NOREG_EXTERN(classname)                                \
  static const int classname##_reflectedSize =                                 \
      _EnumWrap<classname>::_reflectedSize;                                    \
  static const char **classname##_reflected = _EnumWrap<classname>{}._reflected;
#define _REFLECTOR_ENUM_NOREG_CLASS(classname)
#define _REFLECTOR_ENUM_NOREG_64(classname)
#define _REFLECTOR_ENUM_NOREG_32(classname)
#define _REFLECTOR_ENUM_NOREG_16(classname)
#define _REFLECTOR_ENUM_NOREG_8(classname)

#define _REFLECTOR_ENUM_SIZE_64 : uint64
#define _REFLECTOR_ENUM_SIZE_32 : uint
#define _REFLECTOR_ENUM_SIZE_16 : ushort
#define _REFLECTOR_ENUM_SIZE_8 : uchar
#define _REFLECTOR_ENUM_SIZE_CLASS
#define _REFLECTOR_ENUM_SIZE_EXTERN

#define _REFLECTOR_ENUM_MAIN_BODY(classname, ...)                              \
  {StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__)};                           \
  template <> struct _EnumWrap<classname> {                                    \
    static const int _reflectedSize = VA_NARGS(__VA_ARGS__);                   \
    const char *_reflected[_reflectedSize] = {                                 \
        StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__)};                         \
    const uchar _reflectedSizes[_reflectedSize] = {                            \
        StaticFor(_REFLECTOR_ADDN_ENUMSIZE, __VA_ARGS__)};                     \
    static uint64 _reflectedValues[_reflectedSize];                            \
    static const JenHash HASH =                                                \
        JenkinsHash(#classname, sizeof(#classname) - 1);                       \
  };

#define _REFLECTOR_ENUM_VER0(classname, ...)                                   \
  enum classname _REFLECTOR_ENUM_MAIN_BODY(classname, __VA_ARGS__);

#define _REFLECTOR_ENUM_VER1(classname, var01, ...)                            \
  enum _REFLECTOR_ENUM_CLASS_##var01 classname                                 \
      _REFLECTOR_ENUM_SIZE_##var01 _REFLECTOR_ENUM_MAIN_BODY(classname,        \
                                                             __VA_ARGS__);     \
  _REFLECTOR_ENUM_NOREG_##var01(classname)

#define _REFLECTOR_ENUM_VER2(classname, var01, var02, ...)                     \
  enum _REFLECTOR_ENUM_CLASS_##var01 _REFLECTOR_ENUM_CLASS_##var02 classname   \
      _REFLECTOR_ENUM_SIZE_##var01 _REFLECTOR_ENUM_SIZE_##var02                \
          _REFLECTOR_ENUM_MAIN_BODY(classname, __VA_ARGS__);                   \
  _REFLECTOR_ENUM_NOREG_##var01(classname);                                    \
  _REFLECTOR_ENUM_NOREG_##var02(classname);

#define _REFLECTOR_ENUM_VER3(classname, var01, var02, var03, ...)              \
  enum _REFLECTOR_ENUM_CLASS_##var01 _REFLECTOR_ENUM_CLASS_##var02             \
      _REFLECTOR_ENUM_CLASS_##var03 classname                                  \
          _REFLECTOR_ENUM_SIZE_##var01 _REFLECTOR_ENUM_SIZE_##var02            \
              _REFLECTOR_ENUM_SIZE_##var03 _REFLECTOR_ENUM_MAIN_BODY(          \
                  classname, __VA_ARGS__);                                     \
  _REFLECTOR_ENUM_NOREG_##var01(classname);                                    \
  _REFLECTOR_ENUM_NOREG_##var02(classname);                                    \
  _REFLECTOR_ENUM_NOREG_##var03(classname);

#define _REFLECTOR_START_VERENUM(classname, numFlags, ...)                     \
  VA_NARGS_EVAL(_REFLECTOR_ENUM_VER##numFlags(classname, __VA_ARGS__))

template <class E> struct _EnumWrap { static const JenHash HASH = 0; };

template <class C> struct _SubReflClassWrap { static const JenHash HASH = 0; };

struct reflType {
  uchar type;            // type of main element
  uchar subType;         // type of sub elements (array item type)
  uchar subSize;         // size if sub element
  uchar ID;              // index of main element within master table
  ushort numItems;       // number of sub elements
  ushort offset;         // offset of main element
  JenHash valueNameHash; // hash of main element's name
  JenHash typeHash;      // lookup hash of main/sub element (enum, sublass)
};
const int __sizeof_RelfType = sizeof(reflType);

#define _REFLECTOR_ADDN(classname, _id, value)                                 \
  reflType{                                                                    \
      _getType<std::remove_reference<decltype(classname::value)>::type>::TYPE, \
      _getType<                                                                \
          std::remove_reference<decltype(classname::value)>::type>::SUBTYPE,   \
      _getType<                                                                \
          std::remove_reference<decltype(classname::value)>::type>::SUBSIZE,   \
      _id,                                                                     \
      _getType<                                                                \
          std::remove_reference<decltype(classname::value)>::type>::NUMITEMS,  \
      static_cast<ushort>(offsetof(classname, value)),                         \
      JenkinsHash(#value, sizeof(#value) - 1),                                 \
      _getType<                                                                \
          std::remove_reference<decltype(classname::value)>::type>::HASH},

#define _REFLECTOR_TYPES(classname, ...)                                       \
  static const reflType __##classname##_types[] = {                            \
      StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__)};

#define _REFLECTOR_MAIN_BODY(classname, namesbody, ...)                        \
  static const reflectorStatic __##classname##_statical = {                    \
      VA_NARGS(__VA_ARGS__), __##classname##_types,                            \
      namesbody JenkinsHash(#classname, sizeof(#classname) - 1)};

#define _REFLECTOR_NAMES_VARNAMES(classname, ...)                              \
  static const char *__##classname##_typeNames[] = {                           \
      StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__)};

#define _REFLECTOR_NAMES_TEMPLATE(...)
#define _REFLECTOR_NAMES_SUBCLASS(...)

#define _REFLECTOR_NAMES_DEF_VARNAMES(classname)                               \
  __##classname##_typeNames, #classname,
#define _REFLECTOR_NAMES_DEF_TEMPLATE(classname)
#define _REFLECTOR_NAMES_DEF_SUBCLASS(classname)

#define _REFLECTOR_TEMPLATE_TEMPLATE template <>
#define _REFLECTOR_TEMPLATE_SUBCLASS
#define _REFLECTOR_TEMPLATE_VARNAMES

#define _REFLECTOR_SUBCLASS_SUBCLASS(classname)                                \
  template <> struct _SubReflClassWrap<classname> {                            \
    static const JenHash HASH =                                                \
        JenkinsHash(#classname, sizeof(#classname) - 1);                       \
  };
#define _REFLECTOR_SUBCLASS_TEMPLATE(classname)
#define _REFLECTOR_SUBCLASS_VARNAMES(classname)

#define _REFLECTOR_START_VER0(classname, ...)                                  \
  _REFLECTOR_TYPES(classname, __VA_ARGS__)                                     \
  _REFLECTOR_MAIN_BODY(classname, _REFLECTOR_NAMES_DEF_SUBCLASS(classname),    \
                       __VA_ARGS__)                                            \
  const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;

#define _REFLECTOR_START_VER1(classname, var01, ...)                           \
  _REFLECTOR_TYPES(classname, __VA_ARGS__)                                     \
  _REFLECTOR_NAMES_##var01(classname, __VA_ARGS__);                            \
  _REFLECTOR_MAIN_BODY(classname, _REFLECTOR_NAMES_DEF_##var01(classname),     \
                       __VA_ARGS__)                                            \
  _REFLECTOR_TEMPLATE_##var01 const reflectorStatic                            \
      *classname::__rfPtrStatic = &__##classname##_statical;                   \
  _REFLECTOR_SUBCLASS_##var01(classname)

#define _REFLECTOR_START_VER2(classname, var01, var02, ...)                    \
  _REFLECTOR_TYPES(classname, __VA_ARGS__)                                     \
  _REFLECTOR_NAMES_##var01(classname, __VA_ARGS__);                            \
  _REFLECTOR_NAMES_##var02(classname, __VA_ARGS__);                            \
  _REFLECTOR_MAIN_BODY(classname,                                              \
                       _REFLECTOR_NAMES_DEF_##var01(classname)                 \
                           _REFLECTOR_NAMES_DEF_##var02(classname),            \
                       __VA_ARGS__)                                            \
  _REFLECTOR_TEMPLATE_##var01 _REFLECTOR_TEMPLATE_##var02 const                \
      reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;   \
  _REFLECTOR_SUBCLASS_##var01(classname) _REFLECTOR_SUBCLASS_##var02(classname)

#define _REFLECTOR_START_VER3(classname, var01, var02, var03, ...)             \
  _REFLECTOR_TYPES(classname, __VA_ARGS__);                                    \
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
      _REFLECTOR_SUBCLASS_##var03(classname)

// Usable flags: VARNAMES, TEMPLATE, SUBCLASS
// Usable enum flags: CLASS, EXTERN, size (64B or 32B or 16B or 8B)
//@numFlags: [0,n] or ENUM, numEnumFlags
#define REFLECTOR_CREATE(classname, numFlags, ...)                             \
  VA_NARGS_EVAL(_REFLECTOR_START_VER##numFlags(classname, __VA_ARGS__))

// Deprecated! Use REFLECTOR_CREATE instead.
#define REFLECTOR_START(classname, ...)                                        \
  REFLECTOR_CREATE(classname, 0, __VA_ARGS__)                                  \
  ES_PRAGMA(message(__FILE__ " REFLECTOR_START is deprecated! Use "            \
                             "REFLECTOR_CREATE instead."))

// Deprecated! Use REFLECTOR_CREATE instead.
#define REFLECTOR_START_WNAMES(classname, ...)                                 \
  REFLECTOR_CREATE(classname, 1, VARNAMES, __VA_ARGS__)                        \
  ES_PRAGMA(message(__FILE__ " REFLECTOR_START_WNAMES is deprecated! Use "     \
                             "REFLECTOR_CREATE instead."))

// Deprecated! Use REFLECTOR_CREATE instead.
#define REFLECTOR_ENUM(classname, ...)                                         \
  _REFLECTOR_START_VERENUM(classname, 1, CLASS, __VA_ARGS__)                   \
  ES_PRAGMA(message(__FILE__ " REFLECTOR_ENUM is deprecated! Use "             \
                             "REFLECTOR_CREATE instead."))

// Deprecated! Use REFLECTOR_CREATE instead.
#define REFLECTOR_ENUM_NAKED(classname, ...)                                   \
  _REFLECTOR_START_VERENUM(classname, 0, __VA_ARGS__)                          \
  ES_PRAGMA(message(__FILE__ " REFLECTOR_ENUM_NAKED is deprecated! Use "       \
                             "REFLECTOR_CREATE instead."))

#define DECLARE_REFLECTOR                                                      \
  static const reflectorStatic *__rfPtrStatic;                                 \
  const reflectorInstanceConst _rfRetreive() const {                           \
    return {__rfPtrStatic, this};                                              \
  }                                                                            \
  const reflectorInstance _rfRetreive() { return {__rfPtrStatic, this}; }

namespace pugi {
class xml_node;
struct xml_node_struct;
} // namespace pugi

struct reflectorStatic {
  const int nTypes;
  const reflType *types;
  const char *const *typeNames;
  const char *className;
  const JenHash classHash;

  reflectorStatic(const int _nTypes, const reflType *_types,
                  const char *const *_typeNames, const char *_className,
                  const JenHash _classHash)
      : nTypes(_nTypes), types(_types), typeNames(_typeNames),
        className(_className), classHash(_classHash) {}

  reflectorStatic(const int _nTypes, const reflType *_types,
                  const JenHash _classHash)
      : nTypes(_nTypes), types(_types), typeNames(nullptr), className(nullptr),
        classHash(_classHash) {}
};

static const reflectorStatic __null_statical = {0, 0, 0, 0, 0};

struct reflectorInstance {
  const reflectorStatic *rfStatic;
  void *rfInstance;
};

struct reflectorInstanceConst {
  const reflectorStatic *rfStatic;
  const void *rfInstance;
};

class Reflector {
  virtual const reflectorInstanceConst _rfRetreive() const {
    return {&__null_statical, nullptr};
  }
  virtual const reflectorInstance _rfRetreive() {
    return {&__null_statical, nullptr};
  }

protected:
  const reflType *GetReflectedType(const JenHash hash) const;
  const reflType *GetReflectedType(const char *name) const;
  const reflType *GetReflectedType(int ID) const;

public:
  struct KVPair {
    const char *name;
    std::string value;
  };

  struct SubClass {
    reflectorInstance inst;
    reflectorInstanceConst instc;
  };

  typedef pugi::xml_node_struct *xmlNodePtr;

  void SetReflectedValue(const char *name, const char *value);
  void SetReflectedValue(const JenHash hash, const char *value);
  void SetReflectedValue(int id, const char *value);

  int GetNumReflectedValues() const;
  const char *GetClassName() const;
  bool UseNames() const;

  std::string GetReflectedValue(int id) const;
  std::string GetReflectedValue(JenHash hash) const;
  std::string GetReflectedValue(const char *name) const;

  bool IsReflectedSubClass(const char *name) const;
  bool IsReflectedSubClass(JenHash hash) const;
  bool IsReflectedSubClass(int id) const;

  const SubClass GetReflectedSubClass(const char *name) const;
  const SubClass GetReflectedSubClass(JenHash hash) const;
  const SubClass GetReflectedSubClass(int id) const;

  const SubClass GetReflectedSubClass(const char *name);
  const SubClass GetReflectedSubClass(JenHash hash);
  const SubClass GetReflectedSubClass(int id);

  KVPair GetReflectedPair(int id) const;
  KVPair GetReflectedPair(JenHash hash) const;

  xmlNodePtr ToXML(const char *filename, bool asNewNode = true) const {
    return _ToXML(filename, asNewNode);
  }
  xmlNodePtr ToXML(const wchar_t *filename, bool asNewNode = true) const {
    return _ToXML(filename, asNewNode);
  }
  template <class T>
  xmlNodePtr ToXML(const UniString<T> filename, bool asNewNode = true) const {
    return _ToXML(filename.c_str(), asNewNode);
  }
  xmlNodePtr ToXML(pugi::xml_node &node, bool asNewNode = true) const;
  xmlNodePtr FromXML(const char *filename, bool lookupClassNode = true) {
    return _FromXML(filename, lookupClassNode);
  }
  xmlNodePtr FromXML(const wchar_t *filename, bool lookupClassNode = true) {
    return _FromXML(filename, lookupClassNode);
  }
  template <class T>
  xmlNodePtr FromXML(const UniString<T> filename, bool lookupClassNode = true) {
    return _FromXML(filename.c_str(), lookupClassNode);
  }
  xmlNodePtr FromXML(pugi::xml_node &node, bool lookupClassNode = true);

private:
  template <class _Ty0>
  xmlNodePtr _ToXML(const _Ty0 *filename, bool asNewNode = true) const;
  template <class _Ty1>
  xmlNodePtr _FromXML(const _Ty1 *filename, bool lookupClassNode = true);
};

template <class C> class ReflectorWrap : public Reflector {
  const reflectorInstanceConst _rfRetreive() const {
    return static_cast<const C *>(data)->_rfRetreive();
  }
  const reflectorInstance _rfRetreive() { return data->_rfRetreive(); }

public:
  C *data;
  ReflectorWrap(C *_data) : data(_data) {}
};

template <class C> class ReflectorWrapConst : public Reflector {
  const reflectorInstanceConst _rfRetreive() const {
    return data->_rfRetreive();
  }

public:
  const C *data;
  ReflectorWrapConst(const C *_data) : data(_data) {}
};

class ReflectorSubClass : public Reflector {
  const reflectorInstanceConst _rfRetreive() const { return data.instc; }
  const reflectorInstance _rfRetreive() { return data.inst; }

public:
  SubClass data;
  ReflectorSubClass(const SubClass &_data) : data(_data) {}
};

template <typename _Ty> struct _getType {
  template <typename U, const reflectorInstance (U::*f)()>
  struct detectorClass {};
  template <class C> static constexpr std::false_type detectorFunc(...);
  template <class C>
  static constexpr std::true_type
  detectorFunc(detectorClass<C, &C::_rfRetreive> *);

  static constexpr bool subReflected =
      decltype(detectorFunc<_Ty>(nullptr))::value;

  static const char TYPE = static_cast<const char>(
      std::is_enum<_Ty>::value ? 13 : (subReflected ? 14 : 0));
  static const JenHash HASH =
      _EnumWrap<_Ty>::HASH + _SubReflClassWrap<_Ty>::HASH;
  static const JenHash SUBHASH = 0;
  static const uchar SUBSIZE = TYPE == 13 ? sizeof(_Ty) : 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<bool> {
  static const char TYPE = 1;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<char> {
  static const char TYPE = 2;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<signed char> {
  static const char TYPE = 2;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<uchar> {
  static const char TYPE = 3;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<wchar_t> {
  static const char TYPE = 4;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<char16_t> {
  static const char TYPE = 4;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<short> {
  static const char TYPE = 4;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<ushort> {
  static const char TYPE = 5;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<char32_t> {
  static const char TYPE = 6;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<int> {
  static const char TYPE = 6;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<uint> {
  static const char TYPE = 7;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<long> {
  static const char TYPE = 6;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<unsigned long> {
  static const char TYPE = 7;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<int64> {
  static const char TYPE = 8;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<uint64> {
  static const char TYPE = 9;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<float> {
  static const char TYPE = 10;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<double> {
  static const char TYPE = 11;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};
template <> struct _getType<long double> {
  static const char TYPE = 11;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<const char *> {
  static const char TYPE = 18;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <> struct _getType<std::string> {
  static const char TYPE = 19;
  static const JenHash HASH = 0;
  static const unsigned char SUBSIZE = 0;
  static const uchar SUBTYPE = 0;
  static const ushort NUMITEMS = 1;
};

template <class C, size_t _Size> struct _getType<C[_Size]> {
  static const char TYPE = 20;
  static const JenHash HASH = _getType<C>::HASH;
  static const uchar SUBSIZE = sizeof(C);
  static const uchar SUBTYPE = _getType<C>::TYPE;
  static const ushort NUMITEMS = _Size;
};

#ifdef _ARRAY_
template <class C, size_t _Size> struct _getType<std::array<C, _Size>> {
  static const char TYPE = 20;
  static const JenHash HASH = _getType<C>::HASH;
  static const uchar SUBSIZE = sizeof(C);
  static const uchar SUBTYPE = _getType<C>::TYPE;
  static const ushort NUMITEMS = _Size;
};
#endif

ES_INLINE const reflType *Reflector::GetReflectedType(const char *name) const {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return GetReflectedType(hash);
}

ES_INLINE const reflType *Reflector::GetReflectedType(int ID) const {
  const reflectorStatic *inst = _rfRetreive().rfStatic;
  return inst->types + ID;
}

ES_INLINE void Reflector::SetReflectedValue(const char *name,
                                            const char *value) {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return SetReflectedValue(hash, value);
}

ES_INLINE std::string Reflector::GetReflectedValue(const char *name) const {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return GetReflectedValue(hash);
}

ES_INLINE int Reflector::GetNumReflectedValues() const {
  return _rfRetreive().rfStatic->nTypes;
}

ES_INLINE bool Reflector::UseNames() const {
  return _rfRetreive().rfStatic->typeNames != nullptr;
}

ES_INLINE std::string Reflector::GetReflectedValue(JenHash hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return "";

  return GetReflectedValue(found->ID);
}

ES_INLINE const Reflector::SubClass
Reflector::GetReflectedSubClass(const char *name) const {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return GetReflectedSubClass(hash);
}

ES_INLINE const Reflector::SubClass
Reflector::GetReflectedSubClass(JenHash hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedSubClass(found->ID);
}

ES_INLINE const Reflector::SubClass
Reflector::GetReflectedSubClass(const char *name) {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return GetReflectedSubClass(hash);
}

ES_INLINE const Reflector::SubClass
Reflector::GetReflectedSubClass(JenHash hash) {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedSubClass(found->ID);
}

ES_INLINE Reflector::KVPair Reflector::GetReflectedPair(int id) const {
  if (id >= GetNumReflectedValues())
    return KVPair{};

  return KVPair{UseNames() ? _rfRetreive().rfStatic->typeNames[id] : nullptr,
                GetReflectedValue(id)};
}

ES_INLINE Reflector::KVPair Reflector::GetReflectedPair(JenHash hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return KVPair{};

  return KVPair{UseNames() ? _rfRetreive().rfStatic->typeNames[found->ID]
                           : nullptr,
                GetReflectedValue(found->ID)};
}

ES_INLINE const char *Reflector::GetClassName() const {
  return _rfRetreive().rfStatic->className;
}

ES_INLINE bool Reflector::IsReflectedSubClass(const char *name) const {
  const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
  return IsReflectedSubClass(hash);
}

ES_INLINE bool Reflector::IsReflectedSubClass(JenHash hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return false;

  return IsReflectedSubClass(found->ID);
}

ES_INLINE bool Reflector::IsReflectedSubClass(int ID) const {
  if (ID >= GetNumReflectedValues())
    return false;

  const reflType fl = _rfRetreive().rfStatic->types[ID];

  return fl.typeHash == 21;
}

#endif