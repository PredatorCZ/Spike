#pragma once
#include "MapsCore.h"

class CompositeTex
{
	struct Interface : FPMixinInterface 
	{
		virtual int Count() = 0;
		virtual void AddLayer() = 0;
		virtual void DeleteLayer(int index) = 0;
		virtual void DuplicateLayer(int index) = 0;
		virtual void MoveLayer(int fromIndex, int toIndex, bool before) = 0;
	};
public:
	enum BlendMode_e
	{
		Normal,
		Average,
		Add,
		Subtract,
		Darken,
		Multiply,
		ColorBurn,
		LinearBurn,
		Lighten,
		Screen,
		ColorDodge,
		LinearDodge,
		Spotlight,
		SpotlightBlend,
		Overlay,
		SoftLight,
		HardLight,
		PinLight,
		HardMix,
		Difference,
		Exclusion,
		Hue,
		Saturation,
		Color,
		Value
	};
	class Layer : public MapBase
	{
		enum Params_e
		{
			eMapEnabled = 1,
			eMaskEnabled = 3,
			eBlendMode = 5,
			eName,
			eDialogOpened,
			eOpacity,
			eMap,
			eMask
		};
	public:
		Layer(IParamBlock2 *ipar, int idx) { params = ipar; index = idx; }
		Layer() = delete;

		ACCFUNC(MapEnabled, int);
		ACCFUNC(MaskEnabled, int);
		ACCFUNC_ENUM(BlendMode);
		ACCFUNC(Name, const MCHAR*);
		ACCFUNC(Opacity, float);
		ACCFUNC(Map, Texmap*);
		ACCFUNC(Mask, Texmap*);
	};
private:
	ES_FORCEINLINE void _assign(MultiTex *input) 
	{
		tmapRef = input;
		cInterface = ((Interface*)input->GetInterface(Interface_ID(0x5f666b75, 0x5f4367db)));
		params = input->GetParamBlock(0);
	}
public:
	ES_FORCEINLINE CompositeTex()
	{
		tmapRef = static_cast<MultiTex*>(NewDefaultCompositeTex());
		_assign(tmapRef);
	}

	ES_FORCEINLINE CompositeTex(MultiTex *input) { _assign(input); }

	ES_FORCEINLINE const int NumLayers() { return cInterface->Count(); }
	ES_FORCEINLINE const Layer AddLayer() { cInterface->AddLayer(); return Layer(params, NumLayers() - 1); }
	ES_FORCEINLINE void DeleteLayer(int index)
	{ 
		int numLayers = NumLayers();

		if (index < 1 && numLayers == 1)
		{
			_DEBUG_ERROR("Cannot delete only last layer.");
			return;
		}

		if (index >= numLayers)
		{
			_DEBUG_ERROR("Index out of range.");
			return;
		}

		cInterface->DeleteLayer(index); 
	}
	ES_FORCEINLINE void DuplicateLayer(int index) { cInterface->DuplicateLayer(index); }
	ES_FORCEINLINE void MoveLayer(int fromIndex, int toIndex, bool before) { cInterface->MoveLayer(fromIndex, toIndex, before); }
	ES_FORCEINLINE const Layer GetLayer(int index) { return Layer(params, index); }

	ES_FORCEINLINE void Resize(int newSize)
	{
		if (newSize < 1)
		{
			_DEBUG_ERROR("New size must be > 0");
			return;
		}

		int numLayers = newSize - NumLayers();

		if (!numLayers)
			return;

		if (numLayers > 0)
		{
			for (int t = 0; t < numLayers; t++)
				AddLayer();
		}
		else
		{
			numLayers *= -1;

			int _numLayers = NumLayers();

			for (int t = 1; t <= numLayers; t++)
				DeleteLayer(_numLayers - t);
		}
	}


	operator MultiTex*() { return tmapRef; }
	operator Texmap*() { return tmapRef; }
private:
	IParamBlock2 *params;
	Interface *cInterface;
	MultiTex *tmapRef;
};