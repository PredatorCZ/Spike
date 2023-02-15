/*  TFileInfo class for parsing/exploding file paths
    more info in README for PreCore Project

    Copyright 2015-2021 Lukas Cone

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
#include <string_view>
#include <vector>

class FileInfo {
public:
  using T = char;
  typedef std::basic_string_view<T> stringref_type;
  typedef std::basic_string<T, std::char_traits<T>, std::allocator<T>>
      string_type;
  typedef std::vector<stringref_type> explode_type;

private:
  string_type fullPath;
  size_t endFolder;
  size_t lastDot;

public:
  FileInfo(stringref_type input) { Load(input); }
  FileInfo() noexcept = default;

  void Load(stringref_type input) {
    fullPath = input;

    if (!fullPath.size())
      return;

    for (auto &c : fullPath)
      if (c == '\\')
        c = '/';

    endFolder = fullPath.find_last_of('/');
    lastDot = fullPath.find_last_of('.');

    if (endFolder != fullPath.npos)
      endFolder++;
    else
      endFolder = 0;

    if (!lastDot || lastDot == fullPath.npos)
      lastDot = fullPath.size();
  }

  stringref_type GetFullPathNoExt() const { return {fullPath.data(), lastDot}; }
  stringref_type GetFullPath() const { return fullPath; }
  stringref_type GetFilenameExt() const {
    return {fullPath.data() + endFolder};
  }
  stringref_type GetFilename() const {
    return {fullPath.data() + endFolder, fullPath.data() + lastDot};
  }
  stringref_type GetExtension() const { return fullPath.data() + lastDot; }
  stringref_type GetFolder() const { return {fullPath.data(), endFolder}; }
  string_type ChangeExtension(stringref_type newExt) const {
    return string_type(GetFullPathNoExt()) + string_type(newExt);
  }
  string_type ChangeExtension2(stringref_type newExt) const {
    return string_type(GetFullPathNoExt()).append(".") + string_type(newExt);
  }

  explode_type Explode() const {
    explode_type resVal;
    const char *lastOffset = fullPath.data();
    const size_t fullSize = fullPath.size();
    const char *pathEnd = std::prev(fullPath.end()).operator->() + 1;

    if (*lastOffset == '/')
      lastOffset++;

    for (size_t c = 1; c < fullSize; c++)
      if (fullPath[c] == '/') {
        resVal.emplace_back(lastOffset, fullPath.data() + c);
        lastOffset = fullPath.data() + c + 1;
      }

    if (lastOffset != pathEnd)
      resVal.emplace_back(lastOffset);

    return resVal;
  }

  string_type CatchBranch(const stringref_type &path) const {
    explode_type exVec = Explode();
    const size_t found = path.find(exVec[0]);

    if (found == path.npos) {
      return string_type(path).append(GetFilenameExt());
    } else if (found == 0) {
      return string_type(GetFullPath());
    } else {
      auto nextPath = GetFullPath();
      size_t sub = nextPath.front() == '/';
      return std::string(path.substr(0, found - sub)).append(nextPath);
    }
  }
};

using AFileInfo = FileInfo;
