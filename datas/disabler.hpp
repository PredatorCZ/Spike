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
#define _DISABLER_E(func) struct disabler##func {\
template<typename U, void (U::*f)()> struct detectorClass {};\
template<class C> static constexpr std::false_type detectorFunc(...) {};\
template<class C> static constexpr std::true_type detectorFunc(detectorClass<C, &C::func>*) {}; };

#define ADD_DISABLERS(classname, ...) typedef classname this_type;\
StaticFor(_DISABLER_E, __VA_ARGS__)

#define disabledFunction(disableVar,retrnType)template<class SC = this_type, bool enabled = decltype(disabler##disableVar::detectorFunc<this_type>(nullptr))::value>\
typename std::enable_if<enabled, retrnType>::type

#define enabledFunction(disableVar,retrnType)template<class SC = this_type, bool enabled = decltype(disabler##disableVar::detectorFunc<this_type>(nullptr))::value>\
typename std::enable_if<!enabled, retrnType>::type
