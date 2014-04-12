#include "ctools.h"

/* ==== HELPERS ==== */

void *my_memset(void *s, int c, u32 n)
{
	u8 *t = (u8*)s;
	u32 i;
	for(i=0; i<n; ++i) {
		t[i] = c;
	}
	return s;
}

char *my_strncpy(char *dest, const char *src, u32 n)
{
	u32 i=0;
	while((src[i] != 0) && (i < n)) {
		dest[i] = src[i];
		i++;
	}
	if((i<n)&&(src[i]==0)) {
		dest[i] = 0;
	}
	return dest;
}
