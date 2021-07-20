/*  DirectoryScanner class is a simple class
    for scanning files inside a folder hierarchy

    Copyright 2019-2021 Lukas Cone

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
#include <string>
#include <vector>
#include "settings.hpp"

class DirectoryScanner {
  bool IsFilteredFile(const std::string &fileName);

public:
  typedef std::vector<std::string> storage_type;
  typedef storage_type::iterator iterator;
  typedef storage_type::const_iterator const_iterator;

  void PC_EXTERN Scan(std::string dir);

  iterator begin() { return files.begin(); }
  iterator end() { return files.end(); }
  const_iterator cbegin() const { return files.cbegin(); }
  const_iterator cend() const { return files.cend(); }
  void AddFilter(const std::string &val) { filters.push_back(val); }
  const storage_type &Files() const { return files; }
  void Clear() { files.clear(); }
  void ClearFilters() { filters.clear(); }

private:
  storage_type files;
  storage_type filters;
};
