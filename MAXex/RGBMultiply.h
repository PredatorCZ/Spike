#pragma once
#include "MapsCore.h"

class RGBMultiply : public MapBase
{
	enum Params_e
	{
		eColor1,
		eColor2,
		eMap1,
		eMap2,
		eMap1Active,
		eMap2Active,
		eAlphaFrom
	};
public:
	enum AlphaFrom_e
	{
		AlphaFrom_Map1,
		AlphaFrom_Map2,
		AlphaFrom_Mult
	};

	ES_FORCEINLINE RGBMultiply()
	{ 
		tmapRef = static_cast<Texmap*>(GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, Class_ID(RGBMULT_CLASS_ID, 0)));
		params = tmapRef->GetParamBlock(0);
	}

	ES_FORCEINLINE RGBMultiply(Texmap *map1, Texmap *map2): RGBMultiply()
	{
		Map1(map1);
		Map2(map2);
	}

	ACCFUNC(Color1, Color);
	ACCFUNC(Color2, Color);
	ACCFUNC(Map1, Texmap*);
	ACCFUNC(Map2, Texmap*);
	ACCFUNC(Map1Active, int);
	ACCFUNC(Map2Active, int);
	ACCFUNC_ENUM(AlphaFrom);
};