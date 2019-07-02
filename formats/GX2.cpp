/*	GX2 format source
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
#include "GX2.hpp"
#include "datas/endian.hpp"
#include "datas/masterprinter.hpp"
#include "datas/binreader.hpp"
#include "datas/macroLoop.hpp"

int GX2::Link(void * file)
{
	data.linked = file;

	int isValid = data.header->Validity();

	if (!isValid)
	{
		data.header->SwapEndian();
	}
	else if (isValid < 0)
	{
		printerror("[GX2] Invalid header.");
		return 1;
	}

	linked = true;
	if (_LoadBlocks())
		return 2;

	return 0;
}

int GX2::_LoadBlocks()
{
	GX2BlockHeader *cBlock = data.header->FirstBlock();

	while (cBlock)
	{
		if (!cBlock)
		{
			printerror("[GX2] Corrupted file.");
			return 1;
		}
		cBlock->SwapEndian();
		GX2Block *cBl = new GX2Block(cBlock);
		

		blocks.push_back(cBl);

		GX2Relocation *relocBlock = cBlock->GetRelocaltion(cBlock->GetData<char>());

		if (relocBlock)
			relocBlock->PatchData(cBlock->GetData<char>());	

		cBl->SwapEndian();

		cBlock = cBlock->NextBlock();
	}

	return 0;
}

GX2::~GX2()
{
	if (!linked && data.masterBuffer)
		free(data.masterBuffer);
}

void GX2Relocation::PatchData(char * dataBuffer)
{
	for (int p = 0; p < patchCount; p++)
	{
		int cPatch = GetPatch(p, dataBuffer);

		if (!cPatch)
			continue;

		GX2Pointer<char> *pointerToPatch = reinterpret_cast<GX2Pointer<char> *>(dataBuffer + cPatch);
		FByteswapper(pointerToPatch->varPtr);
		pointerToPatch->Fixup(dataBuffer);
		FByteswapper(pointerToPatch->varPtr);
	}
}

ES_INLINE void GX2Relocation::SwapEndian(char *dataBuffer)
{
	FArraySwapper<int>(*this);
	int *patches = GetPatches(dataBuffer);

	for (int p = 0; p < patchCount; p++)
		FByteswapper(patches[p]);
}

ES_INLINE void GX2BlockHeader::SwapEndian()
{
	FArraySwapper<int>(*this);
}

ES_INLINE void GX2Header::SwapEndian()
{
	FArraySwapper<int>(*this);
}

template<class _Ty0>
int GX2::_Load(const _Ty0 * fileName, bool suppressErrors)
{
	BinReader rd(fileName);

	if (!rd.IsValid())
	{
		if (!suppressErrors)
		{
			printerror("[GX2] Cannot load file: ", << fileName);
		}

		return 1;
	}

	int magic;
	rd.Read(magic);

	if (magic != GX2Header::ID)
	{
		if (!suppressErrors)
		{
			printerror("[GX2] Invalid header.");
		}

		return 2;
	}

	rd.Seek(0);

	const size_t fileSize = rd.GetSize();

	data.masterBuffer = static_cast<char *>(malloc(fileSize));
	rd.ReadBuffer(data.masterBuffer, fileSize);
	data.header->SwapEndian();

	if (_LoadBlocks())
		return 3;

	return 0;
}

template int GX2::_Load(const wchar_t *fileName, bool suppressErrors);
template int GX2::_Load(const char *fileName, bool suppressErrors);

void GX2Sampler::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2UniformValue::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2UniformBlock::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2Attribute::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2UniformInitialValue::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2Loop::SwapEndian()
{
	FArraySwapper<int>(*this);
}

void GX2VertexShaderHeader::SwapEndian()
{
	FArraySwapper<int>(*this);

	char *thisData = reinterpret_cast<char *>(this);

	for (int u = 0; u < uniformBlockCount; u++)
		uniformBlocks.Get(thisData)[u].SwapEndian();

	for (int u = 0; u < uniformVarCount; u++)
		uniformValues.Get(thisData)[u].SwapEndian();

	for (int i = 0; i < initialValueCount; i++)
		initialValues.Get(thisData)[i].SwapEndian();

	for (int l = 0; l < loopVarCount; l++)
		loops.Get(thisData)[l].SwapEndian();

	for (int s = 0; s < samplerVarCount; s++)
		samplers.Get(thisData)[s].SwapEndian();

	for (int a = 0; a < attribVarCount; a++)
		attributes.Get(thisData)[a].SwapEndian();
}

template<class C> void *_blockCreatorDummy() { return new C; }

#define _ADD_BLOCKTYPE_CLASS(v) {GX2BlockType::v, _blockCreatorDummy<GX2##v>},

static const std::map<GX2BlockType, void*(*)()> blockTypesRegistry =
{
	StaticFor(_ADD_BLOCKTYPE_CLASS, VertexShaderHeader)
};

GX2Block::GX2Block(GX2BlockType type)
{
	header = new GX2BlockHeader;
	header->blockType = type;

	if (blockTypesRegistry.count(type))
		data = blockTypesRegistry.at(type)();
}

template<class C> void _SEndianSwap(void *data) { reinterpret_cast<C*>(data)->SwapEndian(); }

#define _ADD_BLOCKTYPE_SENDIAN(v) {GX2BlockType::v, _SEndianSwap<GX2##v>},

static const std::map<GX2BlockType, void(*)(void*)> blockSwappers = 
{
	StaticFor(_ADD_BLOCKTYPE_SENDIAN, VertexShaderHeader)
};

void GX2Block::SwapEndian()
{
	if (blockSwappers.count(header->blockType))
		blockSwappers.at(header->blockType)(data);
}
