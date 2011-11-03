#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "wlanapi.h"

int main(int argc, char **argv){

    wchar_t profilename[64];
    HANDLE h = NULL;
    DWORD maxclient = 2;
    DWORD curversion = 0;
    DWORD res = 0;
    DWORD retval = 0;
    unsigned int i;

    PWLAN_INTERFACE_INFO_LIST iflist = NULL;
    PWLAN_INTERFACE_INFO ifinfo = NULL;

    LPWSTR profilexml = NULL;
    DWORD flags = 0;
    DWORD granted = 0;
   
    if (argc < 2) {
        wprintf(L"usage: %hs <profile>\n", argv[0]);
        wprintf(L"   Gets a wireless profile\n");
        wprintf(L"   Example\n");
        wprintf(L"       %hs \"Default Wireless\"\n", argv[0]);
        exit(1);
    }

    mbstowcs(profilename, argv[1], 64);

    wprintf(L"Information for profile: %ls\n\n", profilename);
    
    res = WlanOpenHandle(maxclient, NULL, &curversion, &h);
    if (res != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %08X\n", res);
        return 1;
    }

    res = WlanEnumInterfaces(h, NULL, &iflist);
    if (res != ERROR_SUCCESS) {
        wprintf(L"WlanEnumInterfaces failed with error: %08X\n", res);
        return 1;
    } else {
        wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");

        wprintf(L"Num Entries: %lu\n", iflist->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n\n", iflist->dwIndex);
        for (i = 0; i < (int)(iflist->dwNumberOfItems); i++) {
            ifinfo = (WLAN_INTERFACE_INFO *)(iflist->InterfaceInfo + i);
            wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
            wprintf(L"  Interface Description[%d]: %ls", i, 
                ifinfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (ifinfo->isState) {
		case wlan_interface_state_not_ready:
		    wprintf(L"Not ready\n");
		    break;
		case wlan_interface_state_connected:
		    wprintf(L"Connected\n");
		    break;
		case wlan_interface_state_ad_hoc_network_formed:
		    wprintf(L"First node in a ad hoc network\n");
		    break;
		case wlan_interface_state_disconnecting:
		    wprintf(L"Disconnecting\n");
		    break;
		case wlan_interface_state_disconnected:
		    wprintf(L"Not connected\n");
		    break;
		case wlan_interface_state_associating:
		    wprintf(L"Attempting to associate with a network\n");
		    break;
		case wlan_interface_state_discovering:
		    wprintf(L"Auto configuration is discovering settings for the network\n");
		    break;
		case wlan_interface_state_authenticating:
		    wprintf(L"In process of authenticating\n");
		    break;
		default:
		    wprintf(L"Unknown state %ld\n", ifinfo->isState);
		    break;
            }
            wprintf(L"\n\n");

            res = WlanGetProfile(h,
		    &ifinfo->InterfaceGuid,
                    profilename,
                    NULL, 
                    &profilexml,
                    &flags,
                    &granted);

            if (res != ERROR_SUCCESS) {
                wprintf(L"WlanGetProfile failed with error: %u\n", res);
            } else {
                wprintf(L"  Profile Name:  %ls\n", profilename);

                wprintf(L"  Profile XML string:\n");
                wprintf(L"%ls\n\n", profilexml);
		if (profilexml != NULL){
	            WlanFreeMemory(profilexml);
		    profilexml = NULL;
		}
            }
        }
	if (iflist != NULL) {
	    WlanFreeMemory(iflist);
	    iflist = NULL;
	}
    }

    return retval;
}



