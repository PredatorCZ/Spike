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
#include "macroLoop.hpp"

#define _REFLECTOR_ADDN_ENUM(value) #value,
#define _REFLECTOR_ADDN_ENUMVAL(value) value,

#define REFLECTOR_ENUM_INCLASS(classname,...) static const int classname##_reflectedSize = VA_NARGS(__VA_ARGS__);\
const char * classname##_reflected[classname##_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
enum classname { StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) };

template<class E>struct _EnumWrap {};

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

#define _REFLECTOR_ADDN(classname, _id, value) reflType { sizeof(classname::value), _getType<decltype(classname::value)>::SUBSIZE, _id, offsetof(classname,value), JenkinsHash(#value, sizeof(#value) -1),\
_getType<decltype(classname::value)>::TYPE , _getType<decltype(classname::value)>::HASH},

#define REFLECTOR_START(classname,...) static const reflType __##classname##_types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
static const reflectorStatic __##classname##_statical = {  VA_NARGS(__VA_ARGS__), __##classname##_types, nullptr, nullptr, JenkinsHash(#classname, sizeof(#classname) - 1) };\
const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;

#define REFLECTOR_START_WNAMES(classname,...) static const reflType __##classname##_types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
static const char *__##classname##_typeNames[] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) };\
static const reflectorStatic __##classname##_statical = { \
	VA_NARGS(__VA_ARGS__), __##classname##_types, __##classname##_typeNames, \
	#classname, JenkinsHash(#classname, sizeof(#classname) - 1) };\
const reflectorStatic *classname::__rfPtrStatic = &__##classname##_statical;

#define DECLARE_REFLECTOR static const reflectorStatic *__rfPtrStatic;\
const reflectorInstanceConst _rfRetreive() const { return { __rfPtrStatic, this }; }\
const reflectorInstance _rfRetreive() { return { __rfPtrStatic, this }; }

namespace pugi
{
	class xml_node;
}

struct reflType
{
	uchar size : 4, subSize : 4;
	uchar ID;
	ushort offset;
	JenHash valueNameHash;
	JenHash typeHash;
	JenHash subtypeHash;
};

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
	ES_INLINE const reflType *GetReflectedType(const char *name) const
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return GetReflectedType(hash);
	}
public:
	struct KVPair
	{
		const char *name;
		std::string value;
	};

	ES_INLINE void SetReflectedValue(const char *name, const char *value)
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return SetReflectedValue(hash, value);
	}
	ES_INLINE std::string GetReflectedValue(const char *name) const
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return GetReflectedValue(hash);
	}

	void SetReflectedValue(const JenHash hash, const char *value);
	ES_INLINE int GetNumReflectedValues() const { return _rfRetreive().rfStatic->nTypes; }
	ES_INLINE bool UseNames() const { return _rfRetreive().rfStatic->typeNames != nullptr; }
	
	std::string GetReflectedValue(int id) const;
	ES_INLINE std::string GetReflectedValue(JenHash hash) const
	{
		const reflType *found = GetReflectedType(hash);

		if (!found)
			return "";

		return GetReflectedValue(found->ID);
	}

	ES_INLINE KVPair GetReflectedPair(int id) const
	{ 
		if (id >= GetNumReflectedValues())
			return KVPair{};

		return KVPair{ UseNames() ? _rfRetreive().rfStatic->typeNames[id] : nullptr, GetReflectedValue(id) };
	};
	ES_INLINE KVPair GetReflectedPair(JenHash hash) const
	{
		const reflType *found = GetReflectedType(hash);

		if (!found)
			return KVPair{};

		return KVPair{ UseNames() ? _rfRetreive().rfStatic->typeNames[found->ID] : nullptr, GetReflectedValue(found->ID) };
	};

	ES_INLINE const char *GetClassName() const { return _rfRetreive().rfStatic->className; }

	int ToXML(const TSTRING filename) const;
	int ToXML(pugi::xml_node &node) const;
	int FromXML(const TSTRING filename);
	int FromXML(pugi::xml_node &node);
};

template <typename _Ty> struct _getType 
{
	static const char TYPE = static_cast<const char>(std::is_enum<_Ty>::value ? 13 : 0);
	static const JenHash HASH = std::is_enum<_Ty>::value ? _EnumWrap<_Ty>::HASH : 0;
	static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<bool> {
	static const char TYPE = 1; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<char> {
	static const char TYPE = 2; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<signed char> {
	static const char TYPE = 2; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<uchar> {
	static const char TYPE = 3; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<wchar_t> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<char16_t> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<short> {
	static const char TYPE = 4; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<ushort> {
	static const char TYPE = 5; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<char32_t> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<int> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<uint> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<long> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<unsigned long> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<int64> {
	static const char TYPE = 8; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<uint64> {
	static const char TYPE = 9; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<float> {
	static const char TYPE = 10; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<double> {
	static const char TYPE = 11; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<long double> {
	static const char TYPE = 11; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<const char*> {
	static const char TYPE = 18; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<std::string> {
	static const char TYPE = 19; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
#endif