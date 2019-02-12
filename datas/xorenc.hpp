/*	XOR encoder/decoder class
	more info in README for PreCore Project

	Copyright 2016-2019 Lukas Cone

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
#include "encryptor.hpp"

class XOREncoder : public IEncryptor 
{
	const char *key;
	int keyLength;
	mutable int iteratorPos;

public:
	ES_FORCEINLINE void Encode(char *buffer, size_t size) const
	{
		for (size_t ii = 0; ii < size; ii++, iteratorPos++)
			buffer[ii] ^= key[iteratorPos % GetKeyLength()];
	}

	ES_FORCEINLINE void Decode(char *buffer, size_t size) const
	{
		Encode(buffer, size);
	}

	ES_FORCEINLINE int GetKeyLength() const
	{
		return keyLength & 0x7FFFFFFF;
	}

	ES_FORCEINLINE void CoderEnable(bool enabled = true)
	{
		if (CoderEnabled() && !enabled)
			keyLength ^= 0x80000000;
		else if (enabled)
			keyLength |= 0x80000000;
	}

	ES_FORCEINLINE int CoderEnabled() const
	{
		return keyLength & 0x80000000;
	}

	ES_FORCEINLINE void ResetIter() { iteratorPos = 0; }

	ES_FORCEINLINE bool Valid() const
	{
		return CoderEnabled() && (key || GetKeyLength());
	}

	ES_FORCEINLINE void SetKey(const char *inKey, int inSize)
	{
		key = inKey;
		keyLength = inSize;
		CoderEnable();
		ResetIter();
	}

	XOREncoder() : key(0), keyLength(0), iteratorPos(0) {}

};