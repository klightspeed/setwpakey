#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "tea.h"

static WLANKEY *keys;
static int numkeys;

int main (void) {
    int status;
    int index = -1;
    WLAN_REASON_CODE reason;
    static WLANKEY key[2];

    status = DecodeWLANKeys(&keys, &numkeys);
    if (status != ERROR_SUCCESS){
        fwprintf (L"Error decoding WLAN keys\n");
        fwprintf (stderr, L"Status: 0x%08X\n", status);
        return 1;
    }

    status = GetWlanAvailableSSID(keys, &index, &reason);
    if (status != ERROR_SUCCESS){
        fwprintf (L"Error getting local SSID\n");
        fwprintf (stderr, L"Status: 0x%08X\n", status);
        if (reason != 0){
            WlanReasonCodeToString (reason, 65536, reasonstr, NULL);
        }
        fwprintf (stderr, L"Reason: %s\n", reasonstr);
        return 1;
    }

    if (index >= 0){
        memset (key, 0, sizeof(WLANKEY) * 2);
        memcpy (key, keys + index, sizeof(WLANKEY));
        status = SetWlanProfiles(key, &reason);
        if (status != ERROR_SUCCESS){
            fwprintf (L"Error adding wireless profile\n");
            fwprintf (stderr, L"Status: 0x%08X\n", status);
            if (reason != 0){
                WlanReasonCodeToString (reason, 65536, reasonstr, NULL);
            }
            fwprintf (stderr, L"Reason: %s\n", reasonstr);
            return 1;
        } else {
            return 0;
        }
    } else {
        fprintf ("No matching SSID available\n");
        return 1;
    }
}
