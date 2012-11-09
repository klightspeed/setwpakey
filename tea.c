#include "tea.h"

static void TEA_Round (unsigned int *v, unsigned int* k) {
	unsigned int sum = 0;
	int i;
	for (i=0; i < 32; i++){
		sum += 0x9e3779b9;
		v[0] += ((v[1] << 4) + k[0]) ^
		        (v[1] + sum) ^
		        ((v[1] >> 5) + k[1]);
		v[1] += ((v[0] << 4) + k[2]) ^
		        (v[0] + sum) ^
		        ((v[1] >> 5) + k[1]);
	}
}

unsigned long long TEA_Rand (unsigned int *key, void *seed){
	static unsigned int val[2] = {0, 0};

	if (key[0] == 0){
		__builtin_memcpy (val, seed, 8);
		key[0] = val[0];
		key[1] = val[1];
		key[2] = val[0];
		key[3] = val[1];
		do {
			TEA_Round(val, key);
			key[0] = val[0];
		} while (key[0] == 0);
		TEA_Round(val, key);
		key[1] = val[0];
		TEA_Round(val, key);
		key[2] = val[0];
		TEA_Round(val, key);
		key[3] = val[0];
		__builtin_memset (val, 0, 8);
	}
	TEA_Round(val, key);
	return (((unsigned long long)val[1]) << 32) | val[0];
}

