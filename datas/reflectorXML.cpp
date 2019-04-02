/*	a XML I/O source for Reflector class
	more info in README for PreCore Project

	Copyright 2019 Lukas Cone

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

#include "reflector.hpp"
#include "pugixml.hpp"

int Reflector::ToXML(const TSTRING filename) const
{
	pugi::xml_document doc = {};

	if (ToXML(doc))
		return 1;

	doc.save_file(filename.c_str(), "\t", pugi::format_write_bom | pugi::format_indent);
	return 0;
}

int Reflector::ToXML(pugi::xml_node &node) const
{
	const reflectorStatic *stat = _rfRetreive().rfStatic;
	std::string className;

	if (UseNames())
		className = stat->className;
	else
	{
		className.resize(15);
		sprintf_s(const_cast<char *>(className.c_str()), 15, "h:%X", stat->classHash);
	}

	pugi::xml_node &thisNode = node.append_child(className.c_str());

	for (int t = 0; t < stat->nTypes; t++)
	{
		std::string &str = GetReflectedValue(t);
		std::string varName;

		if (UseNames())
			varName = stat->typeNames[t];
		else
		{
			varName.resize(15);
			sprintf_s(const_cast<char *>(varName.c_str()), 15, "h:%X", stat->types[t].valueNameHash);
		}

		pugi::xml_node &cNode = thisNode.append_child(varName.c_str());
		cNode.append_buffer(str.c_str(), str.size());
	}
	return 0;
}

int Reflector::FromXML(const TSTRING filename)
{
	pugi::xml_document doc = {};
	auto reslt = doc.load_file(filename.c_str());

	if (FromXML(doc))
		return 1;

	return 0;
}

int Reflector::FromXML(pugi::xml_node &node)
{
	const reflectorStatic *stat = _rfRetreive().rfStatic;
	pugi::xml_node thisNode;
	
	for (auto &a : node.children())
	{
		JenHash vHash = (*a.name() == 'h' && *(a.name() + 1) == ':') ? 
			strtoul(a.name() + 2, nullptr, 16) : 
			JenkinsHash(a.name(), strlen(a.name()));

		if (vHash == stat->classHash)
		{
			thisNode = a;
			break;
		}
	}
	
	if (thisNode.empty())
		return 1;

	for (auto &a : thisNode.children())
	{
		JenHash vHash = (*a.name() == 'h' && *(a.name() + 1) == ':') ?
			strtoul(a.name() + 2, nullptr, 16) :
			JenkinsHash(a.name(), strlen(a.name()));

		SetReflectedValue(vHash, a.text().as_string());
	}

	return 0;
}