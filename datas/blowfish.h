/*	Blowfish encoder/decoder class
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

#pragma once
#include "supercore.hpp"
#include "encryptor.hpp"

#define BF_NUMPBOXES 18
#define BF_NUMSBOXES 1024

class BlowfishEncoder : public IEncryptor
{
public:
	enum Mode
	{
		ECB,
		CBC,
		PCBC,
		CFB,
		OFB
	};
protected:
	uint pboxes[BF_NUMPBOXES];
	uint sboxes[BF_NUMSBOXES];
	mutable ulong eVector;
	int enabled;
	Mode mode;
public:
	BlowfishEncoder() : enabled(0), mode(ECB) {};
	virtual void SetKey(const char *inKey, int inSize);
	void Encode(char *buffer, size_t size) const;
	void Decode(char *buffer, size_t size) const;

	ES_INLINE void EncodeECB(char *buffer, size_t size) const;
	ES_INLINE void DecodeECB(char *buffer, size_t size) const;
	ES_INLINE void EncodeCBC(char *buffer, size_t size) const;
	ES_INLINE void DecodeCBC(char *buffer, size_t size) const;
	ES_INLINE void EncodePCBC(char *buffer, size_t size) const;
	ES_INLINE void DecodePCBC(char *buffer, size_t size) const;
	ES_INLINE void EncodeCFB(char *buffer, size_t size) const;
	ES_INLINE void DecodeCFB(char *buffer, size_t size) const;
	ES_INLINE void EncodeOFB(char *buffer, size_t size) const;
	ES_INLINE void DecodeOFB(char *buffer, size_t size) const;

	ES_FORCEINLINE void Enable(bool enable = true) { enabled = enable; }
	ES_FORCEINLINE bool Valid() const { return enabled != 0; }
	ES_FORCEINLINE void Mode(Mode _mode) { mode = _mode; }
	ES_FORCEINLINE void Vector(ulong vec) { eVector = vec; }
	ES_FORCEINLINE ulong Vector() const { return eVector; }
private:
	virtual void EncodeBlock(ulong &block) const;
	virtual void DecodeBlock(ulong &block) const;
protected:
	void CreateVector();
};