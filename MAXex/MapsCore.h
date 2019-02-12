#pragma once
#include "datas/supercore.hpp"
#include <iparamm2.h>
#include <iFnPub.h>

#define ACCFUNC(fname, type) ES_FORCEINLINE void fname(type input) const { Set<Params_e, e##fname, type>(input); } \
ES_FORCEINLINE type fname() const { return Get<Params_e, e##fname, type>(); }

#define ACCFUNC_ENUM(fname) ES_FORCEINLINE void fname(fname##_e input) const { Set<Params_e, e##fname, int>(input); } \
ES_FORCEINLINE fname##_e fname() const { return static_cast<fname##_e>(Get<Params_e, e##fname, int>()); }

#define ACCFUNC_CAST(fname, type, outType) ES_FORCEINLINE void fname(type input) const { Set<Params_e, e##fname, type>(input); } \
ES_FORCEINLINE type fname() const { return static_cast<outType>(Get<Params_e, e##fname, type>()); }

template<class C>class MapBase_t
{
protected:
	template<class C, C E, class T> ES_FORCEINLINE T Get() const { T retval; ivalid.SetInfinite(); params->GetValue(E, 0, retval, ivalid, index); return retval; }
	template<class C, C E, class T> ES_FORCEINLINE void Set(T ival) const { params->SetValue(E, 0, ival, index); }
protected:
	IParamBlock2 *params;
	C *tmapRef;
	mutable Interval ivalid;
	int index;
public:
	operator C*() { return tmapRef; }
	MapBase_t() : params(nullptr), tmapRef(nullptr), index(0) {}
};

typedef MapBase_t<Texmap> MapBase;
typedef MapBase_t<StdMat2> MaterialBase;