/*	t_Flags class to store bit flags
	EnumFlags class to store bit flags by enum
	esEnum class is enum with explicit size
	more info in README for PreCore Project

	Copyright 2015-2019 Lukas Cone

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


#ifndef ES_FLAGS_DEFINED
#define ES_FLAGS_DEFINED
#include "supercore.hpp"
template <typename T> class t_Flags
{
	T Value;
public:
	typedef T ValueType;
	t_Flags(): Value(0){}
	t_Flags(T inval) { Value = inval; }
	ES_FORCEINLINE void operator=(T inval) { Value = inval; }
	ES_FORCEINLINE bool operator[](int pos) const { return (Value & (1 << pos)) != 0; }
	ES_FORCEINLINE void operator()(int pos, bool val) { val ? Value |= (1 << pos) : Value &= ~(1 << pos); }
};

template<class T, class E> class EnumFlags : public t_Flags<T>
{
public:
	EnumFlags() {}
	EnumFlags(T inval) : t_Flags(inval) {}
	typedef E EnumClass;
	typedef T ValueType;
};

template<class T, class E> class esEnum
{
	T storage;
public:
	operator E() const
	{
		return static_cast<E>(storage);
	}
	void operator=(E input)
	{
		storage = static_cast<T>(input);
	}
};

#endif //ES_FLAGS_DEFINED

#ifdef ES_REFLECTOR_DEFINED
#ifndef ES_FLAGS_TEMPLATES_DEFINED
#define ES_FLAGS_TEMPLATES_DEFINED
template <class C, class E> struct _getType<EnumFlags<C, E>> {
	static const char TYPE = 12; static const JenHash HASH = _EnumWrap<E>::HASH; static const uchar SUBSIZE = 0;
};
template <class C, class E> struct _getType<esEnum<C, E>> {
	static const char TYPE = 14; static const JenHash HASH = _EnumWrap<E>::HASH; static const uchar SUBSIZE = 0;
};
#endif
#endif // ES_REFLECTOR_DEFINED
