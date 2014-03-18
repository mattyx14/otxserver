////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#include "inputbox.h"
#include "otpch.h"

#if defined(WINDOWS) && !defined(_CONSOLE)
HFONT CInputBox::m_hFont = NULL;
HWND  CInputBox::m_hWndInputBox = NULL;
HWND  CInputBox::m_hWndParent = NULL;
HWND  CInputBox::m_hWndEdit = NULL;
HWND  CInputBox::m_hWndOK = NULL;
HWND  CInputBox::m_hWndCancel = NULL;
HWND  CInputBox::m_hWndPrompt = NULL;

HINSTANCE CInputBox::m_hInst = NULL;

CInputBox::CInputBox(HWND hWndParent)
{
	HINSTANCE hInst = GetModuleHandle(NULL);
	WNDCLASSEX wcex;
	if (!GetClassInfoEx(hInst, "InputBox", &wcex))
	{
		wcex.cbSize 		= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInst;
		wcex.hIcon			= NULL;//LoadIcon(hInst, (LPCTSTR)IDI_MYINPUTBOX);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_BACKGROUND);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= "InputBox";
		wcex.hIconSm		= NULL;
		if(RegisterClassEx(&wcex) == 0)
			MessageBoxA(NULL, "Cannot create InputBox!", "Error", MB_OK);
	}
	m_hWndParent = hWndParent;
	Text = NULL;
}

CInputBox::~CInputBox()
{
	delete[] Text;
}

LRESULT CALLBACK CInputBox::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LOGFONT lfont;
	switch(message)
	{
		case WM_CREATE:
			memset(&lfont, 0, sizeof(lfont));
			lstrcpy(lfont.lfFaceName, _T("Arial"));
			lfont.lfHeight = 16;
			lfont.lfWeight = FW_NORMAL;
			lfont.lfItalic = FALSE;
			lfont.lfCharSet = DEFAULT_CHARSET;
			lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lfont.lfQuality = DEFAULT_QUALITY;
			lfont.lfPitchAndFamily = DEFAULT_PITCH;
			m_hFont = CreateFontIndirect(&lfont);
			m_hInst = GetModuleHandle(NULL);
			m_hWndEdit = CreateWindowEx(WS_EX_STATICEDGE, "edit", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 5, INPUTBOX_HEIGHT - 50, INPUTBOX_WIDTH - 16, 20, hWnd, NULL, m_hInst, NULL);
			SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)m_hFont, 0);
			m_hWndOK = CreateWindowEx(0, "button", "OK", WS_VISIBLE | WS_CHILD | WS_TABSTOP, INPUTBOX_WIDTH - 100, 10, 90, 25, hWnd, NULL, m_hInst, NULL);
			SendMessage(m_hWndOK, WM_SETFONT, (WPARAM)m_hFont, 0);
			m_hWndCancel = CreateWindowEx(0, "button", "Cancel", WS_VISIBLE | WS_CHILD | WS_TABSTOP, INPUTBOX_WIDTH - 100, 40, 90, 25, hWnd, NULL, m_hInst, NULL);
			SendMessage(m_hWndCancel, WM_SETFONT, (WPARAM)m_hFont, 0);
			m_hWndPrompt = CreateWindowEx(WS_EX_STATICEDGE, "static", "", WS_VISIBLE | WS_CHILD, 5, 10, INPUTBOX_WIDTH - 110, INPUTBOX_HEIGHT - 70, hWnd, NULL, m_hInst, NULL);
			SendMessage(m_hWndPrompt, WM_SETFONT, (WPARAM)m_hFont, 0);
			SetFocus(m_hWndEdit);
			break;
		case WM_DESTROY:
			DeleteObject(m_hFont);
			EnableWindow(m_hWndParent, TRUE);
			SetForegroundWindow(m_hWndParent);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					if((HWND)lParam == m_hWndOK)
						PostMessage(m_hWndInputBox, WM_KEYDOWN, VK_RETURN, 0);
					if((HWND)lParam == m_hWndCancel)
						PostMessage(m_hWndInputBox, WM_KEYDOWN, VK_ESCAPE, 0);
					break;
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CInputBox::DoModal(LPCTSTR szCaption, LPCTSTR szPrompt)
{
	RECT r;
	GetWindowRect(GetDesktopWindow(), &r);
	m_hWndInputBox = CreateWindowEx(WS_EX_TOOLWINDOW, "InputBox", szCaption, WS_POPUPWINDOW | WS_CAPTION | WS_TABSTOP, (r.right - INPUTBOX_WIDTH) / 2, (r.bottom - INPUTBOX_HEIGHT) / 2, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, m_hWndParent, NULL, m_hInst, NULL);
	if(m_hWndInputBox == NULL)
		return FALSE;
	SetWindowText(m_hWndPrompt, szPrompt);
	SetForegroundWindow(m_hWndInputBox);
	EnableWindow(m_hWndParent, FALSE);
	ShowWindow(m_hWndInputBox, SW_SHOW);
	UpdateWindow(m_hWndInputBox);
	BOOL ret = 0;
	MSG msg;
	HWND hWndFocused;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_KEYDOWN)
		{
			if(msg.wParam == VK_ESCAPE)
			{
				SendMessage(m_hWndInputBox, WM_DESTROY, 0, 0);
				ret = 0;
			}
			if(msg.wParam == VK_RETURN)
			{
				int nCount = GetWindowTextLength(m_hWndEdit);
				nCount++;
				if(Text)
				{
					delete[] Text;
					Text = NULL;
				}
				Text = new TCHAR[nCount];
				GetWindowText(m_hWndEdit, Text, nCount);
				SendMessage(m_hWndInputBox, WM_DESTROY, 0, 0);
				ret = 1;
			}
			if(msg.wParam == VK_TAB)
			{
				hWndFocused = GetFocus();
				if(hWndFocused == m_hWndEdit)
					SetFocus(m_hWndOK);
				if(hWndFocused == m_hWndOK)
					SetFocus(m_hWndCancel);
				if(hWndFocused == m_hWndCancel)
					SetFocus(m_hWndEdit);
			}
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return ret;
}
#endif
