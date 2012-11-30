#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "tea.h"
#include "dialog.h"

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))

static WLANKEY *keys;
static int numkeys;
static WNDPROC origComboEditWndProc;

/*
 * Based on DoAutoComplete() from AutoCombo.c
 */
static WCHAR DoAutoComplete(HWND hWnd, WCHAR ch){
	static WCHAR buf[256];
	static WCHAR toFind[256];
	int index = 0;

	if (ch == VK_BACK){
		index = ComboBox_GetCurSel(hWnd);
		int bs = LOWORD(ComboBox_GetEditSel(hWnd)) - 1;
		if (bs < 0) bs = 0;
		ComboBox_SetCurSel(hWnd, index);
		ComboBox_SetEditSel(hWnd, bs, -1);
	} else if (!iswcntrl(ch)){
		ComboBox_ShowDropdown(hWnd, TRUE);
		GetWindowTextW(hWnd, buf, NELEMS(buf));
		buf[LOWORD(ComboBox_GetEditSel(hWnd))] = 0;
		_snwprintf(toFind, NELEMS(toFind), L"%s%c", buf, ch);
		index = SendMessageW(hWnd, CB_FINDSTRINGEXACT, -1, (LPARAM)toFind);
		if (index == CB_ERR){
			index = SendMessageW(hWnd, CB_FINDSTRING, -1, (LPARAM)toFind);
		}
		if (index == CB_ERR){
			MessageBeep(0xFFFFFFFF);
		} else {
			ComboBox_SetCurSel(hWnd, index);
			ComboBox_SetEditSel(hWnd, wcslen(toFind), -1);
		}
	} else {
		return ch;
	}
	return 0;
}

/*
 * Based on ComboBox_Proc() from AutoCombo.c
 */
static LRESULT CALLBACK ComboBox_Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static HWND hCombo;
	WCHAR ch;

	switch (msg){
		case WM_CHAR:
			hCombo = GetParent(hWnd);
			ch = DoAutoComplete(hCombo, (WCHAR)wParam);
			if (ch != 0){
				return CallWindowProc(origComboEditWndProc, hWnd, msg, ch, lParam);
			}
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (DWORD)origComboEditWndProc);
			break;
		default:
			return CallWindowProc(origComboEditWndProc, hWnd, msg, wParam, lParam);
	}

	return FALSE;
}

/*
 * Based on MakeAutocompleteCombo() from AutoCombo.c
 */
static void MakeAutocompleteComboBox(HWND hCtl){
	HWND hEdit = FindWindowEx(hCtl, NULL, WC_EDIT, NULL);
	origComboEditWndProc = (WNDPROC)GetWindowLongPtr(hEdit, GWLP_WNDPROC);
	SubclassWindow(hEdit, ComboBox_Proc);
	ComboBox_LimitText(hCtl, 255);
}

static void WPASelect_DialogInit(HWND hWnd){
	int index;
	int i, j;
	//wchar_t name[257];
	WLAN_REASON_CODE reason;
	HWND hCtl = GetDlgItem (hWnd, IDC_WPASELECT);
	MakeAutocompleteComboBox(hCtl);
	
	for (i = 0; keys[i].ssid[0] != 0; i++){
		/*
		for (j = 0; keys[i].displayname[j] != 0; j++){
			name[j] = keys[i].displayname[j];
		}
		name[j] = 0;
		 */
		SendMessageW (hCtl, CB_ADDSTRING, 0, (LPARAM)(keys[i].displayname));
	}
	
	GetWlanAvailableSSID(keys, &index, &reason);
	if (index >= 0){
		SendMessage (hCtl, CB_SETCURSEL, index, 0);
	}
}

static void WPASelect_DialogCommand(HWND hWnd, WORD control_id, WORD command){
	WLAN_REASON_CODE reason;
	int status;
	fflush(stderr);
	if (command == BN_CLICKED){
		if (control_id == IDC_ADDNET){
			HWND hCtl = GetDlgItem (hWnd, IDC_WPASELECT);
			WLANKEY *key = malloc(sizeof(WLANKEY) * 2);
			int index = SendMessage (hCtl, CB_GETCURSEL, 0, 0);
			if (index >= 0){
				memset (key, 0, sizeof(WLANKEY) * 2);
				memcpy (key, keys + index, sizeof(WLANKEY));
				status = SetWlanProfiles(key, TRUE, &reason);
				ReorderWlanProfiles(keys);
				if (status != ERROR_SUCCESS){
					wchar_t msgbuf[64];
					snwprintf (msgbuf, sizeof(msgbuf), L"Error %08X; Reason %08X", status, reason);
					MessageBoxW(hWnd, msgbuf, L"Error adding WLAN profile", MB_OK | MB_ICONERROR);
				} else {
					wchar_t msgbuf[512];
					snwprintf (msgbuf, sizeof(msgbuf), L"Successfully added profile %ls [%ls]", key->displayname, key->ssid);
					MessageBoxW(hWnd, msgbuf, L"Successfully added WLAN profile", MB_OK | MB_ICONINFORMATION);
				}
			}
		} else if (control_id == IDC_DELNET){
			int clicked = MessageBoxW(hWnd, L"Are you sure you want to remove all school wireless profiles?\nDoing so will disconnect you from the school wireless network.", L"Are you sure", MB_YESNO | MB_ICONWARNING);
			if (clicked == IDYES){
				RemoveWlanProfiles(keys, NULL);
			}
		} else if (control_id == IDC_DELALL){
			int clicked = MessageBoxW(hWnd, L"Are you sure you want to remove ALL wireless profiles?\nDoing so will disconnect you from the school wireless network.", L"Are you sure", MB_YESNO | MB_ICONWARNING);
			if (clicked == IDYES){
				RemoveWlanProfiles(NULL, NULL);
			}
		}
	}
}

static int CALLBACK WPASelect_DialogProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
		case WM_INITDIALOG:
			WPASelect_DialogInit(hWnd);
			return TRUE;
		case WM_COMMAND:
			WPASelect_DialogCommand(hWnd, LOWORD(wParam), HIWORD(wParam));
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
		case WM_ENDSESSION:
			return TRUE;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return TRUE;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	int status;
	MSG msg;
	HWND hDlg;

	status = DecodeWLANKeys (&keys, &numkeys);
	hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_WPASELECT), NULL, WPASelect_DialogProc);

	while (GetMessage(&msg, NULL, 0, 0)){
		if (!IsDialogMessage (hDlg, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;    
}

