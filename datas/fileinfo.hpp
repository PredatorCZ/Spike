/*	TFileInfo class for parsing/exploding file paths
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

#pragma once
#include "supercore.hpp"
#include <string>
#include <vector>

template<class T> class _FileInfo_t
{
public:
	typedef std::basic_string<T, std::char_traits<T>, std::allocator<T>> _strType;
	typedef std::vector<_strType> _vecType;
private:
	_strType fullPath;
	_strType path;
	_strType filename;
	_strType extension;
	_strType filenameFull;
public:
	ES_FORCEINLINE _FileInfo_t(const _strType &fname): _FileInfo_t(fname.c_str()){}
	ES_FORCEINLINE _FileInfo_t(const T *fname )
	{
		fullPath = fname;
		
		if (!fullPath.size())
			return;

		for (auto &c : fullPath)
			if (c == '\\')
				c = '/';

		size_t pathindex = fullPath.find_last_of('/');
		size_t dotIndex = fullPath.find_last_of('.');

		if (pathindex != fullPath.npos)
			pathindex++;

		path = fullPath.substr(0, pathindex);
		
		if (dotIndex == fullPath.npos)
			dotIndex = fullPath.size();

		filename = fullPath.substr(pathindex, dotIndex - pathindex);
		filenameFull = fullPath.substr(pathindex);	
		extension = fullPath.substr(dotIndex);
	}
	ES_FORCEINLINE const _strType &GetFullPath() const {return fullPath;}
	ES_FORCEINLINE const _strType &GetFilenameFull() const {return filenameFull;}
	ES_FORCEINLINE const _strType &GetFileName() const {return filename;}
	ES_FORCEINLINE const _strType &GetExtension() const {return extension;}
	ES_FORCEINLINE const _strType &GetPath() const {return path;}
	ES_FORCEINLINE void Explode(_vecType &rVec) const
	{
		const size_t fullSize = fullPath.size();
		size_t lastOffset = 2;

		for (size_t c = 0; c < fullSize; c++)
			if (fullPath[c] == '/')
				lastOffset++;

		rVec.reserve(lastOffset);
		lastOffset = 0;

		for (size_t c = 0; c < fullSize; c++)
			if (fullPath[c] == '/')
			{
				size_t curOffset = lastOffset + (lastOffset ? 1 : 0);
				rVec.push_back(fullPath.substr(curOffset, c - curOffset));
				lastOffset = c;
			}

		rVec.push_back(fullPath.substr(lastOffset + (lastOffset ? 1 : 0)));
	}
	ES_FORCEINLINE const _strType CatchBranch(const _strType &path) const
	{
		_vecType exVec = {};
		Explode(exVec);

		size_t found = path.find(exVec[0].c_str());

		if (found == path.npos)
			return path + GetFilenameFull();
		else
			return path.substr(0, found) + GetFullPath();
	}

};

typedef _FileInfo_t<char> AFileInfo;
typedef _FileInfo_t<wchar_t> WFileInfo;

#ifdef _UNICODE
typedef WFileInfo TFileInfo;
#else
typedef AFileInfo TFileInfo;
#endif