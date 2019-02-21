#pragma once
#include <vector>
#include <algorithm>
#include "datas/supercore.hpp"

class INode;

class INodeSuffixer : public std::vector<int>
{
	bool skin : 1, morph : 1, normals : 1, color : 1;

public:
	ES_FORCEINLINE INodeSuffixer() : skin(false), morph(false), normals(false), color(false) { reserve(4); }
	ES_FORCEINLINE void AddChannel(int id) { push_back(id); }
	ES_FORCEINLINE void AddColor() 
	{ 
		if (!color)
		{
			push_back(0);
			color = true;
		}
	}
	ES_FORCEINLINE void AddFullColor() 
	{ 
		if (!color)
			push_back(0);

		push_back(-2);
		color = true;
	}

	ES_FORCEINLINE void UseNormals() { normals = true; }
	ES_FORCEINLINE void UseSkin() { normals = true; }
	ES_FORCEINLINE void UseMorph() { normals = true; }

	ES_INLINE TSTRING Generate(TSTRING *className = nullptr)
	{
		TSTRING retString = {};
		retString.reserve(5 + size() + (className ? 2 + className->size() : 0));

		if (className)
		{
			retString.push_back('(');
			retString.append(*className);
		}

		retString.push_back('[');

		if (size())
		{
			std::sort(begin(), end());
			retString.push_back('U');

			for (auto &n : *this)
				retString.append(ToTSTRING(n));
		}

		if (normals)
			retString.push_back('N');

		if (skin)
			retString.push_back('S');

		if (morph)
			retString.push_back('M');

		retString.push_back(']');

		if (className)
			retString.push_back(')');

		return retString;
	}

	ES_FORCEINLINE operator INode*() { return node; }

public:
	INode *node;
};