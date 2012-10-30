#ifndef WLANAPI_H
#define WLANAPI_H

#include <windows.h>
typedef DWORD WLAN_REASON_CODE;

#define WLAN_MAX_NAME_LENGTH     256
#define DOT11_SSID_MAX_LENGTH    32
#define WLAN_MAX_PHY_TYPE_NUMBER 8

typedef struct {
    WCHAR strProfileName[256];
    DWORD dwFlags;
} WLAN_PROFILE_INFO, *PWLAN_PROFILE_INFO;

typedef struct {
    DWORD dwNumberOfItems;
    DWORD dwIndex;
    WLAN_PROFILE_INFO ProfileInfo[1];
} WLAN_PROFILE_INFO_LIST, *PWLAN_PROFILE_INFO_LIST;

typedef enum {
    wlan_interface_state_not_ready = 0,
    wlan_interface_state_connected,
    wlan_interface_state_ad_hoc_network_formed,
    wlan_interface_state_disconnecting,
    wlan_interface_state_disconnected,
    wlan_interface_state_associating,
    wlan_interface_state_discovering,
    wlan_interface_state_authenticating
} WLAN_INTERFACE_STATE, *PWLAN_INTERFACE_STATE;

typedef struct {
    GUID InterfaceGuid;
    WCHAR strInterfaceDescription[WLAN_MAX_NAME_LENGTH];
    WLAN_INTERFACE_STATE isState;
} WLAN_INTERFACE_INFO, *PWLAN_INTERFACE_INFO;

typedef struct {
    DWORD dwNumberOfItems;
    DWORD dwIndex;
    WLAN_INTERFACE_INFO InterfaceInfo[1];
} WLAN_INTERFACE_INFO_LIST, *PWLAN_INTERFACE_INFO_LIST;

typedef struct {
    ULONG uSSIDLength;
    UCHAR ucSSID[DOT11_SSID_MAX_LENGTH];
} DOT11_SSID, * PDOT11_SSID;

typedef enum {
    dot11_BSS_type_infrastructure = 1,
    dot11_BSS_type_independent = 2,
    dot11_BSS_type_any = 3
} DOT11_BSS_TYPE, * PDOT11_BSS_TYPE;

typedef enum {
    dot11_phy_type_unknown = 0,
    dot11_phy_type_any = dot11_phy_type_unknown,
    dot11_phy_type_fhss = 1,
    dot11_phy_type_dsss = 2,
    dot11_phy_type_irbaseband = 3,
    dot11_phy_type_ofdm = 4,
    dot11_phy_type_hrdsss = 5,
    dot11_phy_type_erp = 6,
    dot11_phy_type_ht = 7,
} DOT11_PHY_TYPE, * PDOT11_PHY_TYPE;

typedef enum {
    DOT11_AUTH_ALGO_80211_OPEN = 1,
    DOT11_AUTH_ALGO_80211_SHARED_KEY = 2,
    DOT11_AUTH_ALGO_WPA = 3,
    DOT11_AUTH_ALGO_WPA_PSK = 4,
    DOT11_AUTH_ALGO_WPA_NONE = 5,
    DOT11_AUTH_ALGO_RSNA = 6,
    DOT11_AUTH_ALGO_RSNA_PSK = 7,
} DOT11_AUTH_ALGORITHM, * PDOT11_AUTH_ALGORITHM;

typedef enum {
    DOT11_CIPHER_ALGO_NONE = 0x00,
    DOT11_CIPHER_ALGO_WEP40 = 0x01,
    DOT11_CIPHER_ALGO_TKIP = 0x02,
    DOT11_CIPHER_ALGO_CCMP = 0x04,
    DOT11_CIPHER_ALGO_WEP104 = 0x05,
    DOT11_CIPHER_ALGO_WPA_USE_GROUP = 0x100,
    DOT11_CIPHER_ALGO_RSN_USE_GROUP = 0x100,
    DOT11_CIPHER_ALGO_WEP = 0x101,
} DOT11_CIPHER_ALGORITHM, * PDOT11_CIPHER_ALGORITHM;

typedef struct _WLAN_AVAILABLE_NETWORK {
    WCHAR strProfileName[WLAN_MAX_NAME_LENGTH];
    DOT11_SSID dot11Ssid;
    DOT11_BSS_TYPE dot11BssType;
    ULONG uNumberOfBssids;
    BOOL bNetworkConnectable;
    DWORD wlanNotConnectableReason;
    ULONG uNumberOfPhyTypes;
    DOT11_PHY_TYPE dot11PhyTypes[WLAN_MAX_PHY_TYPE_NUMBER];
    BOOL bMorePhyTypes;
    ULONG wlanSignalQuality;
    BOOL bSecurityEnabled;
    DOT11_AUTH_ALGORITHM dot11DefaultAuthAlgorithm;
    DOT11_CIPHER_ALGORITHM dot11DefaultCipherAlgorithm;
    DWORD dwFlags;
    DWORD dwReserved;
} WLAN_AVAILABLE_NETWORK, *PWLAN_AVAILABLE_NETWORK;

typedef struct _WLAN_AVAILABLE_NETWORK_LIST {
    DWORD dwNumberOfItems;
    DWORD dwIndex;
    WLAN_AVAILABLE_NETWORK Network[1];
} WLAN_AVAILABLE_NETWORK_LIST, *PWLAN_AVAILABLE_NETWORK_LIST;


DWORD WINAPI WlanDeleteProfile(HANDLE,const GUID*,LPCWSTR,PVOID);
DWORD WINAPI WlanSetProfile(HANDLE,const GUID*,DWORD,LPCWSTR,LPCWSTR,BOOL,PVOID,PDWORD);
DWORD WINAPI WlanGetProfile(HANDLE,const GUID*,LPCWSTR,PVOID,LPWSTR*,DWORD*,DWORD*);
DWORD WINAPI WlanOpenHandle(DWORD,PVOID,PDWORD,PHANDLE);
DWORD WINAPI WlanCloseHandle(HANDLE,PVOID);
DWORD WINAPI WlanGetProfileList(HANDLE,const GUID*,PVOID,PWLAN_PROFILE_INFO_LIST *);
DWORD WINAPI WlanGetAvailableNetworkList(HANDLE,const GUID *,DWORD,PVOID,PWLAN_AVAILABLE_NETWORK_LIST *);
DWORD WINAPI WlanEnumInterfaces(HANDLE,PVOID,PWLAN_INTERFACE_INFO_LIST *);
void WINAPI WlanFreeMemory(PVOID);
DWORD WINAPI WlanReasonCodeToString(DWORD,DWORD,PWCHAR,PVOID);
DWORD WINAPI WlanSetProfilePosition(HANDLE,const GUID *,LPCWSTR,DWORD,PVOID);

#endif /* WLANAPI_H */
