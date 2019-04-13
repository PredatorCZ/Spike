/*	Binary data writter class,
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

#include "bincore.hpp"

class _BinCoreOut
{
public:
	typedef std::ostream StreamType;
protected:
	_BinCoreOut() : localseek(0) {}
	size_t localseek;
	StreamType *BaseStream;
	static const std::ios_base::openmode MODE = std::ios::binary | std::ios::out;
public:
	ES_FORCEINLINE const size_t Tell(const bool relative = true) const
	{
		return static_cast<size_t>(BaseStream->tellp()) - (relative ? localseek : 0);
	}

	ES_FORCEINLINE void Seek(const size_t position, const std::ios_base::seekdir vay = std::ios_base::beg, const bool relative = true) const
	{
		BaseStream->seekp(position + ((relative && !vay) ? localseek : 0), vay);
	}

	ES_FORCEINLINE void Skip(const size_t length) { Seek(length, std::ios_base::cur); }

	template<typename T> ES_FORCEINLINE void Skip() { Seek(sizeof(T), std::ios_base::cur); }
};

class BinWritter : public _BinCore<_BinCoreOut>
{
public:

	template<typename T> BinWritter(const UniString<T> &filePath) { _Open(filePath); }
	BinWritter(StreamType &instream) { SetStream(instream); }
	//BinWritter(const wchar_t *filePath) { _Open(filePath); }
	BinWritter(const char *filePath) { _Open(filePath); }

	ES_FORCEINLINE void WriteBuffer(const char *buffer, size_t size) const
	{

#ifdef ES_ENCRYPTION_DEFINED
		if (enc && enc->Valid())
		{
			char *outBuffer = static_cast<char*>(malloc(size));
			memcpy(outBuffer, buffer, size);
			enc->Encode(outBuffer, size);
			BaseStream->write(outBuffer, size);
			free(outBuffer);
		}
		else
#endif
			BaseStream->write(buffer, size);
	}
	
	/// Will write any std container using std::allocator class, eg. vector, basic_string, etc..
	/// swapType : will force not to swap endianess, when used with class that does not have SwapEndian method or is not defined for structural swap
	template<
		class _containerClass, 
		class T = typename _containerClass::value_type
	> const void WriteContainer(_containerClass &input, _e_swapEndian) const
	{
		const size_t elesize = sizeof(T);
		const size_t capacity = input.size()*elesize;
#ifdef ES_ENDIAN_DEFINED
		if (swapEndian && elesize > 1)
		{
			for (T &a : input)
			{
				T outCopy = a;
				FByteswapper(outCopy);
				WriteBuffer(reinterpret_cast<const char *>(&outCopy), elesize);
			}
		}
		else
#endif	
			WriteBuffer(reinterpret_cast<const char*>(input.begin().operator->()), capacity);
	}

	/// Will write any std container using std::allocator class, eg. vector, basic_string, etc..
	/// Will write number of items first
	/// swapType : will force not to swap endianess, when used with class that does not have SwapEndian method or is not defined for structural swap
	template<class _countType = int, class _containerClass> const void WriteContainerWCount(_containerClass &input, EndianSwap swapType = SWAP) const
	{
		const _countType numElements = static_cast<_countType>(input.size());
		Write(numElements);
		WriteContainer(input, swapType);
	}

	///cut : will remove \0
	void WriteT(const char *input, bool cut = false) const { WriteBuffer(input, strlen(input) + (cut ? 0 : 1)); }
	///cut : will remove \0
	void WriteT(const wchar_t *input, bool cut = false) const
	{
		size_t size = wcslen(input);
		if (!cut)
			size++;
		size_t capacity = size * 2;
#ifdef ES_ENDIAN_DEFINED
		if (swapEndian)
		{
			std::wstring outBuffer = input;

			if (cut)
				outBuffer.resize(size);

			WriteContainer(outBuffer);
		}
		else
#endif
			WriteBuffer(reinterpret_cast<const char *>(input), capacity);
	}

	/// swapType : will force not to swap endianess, when used with class that does not have SwapEndian method or is not defined for structural swap
	template<class T> void Write(const T input, _e_swapEndian) const
	{ 
		const size_t capacity = sizeof(T);
#ifdef ES_ENDIAN_DEFINED
		if (swapType && swapEndian && capacity > 1)
		{
			T outCopy = input;
			FByteswapper(outCopy);
			WriteBuffer(reinterpret_cast<const char *>(&outCopy), capacity);
		}
		else
#endif	
			WriteBuffer(reinterpret_cast<const char *>(&input), capacity);
	}
};