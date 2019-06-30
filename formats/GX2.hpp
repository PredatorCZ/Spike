/*	GX2 format
	more info in README for PreCore Project

	Copyright 2019 Lukas Cone

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
#include "../datas/flags.hpp"

class GX2Block;

class GX2Relocation
{
	static const int ID = CompileFourCC("}BLK");

	int magic,
		headerSize,
		null,
		dataSize,
		dataOffset,
		textSize,
		textOffset,
		patchBase,
		patchCount,
		patchOffset;
public:

	GX2Relocation() : magic(ID), headerSize(sizeof(GX2Relocation)), null(0), dataSize(0),
		dataOffset(0), textSize(0), textOffset(0), patchBase(0), patchCount(0), patchOffset(0) {};

	ES_FORCEINLINE operator bool()
	{
		return magic == ID && headerSize == sizeof(GX2Relocation);
	}
};

enum class GX2BlockType : int
{
	UNKNOWN,
	EndOfFile,
	Pad,
	VertexShaderHeader,
	VertexShaderProgram = 5,
	PixelShaderHeader,
	PixelShaderProgram = 7,
	GeometryShaderHeader,
	GeometryShaderProgram,
	GeometryShaderProgramCopy,
	TextureHeader,
	TextureImage,
	TextureMipmap,
	ComputeShaderHeader,
	ComputeShaderProgram
};

class GX2Block
{
	static const int ID = CompileFourCC("BLK{");

	int magic,
		headerSize,
		majorVersion,
		minorVersion;
	GX2BlockType blockType;
	int dataSize,
		hash,
		index;

	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }

public:

	GX2Block() : magic(ID), headerSize(sizeof(GX2Block)), majorVersion(1), minorVersion(0), blockType(GX2BlockType::UNKNOWN), dataSize(0), hash(0), index(0) {}

	ES_FORCEINLINE operator bool()
	{
		return magic == ID && headerSize == sizeof(GX2Block) && majorVersion == 1 && minorVersion == 0;
	}

	template<class C> ES_FORCEINLINE C *GetData() { return reinterpret_cast<C *>(GetMe() + headerSize); }
	ES_FORCEINLINE GX2Block *NextBlock() { return blockType == GX2BlockType::EndOfFile ? nullptr : reinterpret_cast<GX2Block *>(GetMe() + headerSize + dataSize); }
	ES_FORCEINLINE GX2Relocation *GetRelocaltion()
	{ 
		GX2Relocation *relloc = reinterpret_cast<GX2Relocation *>(GetMe() + headerSize + dataSize - sizeof(GX2Relocation));

		return *relloc ? relloc : nullptr;
	}

};

enum class GX2Alignment : int
{
	None,
	X86 = 4,
	X64 = 8,
	Special = 16
};

class GX2Header
{
	static const int ID = CompileFourCC("Gfx2");

	int magic,
		headerSize,
		majorVersion,
		minorVersion,
		GPUVersion;
	GX2Alignment alignment;
	int reserved[2];
public:

	GX2Header() : magic(ID), headerSize(sizeof(GX2Header)), majorVersion(7), minorVersion(1), GPUVersion(2), alignment(GX2Alignment::None), reserved() {}

	ES_FORCEINLINE operator bool() 
	{
		return magic == ID && headerSize == sizeof(GX2Header) && majorVersion == 7 && minorVersion == 1 && GPUVersion == 2;
	}

	ES_FORCEINLINE GX2Block *FirstBlock() { return reinterpret_cast<GX2Block *>(this + 1); }
};

static_assert(sizeof(GX2Header) == 32, "Check assumptions");
static_assert(sizeof(GX2Block) == 32, "Check assumptions");
static_assert(sizeof(GX2Relocation) == 40, "Check assumptions");

class GX2
{
	union
	{
		void *linked;
		char *masterBuffer;
		BCHeader *header;
	}data;
};