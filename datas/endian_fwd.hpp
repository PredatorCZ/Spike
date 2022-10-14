/*  Forward declaration for endian
    more info in README for PreCore Project

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
#include <cstddef>
#include <type_traits>

template <class C>
concept IsSwapableArith = std::is_arithmetic_v<C> || requires(C t) {
  t.Swap();
} || std::is_enum_v<C>;

template <class C>
concept IsSwapableMem = requires(C &t) {
  t.SwapEndian();
};

template <class C>
concept IsSwapableMemn = requires(C &t) {
  t.SwapEndian(true);
};

template <class C>
concept IsSwapableClass = std::is_class_v<C> && !IsSwapableMemn<C> &&
                          !IsSwapableMem<C> && !IsSwapableArith<C>;

template <IsSwapableArith T> void FByteswapper(T &, bool = false);
template <IsSwapableMem C> void FByteswapper(C &input, bool = false) {
  input.SwapEndian();
}

template <IsSwapableMemn C> void FByteswapper(C &input, bool outWay = false) {
  input.SwapEndian(outWay);
}
template <IsSwapableClass T> void FByteswapper(T &, bool = false);

template <class C, size_t _size> void FByteswapper(C (&)[_size], bool = false);
