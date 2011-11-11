#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include "wlanapi.h"
#include "wlankey.h"
#include "wlankeys.h"
#include "tea.h"

/***************
 * XML Profile Strings
 ***************/
static wchar_t *profilexmltpl =
	L"<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\n"
	L"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n"
	L" <name>%ls</name>\n"
	L" <SSIDConfig>\n"
	L"  <SSID>\n"
	L"   <name>%ls</name>\n"
	L"  </SSID>\n"
	L" </SSIDConfig>\n"
	L" <connectionType>ESS</connectionType>\n"
	L" <connectionMode>auto</connectionMode>\n"
	L" <autoSwitch>true</autoSwitch>\n"
	L" <MSM>\n"
	L"  <security>\n"
	L"   <authEncryption>\n"
	L"    <authentication>%ls</authentication>\n"
	L"    <encryption>%ls</encryption>\n"
	L"    <useOneX>%ls</useOneX>\n"
	L"   </authEncryption>\n"
	L"%ls"
	L"  </security>\n"
	L" </MSM>\n"
	L"</WLANProfile>\n";

static wchar_t *securityxmlpskpass =
	L"   <sharedKey>\n"
	L"    <keyType>passPhrase</keyType>\n"
	L"    <protected>false</protected>\n"
	L"    <keyMaterial xml:space=\"preserve\">%ls</keyMaterial>\n"
	L"   </sharedKey>\n";

static wchar_t *securityxmlpskkey =
	L"   <sharedKey>\n"
	L"    <keyType>networkKey</keyType>\n"
	L"    <protected>false</protected>\n"
	L"    <keyMaterial xml:space=\"preserve\">%ls</keyMaterial>\n"
	L"   </sharedKey>\n";

static wchar_t *securityxmlonex =
	L"   <OneX xmlns=\"http://www.microsoft.com/networking/OneX/v1\">\n"
	L"%ls"
	L"    <EAPConfig>\n"
	L"     <EapHostConfig xmlns=\"http://www.microsoft.com/provisioning/EapHostConfig\">\n"
	L"      <EapMethod>\n"
	L"       <Type xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">%d</Type>\n"
	L"       <AuthorId xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</AuthorId>\n"
	L"      </EapMethod>\n"
	L"      <Config xmlns=\"http://www.microsoft.com/provisioning/EapHostConfig\">\n"
	L"       <Eap xmlns=\"http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1\">\n"
	L"        <Type>%d</Type>\n"
	L"%ls"
	L"       </Eap>\n"
	L"      </Config>\n"
	L"     </EapHostConfig>\n"
	L"    </EAPConfig>\n"
	L"   </OneX>\n";

static wchar_t *securityxmlsso =
	L"    <singleSignOn>\n"
	L"     <type>preLogon</type>\n"
	L"    </singleSignOn>\n";

static wchar_t *securityxmleaptls =
	L"        <EapType xmlns=\"http://www.microsoft.com/provisioning/EapTlsConnectionPropertiesV1\">\n"
	L"         <CredentialsSource>\n"
	L"          <CertificateStore />\n"
	L"         </CredentialsSource>\n"
	L"         <ServerValidation>\n"
	L"          <DisableUserPromptForServerValidation>false</DisableUserPromptForServerValidation>\n"
	L"%ls"
	L"         </ServerValidation>\n"
	L"         <DifferentUsername>%ls</DifferentUsername>\n"
	L"        </EapType>\n";

static wchar_t *securityxmlpeapmschap =
	L"        <EapType xmlns=\"http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV1\">\n"
	L"         <ServerValidation>\n"
	L"          <DisableUserPromptForServerValidation>false</DisableUserPromptForServerValidation>\n"
	L"          <ServerNames />\n"
	L"%ls"
	L"         </ServerValidation>\n"
	L"         <FastReconnect>true</FastReconnect>\n"
	L"         <InnerEapOptional>0</InnerEapOptional>\n"
	L"         <Eap xmlns=\"http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1\">\n"
	L"          <Type>26</Type>\n"
	L"          <EapType xmlns=\"http://www.microsoft.com/provisioning/MsChapV2ConnectionPropertiesV1\">\n"
	L"           <UseWinLogonCredentials>%ls</UseWinLogonCredentials>\n"
	L"          </EapType>\n"
	L"         </Eap>\n"
	L"         <EnableQuarantineChecks>false</EnableQuarantineChecks>\n"
	L"         <RequireCryptoBinding>false</RequireCryptoBinding>\n"
	L"         <PeapExtensions />\n"
	L"        </EapType>\n";

