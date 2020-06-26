/*  source for DirectoryScanner class

    Copyright 2019-2020 Lukas Cone

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

#include "directory_scanner.hpp"
#include "tchar.hpp"

#if defined(__GNUC__) || defined(__GNUG__)
#include <dirent.h>
#else
#include <windows.h>
#endif

bool DirectoryScanner::IsFilteredFile(const std::string &fileName) {
  if (!filters.size())
    return true;

  for (auto &f : filters)
    if (fileName.find(f) != f.npos)
      return true;

  return false;
}

void DirectoryScanner::Scan(std::string dir) {
  if (!dir.empty()) {
    char lastWord = *std::prev(dir.end());

    if (lastWord == '"') {
      dir.pop_back();
    }

    if (lastWord != '\\' && lastWord != '/') {
      dir.push_back('/');
    }
  }

#if defined(__GNUC__) || defined(__GNUG__)
  DIR *cDir = opendir(dir.data());

  if (!cDir) {
    return;
  }

  dirent *cFile = nullptr;

  while ((cFile = readdir(cDir)) != nullptr) {
    if (!strcmp(cFile->d_name, ".") || !strcmp(cFile->d_name, "..")) {
      continue;
    }

    std::string miniFile(cFile->d_name);
    std::string subFile = dir;
    subFile += miniFile;

    if (cFile->d_type == DT_DIR) {
      Scan(subFile);
    } else if (IsFilteredFile(miniFile)) {
      files.push_back(subFile);
    }
  }

  closedir(cDir);
#else
  dir.push_back('*');
  const auto wdir = ToTSTRING(dir);

  WIN32_FIND_DATA foundData = {};
  HANDLE fleHandle = FindFirstFile(wdir.data(), &foundData);

  if (fleHandle == INVALID_HANDLE_VALUE) {
    return;
  }

  while (FindNextFile(fleHandle, &foundData)) {
    if (!_tcscmp(foundData.cFileName, _T(".")) ||
        !_tcscmp(foundData.cFileName, _T("..")) ||
        (foundData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) != 0) {
      continue;
    }

    std::string subFile = dir;
    subFile.pop_back();
    std::string cFileName = std::to_string(foundData.cFileName);
    subFile += cFileName;

    if ((foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      Scan(subFile);
    } else if (IsFilteredFile(cFileName)) {
      files.push_back(subFile);
    }
  }

  FindClose(fleHandle);
#endif
}