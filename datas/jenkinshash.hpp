/*	compile time Jenkins one at time hashing function
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

typedef unsigned int JenHash;
typedef unsigned long long _SuperJenHash;
#define _SuperResVal static_cast<_SuperJenHash>(resval)

constexpr JenHash JenkinsHash(const char* str, const int strlen, const JenHash resval = 0, const int index = 0)
{
	return index < strlen ? JenkinsHash(str, strlen,
		((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(str[index]))) + ((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(str[index]))) << 10)) ^
		(((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(str[index]))) + ((_SuperResVal + static_cast<JenHash>(static_cast<const unsigned char>(str[index]))) << 10)) >> 6),
		index + 1) :
		((_SuperResVal + (_SuperResVal << 3)) ^ ((_SuperResVal + (_SuperResVal << 3)) >> 11)) + (((_SuperResVal + (_SuperResVal << 3)) ^ ((_SuperResVal + (_SuperResVal << 3)) >> 11)) << 15);
}
