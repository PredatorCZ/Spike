#include "resource.h"
#include <Richedit.h>
#include <gdiplus.h>

static const TCHAR _license00[] = _T("\
 is free software : you can redistribute it and/or modify\
 it under the terms of the GNU General Public License as published by\
 the Free Software Foundation, either version 3 of the License, or\
 (at your option) any later version.\n\n");
static const TCHAR _license01[] = _T("\
 is distributed in the hope that it will be useful,\
 but WITHOUT ANY WARRANTY; without even the implied warranty of\
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\
 GNU General Public License for more details.\n\n");
static const TCHAR _license02[] = _T("\
You should have received a copy of the GNU General Public License\
 along with this program. If not, see : <https://www.gnu.org/licenses/>\n\n");

static INT_PTR CALLBACK AboutCallbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		CenterWindow(hWnd, GetParent(hWnd));
		const int licBuffSize00 = sizeof(_name) + sizeof(_info);
		TCHAR * licBuff = static_cast<TCHAR*>(GlobalAlloc(GPTR, licBuffSize00));
		_tcscat_s(licBuff, licBuffSize00, _name);
		_tcscat_s(licBuff, licBuffSize00, _info);
		SetDlgItemText(hWnd, IDC_LB_INFO, licBuff);
		GlobalFree(licBuff);

		const int licBuffSize = sizeof(_license) + sizeof(_license00) + sizeof(_license01) + sizeof(_license02) + (sizeof(_name) * 2);
		licBuff = static_cast<TCHAR*>(GlobalAlloc(GPTR, licBuffSize));

		_tcscat_s(licBuff, licBuffSize, _name);
		_tcscat_s(licBuff, licBuffSize, _license00);
		_tcscat_s(licBuff, licBuffSize, _name);
		_tcscat_s(licBuff, licBuffSize, _license01);
		_tcscat_s(licBuff, licBuffSize, _license02);
		_tcscat_s(licBuff, licBuffSize, _license);
		SetDlgItemText(hWnd, IDC_RICHEDIT21, licBuff);
		GlobalFree(licBuff);

		SendDlgItemMessage(hWnd, IDC_RICHEDIT21, EM_SETBKGNDCOLOR, 0, 0x444444);

		CHARFORMATA tData = {};

		SendDlgItemMessage(hWnd, IDC_RICHEDIT21, EM_GETCHARFORMAT, SCF_ALL, (LPARAM)&tData);

		tData.cbSize = sizeof(tData);
		tData.dwMask = CFM_COLOR;
		tData.crTextColor = 0xffffff;;
		if (tData.dwEffects  & CFE_AUTOCOLOR) tData.dwEffects ^= CFE_AUTOCOLOR;

		SendDlgItemMessage(hWnd, IDC_RICHEDIT21, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&tData);

		LITEM item = {};
		item.iLink = 0;
		item.mask = LIF_ITEMINDEX | LIF_STATE;
		item.state = LIS_DEFAULTCOLORS;
		item.stateMask = LIS_DEFAULTCOLORS;

		SendDlgItemMessage(hWnd, IDC_SYSLINK1, LM_SETITEM, 0, (LPARAM)&item);

		SendDlgItemMessage(hWnd, IDC_SYSLINK2, LM_SETITEM, 0, (LPARAM)&item);

		HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(IDB_PNG1), _T("PNG"));
		DWORD imageSize = SizeofResource(hInstance, hResource);
		const void* pResourceData = LockResource(LoadResource(hInstance, hResource));
		HGLOBAL m_hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (m_hBuffer)
		{
			void* pBuffer = GlobalLock(m_hBuffer);
			if (pBuffer)
			{
				CopyMemory(pBuffer, pResourceData, imageSize);

				IStream* pStream = NULL;
				if (CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
				{
					HBITMAP hBmp;
					Gdiplus::Bitmap btm(pStream);
					btm.GetHBITMAP(0, &hBmp);
					SendDlgItemMessage(hWnd, IDC_ABOUTPICTURE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
					pStream->Release();
				}
			}
			GlobalFree(pBuffer);
			GlobalUnlock(m_hBuffer);
		}
		return TRUE;
	}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:
		case NM_RETURN:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SYSLINK1:
				ShellExecute(NULL, _T("open"), _homePage, NULL, NULL, SW_SHOWNORMAL);
				break;
			case IDC_SYSLINK2:
				ShellExecute(NULL, _T("open"), _T("https://lukascone.wordpress.com/support/"), NULL, NULL, SW_SHOWNORMAL);
				break;
			}	
			break;
		}
		}
		break;
	}

	return FALSE;
}

void ShowAboutDLG(HWND hWnd)
{
	DialogBoxParam(hInstance,
		MAKEINTRESOURCE(IDD_ABOUT),
		hWnd,
		AboutCallbacks, NULL);
}