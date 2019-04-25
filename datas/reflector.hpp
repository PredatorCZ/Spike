/*	Contains macros/classes for reflection of classes/enums
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
#include "supercore.hpp"
#include "jenkinshash.hpp"
#include <string>
#include <cstring>
#include "macroLoop.hpp"

#define _REFLECTOR_ADDN_ENUM(value) #value,
#define _REFLECTOR_ADDN_ENUMVAL(value) value,

#define REFLECTOR_ENUM_INCLASS(classname,...) static const int classname##_reflectedSize = VA_NARGS(__VA_ARGS__);\
const char * classname##_reflected[classname##_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
enum classname { StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) };

template<class E> struct _EnumWrap 
{
	static const JenHash HASH = 0;
};

#define REFLECTOR_ENUM(classname, ...) namespace _##classname { enum classname{ StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) }; };\
typedef _##classname::classname classname; \
template<> struct _EnumWrap<classname> { \
static const int _reflectedSize = VA_NARGS(__VA_ARGS__);\
const char *_reflected[_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
static const JenHash HASH = JenkinsHash(#classname, sizeof(#classname) - 1);\
};

#define REFLECTOR_ENUM_NAKED(classname, ...) enum classname{ StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) };\
template<> struct _EnumWrap<classname> { \
static const int _reflectedSize = VA_NARGS(__VA_ARGS__);\
const char *_reflected[_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
static const JenHash HASH = JenkinsHash(#classname, sizeof(#classname) - 1);\
};

template<class C> struct _SubReflClassWrap 
{
	static const JenHash HASH = 0;
};

#define _REFLECTOR_ADDN(classname, _id, value) reflType { \
_getType<std::remove_reference<decltype(classname::value)>::type>::SUBSIZE, _id, \
_getType<std::remove_reference<decltype(classname::value)>::type>::NUMITEMS, \
offsetof(classname,value), JenkinsHash(#value, sizeof(#value) -1),\
_getType<std::remove_reference<decltype(classname::value)>::type>::TYPE , \
_getType<std::remove_reference<decltype(classname::value)>::type>::HASH},

#define REFLECTOR_START(classname,...) static const reflType __##classname##_types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
static const reflectorStatic __##classname##_statical = {  VA_NARGS(__VA_ARGS__), __##classname##_types, nullptr, nullptr, JenkinsHash(#classname, sizeof(#classname) - 1) };\
const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;

#define REFLECTOR_START_WNAMES(classname,...) static const reflType __##classname##_types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
static const char *__##classname##_typeNames[] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) };\
static const reflectorStatic __##classname##_statical = { \
	VA_NARGS(__VA_ARGS__), __##classname##_types, __##classname##_typeNames, \
	#classname, JenkinsHash(#classname, sizeof(#classname) - 1) };\
const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;\
template<> struct _SubReflClassWrap<classname> { static const JenHash HASH = JenkinsHash(#classname, sizeof(#classname) - 1); };

#define DECLARE_REFLECTOR static const reflectorStatic *__rfPtrStatic;\
const reflectorInstanceConst _rfRetreive() const { return { __rfPtrStatic, this }; }\
const reflectorInstance _rfRetreive() { return { __rfPtrStatic, this }; }

namespace pugi
{
	class xml_node;
	struct xml_node_struct;
}

struct reflType
{
	uchar subSize;
	uchar ID;
	ushort numItems;
	ushort offset;
	JenHash valueNameHash;
	JenHash typeHash;
	JenHash subtypeHash;
};
const int __sizeof_RelfType = sizeof(reflType);

struct reflectorStatic
{
	const int nTypes;
	const reflType *types;
	const char *const *typeNames;
	const char *className;
	const JenHash classHash;
};

static const reflectorStatic __null_statical = { 0, 0, 0, 0, 0 };

struct reflectorInstance
{
	const reflectorStatic *rfStatic;
	void *rfInstance;
};

struct reflectorInstanceConst
{
	const reflectorStatic *rfStatic;
	const void *rfInstance;
};

class Reflector
{
	virtual const reflectorInstanceConst _rfRetreive() const { return { &__null_statical, nullptr }; }
	virtual const reflectorInstance _rfRetreive() { return { &__null_statical, nullptr }; }
protected:
	const reflType *GetReflectedType(const JenHash hash) const;
	const reflType *GetReflectedType(const char *name) const;
public:
	struct KVPair
	{
		const char *name;
		std::string value;
	};

	struct SubClass
	{
		reflectorInstance inst;
		reflectorInstanceConst instc;
	};

	typedef pugi::xml_node_struct *xmlNodePtr;

	void SetReflectedValue(const char *name, const char *value);
	void SetReflectedValue(const JenHash hash, const char *value);

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
	
	xmlNodePtr ToXML(const TSTRING filename, bool asNewNode = true) const;
	xmlNodePtr ToXML(pugi::xml_node &node, bool asNewNode = true) const;
	xmlNodePtr FromXML(const TSTRING filename, bool lookupClassNode = true);
	xmlNodePtr FromXML(pugi::xml_node &node, bool lookupClassNode = true);
};

template<class C>
class ReflectorWrap : public Reflector
{
	const reflectorInstanceConst _rfRetreive() const { return static_cast<const C*>(data)->_rfRetreive(); }
	const reflectorInstance _rfRetreive() { return data->_rfRetreive(); }
public:
	C *data;
	ReflectorWrap(C *_data) : data(_data) {}
};

template<class C>
class ReflectorWrapConst : public Reflector
{
	const reflectorInstanceConst _rfRetreive() const { return data->_rfRetreive(); }
public:
	const C *data;
	ReflectorWrapConst(const C *_data) : data(_data) {}
};

class ReflectorSubClass : public Reflector
{
	const reflectorInstanceConst _rfRetreive() const { return data.instc; }
	const reflectorInstance _rfRetreive() { return data.inst; }
public:
	SubClass data;
	ReflectorSubClass(const SubClass &_data) : data(_data) {}
};

template <typename _Ty> struct _getType 
{
	template<typename U, const reflectorInstance (U:: *f)()> struct detectorClass {};
	template<class C> static constexpr std::false_type detectorFunc(...);
	template<class C> static constexpr std::true_type detectorFunc(detectorClass<C, &C::_rfRetreive> *);

	static constexpr bool subReflected = decltype(detectorFunc<_Ty>(nullptr))::value;

	static const char TYPE = static_cast<const char>(std::is_enum<_Ty>::value ? 13 : (subReflected ? 21 : 0));
	static const JenHash HASH =  _EnumWrap<_Ty>::HASH + _SubReflClassWrap<_Ty>::HASH;
	static const uchar SUBSIZE = 0;
	static const ushort NUMITEMS = 1;
};
template <> struct _getType<bool> {
	static const char TYPE = 1; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<char> {
	static const char TYPE = 2; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<signed char> {
	static const char TYPE = 2; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<uchar> {
	static const char TYPE = 3; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<wchar_t> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<char16_t> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<short> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<ushort> {
	static const char TYPE = 5; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<char32_t> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<int> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<uint> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<long> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<unsigned long> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<int64> {
	static const char TYPE = 8; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<uint64> {
	static const char TYPE = 9; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<float> {
	static const char TYPE = 10; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<double> {
	static const char TYPE = 11; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};
template <> struct _getType<long double> {
	static const char TYPE = 11; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<const char*> {
	static const char TYPE = 18; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <> struct _getType<std::string> {
	static const char TYPE = 19; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0; static const ushort NUMITEMS = 1;
};

template <class C, size_t _Size> struct _getType<C[_Size]> {
	static const char TYPE = 20; static const JenHash HASH = _getType<C>::TYPE; static const uchar SUBSIZE = sizeof(C); static const ushort NUMITEMS = _Size;
};

#ifdef _ARRAY_
template <class C, size_t _Size> struct _getType<std::array<C, _Size>> {
	static const char TYPE = 20; static const JenHash HASH = _getType<C>::TYPE; static const uchar SUBSIZE = sizeof(C); static const ushort NUMITEMS = _Size;
};
#endif

ES_INLINE const reflType *Reflector::GetReflectedType(const char *name) const
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return GetReflectedType(hash);
}

ES_INLINE void Reflector::SetReflectedValue(const char *name, const char *value)
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return SetReflectedValue(hash, value);
}

ES_INLINE std::string Reflector::GetReflectedValue(const char *name) const
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return GetReflectedValue(hash);
}

ES_INLINE int Reflector::GetNumReflectedValues() const { return _rfRetreive().rfStatic->nTypes; }

ES_INLINE bool Reflector::UseNames() const { return _rfRetreive().rfStatic->typeNames != nullptr; }

ES_INLINE std::string Reflector::GetReflectedValue(JenHash hash) const
{
	const reflType *found = GetReflectedType(hash);

	if (!found)
		return "";

	return GetReflectedValue(found->ID);
}

ES_INLINE const Reflector::SubClass Reflector::GetReflectedSubClass(const char *name) const
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return GetReflectedSubClass(hash);
}

ES_INLINE const Reflector::SubClass Reflector::GetReflectedSubClass(JenHash hash) const
{
	const reflType *found = GetReflectedType(hash);

	if (!found)
		return {};

	return GetReflectedSubClass(found->ID);
}

ES_INLINE const Reflector::SubClass Reflector::GetReflectedSubClass(const char *name)
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return GetReflectedSubClass(hash);
}

ES_INLINE const Reflector::SubClass Reflector::GetReflectedSubClass(JenHash hash)
{
	const reflType *found = GetReflectedType(hash);

	if (!found)
		return {};

	return GetReflectedSubClass(found->ID);
}

ES_INLINE Reflector::KVPair Reflector::GetReflectedPair(int id) const
{
	if (id >= GetNumReflectedValues())
		return KVPair{};

	return KVPair{ UseNames() ? _rfRetreive().rfStatic->typeNames[id] : nullptr, GetReflectedValue(id) };
}

ES_INLINE Reflector::KVPair Reflector::GetReflectedPair(JenHash hash) const
{
	const reflType *found = GetReflectedType(hash);

	if (!found)
		return KVPair{};

	return KVPair{ UseNames() ? _rfRetreive().rfStatic->typeNames[found->ID] : nullptr, GetReflectedValue(found->ID) };
}

ES_INLINE const char *Reflector::GetClassName() const { return _rfRetreive().rfStatic->className; }

ES_INLINE bool Reflector::IsReflectedSubClass(const char *name) const
{
	const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
	return IsReflectedSubClass(hash);
}

ES_INLINE bool Reflector::IsReflectedSubClass(JenHash hash) const
{
	const reflType *found = GetReflectedType(hash);

	if (!found)
		return false;

	return IsReflectedSubClass(found->ID);
}

ES_INLINE bool Reflector::IsReflectedSubClass(int ID) const
{
	if (ID >= GetNumReflectedValues())
		return false;

	const reflType fl = _rfRetreive().rfStatic->types[ID];

	return fl.typeHash == 21;
}

#endif