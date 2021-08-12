/*  fwd decl for bincore classes

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

enum class BinCoreOpenMode {
  Text = 0,
  Append = 1,
  Ate = 2,
  Binary = 4,
  Truncate = 8,
};

constexpr BinCoreOpenMode operator|(BinCoreOpenMode o1, BinCoreOpenMode o2) {
  return static_cast<BinCoreOpenMode>(static_cast<long>(o1) |
                                      static_cast<long>(o2));
}

constexpr bool operator&(BinCoreOpenMode o1, BinCoreOpenMode o2) {
  return static_cast<long>(o1) & static_cast<long>(o2);
}

template <BinCoreOpenMode MODE = BinCoreOpenMode::Binary> class BinReader;
class BinStreamInTraits;
template <class _Traits> class BinReaderRef_t;
typedef BinReaderRef_t<BinStreamInTraits> BinReaderRef;
template <BinCoreOpenMode MODE = BinCoreOpenMode::Binary> class BinWritter;
class BinStreamOutTraits;
template <class _Traits> class BinWritterRef_t;
typedef BinWritterRef_t<BinStreamOutTraits> BinWritterRef;
