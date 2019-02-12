/*	a header file that converts 16bit float
	more info in README for PreCore Project

	Copyright 2018-2019 Lukas Cone

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

static constexpr int hf_signMask = 0x8000;
static constexpr int hf_exponentMask = 0x7C00;
static constexpr int hf_fractionMask = (~(hf_exponentMask | hf_signMask)) & 0xffff;

ES_INLINE float halfToFloat(unsigned short half)
{
	int result = (half & hf_signMask) << 16;
	int exponent = ((half & hf_exponentMask) >> 10) - 16;
	exponent += 127;
	result |= (exponent << 23) | ((half & hf_fractionMask) << 13);
	return *reinterpret_cast<float*>(&result) * 2.f;
}

class float16
{
	unsigned short data;

public:
	float16() : data(0) {}
	explicit float16(float val) : data(0) {}
	float16(int val): data(val) {}
	operator float() const { return halfToFloat(data); }
};