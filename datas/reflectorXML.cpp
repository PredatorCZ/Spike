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
#include "masterprinter.hpp"
#include "pugiex.hpp"

#ifndef _MSC_VER
#define sprintf_s(buffer, sizeofBuffer, format, ...) sprintf(buffer, format, __VA_ARGS__)
#endif

template<class _Ty0>
Reflector::xmlNodePtr Reflector::_ToXML(const _Ty0 *filename, bool asNewNode) const
{
	pugi::xml_document doc = {};

	ToXML(doc, asNewNode);

	doc.save_file(filename, "\t", pugi::format_write_bom | pugi::format_indent);

	return doc.internal_object();
}

template Reflector::xmlNodePtr Reflector::_ToXML<char>(const char*, bool) const;
template Reflector::xmlNodePtr Reflector::_ToXML<wchar_t>(const wchar_t *, bool) const;

Reflector::xmlNodePtr Reflector::ToXML(pugi::xml_node &node, bool asNewNode) const
{
	const reflectorStatic *stat = _rfRetreive().rfStatic;
	pugi::xml_node thisNode = node;

	if (asNewNode)
	{
		std::string className;

		if (UseNames())
			className = stat->className;
		else
		{
			className.resize(15);
			sprintf_s(const_cast<char *>(className.c_str()), 15, "h:%X", stat->classHash);
		}

		thisNode = node.append_child(className.c_str());
	}

	for (int t = 0; t < stat->nTypes; t++)
	{	
		std::string varName;

		if (UseNames())
		{
			varName = stat->typeNames[t];
			size_t fndBrace = varName.find('[');
			size_t fndBraceEnd = varName.find(']');

			if (fndBraceEnd != varName.npos)
				varName.pop_back();

			if (fndBrace != varName.npos)
				varName.replace(fndBrace, 1, 1, ':');
		}
		else
		{
			varName.resize(15);
			sprintf_s(const_cast<char *>(varName.c_str()), 15, "h:%X", stat->types[t].valueNameHash);
		}

		pugi::xml_node cNode = thisNode.append_child(varName.c_str());
		
		if (IsReflectedSubClass(t))
		{
			ReflectorSubClass subCl(GetReflectedSubClass(t));
			subCl.ToXML(cNode, false);
		}
		else
		{
			std::string str = GetReflectedValue(t);
			cNode.append_buffer(str.c_str(), str.size());
		}
	}
	return thisNode.internal_object();
}

template<class _Ty1>
Reflector::xmlNodePtr Reflector::_FromXML(const _Ty1 *filename, bool lookupClassNode)
{
	pugi::xml_document doc = {};
	auto reslt = doc.load_file(filename);

	if (!reslt)
	{
		printerror("[Reflector] Couldn't load xml file. " << _EnumWrap<XMLError>{}._reflected[reslt.status] << " at offset: " << reslt.offset);
		return nullptr;
	}

	return FromXML(doc, lookupClassNode);
}

template Reflector::xmlNodePtr Reflector::_FromXML<char>(const char *, bool);
template Reflector::xmlNodePtr Reflector::_FromXML<wchar_t>(const wchar_t *, bool);

Reflector::xmlNodePtr Reflector::FromXML(pugi::xml_node &node, bool lookupClassNode)
{
	const reflectorStatic *stat = _rfRetreive().rfStatic;
	pugi::xml_node thisNode;
	
	if (lookupClassNode)
	{
		for (auto &a : node.children())
		{
			JenHash vHash = (*a.name() == 'h' && *(a.name() + 1) == ':') ?
				strtoul(a.name() + 2, nullptr, 16) :
				JenkinsHash(a.name(), static_cast<int>(strlen(a.name())));

			if (vHash == stat->classHash)
			{
				thisNode = a;
				break;
			}
		}
	}
	else
		thisNode = node;

	
	if (!thisNode.empty())
		for (auto &a : thisNode.children())
		{
			JenHash vHash = (*a.name() == 'h' && *(a.name() + 1) == ':') ?
				strtoul(a.name() + 2, nullptr, 16) :
				JenkinsHash(a.name(), static_cast<int>(strlen(a.name())));

			SetReflectedValue(vHash, a.text().as_string());
		}

	return thisNode.internal_object();
}