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
#include <vector>
#include <map>
#include "../datas/flags.hpp"

class GX2BlockHeader;

class GX2Relocation
{
public:
	static const int ID = CompileFourCC("}BLK");
	static const int IDS = CompileFourCC("KLB}");
	static const int OFFSET_MASK = 0xfffff;
private:
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

	GX2Relocation() : magic(IDS), headerSize(sizeof(GX2Relocation)), null(0), dataSize(0),
		dataOffset(0), textSize(0), textOffset(0), patchBase(0), patchCount(0), patchOffset(0) {};

	ES_FORCEINLINE operator bool()
	{
		return magic == IDS && headerSize == sizeof(GX2Relocation);
	}

	ES_FORCEINLINE int Validity() 
	{
		if (magic == ID)
			return 0;
		else if (magic == IDS)
			return 1;
		else 
			return -1;
	}

	ES_FORCEINLINE int *GetPatches(char *dataBuffer) { return reinterpret_cast<int*>(dataBuffer + (patchOffset & OFFSET_MASK)); }
	ES_FORCEINLINE int GetPatch(int id, char *dataBuffer) { return GetPatches(dataBuffer)[id] & OFFSET_MASK; }

	void PatchData(char *dataBuffer);
	void SwapEndian(char *dataBuffer);
};

template<class C, bool X64> union _GX2Pointer;

template<class C> union _GX2Pointer<C, true>
{
	int offset;
	int varPtr;
	ES_FORCEINLINE void Fixup(char *) { offset &= GX2Relocation::OFFSET_MASK; }
	ES_FORCEINLINE C *Get(char *masterBuffer) { return offset ? reinterpret_cast<C *>(masterBuffer + offset) : nullptr; }
};

template<class C> union _GX2Pointer<C, false>
{
	int varPtr;
	C *ptr;
	char *cPtr;

	ES_FORCEINLINE void Fixup(char *masterBuffer) 
	{ 
		if (varPtr)
			cPtr = masterBuffer + (varPtr & GX2Relocation::OFFSET_MASK);
	}

	ES_FORCEINLINE C *Get(char *) { return ptr; }
};

template <class C> using GX2Pointer = _GX2Pointer<C, ES_X64>;

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

class GX2BlockHeader
{
	static const int ID = CompileFourCC("BLK{");
	static const int IDS = CompileFourCC("{KLB");

	int magic,
		headerSize,
		majorVersion,
		minorVersion;
public:
	GX2BlockType blockType;
	int dataSize,
		hash,
		index;
private:
	ES_FORCEINLINE char *GetMe() { return reinterpret_cast<char *>(this); }

public:

	GX2BlockHeader() : magic(IDS), headerSize(sizeof(GX2BlockHeader)), majorVersion(1), minorVersion(0), 
		blockType(GX2BlockType::UNKNOWN), dataSize(0), hash(0), index(0) {}

	ES_FORCEINLINE operator bool()
	{
		return magic == IDS && headerSize == sizeof(GX2BlockHeader) && majorVersion == 1 && minorVersion == 0;
	}

	template<class C> ES_FORCEINLINE C *GetData() { return reinterpret_cast<C *>(GetMe() + headerSize); }
	ES_FORCEINLINE GX2BlockHeader *NextBlock()
	{
		return blockType == GX2BlockType::EndOfFile ? nullptr : reinterpret_cast<GX2BlockHeader *>(GetMe() + headerSize + dataSize); 
	}
	ES_FORCEINLINE GX2Relocation *GetRelocaltion(char *dataBuffer)
	{ 
		GX2Relocation *relloc = reinterpret_cast<GX2Relocation *>(GetMe() + headerSize + dataSize - sizeof(GX2Relocation));
		int rellocValid = relloc->Validity();

		if (!rellocValid)
			relloc->SwapEndian(dataBuffer);
		else if (rellocValid < 0)
			return nullptr;

		return *relloc ? relloc : nullptr;
	}

	ES_FORCEINLINE int Validity()
	{
		if (magic == ID)
			return 0;
		else if (magic == IDS)
			return 1;
		else
			return -1;
	}

	void SwapEndian();
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
public:
	static const int ID = CompileFourCC("Gfx2");
	static const int IDS = CompileFourCC("2xfG");
private:
	int magic,
		headerSize,
		majorVersion,
		minorVersion,
		GPUVersion;
	GX2Alignment alignment;
	int reserved[2];
public:

	GX2Header() : magic(IDS), headerSize(sizeof(GX2Header)), majorVersion(7), minorVersion(1), 
		GPUVersion(2), alignment(GX2Alignment::None), reserved() {}

	ES_FORCEINLINE operator bool() 
	{
		return magic == IDS && headerSize == sizeof(GX2Header) && majorVersion == 7 && minorVersion == 1 && GPUVersion == 2;
	}

	ES_FORCEINLINE int Validity()
	{
		if (magic == ID)
			return 0;
		else if (magic == IDS)
			return 1;
		else
			return -1;
	}

