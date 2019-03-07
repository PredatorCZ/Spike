#pragma once
#include "datas/flags.hpp"
#include "datas/macroLoop.hpp"

#define IDConfigValue(id) id##_value
#define IDConfigBool(id) id##_checked
#define IDConfigVisible(id) id##_enabled

#define IDDistributeBooleans(value) (1 << value) |

#define NewIDConfigValue(id) float id##_value

#define IDConfigText(id) _T(# id)
#define CFGBufferSize 16

ES_FORCEINLINE uint GetBufferValue(const TCHAR *name, const TCHAR *CFGFile, const TCHAR *key, TCHAR *buffer, const int bufferSize = CFGBufferSize)
{
	*buffer = 0; 
	return GetPrivateProfileString(name, key, _T(""), buffer, bufferSize, CFGFile);
}

template<class T, class E>
ES_FORCEINLINE void Enabled(const TCHAR *name, EnumFlags<T,E> &storage, E item, const TCHAR *CFGFile, TCHAR *buffer, HWND hWnd)
{
	GetBufferValue(name, CFGFile, _T("enabled"), buffer);
	
	bool value = storage[item];

	if (*buffer)
	{
		value = *buffer == 't';
		storage(item, value);
	}
	
	EnableWindow(hWnd, value);	
}

template<class T, class E>
ES_FORCEINLINE void Checked(const TCHAR *name, EnumFlags<T, E> &storage, E item, const TCHAR *CFGFile, TCHAR *buffer, HWND hWnd, int dlgItem)
{
	GetBufferValue(name, CFGFile, _T("checked"), buffer);

	bool value = storage[item];

	if (*buffer)
	{
		value = *buffer == 't';
		storage(item, value);
	}
	
	CheckDlgButton(hWnd, dlgItem, value);
}

ES_FORCEINLINE void GetValue(const TCHAR *name, float &value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *kVal = _T("value"))
{
	GetBufferValue(name, CFGFile, kVal, buffer);

	if (!*buffer)
		return;

	value = static_cast<float>(_ttof(buffer));
}

ES_FORCEINLINE void GetIndex(const TCHAR *name, int &value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *kVal = _T("index"))
{
	GetBufferValue(name, CFGFile, kVal, buffer);

	if (!*buffer)
		return;

	value = _ttoi(buffer);
}

ES_FORCEINLINE void GetText(const TCHAR *name, TSTRING &value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *kVal = _T("text"))
{
	GetBufferValue(name, CFGFile, kVal, buffer);

	if (!*buffer)
		return;

	TSTRING initialBufferValue = buffer;
	TCHAR countBuffer[3] = { *buffer, buffer[1], 0 };
	size_t dummy = 0;
	uint textSize = std::stoul(countBuffer, &dummy, 16);

	if (textSize > CFGBufferSize)
	{
		initialBufferValue.clear();
		initialBufferValue.resize(textSize);
		GetBufferValue(name, CFGFile, kVal, const_cast<TCHAR*>(initialBufferValue.c_str()), textSize);
	}

	value = initialBufferValue.substr(2);
}

ES_FORCEINLINE void WriteValue(const TCHAR *name, float value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *kVal = _T("value"))
{
	_stprintf_s(buffer, CFGBufferSize, _T("%f"), value);
	WritePrivateProfileString(name, kVal, buffer, CFGFile);
}

ES_FORCEINLINE void WriteIndex(const TCHAR *name, int value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *kVal = _T("index"))
{
	_stprintf_s(buffer, CFGBufferSize, _T("%i"), value);
	WritePrivateProfileString(name, kVal, buffer, CFGFile);
}

ES_FORCEINLINE void WriteText(const TCHAR *name, const TSTRING value, const TCHAR *CFGFile, const TCHAR *kVal = _T("text"))
{
	const int strSize = static_cast<int>(value.size() + 3);

	if (strSize < 3)
		return;

	TSTRING finalText;
	finalText.reserve(strSize);
	
	int strOffset = 0;

	if (strSize < 16)
	{
		strOffset = 1;
		finalText.push_back('0');
	}

	finalText.resize(2);

	_stprintf_s(const_cast<TCHAR*>(finalText.c_str() + strOffset), 3, _T("%X"), strSize);
	finalText.append(value);
	WritePrivateProfileString(name, kVal, finalText.c_str(), CFGFile);
}

#define GetCFGEnabled(name) Enabled(_T(# name), flags, name##_enabled, CFGFile, buffer, GetDlgItem(hWnd, name))
#define GetCFGChecked(name) Checked(_T(# name), flags, name##_checked, CFGFile, buffer, hWnd, name)
#define GetCFGCValue(name) GetValue(_T(# name), name##_value, CFGFile, buffer)
#define GetCFGCIndex(name) GetIndex(_T(# name), name##_index, CFGFile, buffer)
#define GetCFGCText(name) GetText(_T(# name), name, CFGFile, buffer)

#define SetCFGString(name, key, value) WritePrivateProfileString(name, key, value, CFGFile)
#define SetCFGEnabled(name) SetCFGString(_T(# name),_T("enabled"), flags[name##_enabled] ? _T("true"):_T("false"))
#define SetCFGChecked(name) SetCFGString(_T(# name),_T("checked"), flags[name##_checked] ? _T("true"):_T("false"))
#define SetCFGValue(name) WriteValue(_T(# name), name##_value, CFGFile, buffer)
#define SetCFGIndex(name) WriteIndex(_T(# name), name##_index, CFGFile, buffer)
#define SetCFGText(name) WriteText(_T(# name), name, CFGFile)

#define MSGCheckbox(itemid) case itemid: imp->flags(std::remove_pointer<decltype(imp)>::type::itemid##_checked, IsDlgButtonChecked(hWnd, itemid) != 0)
#define MSGEnable(itemid, itemenable) imp->flags(itemenable##_enabled,  imp->flags[itemid##_checked]); EnableWindow(GetDlgItem(hWnd, itemenable), imp->flags[itemid##_checked]);

static const TCHAR hkpresetgroup[] = _T("HK_PRESET");
static const TCHAR _cormatElements[] = { 'X', 'Y', 'Z' };


ES_INLINE void WriteCorrectionMatrix(const Matrix3 &value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *name = hkpresetgroup)
{
	int bufferPos = 0;

	for (int r = 0; r < 3; r++)
		for (int e = 0; e < 3; e++)
			if (value[r][e] != 0.0f)
			{
				if (value[r][e] < 0.0f)
					buffer[bufferPos++] = '-';

				buffer[bufferPos++] = _cormatElements[e];
			}

	buffer[bufferPos] = 0;

	WritePrivateProfileString(name, _T("Matrix"), buffer, CFGFile);
}

ES_INLINE void GetCorrectionMatrix(Matrix3 &value, const TCHAR *CFGFile, TCHAR *buffer, const TCHAR *name = hkpresetgroup)
{
	GetBufferValue(name, CFGFile, _T("Matrix"), buffer);

	if (!*buffer)
		return;

	float sign = 1.0f;
	int curRow = 0;

	for (int s = 0; s < 6 || curRow == 3; s++)
	{
		if (buffer[s] == '-')
			sign = -1.0f;
		else
		{
			switch (buffer[s])
			{
			case 'X':
				value.SetRow(curRow, Point3(sign, 0.0f, 0.0f));
				break;
			case 'Y':
				value.SetRow(curRow, Point3(0.0f, sign, 0.0f));
				break;
			case 'Z':
				value.SetRow(curRow, Point3(0.0f, 0.0f, sign));
				break;
			default:
				break;
			}
			curRow++;
			sign = 1.0f;
		}
	}	
}
