/*	Base class for Binary writter/reader

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

#pragma once
#include "supercore.hpp"
#include <fstream>

#define getBlockSize(classname, startval, endval) offsetof(classname, endval) - offsetof(classname, startval) + sizeof(classname::endval)

#ifdef ES_ENDIAN_DEFINED 
#define _e_swapEndian EndianSwap swapType = SWAP
#define _es_endian_flag | 1
#else
#define _e_swapEndian EndianSwap = SWAP
#define _es_endian_flag
#endif

#ifdef ES_ENCRYPTION_DEFINED
#define _es_enc_flag | 2
#else
#define _es_enc_flag
#endif

// We need to set template flags for correct instantiation by compiler, since compiler doesn't care about method/class content
#define _t_flags const int flags = 0 _es_endian_flag _es_enc_flag

template<class _driverClass, _t_flags>
class _BinCore : public _driverClass
{
protected:
	std::fstream FileStream;

#ifdef ES_ENCRYPTION_DEFINED
	mutable IEncryptor *enc;
#endif

	bool swapEndian;
	mutable bool encCreated;

	template<typename T> ES_FORCEINLINE bool _Open(const T _fileName)
	{
		FileStream.open(_fileName, _driverClass::MODE);

		if (FileStream.fail())
			return false;

		this->BaseStream = &FileStream;
		return true;
	}

	_BinCore() :swapEndian(0), encCreated(0)
#ifdef ES_ENCRYPTION_DEFINED
		, enc(nullptr)
#endif
	{}
	ES_FORCEINLINE void DisposeEncryptor()
	{
#ifdef ES_ENCRYPTION_DEFINED
		if (enc && encCreated)
			delete enc;
#endif
		encCreated = false;
	}
public:
	typedef typename _driverClass::StreamType StreamType;

	enum EndianSwap
	{
		DONT_SWAP,
		SWAP
	};

	ES_INLINE ~_BinCore()
	{
		if (IsValid())
			FileStream.close();
		DisposeEncryptor();
	}

	ES_FORCEINLINE void SetStream(StreamType &instream)
	{
		if (IsValid())
		{
			FileStream.close();
			swapEndian = 0;
			encCreated = 0;
#ifdef ES_ENCRYPTION_DEFINED
			enc = nullptr;
#endif
		}

		this->BaseStream = &instream;
	}

	ES_FORCEINLINE void SetRelativeOrigin(const size_t newOrigin, bool useSeek = true)
	{
		this->localseek = newOrigin;

		if (useSeek)
			this->Seek(0);
	}

	template<typename T> ES_FORCEINLINE bool Open(const UniString<T> &filePath) { return _Open(filePath); }
	ES_FORCEINLINE bool Open(const char *filePath) { return _Open(filePath); }
	ES_FORCEINLINE bool Open(const wchar_t *filePath) { return _Open(filePath); }
	ES_FORCEINLINE bool IsValid() const { return !FileStream.fail(); }
	ES_FORCEINLINE void SwapEndian(bool swap) { swapEndian = swap; }
	ES_FORCEINLINE bool SwappedEndian() { return swapEndian; }
	ES_FORCEINLINE void ResetRelativeOrigin(bool useSeek = true) { SetRelativeOrigin(0, useSeek); };
	ES_FORCEINLINE StreamType *GetStream() { return this->BaseStream; }

#ifdef ES_ENCRYPTION_DEFINED
	template<class C>IEncryptor *Encryptor()
	{
		DisposeEncryptor();
		enc = new C;
		encCreated = true;
		return enc;
	}

	ES_FORCEINLINE void Encryptor(IEncryptor &encryptor)
	{
		DisposeEncryptor();
		enc = &encryptor;
	}

	ES_FORCEINLINE IEncryptor *Encryptor() { return enc; }
#endif
};
