#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "wlankeys.h"
#include "tea.h"
#include "setwpakey.h"
#include "dialog.h"

static WLANKEY *keys;
static int numkeys;

static void WPASelect_DialogInit(HWND hWnd){
    int index;
    int i, j;
    char name[257];
    WLAN_REASON_CODE reason;
    HWND hCtl = GetDlgItem (hWnd, IDC_WPASELECT);
    
    for (i = 0; keys[i].ssid[0] != 0; i++){
	for (j = 0; keys[i].displayname[j] != 0; j++){
	    name[j] = keys[i].displayname[j];
	}
	name[j] = 0;
	SendMessageA (hCtl, CB_ADDSTRING, 0, (LPARAM)name);
    }
    
    GetWlanAvailableSSID(keys, &index, &reason);
    if (index >= 0){
	SendMessage (hCtl, CB_SETCURSEL, index, 0);
    }
}

static void WPASelect_DialogCommand(HWND hWnd, WORD control_id, WORD command){
    WLAN_REASON_CODE reason;
    int status;
    if (command == BN_CLICKED){
	if (control_id == IDC_ADDNET){
	    HWND hCtl = GetDlgItem (hWnd, IDC_WPASELECT);
	    WLANKEY *key = malloc(sizeof(WLANKEY) * 2);
	    int index = SendMessage (hCtl, CB_GETCURSEL, 0, 0);
	    if (index >= 0){
		memset (key, 0, sizeof(WLANKEY) * 2);
		memcpy (key, keys + index, sizeof(WLANKEY));
		status = SetWlanProfiles(key, &reason);
		if (status != ERROR_SUCCESS){
		    char msgbuf[64];
		    snprintf (msgbuf, sizeof(msgbuf), "Error %08X; Reason %08X", status, reason);
		    MessageBoxA(hWnd, msgbuf, "Error adding WLAN profile", MB_OK | MB_ICONERROR);
		} else {
		    char msgbuf[512];
		    snprintf (msgbuf, sizeof(msgbuf), "Successfully added profile %ls [%ls]", key->displayname, key->ssid);
		    MessageBoxA(hWnd, msgbuf, "Successfully added WLAN profile", MB_OK | MB_ICONINFORMATION);
		}
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

    fprintf (stderr, "Entering WinMain\n");
    status = DecodeWLANKeys (&keys, &numkeys);
    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_WPASELECT), NULL, WPASelect_DialogProc);

    while (GetMessage(&msg, NULL, 0, 0)){
	if (!IsDialogMessage (hDlg, &msg)){
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }
    fprintf (stderr, "Exiting WinMain\n");
    return msg.wParam;    
}

