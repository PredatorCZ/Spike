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

using V4SimdIntType = V4SimdIntType_t<mint32>;
using mint32x4a16 = t_Vector4_<V4SimdIntType>;
using muint32x4a16 = t_Vector4_<V4SimdIntType_t<muint32>>;
using mreal32x4a16 = t_Vector4_<V4SimdFltType>;

using mreal32x2 = t_Vector2<mreal32>;
using mint32x2 = t_Vector2<mint32>;
using mint16x2 = t_Vector2<mint16>;
using mint8x2 = t_Vector2<mint8>;
using muint32x2 = t_Vector2<muint32>;
using muint16x2 = t_Vector2<muint16>;
using muint8x2 = t_Vector2<muint8>;

using mreal32x3 = t_Vector<mreal32>;
using mint32x3 = t_Vector<mint32>;
using mint16x3 = t_Vector<mint16>;
using mint8x3 = t_Vector<mint8>;
using muint32x3 = t_Vector<muint32>;
using muint16x3 = t_Vector<muint16>;
using muint8x3 = t_Vector<muint8>;

using mreal32x4 = t_Vector4<mreal32>;
using mint32x4 = t_Vector4<mint32>;
using mint16x4 = t_Vector4<mint16>;
using mint8x4 = t_Vector4<mint8>;
using muint32x4 = t_Vector4<muint32>;
using muint16x4 = t_Vector4<muint16>;
using muint8x4 = t_Vector4<muint8>;

using int32x4a16 = const mint32x4a16;
using uint32x4a16 = const muint32x4a16;
using real32x4a16 = const mreal32x4a16;

using real32x2 = const mreal32x2;
using int32x2 = const mint32x2;
using int16x2 = const mint16x2;
using int8x2 = const mint8x2;
using uint32x2 = const muint32x2;
using uint16x2 = const muint16x2;
using uint8x2 = const muint8x2;

using real32x3 = const mreal32x3;
using int32x3 = const mint32x3;
using int16x3 = const mint16x3;
using int8x3 = const mint8x3;
using uint32x3 = const muint32x3;
using uint16x3 = const muint16x3;
using uint8x3 = const muint8x3;

using real32x4 = const mreal32x4;
using int32x4 = const mint32x4;
using int16x4 = const mint16x4;
using int8x4 = const mint8x4;
using uint32x4 = const muint32x4;
using uint16x4 = const muint16x4;
using uint8x4 = const muint8x4;
