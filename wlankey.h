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
	int nonbcast:1;
	int useonex:1;
	int preferred:1;
	int autoconnect:1;
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

DWORD RemoveIfaceProfiles (HANDLE h, 
                           const GUID *iface, 
                           const WLANKEY *profiles,
                           const WLANKEY *keep);
DWORD RemoveWlanProfiles (const WLANKEY *profiles,
                          const WLANKEY *keep);

DWORD SetIfaceProfile (HANDLE h,
                       const GUID *iface,
                       WLANKEY *key,
                       BOOL replace,
		       WLAN_REASON_CODE *reason);
DWORD SetIfaceProfiles (HANDLE h,
                        const GUID *iface,
                        const wchar_t *ifname,
                        WLANKEY *keys,
                        BOOL replace,
                        WLAN_REASON_CODE *reason);
DWORD SetWlanProfiles (WLANKEY *keys,
                       BOOL replace,
                       WLAN_REASON_CODE *reason);

DWORD ReorderIfaceProfiles (HANDLE h,
		            const GUID *iface,
			    const wchar_t *ifname,
			    WLANKEY *keys);
DWORD ReorderWlanProfiles (WLANKEY *keys);

DWORD GetIfaceAvailableSSID (HANDLE h,
                             const GUID *iface,
                             const wchar_t *ifname,
                             WLANKEY *keys,
                             int *index,
                             WLAN_REASON_CODE *reason);
DWORD GetWlanAvailableSSID (WLANKEY *keys,
                            int *index,
                            WLAN_REASON_CODE *reason);
#endif /* WLANKEY_H */
