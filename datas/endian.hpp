/*	FByteswapper function, for endianness swapping, swaps any datatype, 
	calls void SwapEndian() on class instead of direct swap, if available,
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

#ifndef ES_ENDIAN_DEFINED
#define ES_ENDIAN_DEFINED
#include "supercore.hpp"
class _SwapClassEndian
{
	template<typename C, void (C::*f)()> struct detectorClass {};
public:
	template<class C> ES_FORCEINLINE static void evaluator(detectorClass<C, &C::SwapEndian>*, void *input) { static_cast<C*>(input)->SwapEndian(); };
	template<class C> ES_INLINE static void evaluator(C *, void *input)
	{ 
		constexpr int size = sizeof(C);
		char tmp[size];
		memcpy(tmp, input, size);
		char *valptr = static_cast<char *>(input);
		for (int i = 0; i < size; i++)
			valptr[size - i - 1] = tmp[i];
	};
};
#pragma warning(push)
#pragma warning(disable: 4127)
template<class C> ES_INLINE void FByteswapper(C &input)
{
	if (std::is_arithmetic<C>::value)
	{
		void *_input = &input;

		if (sizeof(C) == 2)
			*static_cast<ushort*>(_input) = (*static_cast<uchar*>(_input) << 8) | *(static_cast<uchar*>(_input) + 1);
		else if (sizeof(C) == 4)
			*static_cast<uint*>(_input) = (*static_cast<uchar*>(_input) << 24) | (*(static_cast<uchar*>(_input) + 1) << 16) | 
			(*(static_cast<uchar*>(_input) + 2) << 8) | *(static_cast<uchar*>(_input) + 3);
		else
			_SwapClassEndian::evaluator<C>(nullptr, &input);
	}
	else
		_SwapClassEndian::evaluator<C>(nullptr, &input);
}
#pragma warning(pop)

template<class E, class C> ES_INLINE void FArraySwapper(C &input)
{
	const size_t numItems = sizeof(C) / sizeof(E);
	E *inputPtr = reinterpret_cast<E *>(&input);

	for (size_t t = 0; t < numItems; t++)
		FByteswapper(*(inputPtr + t));
}
#endif
