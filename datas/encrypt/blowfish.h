/*  Blowfish encoder/decoder class
    more info in README for PreCore Project

    Copyright 2018-2021 Lukas Cone

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
#include "../settings.hpp"
#include "../supercore.hpp"
#include "encryptor.hpp"
#include <memory>

class BlowfishContext;

class PC_EXTERN BlowfishEncoder : public IBlockEncryptor {
public:
  BlowfishEncoder();
  ~BlowfishEncoder();
  void SetKey(std::string_view key) override;
  void Encode(char *buffer, size_t size) const override;
  void Decode(char *buffer, size_t size) const override;
  size_t GetStride() const override;

  void EncodeECB(char *buffer, size_t size) const;
  void DecodeECB(char *buffer, size_t size) const;
  void EncodeCBC(char *buffer, size_t size) const;
  void DecodeCBC(char *buffer, size_t size) const;
  void EncodePCBC(char *buffer, size_t size) const;
  void DecodePCBC(char *buffer, size_t size) const;
  void EncodeCFB(char *buffer, size_t size) const;
  void DecodeCFB(char *buffer, size_t size) const;
  void EncodeOFB(char *buffer, size_t size) const;
  void DecodeOFB(char *buffer, size_t size) const;

  void Vector(uint64 vec);
  uint64 Vector() const;

protected:
  BlowfishEncoder(BlowfishContext *ctx);
  std::unique_ptr<BlowfishContext> pi;
};
