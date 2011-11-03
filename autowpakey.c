#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "wlankeys.h"
#include "tea.h"
#include "setwpakey.h"

int main (int argc, char **argv){
    WLAN_REASON_CODE reason;
    static wchar_t reasonstr[65536];
    static wchar_t inipath[65536];
    static wchar_t wargv1[65536];
    int status = ERROR_SUCCESS;
    int i, j, k;
    int numkeys = 0;
    WLANKEY *allkeys;
    WLANKEY *keys;
    fprintf (stderr, "Entering main\n");

    reason = 0;
    memset (reasonstr, 0, 65536 * sizeof(wchar_t));
    status = DecodeWLANKeys (&allkeys, &numkeys);
    
    if (status == ERROR_SUCCESS){
        if (argc == 1){
            keys = allkeys;
        } else {
            keys = malloc(numkeys * sizeof(WLANKEY));
            if (keys == NULL){
                status = errno;
            } else {
                k = 0;
                for (i = 0; i < numkeys; i++){
                    char ssid[33];
                    for (j = 0; j < 33; j++){
                        ssid[j] = (char)(allkeys[i].ssid[j]);
                    }

                    for (j = 1; j < argc; j++){
                        if (!strcmp(argv[j], ssid)){
                            memcpy (keys + k, allkeys + i, sizeof(WLANKEY));
                            k++;
                        }
                    }
                }
            }
        }
    }
#ifndef TEST
    if (status == ERROR_SUCCESS){
        status = SetWlanProfiles (keys, &reason);
    }
    if (status != ERROR_SUCCESS){
        _wperror (L"Error");
        fwprintf (stderr, L"Status: 0x%08X\n", status);
        if (reason != 0){
            WlanReasonCodeToString (reason, 65536, reasonstr, NULL);
        }
        fwprintf (stderr, L"Reason: %s\n", reasonstr);
        return 1;
    }
#else
    for (i = 0; keys[i].ssid[0] != 0; i++){
        SetIfaceProfile(NULL, NULL, NULL, keys + i, NULL);
    }
#endif
    fprintf (stderr, "Exiting main\n");
    return 0;
}