	ES_FORCEINLINE GX2BlockHeader *FirstBlock() { return reinterpret_cast<GX2BlockHeader *>(this + 1); }

	void SwapEndian();
};

static_assert(sizeof(GX2Header) == 32, "Check assumptions");
static_assert(sizeof(GX2BlockHeader) == 32, "Check assumptions");
static_assert(sizeof(GX2Relocation) == 40, "Check assumptions");

struct GX2Sampler
{
	GX2Pointer<char> name;
	int type,
		location;

	ES_FORCEINLINE const char *Getname(char *dataBuffer) { return name.Get(dataBuffer); }

	void SwapEndian();
};

struct GX2UniformValue
{
	GX2Pointer<char> name;
	int varType,
		arrayCount,
		offset,
		blockIndex;

	ES_FORCEINLINE const char *Getname(char *dataBuffer) { return name.Get(dataBuffer); }

	void SwapEndian();
};

struct GX2UniformBlock
{
	GX2Pointer<char> name;
	int offset,
		size;

	ES_FORCEINLINE const char *Getname(char *dataBuffer) { return name.Get(dataBuffer); }

	void SwapEndian();
};

struct GX2Attribute
{
	GX2Pointer<char> name;
	int varType,
		arrayCount,
		location;

	ES_FORCEINLINE const char *Getname(char *dataBuffer) { return name.Get(dataBuffer); }

	void SwapEndian();
};

struct GX2Loop
{
	int offset,
		value;

	void SwapEndian();
};

struct GX2UniformInitialValue
{
	float value[4];
	int offset;

	void SwapEndian();
};

enum class GX2RResourceFlags
{
	BIND_TEXTURE,
	BIND_COLOR_BUFFER,
	BIND_DEPTH_BUFFER,
	BIND_SCAN_BUFFER,
	BIND_VERTEX_BUFFER,
	BIND_INDEX_BUFFER,
	BIND_UNIFORM_BLOCK,
	BIND_SHADER_PROGRAM,
	BIND_STREAM_OUTPUT,
	BIND_DISPLAY_LIST,
	BIND_GS_RING_BUFFER,
	USAGE_CPU_READ,
	USAGE_CPU_WRITE,
	USAGE_GPU_READ,
	USAGE_GPU_WRITE,
	USAGE_DMA_READ,
	USAGE_DMA_WRITE,
	USAGE_FORCE_MEM1,
	USAGE_FORCE_MEM2,
	DISABLE_CPU_INVALIDATE = 20,
	DISABLE_GPU_INVALIDATE,
	LOCKED_READ_ONLY,
	USER_MEMORY = 29,
	LOCKED,
};

class GX2RBuffer
{
public:
	esFlags<int, GX2RResourceFlags> flags;
	int elemSize,
		elemCount;
	GX2Pointer<char> buffer;
};

enum class GX2ShaderMode : int
{
	UNIFORM_REGISTER,
	UNIFORM_BLOCK,
	GEOMETRY_SHADER,
	COMPUTE_SHADER,
};

class GX2VertexShaderHeader
{
public:
	int registry[52],
		programSize;
	GX2Pointer<char> program;
	GX2ShaderMode shaderMode;

	int uniformBlockCount;
	GX2Pointer<GX2UniformBlock> uniformBlocks;

	int uniformVarCount;
	GX2Pointer<GX2UniformValue> uniformValues;

	int initialValueCount;
	GX2Pointer<GX2UniformInitialValue> initialValues;

	int loopVarCount;
	GX2Pointer<GX2Loop> loops;

	int samplerVarCount;
	GX2Pointer<GX2Sampler> samplers;

	int attribVarCount;
	GX2Pointer<GX2Attribute> attributes;

	int ringItemSize,
		hasStreamOut,
		streamOutStride[4];

	GX2RBuffer gx2RBuffer;

	void SwapEndian();
};

class GX2Block
{
public:
	GX2BlockHeader *header;
	void *data;
public:
	GX2Block(GX2BlockHeader *input) : header(input)
	{
		if (header->dataSize)
			data = header->GetData<void>();
	}
	GX2Block(GX2BlockType type);
	template<class C> C *GetData() { return reinterpret_cast<C*>(data); }
	void SwapEndian();
};

class GX2
{
	union
	{
		void *linked;
		char *masterBuffer;
		GX2Header *header;
	}data;
public:
	std::vector<GX2Block*> blocks;
private:
	bool linked;

	template<class _Ty0>
	//typedef wchar_t _Ty0;
	int _Load(const _Ty0 *fileName, bool suppressErrors);
	int _LoadBlocks();

public:
	GX2() : data(), linked(false) { }
	~GX2();

	ES_FORCEINLINE int Load(const char *fileName, bool suppressErrors = false) { return _Load(fileName, suppressErrors); }
	ES_FORCEINLINE int Load(const wchar_t *fileName, bool suppressErrors = false) { return _Load(fileName, suppressErrors); }

	int Link(void *file);
};