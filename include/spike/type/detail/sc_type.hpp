/*  Supercore data types

    Copyright 2018-2025 Lukas Cone

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

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

static_assert(sizeof(uint64) == 8, "Unexpected integer size!");
static_assert(sizeof(uint32) == 4, "Unexpected integer size!");
static_assert(sizeof(uint16) == 2, "Unexpected integer size!");
static_assert(sizeof(uint8) == 1, "Unexpected integer size!");
static_assert(sizeof(int64) == 8, "Unexpected integer size!");
static_assert(sizeof(int32) == 4, "Unexpected integer size!");
static_assert(sizeof(int16) == 2, "Unexpected integer size!");
static_assert(sizeof(int8) == 1, "Unexpected integer size!");

template <int size> struct TypeFromSize { typedef void type; };
template <> struct TypeFromSize<1> { typedef uint8 type; };
template <> struct TypeFromSize<2> { typedef uint16 type; };
template <> struct TypeFromSize<4> { typedef uint32 type; };
template <> struct TypeFromSize<8> { typedef uint64 type; };
template <> struct TypeFromSize<-1> { typedef int8 type; };
template <> struct TypeFromSize<-2> { typedef int16 type; };
template <> struct TypeFromSize<-4> { typedef int32 type; };
template <> struct TypeFromSize<-8> { typedef int64 type; };

using intptr = TypeFromSize<-int(sizeof(void*))>::type;
using uintptr = TypeFromSize<sizeof(void*)>::type;

static_assert(sizeof(intptr) == sizeof(void*));
static_assert(sizeof(uintptr) == sizeof(void*));
