/*  forward declarations for Vector, Vector2, Vector4

    Copyright 2018-2023 Lukas Cone

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
#include "detail/sc_type.hpp"

template <typename T> class t_Vector2;
template <typename T> class t_Vector;
template <typename T> class t_Vector4_;
template <typename T> class V4ScalarType;
template <typename T> using t_Vector4 = t_Vector4_<V4ScalarType<T>>;
template <typename T> class V4SimdIntType_t;
class V4SimdFltType;

using V4SimdIntType = V4SimdIntType_t<int32>;
using IVector4A16 = t_Vector4_<V4SimdIntType>;
using UIVector4A16 = t_Vector4_<V4SimdIntType_t<uint32>>;
using Vector4A16 = t_Vector4_<V4SimdFltType>;

using Vector2 = t_Vector2<float>;
using FVector2 = Vector2;
using IVector2 = t_Vector2<int32>;
using SVector2 = t_Vector2<int16>;
using CVector2 = t_Vector2<int8>;
using UIVector2 = t_Vector2<uint32>;
using USVector2 = t_Vector2<uint16>;
using UCVector2 = t_Vector2<uint8>;

using Vector = t_Vector<float>;
using FVector = Vector;
using IVector = t_Vector<int32>;
using SVector = t_Vector<int16>;
using CVector = t_Vector<int8>;
using UIVector = t_Vector<uint32>;
using USVector = t_Vector<uint16>;
using UCVector = t_Vector<uint8>;

using Vector4 = t_Vector4<float>;
using FVector4 = Vector4;
using IVector4 = t_Vector4<int32>;
using SVector4 = t_Vector4<int16>;
using CVector4 = t_Vector4<int8>;
using UIVector4 = t_Vector4<uint32>;
using USVector4 = t_Vector4<uint16>;
using UCVector4 = t_Vector4<uint8>;
