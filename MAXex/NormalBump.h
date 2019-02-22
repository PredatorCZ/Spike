#pragma once
#include "MapsCore.h"
#ifndef GNORMAL_CLASS_ID
#define GNORMAL_CLASS_ID	Class_ID(0x243e22c6, 0x63f6a014)
#endif
class NormalBump : public MapBase
{
	enum Params_e
	{
		eNormalAmmount,
		eBumpAmmount,
		eNormal,
		eBump,
		eNormalActive,
		eBumpActive,
		eMethod,
		eFlipX,
		eFlipY,
		eSwapXY
	};
public:
	enum Method_e
	{
		Method_Tangent,
		Method_Local,
		Method_Scene,
		Methpd_World
	};
	ES_FORCEINLINE NormalBump(Texmap *input) : NormalBump()
	{
		Normal(input);
	}

	ES_FORCEINLINE NormalBump() 
	{ 
		tmapRef = static_cast<Texmap*>(GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID)); 
		params = tmapRef->GetParamBlock(0);
	}

	ACCFUNC(Normal, Texmap*);
	ACCFUNC(Bump, Texmap*);
	ACCFUNC(NormalAmmount, float);
	ACCFUNC(BumpAmmount, float);
	ACCFUNC(NormalActive, int);
	ACCFUNC(BumpActive, int);
	ACCFUNC_ENUM(Method);
	ACCFUNC(FlipX, int);
	ACCFUNC(FlipY, int);
	ACCFUNC(SwapXY, int);
};