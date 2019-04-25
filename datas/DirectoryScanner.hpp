/*	DirectoryScanner class is a simple winAPI wrapper 
	for scanning files inside a folder hierarchy
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
#if defined(__GNUC__) || defined(__GNUG__)
#include <dirent.h>
#else
#ifndef _INC_TCHAR
#ifdef _UNICODE
#define _tcscmp wcscmp
#else
#define _tcscmp strcmp
#endif
#endif
#include <windows.h>
#endif
#include <cstring>
#include <vector>
#include "esstring.h"

class DirectoryScanner
{
	ES_INLINE bool IsFilteredFile(const TCHAR *fileName)
	{
		if (!filters.size())
			return true;

		for (auto &f : filters)
			if (TSTRING(fileName).find(f) != f.npos)
				return true;

		return false;
	}
public:
	typedef std::vector<TSTRING> storage_type;
	typedef storage_type::iterator iterator;
	typedef storage_type::const_iterator const_iterator;

	void Scan(const TSTRING & dir);

	ES_FORCEINLINE iterator begin() { return files.begin(); }
	ES_FORCEINLINE iterator end() { return files.end(); }
	ES_FORCEINLINE const_iterator cbegin() const { return files.cbegin(); }
	ES_FORCEINLINE const_iterator cend() const { return files.cend(); }
	void AddFilter(TSTRING val) { filters.push_back(val); }
	const storage_type &Files() const { return files; }

private:
	storage_type files;
	storage_type filters;
};

#if defined(__GNUC__) || defined(__GNUG__)
	void DirectoryScanner::Scan(const TSTRING & dir)
	{
		TSTRING wdir = dir;
		TCHAR lastword = wdir[wdir.length() - 1];

		if (lastword == '"')
			wdir.pop_back();

		if (lastword != '\\' && lastword != '/')
			wdir.push_back('/');

		DIR *cDir = opendir(wdir.c_str());

		if (!cDir)
			return;

		dirent *cFile = nullptr;

		while ((cFile = readdir(cDir)) != nullptr)
		{
			if (!strcmp(cFile->d_name, ".") || 
				!strcmp(cFile->d_name, ".."))
				continue;

			TSTRING miniFile = esString(cFile->d_name);
			TSTRING subFile = wdir;
			subFile += miniFile;

			if (cFile->d_type == DT_DIR)
				Scan(subFile);
			else if (IsFilteredFile(miniFile.c_str()))
				files.push_back(subFile);

		}

		closedir(cDir);
	}
#else
	void DirectoryScanner::Scan(const TSTRING & dir)
	{
		TSTRING wdir = dir;
		TCHAR lastword = wdir[wdir.length() - 1];

		if (lastword == '"')
			wdir.pop_back();

		if (lastword != '\\' && lastword != '/')
			wdir.push_back('/');

		wdir.push_back('*');

		WIN32_FIND_DATA foundData = {};
		HANDLE fleHandle = FindFirstFile(wdir.c_str(), &foundData);

		if (fleHandle == INVALID_HANDLE_VALUE)
			return;

		while (FindNextFile(fleHandle, &foundData))
		{
			if (!_tcscmp(foundData.cFileName, _T(".")) ||
				!_tcscmp(foundData.cFileName, _T("..")) ||
				(foundData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) != 0)
				continue;

			TSTRING subFile = wdir;
			subFile.pop_back();
			subFile += foundData.cFileName;

			if ((foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				Scan(subFile);
			else if (IsFilteredFile(foundData.cFileName))
				files.push_back(subFile);

		}

		FindClose(fleHandle);
	}
#endif