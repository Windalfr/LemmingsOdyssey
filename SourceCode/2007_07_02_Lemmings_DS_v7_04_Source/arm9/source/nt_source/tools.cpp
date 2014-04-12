#include "tools.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <nds.h>

void *mymemalign(size_t blocksize, size_t bytes)
{
	void *buf = memalign(blocksize, bytes);
	if((u32)buf & blocksize != 0) {
		printf("Memalign error! %p is not %u-aligned\n", buf, (u32)blocksize);
		return 0;
	} else {
		return buf;
	}
}
