#pragma once
#include "MapsCore.h"

#define PHYSIC_MAT_CLASSID Class_ID(0x3d6b1cec, 0xDEADC001)

class PhysicalMaterial : public MaterialBase
{
	enum Params_e
	{
		eMaterialMode,
		eBaseWeight,
		eBaseColor,
		eReflectivity,
		eRoughness,
		eInvertRoughness,
		eMetalness,
		eReflectionColor = 10,
		eDiffRoughness,
		eBRDFMode,
		eBRDFLow,
		eBRDFHigh,
		eBRDFCurve,
		eAnisotropy = 20,
		eAnisotropyAngle,
		eAnisotropyMode,
		eAnisotropyChannel,
		eTransparency = 30,
		eTransparencyColor,
		eTransparencyDepth,
		eTransparencyRoughness = 35,
		eInvertTransparencyRoughness,
		eLockTransparencyRoughness = 34,
		eIOR = 33,
		eThinWalled = 37,
		eScattering = 40,
		eSSSColor,
		eSSSDepth,
		eSSSScale,
		eSSSScatterColor,
		eEmission = 50,
		eEmissionColor,
		eEmissionLuminance,
		eEmissionKelvin = 54,
		eCoating = 60,
		eCoatingColor,
		eCoatingRoughness,
		eInvertCoatingRoughness,
		eCoatingAffectColor,
		eCoatingAffectRoughness,
		eCoatingIOR,
		eBaseWeightMap = 100,
		eBaseColorMap,
		eReflectMap,
		eReflColorMap,
		eRoughnessMap,
		eMetalnessMap,
		eDiffRoughnessMap,
		eAnisotropyMap,
		eAnisoAngleMap,
		eTransparencyMap,
		eTransColorMap,
		eTransRoughnessMap,
		eIORMap,
		eScatteringMap,
		eSSSColorMap,
		sSSSScaleMap,
		eEmissionMap,
		eEmisionColorMap,
		eCoatingMap,
		eCoatingColorMap,
		eCoatingRoughnessMap,
		eBumpMap = 130,
		eCoatingBumpMap,
		eDisplacementMap,
		eCutoutMap,
		eBaseWeightMapActive = 150,
		eBaseColorMapActive,
		eReflectMapActive,
		eReflColorMapActive,
		eRoughnessMapActive,
		eMetalnessMapActive,
		eDiffRoughnessMapActive,
		eAnisotropyMapActive,
		eAnisoAngleMapActive,
		eTransparencyMapActive,
		eTransColorMapActive,
		eTransRoughnessMapActive,
		eIORMapActive,
		eScatteringMActiveap,
		eSSSColorMapActive,
		eSSSScaleMapActive,
		eEmissionMapActive,
		eEmisionColorMapActive,
		eCoatingMapActive,
		eCoatingColorMapActive,
		eCoatingRoughnessMapActive,
		eBumpMapActive = 180,
		eCoatingBumpMapActive,
		eDisplacementMapActive,
		eCutoutMapActive,
		eBumpMapAmmount = 230,
		eCoatingBumpMapAmmount,
		eDisplacementMapAmmount
	};
public:
	enum MaterialMode_e
	{
		MaterialMode_Standard,
		MaterialMode_Advanced
	};
	enum BRDFMode_e
	{
		BRDFMode_CustomCurve,
		BRDFMode_IOR
	};
	enum AnisotropyMode_e
	{
		AnisotropyMode_Auto,
		AnisotropyMode_MapChannel
	};
	enum Maps_e
	{
		BaseWeightMap,
		BaseColorMap,
		ReflectMap,
		ReflColorMap,
		RoughnessMap,
		MetalnessMap,
		DiffRoughnessMap,
		AnisotropyMap,
		AnisoAngleMap,
		TransparencyMap,
		TransColorMap,
		TransRoughnessMap,
		IORMap,
		ScatteringMap,
		SSSColorMap,
		SSSScaleMap,
		EmissionMap,
		EmisionColorMap,
		CoatingMap,
		CoatingColorMap,
		CoatingRoughnessMap,
		BumpMap = 30,
		CoatingBumpMap,
		DisplacementMap,
		CutoutMap
	};

