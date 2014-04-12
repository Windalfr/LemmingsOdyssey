// More memory stuff.

#ifndef __DS_MEMORY_H__
#define __DS_MEMORY_H__

#include "types.h"

#define BG_GFX_MAIN_S_TILE ((S_TILE*)(BG_GFX))
#define BG_GFX_MAIN_D_TILE ((D_TILE*)(BG_GFX))

#define BG_GFX_SUB_S_TILE ((S_TILE*)(BG_GFX_SUB))
#define BG_GFX_SUB_D_TILE ((D_TILE*)(BG_GFX_SUB))

#define S_TILE_SIZE_u16 ((sizeof(S_TILE))>>1)
#define D_TILE_SIZE_u16 ((sizeof(D_TILE))>>1)

#endif // __DS_MEMORY_H__

