#ifndef WLANKEY_H
#define WLANKEY_H

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

#endif /* WLANKEY_H */
