/*	Registering enum reflection IO
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
#include <map>

struct RefEnumFunctions
{
	int(*MultiConstructor)(const char*);
	void(*MultiDestructor)(std::string &, int value);
	int(*Constructor)(const char*);
	void(*Destructor)(std::string &, int value);
};

typedef std::map<unsigned int, RefEnumFunctions> RefEnumMapper;
extern RefEnumMapper REFEnumStorage;

template<class E>
int EnumConstructor_t(const char *value)
{
	E enumInstance = {};
	int resval = 0;

	for (int t = 0; t < enumInstance._reflectedSize; t++)
		if (!strcmp(value, enumInstance._reflected[t]))
		{
			resval = t;
			break;
		}
	return resval;
}

template<class E>
int EnumMultiConstructor_t(const char *value)
{
	E enumInstance = {};
	int resval = 0;
	const char *c = value;
	std::string currentValue = {};
	currentValue.reserve(64);

	while (true)
	{
		if (*c == ' ' || *c == '|' || !*c)
		{
			if (currentValue.size())
			{
				for (int t = 0; t < enumInstance._reflectedSize; t++)
					if (!currentValue.compare(enumInstance._reflected[t]))
					{
						resval |= 1 << t;
						break;
					}
				currentValue.clear();
			}
			else if (!*c)
			{
				break;
			}
			else
			{
				c++;
				continue;
			}
		}
		else
			currentValue.push_back(*c);

		c++;
	}

	return resval;
}

template<class E>
void EnumMultiDestructor_t(std::string &output, int value)
{
	E enumInstance = {};

	for (int i = 0; i < enumInstance._reflectedSize; i++)
		if (value & (1 << i))
		{
			if (output.size())
				output.append(" | ");

			output.append(enumInstance._reflected[i]);
		}

	if (!output.size())
		output = "NULL";
}

template<class E>
void EnumDestructor_t(std::string &output, int value)
{
	E enumInstance = {};

	if (value < enumInstance._reflectedSize)
		output = enumInstance._reflected[value];
}


#define REGISTER_ENUM(classname) REFEnumStorage[static_cast<const JenHash>(_EnumWrap<classname>::HASH)] = RefEnumFunctions{ EnumMultiConstructor_t<_EnumWrap<classname>>, EnumMultiDestructor_t<_EnumWrap<classname>>, EnumConstructor_t<_EnumWrap<classname>>, EnumDestructor_t<_EnumWrap<classname>> };
