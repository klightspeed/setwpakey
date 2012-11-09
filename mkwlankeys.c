#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ddk/ntifs.h>
#include "wlankey.h"
#include "tea.h"

void gethashbytes (unsigned char *out, wchar_t *in){
	int i;
	int c;
	int p = 0;

	for (i = 0; i < 20; i++){
		c = 0;
		swscanf(in + p, L"%02X %n", &c, &p);
		out[i] = c;
	}
}

int main (void){
	unsigned char *out;
	unsigned char *compr;
	unsigned char *wkspace;
	int nrkeys;
	int datalen;
	int comprlen;
	int wkspacelen;
	int i, j;
	int c;
	int p, np;
	static wchar_t ssids[65536];
	static wchar_t inipath[65536];
	FILETIME seed;
	unsigned int key[4];
	wchar_t *ssid;
	static wchar_t tmp[64];
	WLANKEY *wlankeys;
	WLANKEY *wlankey;
	int key_index;
	int key_number;
	int key_weight;
	int prev_index;
	int key_first;
	int key_last;

	GetFullPathNameW (L"wlankeys.ini", 65536, inipath, NULL);
	
	memset (ssids, 0, 65536 * sizeof(wchar_t));
	GetPrivateProfileSectionNamesW (ssids, 65536, inipath);
	ssid = ssids;
	nrkeys = 0;
	while (ssid[0] != 0){
		nrkeys++;
		ssid += wcslen(ssid) + 1;
	}
	nrkeys++;
	
	datalen = nrkeys * sizeof(WLANKEY);
	wkspacelen = 0;
	RtlGetCompressionWorkSpaceSize(
		COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM,
		(ULONG *)&wkspacelen,
		(ULONG *)&comprlen
	);

	out = malloc (datalen);
	compr = malloc (datalen);
	wkspace = malloc (wkspacelen);

	if (out == NULL || compr == NULL){
		perror ("malloc");
		return 1;
	}

	//pout = out;
	wlankeys = (WLANKEY *)out;
	key_number = 0;
	key_first = 0xFFFFFFFF;
	key_last = 0xFFFFFFFF;

	ssid = ssids;
	while (ssid[0] != 0){
		wchar_t psk[128];
		wlankey = wlankeys + key_number;
		memset (wlankey, 0, sizeof(WLANKEY));
		GetPrivateProfileStringW (ssid, L"SSID", ssid, wlankey->ssid, 64, inipath);
		GetPrivateProfileStringW (ssid, L"DisplayName", ssid, wlankey->displayname, 64, inipath);
		GetPrivateProfileStringW (ssid, L"Preferred", L"true", tmp, 64, inipath);
		wlankey->preferred = wcsicmp(tmp, L"true") == 0;
		GetPrivateProfileStringW (ssid, L"Auth", L"WPA2PSK", wlankey->auth, 16, inipath);
		GetPrivateProfileStringW (ssid, L"Enc", L"AES", wlankey->enc, 16, inipath);
		GetPrivateProfileStringW (ssid, L"Weight", L"0", tmp, 16, inipath);
		wlankey->weight = _wtol(tmp);
		GetPrivateProfileStringW (ssid, L"UseOneX", L"false", tmp, 64, inipath);
		wlankey->useonex = wcsicmp(tmp, L"true") == 0;
		if (wcscmp(wlankey->auth, L"WPAPSK") == 0 ||
		    wcscmp(wlankey->auth, L"WPA2PSK") == 0){
			GetPrivateProfileStringW (ssid, L"PSK", NULL, psk, 128, inipath);
			memcpy (wlankey->wpa.psk, psk, 64 * sizeof(wchar_t));
		} else if (wlankey->useonex){
			GetPrivateProfileStringW (ssid, L"SingleSignOn", L"false", tmp, 64, inipath);
			wlankey->wpa.onex.sso = wcsicmp(tmp, L"true") == 0;
			GetPrivateProfileStringW (ssid, L"TrustedRootCA", L"", tmp, 64, inipath);
			p = 0;
			if (tmp[0] != 0){
				for (i = 0; i < 20; i++){
					c = 0;
					np = 0;
					swscanf(tmp + p, L"%02X %n", &c, &np);
					p += np;
					wlankey->wpa.onex.rootca[0].hash[i] = c;
				}
			}

			GetPrivateProfileStringW (ssid, L"TrustedRootCA1", L"", tmp, 64, inipath);
			p = 0;
			if (tmp[0] != 0){
				for (i = 0; i < 20; i++){
					c = 0;
					np = 0;
					swscanf(tmp + p, L"%02X %n", &c, &np);
					p += np;
					wlankey->wpa.onex.rootca[0].hash[i] = c;
				}
			}

			GetPrivateProfileStringW (ssid, L"TrustedRootCA2", L"", tmp, 64, inipath);
			p = 0;
			if (tmp[0] != 0){
				for (i = 0; i < 20; i++){
					c = 0;
					np = 0;
					swscanf(tmp + p, L"%02X %n", &c, &np);
					p += np;
					wlankey->wpa.onex.rootca[1].hash[i] = c;
				}
			}

			GetPrivateProfileStringW (ssid, L"TrustedRootCA3", L"", tmp, 64, inipath);
			p = 0;
			if (tmp[0] != 0){
				for (i = 0; i < 20; i++){
					c = 0;
					np = 0;
					swscanf(tmp + p, L"%02X %n", &c, &np);
					p += np;
					wlankey->wpa.onex.rootca[2].hash[i] = c;
				}
			}

			GetPrivateProfileStringW (ssid, L"TrustedRootCA4", L"", tmp, 64, inipath);
			p = 0;
			if (tmp[0] != 0){
				for (i = 0; i < 20; i++){
					c = 0;
					np = 0;
					swscanf(tmp + p, L"%02X %n", &c, &np);
					p += np;
					wlankey->wpa.onex.rootca[3].hash[i] = c;
				}
			}

			GetPrivateProfileStringW (ssid, L"OneXType", L"PEAP-MSCHAP", tmp, 64, inipath);
			if (wcsicmp(tmp, L"PEAP-MSCHAP") == 0){
				wlankey->wpa.onex.eaptype = 25;
				GetPrivateProfileStringW (ssid, L"UseLogonCredentials", L"false", tmp, 64, inipath);
				wlankey->wpa.onex.eap.mschap.usecred = wcsicmp(tmp, L"true") == 0;
			} else if (wcsicmp(tmp, L"EAP-TLS") == 0){
				wlankey->wpa.onex.eaptype = 13;
				GetPrivateProfileStringW (ssid, L"DifferentUser", L"false", tmp, 64, inipath);
				wlankey->wpa.onex.eap.tls.diffuser = wcsicmp(tmp, L"true") == 0;
			}
		}

		key_index = key_first;
		prev_index = 0xFFFFFFFF;

		while (1){
			if (key_index < 0 || wlankeys[key_index].weight >= wlankey->weight){
				wlankeys[key_number].next_index = key_index;
				wlankeys[key_number].prev_index = prev_index;
				if (key_index >= 0){
					wlankeys[key_index].prev_index = key_number;
				} else {
					key_last = key_number;
				}
				if (prev_index >= 0){
					wlankeys[prev_index].next_index = key_number;
				} else {
					key_first = key_number;
				}
				break;
			}
			prev_index = key_index;
			key_index = wlankeys[key_index].next_index;
		}

		ssid += wcslen(ssid) + 1;
		key_number++;
	}

	comprlen = 0;
	RtlCompressBuffer(
		COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM, 
		out,
		(ULONG)datalen,
		compr,
		(ULONG)datalen,
		4096,
		(ULONG *)&comprlen,
		wkspace
	);

	memset (key, 0, sizeof(key));
	GetSystemTimeAsFileTime(&seed);

	for (i = 0; i < ((comprlen + 7) / 8); i++){
		((unsigned long long *)compr)[i] ^= TEA_Rand(key, &seed);
	}
	
	printf ("#ifndef WLANKEYS_H\n"
	        "#define WLANKEYS_H\n"
	        "\n"
	        "#define WLANKEYDATALEN %d\n"
	        "#define WLANKEYFIRSTKEY %d\n"
	        "#define WLANKEYDATACOMPRLEN %d\n"
	        "\n"
	        "#define WLANKEYENCKEY 0x%08X, 0x%08X, 0x%08X, 0x%08X\n"
	        "\n"
	        "#define WLANKEYENCDATA ",
	        datalen, key_first, comprlen, key[0], key[1], key[2], key[3]);

	for (i=0; i < ((comprlen + 7) & -8); i++){
		if ((i & 7) == 0){
			printf ("\\\n    ");
		}
		printf ("0x%02X, ", compr[i]);
	}

	printf ("\\\n"
	        "    0x00\n"
	        "\n"
	        "#endif /* WLANKEYS_H */\n");

	return 0;
}

