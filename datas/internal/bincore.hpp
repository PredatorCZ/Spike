/*  Base class for Binary writter/reader

    Copyright 2018-2020 Lukas Cone

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
#include "bincore_endian.hpp"
#include "bincore_navi.hpp"
#include "../bincore_fwd.hpp"

#define getBlockSize(classname, startval, endval)                              \
  offsetof(classname, endval) - offsetof(classname, startval) +                \
      sizeof(classname::endval)
