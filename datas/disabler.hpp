/*	Function enabler/disabler for template classes,
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
#include "macroLoop.hpp"
#define _DISABLER_E(funcName)template<class _Cl> static constexpr std::true_type detector##funcName(detectorClass<_Cl, &_Cl::funcName>*);\
template<class _Cl> static constexpr std::false_type detector##funcName(...);

#define ADD_DISABLERS(classname, ...) typedef classname this_type;\
template<typename U, void (U::*f)()> struct detectorClass {};\
StaticFor(_DISABLER_E, __VA_ARGS__)

#define disabledFunction(funcName, retrnType)template<class SC = this_type, bool enabled = decltype(detector##funcName<this_type>(nullptr))::value>\
typename std::enable_if<enabled, retrnType>::type

#define enabledFunction(funcName, retrnType)template<class SC = this_type, bool enabled = decltype(detector##funcName<this_type>(nullptr))::value>\
typename std::enable_if<!enabled, retrnType>::type
