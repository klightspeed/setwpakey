#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "tea.h"

void Usage(char *progname, WLANKEY *profiles){
	fprintf (stderr, 
	         "Usage:\n"
	         "    %s [--clean|--cleanall] [--noreplace] --all\n"
	         "    %s [--clean|--cleanall] [--noreplace] [--] {SSID} [{SSID} ...]\n"
		 "    %s [--remove {SSID} [{SSID}]] [--add {SSID} [{SSID}]]\n"
	         "\n"
	         "    -a,    --all        Install all following listed profiles\n"
	         "    -c,    --clean      Remove following listed profiles\n"
	         "    -c -c, --cleanall   Remove all profiles\n"
	         "    -n,    --noreplace  Do not replace existing profiles\n"
		 "    -r,    --remove     Remove profiles identified by SSIDs\n"
		 "    -s,    --add        Add profiles identified by SSIDs\n"
	         "           --           All following arguments are SSIDs\n"
	         "           {SSID}       SSID of profile to install\n",
	         progname,
	         progname
	);

	if (profiles){
		fprintf (stderr, 
		         "\n"
		         "Available profiles:\n"
		         "SSID            Profile Name\n");

		while (profiles->ssid[0] != 0){
			fprintf (stderr, " %-14ls  %ls\n", profiles->ssid, profiles->displayname);
			profiles++;
		}

		fprintf (stderr, "\n");
	}
}

WLANKEY *LookupKeys(char **ssids, int numkeys, WLANKEY *allkeys, int *status){
	int i, j, k;
	WLANKEY *keys = malloc(numkeys * sizeof(WLANKEY));
	if (keys == NULL){
		*status = errno;
	} else {
		k = 0;
		for (i = 0; i < numkeys; i++){
			char ssid[33];
			for (j = 0; j < 33; j++){
				ssid[j] = (char)(allkeys[i].ssid[j]);
			}

			for (j = 0; ssids[j] != NULL; j++){
				if (!stricmp(ssids[j], ssid)){
					memcpy (keys + k, allkeys + i, sizeof(WLANKEY));
					k++;
					ssid[0] = 0;
				}
			}
		}
	}

	return keys;
}

int main (int argc, char **argv){
	WLAN_REASON_CODE reason;
	static wchar_t reasonstr[65536];
	static wchar_t inipath[65536];
	static wchar_t wargv1[65536];
	int status = ERROR_SUCCESS;
	int i, j, k;
	int nraddkeys = 0;
	int nrdelkeys = 0;
	int numkeys = 0;
	BOOL clean_profiles = FALSE;
	BOOL clean_all_profiles = FALSE;
	BOOL replace = TRUE;
	BOOL all_profiles = FALSE;
	BOOL get_help = FALSE;
	BOOL dashdash = FALSE;
	BOOL remove = FALSE;
	WLANKEY *allkeys;
	WLANKEY *delkeys;
	WLANKEY *addkeys;
	char *addssids[argc];
	char *delssids[argc];

	memset (addssids, 0, sizeof(addssids));
	memset (delssids, 0, sizeof(delssids));
	
	for (i = 1; i < argc; i++){
		if (!dashdash && argv[i][0] == '-'){
			if (argv[i][1] == '-'){
				if (argv[i][2] == 0){
					dashdash = TRUE;
				} else if (!strcmp(argv[i], "--all")){
					all_profiles = TRUE;
				} else if (!strcmp(argv[i], "--clean")){
					clean_profiles = TRUE;
					clean_all_profiles = FALSE;
				} else if (!strcmp(argv[i], "--cleanall")){
					clean_profiles = TRUE;
					clean_all_profiles = TRUE;
				} else if (!strcmp(argv[i], "--noreplace")){
					replace = FALSE;
				} else if (!strcmp(argv[i], "--remove")){
					remove = TRUE;
				} else if (!strcmp(argv[i], "--add")){
					remove = FALSE;
				} else {
					get_help = TRUE;
					break;
				}
			} else if (!strcmp(argv[i], "-a")){
				all_profiles = TRUE;
			} else if (!strcmp(argv[i], "-c")){
				if (clean_profiles){
					clean_all_profiles = TRUE;
				}
				clean_profiles = TRUE;
			} else if (!strcmp(argv[i], "-r")){
				remove = TRUE;
			} else if (!strcmp(argv[i], "-s")){
				remove = FALSE;
			} else if (!strcmp(argv[i], "-n")){
				replace = FALSE;
			} else {
				get_help = TRUE;
			}
		} else {
			if (remove) {
				delssids[nrdelkeys++] = argv[i];
			} else {
				addssids[nraddkeys++] = argv[i];
			}
		}
	}

	if ((all_profiles && nraddkeys != 0) ||
	    (!all_profiles && nraddkeys == 0 && nrdelkeys == 0)){
		get_help = TRUE;
	}


	reason = 0;
	memset (reasonstr, 0, 65536 * sizeof(wchar_t));
	status = DecodeWLANKeys (&allkeys, &numkeys);
	
	if (status == ERROR_SUCCESS){
		if (get_help){
			Usage(argv[0], allkeys);
			return 1;
		}

		if (clean_profiles){
			if (clean_all_profiles){
				delkeys = NULL;
				nrdelkeys = -1;
			} else {
				delkeys = allkeys;
				nrdelkeys = numkeys;
			}
		} else {
			delkeys = LookupKeys(delssids, numkeys, allkeys, &status);
		}

		if (status == ERROR_SUCCESS){
			if (all_profiles){
				addkeys = allkeys;
			} else {
				addkeys = LookupKeys(addssids, numkeys, allkeys, &status);
			}
		}
	}

#ifndef TEST
	if (status == ERROR_SUCCESS){
		if (nrdelkeys != 0){
			RemoveWlanProfiles(delkeys, addkeys);
		}
		status = SetWlanProfiles (addkeys, replace, &reason);
		ReorderWlanProfiles(allkeys);
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
	return 0;
}

