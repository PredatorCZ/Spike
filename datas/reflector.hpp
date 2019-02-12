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

#define REFLECTOR_ENUM(classname, classHash, ...) namespace _##classname { enum classname{ StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) }; };\
typedef _##classname::classname classname; \
template<> struct _EnumWrap<classname> { \
static const int _reflectedSize = VA_NARGS(__VA_ARGS__);\
const char *_reflected[_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
static const JenHash HASH = classHash;\
};

#define REFLECTOR_ENUM_NAKED(classname, classHash, ...) enum classname{ StaticFor(_REFLECTOR_ADDN_ENUMVAL, __VA_ARGS__) };\
template<> struct _EnumWrap<classname> { \
static const int _reflectedSize = VA_NARGS(__VA_ARGS__);\
const char *_reflected[_reflectedSize] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) }; \
static const JenHash HASH = classHash;\
};

#define _REFLECTOR_ADDN(classname, _id, value) reflType { sizeof(classname::value), _getType<decltype(classname::value)>::SUBSIZE, _id, offsetof(classname,value), JenkinsHash(#value, sizeof(#value) -1),\
_getType<decltype(classname::value)>::TYPE , _getType<decltype(classname::value)>::HASH},

#define REFLECTOR_START(classname,...) const reflType classname::types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
const int classname::nTypes = sizeof(classname::types) / sizeof(reflType);

#define REFLECTOR_START_WNAMES(classname,...) const reflType classname::types[] = { StaticForArgID(_REFLECTOR_ADDN, classname, __VA_ARGS__) }; \
const int classname::nTypes = (sizeof(classname::types) / sizeof(reflType)) | 0x80000000;\
const char * classname::typeNames[] = { StaticFor(_REFLECTOR_ADDN_ENUM, __VA_ARGS__) };

struct reflType
{
	uchar size : 4, subSize : 4;
	uchar ID;
	ushort offset;
	JenHash valueNameHash;
	JenHash typeHash;
	JenHash subtypeHash;
};

class IReflector
{
public:
	struct KVPair
	{
		const char *name;
		std::string value;
	};

	virtual void SetReflectedValue(const JenHash hash, const char *value) = 0;
	ES_INLINE void SetReflectedValue(const char *name, const char *value)
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return SetReflectedValue(hash, value);
	}
	virtual int GetNumReflectedValues() const = 0;
	virtual std::string GetReflectedValue(int id) const = 0;
	virtual std::string GetReflectedValue(JenHash hash) const = 0;
	ES_INLINE std::string GetReflectedValue(const char *name) const
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return GetReflectedValue(hash);
	}
	virtual KVPair GetReflectedPair(int id) const = 0;
	virtual KVPair GetReflectedPair(JenHash hash) const = 0;

};

class Reflector : public IReflector
{
protected:
	const reflType *_types;
	const char **_typeNames;
	char *thisAddr;
	int _nTypes;

	const reflType *GetReflectedType(const JenHash hash) const;
	ES_INLINE const reflType *GetReflectedType(const char *name) const
	{
		const JenHash hash = JenkinsHash(name, static_cast<JenHash>(strlen(name)));
		return GetReflectedType(hash);
	}
public:

	using IReflector::GetReflectedValue;
	using IReflector::SetReflectedValue;

	void SetReflectedValue(const JenHash hash, const char *value);
	int GetNumReflectedValues() const { return _nTypes & 0x7fffffff; }
	ES_INLINE bool UseNames() const { return (_nTypes & 0x80000000) != 0; }
	
	std::string GetReflectedValue(int id) const;
	ES_INLINE std::string GetReflectedValue(JenHash hash) const
	{
		const reflType *found = GetReflectedType(hash);

		if (!found)
			return "";

		return GetReflectedValue(found->ID);
	}

	KVPair GetReflectedPair(int id) const 
	{ 
		if (id >= GetNumReflectedValues())
			return KVPair{};

		return KVPair{ UseNames() ? _typeNames[id] : nullptr, GetReflectedValue(id) };
	};
	KVPair GetReflectedPair(JenHash hash) const 
	{
		const reflType *found = GetReflectedType(hash);

		if (!found)
			return KVPair{};

		return KVPair{ UseNames() ? _typeNames[found->ID] : nullptr, GetReflectedValue(found->ID) };
	};
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
template <> struct _getType<unsigned char> {
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
template <> struct _getType<unsigned short> {
	static const char TYPE = 5; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<char32_t> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<int> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<unsigned int> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<long> {
	static const char TYPE = 6; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<unsigned long> {
	static const char TYPE = 7; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};

template <> struct _getType<_LONGLONG> {
	static const char TYPE = 8; static const JenHash HASH = 0; static const unsigned char SUBSIZE = 0;
};
template <> struct _getType<_ULONGLONG> {
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


#define DECLARE_REFLECTOR_STATIC static const int nTypes; static const reflType types[];

#define DECLARE_REFLECTOR_WNAMES_STATIC DECLARE_REFLECTOR_STATIC static const char *typeNames[];

#define DECLARE_REFLECTOR_WNAMES DECLARE_REFLECTOR_WNAMES_STATIC \
ES_INLINE void ConstructReflection(){_nTypes = nTypes;_typeNames = typeNames;_types = types; thisAddr = reinterpret_cast<char*>(this);}

#define DECLARE_REFLECTOR DECLARE_REFLECTOR_STATIC \
ES_INLINE void ConstructReflection(){_nTypes = nTypes;_types = types; thisAddr = reinterpret_cast<char*>(this);}

#define DECLARE_REFLECTOR_EMPTY ES_INLINE void ConstructReflection() {}

#endif