/*****************
 * Internal functions
 *****************/

/*
 * Converts special characters into XML entities
 */
void XMLEntities (wchar_t *out, const wchar_t *in, const unsigned int outlen){
	unsigned int i, j;
	unsigned int tmp;
	static wchar_t otmp[64];
	out[0] = 0;
	j = 0;
	for (i=0; i<wcslen(in) && j<outlen-10; i++){
		tmp = (unsigned int)in[i];
		memset (otmp, 0, 64 * sizeof(wchar_t));
		if (tmp < 32 || tmp >= 127){
			_snwprintf (otmp, 63, L"&#%u;", tmp);
		} else if (tmp == '"'){
			_snwprintf (otmp, 63, L"&quot;");
		} else if (tmp == '>'){
			_snwprintf (otmp, 63, L"&gt;");
		} else if (tmp == '<'){
			_snwprintf (otmp, 63, L"&lt;");
		} else if (tmp == '&'){
			_snwprintf (otmp, 63, L"&amp;");
		} else {
			_snwprintf (otmp, 63, L"%c", tmp);
		}
		wcsncat (out, otmp, outlen - j);
		j += wcslen (otmp);
	}
	out[j] = 0;
}

/*
 * Decrypts the WLAN key structure
 */
int DecodeWLANKeys (WLANKEY **keys, int *numkeys){
	unsigned char tmp[3];
	unsigned long long *out;
	unsigned long long *in;
	int i;
	
	*keys = malloc (WLANKEYDATALEN);

	if (*keys == NULL){
		return errno;
	}

	tmp[2] = 0;

	out = (unsigned long long *)(*keys);
	in = (unsigned long long *)encwlankeys;

	for (i=0; i < WLANKEYDATALEN / 8; i++){
		out[i] = in[i] ^ TEA_Rand(enckey, NULL);
	}

	if (numkeys != NULL){
		*numkeys = WLANKEYDATALEN / sizeof(WLANKEY) - 1;
	}

	return ERROR_SUCCESS;
}

/*****************
 * Profile removal functions
 *****************/

/*
 * Remove all profiles on an interface
 */
DWORD RemoveIfaceProfiles (HANDLE h, 
			   const GUID *iface){
	WLAN_PROFILE_INFO_LIST *profile_list = NULL;
	DWORD status;
	int i;

	status = WlanGetProfileList(h, iface, NULL, &profile_list);
	if (status == ERROR_SUCCESS && profile_list != NULL){
		for (i = 0; status == ERROR_SUCCESS && i < profile_list->dwNumberOfItems; i++){
			WLAN_PROFILE_INFO *profile = (WLAN_PROFILE_INFO *)(profile_list->ProfileInfo + i);
			if (profile != NULL){
				status = WlanDeleteProfile(h, iface, profile->strProfileName, NULL);
			}
		}
		WlanFreeMemory(profile_list);
	}

	return status;
}

/*
 * Removes existing profiles on all interfaces
 */
DWORD RemoveWlanProfiles (){
	DWORD apiver;
	DWORD status;
	HANDLE h;
	WLAN_INTERFACE_INFO_LIST *iflist;
	GUID *iface;
	int i;

	status = WlanOpenHandle (1, NULL, &apiver, &h);
	if (status != ERROR_SUCCESS){
		return status;
	}
	status = WlanEnumInterfaces (h, NULL, &iflist);
	for (i=0; status == ERROR_SUCCESS && i < iflist->dwNumberOfItems; i++){
		iface = &(iflist->InterfaceInfo[i].InterfaceGuid);
		status = RemoveIfaceProfiles (h, iface);
	}

	WlanFreeMemory (iflist);
	WlanCloseHandle (h, NULL);
	return status;
}

/*****************
 * Profile addition functions
 *****************/