	ES_FORCEINLINE PhysicalMaterial()
	{ 
		tmapRef = static_cast<StdMat2*>(GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PHYSIC_MAT_CLASSID));
		params = tmapRef->GetParamBlock(0);

		/*for (int i = 0; i < params->GetDesc()->count; i++)
		{
			printer << params->GetDesc()->paramdefs[i].int_name >> 1;
			printer << params->GetDesc()->paramdefs[i].ID >> 1;
		}*/

	}

	ES_FORCEINLINE PhysicalMaterial(StdMat2 *input)
	{
		tmapRef = input;
		params = tmapRef->GetParamBlock(0);
	}

	ACCFUNC_ENUM(MaterialMode);
	ACCFUNC(BaseWeight, float);
	ACCFUNC(BaseColor, Color);
	ACCFUNC(Reflectivity, float);
	ACCFUNC(Roughness, float);
	ACCFUNC(InvertRoughness, int);
	ACCFUNC(Metalness, float);
	ACCFUNC(ReflectionColor, Color);
	ACCFUNC(DiffRoughness, float);
	ACCFUNC_ENUM(BRDFMode);
	ACCFUNC(BRDFLow, float);
	ACCFUNC(BRDFHigh, float);
	ACCFUNC(BRDFCurve, float);
	ACCFUNC(Anisotropy, float);
	ACCFUNC(AnisotropyAngle, float);
	ACCFUNC_ENUM(AnisotropyMode);
	ACCFUNC(AnisotropyChannel, int);
	ACCFUNC(Transparency, float);
	ACCFUNC(TransparencyColor, Color);
	ACCFUNC(TransparencyDepth, float);
	ACCFUNC(TransparencyRoughness, float);
	ACCFUNC(InvertTransparencyRoughness, int);
	ACCFUNC(LockTransparencyRoughness, int);
	ACCFUNC(IOR, float);
	ACCFUNC(ThinWalled, int);
	ACCFUNC(Scattering, float);
	ACCFUNC(SSSColor, Color);
	ACCFUNC(SSSDepth, float);
	ACCFUNC(SSSScale, float);
	ACCFUNC(SSSScatterColor, Color);
	ACCFUNC(Emission, float);
	ACCFUNC(EmissionColor, Color);
	ACCFUNC(EmissionLuminance, float);
	ACCFUNC(EmissionKelvin, float);
	ACCFUNC(Coating, float);
	ACCFUNC(CoatingColor, Color);
	ACCFUNC(CoatingRoughness, float);
	ACCFUNC(InvertCoatingRoughness, int);
	ACCFUNC(CoatingAffectColor, float);
	ACCFUNC(CoatingAffectRoughness, float);
	ACCFUNC(CoatingIOR, float);
	ACCFUNC(BaseWeightMapActive, int);
	ACCFUNC(BaseColorMapActive, int);
	ACCFUNC(ReflectMapActive, int);
	ACCFUNC(ReflColorMapActive, int);
	ACCFUNC(RoughnessMapActive, int);
	ACCFUNC(MetalnessMapActive, int);
	ACCFUNC(DiffRoughnessMapActive, int);
	ACCFUNC(AnisotropyMapActive, int);
	ACCFUNC(AnisoAngleMapActive, int);
	ACCFUNC(TransparencyMapActive, int);
	ACCFUNC(TransColorMapActive, int);
	ACCFUNC(TransRoughnessMapActive, int);
	ACCFUNC(IORMapActive, int);
	ACCFUNC(ScatteringMActiveap, int);
	ACCFUNC(SSSColorMapActive, int);
	ACCFUNC(SSSScaleMapActive, int);
	ACCFUNC(EmissionMapActive, int);
	ACCFUNC(EmisionColorMapActive, int);
	ACCFUNC(CoatingMapActive, int);
	ACCFUNC(CoatingColorMapActive, int);
	ACCFUNC(CoatingRoughnessMapActive, int);
	ACCFUNC(BumpMapActive, int);
	ACCFUNC(CoatingBumpMapActive, int);
	ACCFUNC(DisplacementMapActive, int);
	ACCFUNC(CutoutMapActive, int);
	ACCFUNC(BumpMapAmmount, float);
	ACCFUNC(CoatingBumpMapAmmount, float);
	ACCFUNC(DisplacementMapAmmount, float);
};