#define IDConfigValue(id) id##_value
#define IDConfigBool(id) id##_checked
#define IDConfigVisible(id) id##_enabled

#define NewIDConfigBool(id) bool id##_checked : 1
#define NewIDConfigVisible(id) bool id##_enabled : 1
#define NewIDConfigValue(id) float id##_value

#define IDConfigText(id) _T(# id)

#define GetCFGString(name, key)*buffer = 0; GetPrivateProfileString(name, key, _T(""), buffer, sizeof(buffer)/2, CFGFile);
#define GetCFGEnabled(name) GetCFGString(_T(# name),_T("enabled")) if (*buffer) name##_enabled = *buffer == 't'; EnableWindow(GetDlgItem(hWnd, name), name##_enabled);
#define GetCFGChecked(name) GetCFGString(_T(# name),_T("checked")) if (*buffer) name##_checked = *buffer == 't'; CheckDlgButton(hWnd, name, name##_checked);
#define GetCFGCValue(name) GetCFGString(_T(# name),_T("value")) if (*buffer) name##_value = _ttof(buffer);
#define GetCFGCIndex(name) GetCFGString(_T(# name),_T("index")) if (*buffer) name##_index = _ttoi(buffer);

#define SetCFGString(name, key, value) WritePrivateProfileString(name, key, value, CFGFile);
#define SetCFGEnabled(name) SetCFGString(_T(# name),_T("enabled"), name##_enabled ? _T("true"):_T("false"));
#define SetCFGChecked(name) SetCFGString(_T(# name),_T("checked"), name##_checked ? _T("true"):_T("false"));

#define SetCFGValue(name) _stprintf_s(buffer, _T("%f"),name##_value ); SetCFGString(_T(# name),_T("value"), buffer);



#define MSGCheckbox(itemid) case itemid: imp->##itemid##_checked = IsDlgButtonChecked(hWnd,itemid) != 0;
#define MSGEnable(itemid, itemenable) imp->##itemenable##_enabled = imp->##itemid##_checked; EnableWindow(GetDlgItem(hWnd, itemenable), imp->##itemid##_checked);

static const TCHAR hkpresetgroup[] = _T("HK_PRESET");
static const TCHAR _cormatElements[] = { 'X', 'Y', 'Z' };

#define SetCFGHKMatrix int bufferPos = 0;\
for (int r = 0; r < 3; r++)\
	for (int e = 0; e < 3; e++)\
		if (corMat[r][e] != 0.0f)\
		{\
			if (corMat[r][e] < 0.0f)\
				buffer[bufferPos++] = '-';\
			buffer[bufferPos++] = _cormatElements[e];\
		}\
buffer[bufferPos] = 0;\
SetCFGString(hkpresetgroup, _T("Matrix"), buffer);