/*
 * Set the key for an SSID on an interface
 */
DWORD SetIfaceProfile (HANDLE h,
                       const GUID *iface,
                       const wchar_t *ifname,
                       WLANKEY *key,
                       WLAN_REASON_CODE *reason){
	static wchar_t profilexml[65536];
	static wchar_t securityxml[65536];
	static wchar_t onexconfigxml[65536];
	static wchar_t rootcaxml[65536];
	static wchar_t tmp[65];
	static wchar_t ssid[512];
	static wchar_t auth[512];
	static wchar_t enc[512];
	static wchar_t psk[512];
	static unsigned char zeroca[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int i, j;
	fwprintf (stderr, L"    Creating profile for SSID %s\n", key->ssid);
	XMLEntities (ssid, key->ssid, 512);
	XMLEntities (auth, key->auth, 512);
	XMLEntities (enc, key->enc, 512);
	securityxml[0] = securityxml[65535] = 0;
	profilexml[0] = profilexml[65535] = 0;
	onexconfigxml[0] = onexconfigxml[65535] = 0;
	rootcaxml[0] = rootcaxml[65535] = 0;
	if (wcscmp (auth, L"WPAPSK") == 0 || wcscmp(auth, L"WPA2PSK") == 0){
		wcsncpy (tmp, key->wpa.psk, 64);
		tmp[64] = 0;
		XMLEntities (psk, tmp, 512);
		if (wcslen(psk) == 64){
			_snwprintf (securityxml, 65535, securityxmlpskkey, psk);
		} else {
			_snwprintf (securityxml, 65535, securityxmlpskpass, psk);
		}
	} else if (key->useonex){
		memset (rootcaxml, 0, 65536);
		for (i = 0; i < 4; i++){
			if (memcmp(key->wpa.onex.rootca[i].hash, zeroca, 20) != 0){
				wcscat (rootcaxml, L"          ");
				wcscat (rootcaxml, L"<TrustedRootCA>");
				for (j = 0; j < 20; j++){
					_snwprintf (tmp, 64, L"%02x ", key->wpa.onex.rootca[i].hash[j]);
					wcscat (rootcaxml, tmp);
				}
				wcscat (rootcaxml, L"</TrustedRootCA>\n");
			}
		}
		
		if (key->wpa.onex.eaptype == 13){ /* EAP-TLS */
			_snwprintf (onexconfigxml, 
			            65536, 
			            securityxmleaptls,
			            rootcaxml,
			            key->wpa.onex.eap.tls.diffuser ? L"true" : L"false");
		} else if (key->wpa.onex.eaptype == 25){ /* PEAP-MSCHAPv2 */
			_snwprintf (onexconfigxml,
			            65536,
			            securityxmlpeapmschap,
			            rootcaxml,
			            key->wpa.onex.eap.mschap.usecred ? L"true" : L"false");
		}
		_snwprintf (securityxml, 
		            65536, 
		            securityxmlonex,
		            key->wpa.onex.sso ? securityxmlsso : L"",
		            key->wpa.onex.eaptype,
		            key->wpa.onex.eaptype,
		            onexconfigxml);
	}
	
	_snwprintf (profilexml, 
	            65535, 
	            profilexmltpl,
	            ssid, 
	            ssid, 
	            auth, 
	            enc, 
	            key->useonex ? L"true" : L"false",
	            securityxml);
	return WlanSetProfile (h, iface, 0, profilexml, NULL, TRUE, NULL, reason);
}

/*
 * Set the keys for SSIDS on an interface
 */
DWORD SetIfaceProfiles (HANDLE h,
                        const GUID *iface,
                        const wchar_t *ifname,
                        WLANKEY *keys,
                        WLAN_REASON_CODE *reason){
	DWORD status;
	int i;
	fwprintf (stderr, L"Creating profiles on interface \"%s\"\n", ifname);
	status = errno;
	while (keys->ssid[0] != 0){
		status = SetIfaceProfile (h, iface, ifname, keys, reason);
		if (status != ERROR_SUCCESS){
			return status;
		}
		keys++;
	}
	fwprintf (stderr, L"Profiles created on interface \"%s\"\n", ifname);
	return status;
}

/*
 * Set the keys for SSIDS on all interfaces
 */
DWORD SetWlanProfiles (WLANKEY *keys,
                       WLAN_REASON_CODE *reason){
	DWORD apiver;
	DWORD status;
	HANDLE h;
	WLAN_INTERFACE_INFO_LIST *iflist;
	GUID *iface;
	wchar_t *ifname;
	unsigned int i;
	status = WlanOpenHandle (1, NULL, &apiver, &h);
	if (status != ERROR_SUCCESS){
		return status;
	}
	status = WlanEnumInterfaces (h, NULL, &iflist);
	for (i=0; status == ERROR_SUCCESS && i < iflist->dwNumberOfItems; i++){
		iface = &(iflist->InterfaceInfo[i].InterfaceGuid);
		ifname = iflist->InterfaceInfo[i].strInterfaceDescription;
		status = SetIfaceProfiles (h, iface, ifname, keys, reason);
	}
	WlanFreeMemory (iflist);
	WlanCloseHandle (h, NULL);
	return status;
}

/*****************
 * Wlan detection functions
 *****************/

/*
 * Get the index of the first available network in the list on an interface
 */
DWORD GetIfaceAvailableSSID (HANDLE h,
                             const GUID *iface,
                             const wchar_t *ifname,
                             WLANKEY *keys,
                             int *index,
                             WLAN_REASON_CODE *reason){
	WLAN_AVAILABLE_NETWORK_LIST *netlist = NULL;
	DWORD status;
	int i;
	int j;
	int k;
	wchar_t det_ssid[65];

	fwprintf (stderr, L"Getting available networks on interface \"%s\"\n", ifname);
	status = WlanGetAvailableNetworkList(h, iface, 0, NULL, &netlist);
	if (netlist != NULL){
		fwprintf (stderr, L"%d networks in NetList\n", netlist->dwNumberOfItems);
		for (j = 0; j < netlist->dwNumberOfItems; j++){
			for (k = 0; k < netlist->Network[j].dot11Ssid.uSSIDLength; k++){
				det_ssid[k] = netlist->Network[j].dot11Ssid.ucSSID[k];
			}
			det_ssid[k] = 0;
			fwprintf (stderr, L"Network %d: SSID=%ls\n", j, det_ssid);
		}
		for (i = 0; keys[i].ssid[0] != 0; i++){
			for (j = 0; j < netlist->dwNumberOfItems; j++){
				for (k = 0; k < netlist->Network[j].dot11Ssid.uSSIDLength; k++){
					det_ssid[k] = netlist->Network[j].dot11Ssid.ucSSID[k];
				}
				det_ssid[k] = 0;
				if (wcscmp(keys[i].ssid, det_ssid) == 0){
					fwprintf (stderr, L"Network %d is in the profile list\n", j);
					*index = i;
					WlanFreeMemory(netlist);
					return status;
				}
			}
		}
		fwprintf (stderr, L"No detected networks are in the profile list\n");
		WlanFreeMemory(netlist);
	} else {
		fwprintf (stderr, L"NetList is NULL\n");
	}
	return status;
}

/*
 * Get the index of the first available network in the list
 */
DWORD GetWlanAvailableSSID (WLANKEY *keys,
                            int *index,
                            WLAN_REASON_CODE *reason){
	DWORD apiver;
	DWORD status;
	HANDLE h;
	WLAN_INTERFACE_INFO_LIST *iflist;
	GUID *iface;
	wchar_t *ifname;
	unsigned int i;
	status = WlanOpenHandle (1, NULL, &apiver, &h);
	if (status != ERROR_SUCCESS){
		return status;
	}
	status = WlanEnumInterfaces (h, NULL, &iflist);
	*index = -1;
	for (i=0; *index == -1 && i < iflist->dwNumberOfItems; i++){
		iface = &(iflist->InterfaceInfo[i].InterfaceGuid);
		ifname = iflist->InterfaceInfo[i].strInterfaceDescription;
		status = GetIfaceAvailableSSID (h, iface, ifname, keys, index, reason);
	}
	WlanFreeMemory (iflist);
	WlanCloseHandle (h, NULL);
	return status;
}

