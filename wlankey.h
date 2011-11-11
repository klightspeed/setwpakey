#ifndef WLANKEY_H
#define WLANKEY_H

#include "wlanapi.h"

typedef struct {
    int prev_index;
    int next_index;
    int weight;
    wchar_t displayname[257];
    wchar_t ssid[33];
    wchar_t auth[16];
    wchar_t enc[16];
    int nonbcast;
    int useonex;
    union {
	wchar_t psk[65];
	struct {
	    int eaptype;
	    int sso;
	    struct {
		unsigned char hash[20];
	    } rootca[4];
	    union {
		struct {
		    int usecred;
		} mschap;
		struct {
		    int diffuser;
		} tls;
	    } eap;
	} onex;
    } wpa;
} WLANKEY;

void XMLEntities (wchar_t *out, const wchar_t *in, const unsigned int outlen);
int DecodeWLANKeys (WLANKEY **keys, int *numkeys);

DWORD RemoveIfaceProfiles (HANDLE h, const GUID *iface);
DWORD RemoveWlanProfiles ();

DWORD SetIfaceProfile (HANDLE h,
                       const GUID *iface,
		       const wchar_t *ifname,
                       WLANKEY *key,
                       WLAN_REASON_CODE *reason);
DWORD SetIfaceProfiles (HANDLE h,
                        const GUID *iface,
			const wchar_t *ifname,
			WLANKEY *keys,
                        WLAN_REASON_CODE *reason);
DWORD SetWlanProfiles (WLANKEY *keys,
                       WLAN_REASON_CODE *reason);

DWORD GetIfaceAvailableSSID (HANDLE h,
                             const GUID *iface,
                             const wchar_t *ifname,
                             WLANKEY *keys,
                             int *index,
                             WLAN_REASON_CODE *reason);
#endif /* WLANKEY_H */
