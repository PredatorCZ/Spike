/*  De/Serializing Reflector data from stream

    Copyright 2020-2024 Lukas Cone

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
#include "reflector.hpp"
#include "spike/io/bincore_fwd.hpp"

class ReflectorBinUtil {
  friend class ReflectorBinUtilFriend;
  static const ReflType *Find(Reflector &ri, JenHash hash);

public:
  static int PC_EXTERN Save(const Reflector &ri, BinWritterRef wr);
  static int PC_EXTERN Load(Reflector &ri, BinReaderRef rd);
};
