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

template<class T, class E = int> class esFlags
{
public:
	typedef E EnumClass;
	typedef T ValueType;
private:
	T Value;

	template <typename _Type>
	const T _eval(const T val, _Type input) { return val | (1 << input); }

	template <typename _Type, typename... _Others>
	const T _eval(const T val, _Type input, _Others... inputs)
	{
		return _eval(val | (1 << input), inputs...);
	}
	
public:
	esFlags() : Value(0) {}
	template<typename... _Type> esFlags(const _Type... inputs)
	{
		Value = _eval(0, inputs...);
	}

	ES_FORCEINLINE void operator=(T inval) { Value = inval; }
	ES_FORCEINLINE bool operator[](E pos) const { return (Value & (1 << pos)) != 0; }
	ES_FORCEINLINE void operator()(E pos, bool val) { val ? Value |= (1 << pos) : Value &= ~(1 << pos); }
	ES_FORCEINLINE void operator+= (E input) { operator()(input, true); }
	ES_FORCEINLINE void operator-= (E input) { operator()(input, false); }

	const bool operator==(const esFlags &input)const { return Value == input.Value; }
};

template<class T, class E> using EnumFlags = esFlags<T, E>;

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
template <class C, class E> struct _getType<esFlags<C, E>> {
	static const char TYPE = 12; static const JenHash HASH = _EnumWrap<E>::HASH; static const uchar SUBSIZE = 0;
};
template <class C, class E> struct _getType<esEnum<C, E>> {
	static const char TYPE = 14; static const JenHash HASH = _EnumWrap<E>::HASH; static const uchar SUBSIZE = 0;
};
#endif
#endif // ES_REFLECTOR_DEFINED
