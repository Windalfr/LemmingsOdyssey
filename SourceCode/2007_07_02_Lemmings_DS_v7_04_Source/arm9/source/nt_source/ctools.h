#ifndef _C_TOOLS_H_
#define _C_TOOLS_H_

#include <nds.h>

#ifndef NULL
  #define NULL	0
#endif

void *my_memset(void *s, int c, u32 n);
char *my_strncpy(char *dest, const char *src, u32 n);

#endif
