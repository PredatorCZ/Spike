#pragma once
#include "MapsCore.h"

class Mix : public MapBase
{
	enum Params_e
	{
		eMixAmmount,
		eLower,
		eUpper,
		eUseCurve,
		eColor1,
		eColor2,
		eMap1,
		eMap2,
		eMask,
		eMap1Active,
		eMap2Active,
		eMaskActive,
		eOutput
	};
public:
	
	ES_FORCEINLINE Mix()
	{ 
		tmapRef = static_cast<Texmap*>(GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, Class_ID(MIX_CLASS_ID, 0)));
		params = tmapRef->GetParamBlock(0);
	}

	ES_FORCEINLINE Mix(Texmap *map1, Texmap *map2, float mix = 50.f) : Mix()
	{
		Map1(map1);
		Map2(map2);
		MixAmmount(mix);
	}
	ES_FORCEINLINE Mix(Texmap *map1, Texmap *map2, Texmap *mask) : Mix()
	{
		Map1(map1);
		Map2(map2);
		Mask(mask);
	}

	ACCFUNC(MixAmmount, float);
	ACCFUNC(Lower, float);
	ACCFUNC(Upper, float);
	ACCFUNC(UseCurve, int);
	ACCFUNC(Color1, Color);
	ACCFUNC(Color2, Color);
	ACCFUNC(Map1, Texmap*);
	ACCFUNC(Map2, Texmap*);
	ACCFUNC(Mask, Texmap*);
	ACCFUNC(Map1Active, int);
	ACCFUNC(Map2Active, int);
	ACCFUNC(MaskActive, int);
	ACCFUNC_CAST(Output, ReferenceTarget*, TextureOutput*);
};