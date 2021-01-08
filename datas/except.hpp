/*  uni exception internal module
    part of uni module
    Copyright 2020-2021 Lukas Cone

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
#include "string_view.hpp"
#include <cinttypes>
#include <cstring>
#include <stdexcept>

namespace es {
class FileNotFoundError : public std::runtime_error {
  using parent = std::runtime_error;

public:
  explicit FileNotFoundError() : parent("Couldn't open file.") {}
  explicit FileNotFoundError(es::string_view fileName)
      : parent("Couldn't open file: " + fileName.to_string()) {}
};

class FileInvalidAccessError : public std::runtime_error {
  using parent = std::runtime_error;

public:
  explicit FileInvalidAccessError() : parent("Couldn't access file.") {}
  explicit FileInvalidAccessError(es::string_view fileName)
      : parent("Couldn't access file: " + fileName.to_string()) {}
};

class InvalidHeaderError : public std::runtime_error {
  using parent = std::runtime_error;
public:
  static std::string DecompileFourCC(size_t magic, size_t size) {
    const char *rMagic = reinterpret_cast<const char *>(&magic);
    const size_t capSize =
        std::min(size, es::string_view::traits_type::length(rMagic));
    bool isAlNum = true;

    for (size_t c = 0; c < capSize; c++) {
      if (!isalnum(rMagic[c])) {
        isAlNum = false;
        break;
      }
    }

    if (isAlNum) {
      return es::string_view{rMagic, capSize}.to_string();
    } else {
      std::string retval;

      for (size_t c = 0; c < capSize; c++) {
        char buffer[8]{};
        snprintf(buffer, sizeof(buffer), "0x%X ", (unsigned char)rMagic[c]);
        retval.append(buffer);
      }

      return retval;
    }
  }

  explicit InvalidHeaderError() : parent("Invalid format.") {}
  InvalidHeaderError(es::string_view magic)
      : parent("Invalid format: " + magic.to_string()) {}
  template <typename T>
  InvalidHeaderError(T magic, size_t size = sizeof(T))
      : parent("Invalid format: " + DecompileFourCC(magic, size)) {}
};

class InvalidVersionError : public std::runtime_error {
  using parent = std::runtime_error;

public:
  explicit InvalidVersionError() : parent("Invalid version.") {}
  explicit InvalidVersionError(size_t version)
      : parent("Invalid version: " + std::to_string(version)) {}
};

class UnexpectedEOS : public std::runtime_error {
  using parent = std::runtime_error;

public:
  UnexpectedEOS() : parent("Unexpected end of stream.") {}
};
} // namespace es
