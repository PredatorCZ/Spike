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
#include "../settings.hpp"
#include "../supercore.hpp"
#include "encryptor.hpp"

#define BF_NUMPBOXES 18
#define BF_NUMSBOXES 1024

class PC_EXTERN BlowfishEncoder : public IEncryptor
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
	uint32 pboxes[BF_NUMPBOXES];
	uint32 sboxes[BF_NUMSBOXES];
	mutable uint64 eVector;
	int enabled;
	Mode mode;
public:
	BlowfishEncoder() : enabled(0), mode(ECB) {};
	void SetKey(es::string_view key) override;
	void Encode(char *buffer, size_t size) const override;
	void Decode(char *buffer, size_t size) const override;

	void EncodeECB(char *buffer, size_t size) const;
	void DecodeECB(char *buffer, size_t size) const;
	void EncodeCBC(char *buffer, size_t size) const;
	void DecodeCBC(char *buffer, size_t size) const;
	void EncodePCBC(char *buffer, size_t size) const;
	void DecodePCBC(char *buffer, size_t size) const;
	void EncodeCFB(char *buffer, size_t size) const;
	void DecodeCFB(char *buffer, size_t size) const;
	void EncodeOFB(char *buffer, size_t size) const;
	void DecodeOFB(char *buffer, size_t size) const;

	void Enable(bool enable = true) { enabled = enable; }
	bool Valid() const { return enabled != 0; }
	void Mode(Mode _mode) { mode = _mode; }
	void Vector(uint64 vec) { eVector = vec; }
	uint64 Vector() const { return eVector; }
private:
	virtual void EncodeBlock(uint64 &block) const;
	virtual void DecodeBlock(uint64 &block) const;
protected:
	void CreateVector();
};
