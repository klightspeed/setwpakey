#ifndef SETWPAKEY_H

void XMLEntities (wchar_t *out, const wchar_t *in, const unsigned int outlen);
int DecodeWLANKeys (WLANKEY **keys, int *numkeys);
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

#endif
