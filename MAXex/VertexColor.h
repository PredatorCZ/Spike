#pragma once
#include "MapsCore.h"

class VertexColor : public MapBase
{
	enum Params_e
	{
		eUVChannel,
		eOutputColor
	};
public:
	enum OutputColor_e
	{
		All,
		Red,
		Green,
		Blue
	};
	ES_FORCEINLINE VertexColor()
	{ 
		tmapRef = static_cast<Texmap*>(GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, Class_ID(VCOL_CLASS_ID, 0)));
		params = tmapRef->GetParamBlock(0);
	}

	ACCFUNC(UVChannel, int);
	ACCFUNC_ENUM(OutputColor);
};