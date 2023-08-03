/*  XOR encoder/decoder class

    Copyright 2016-2023 Lukas Cone

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
#include "encryptor.hpp"
#include "spike/util/supercore.hpp"

class XOREncoder : public IEncryptor {
  std::string_view key;

public:
  void Encode(char *buffer, size_t size) const {
    for (size_t ii = 0; ii < size; ii++)
      buffer[ii] ^= key[ii % key.size()];
  }

  void Decode(char *buffer, size_t size) const { Encode(buffer, size); }

  void SetKey(std::string_view iKey) { key = iKey; }
};
