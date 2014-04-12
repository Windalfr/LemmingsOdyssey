// More memory stuff.

#ifndef __DS_MEMORY_H__
#define __DS_MEMORY_H__
 
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define BG_GFX_MAIN_S_TILE ((S_TILE*)(BG_GFX))
#define BG_GFX_MAIN_D_TILE ((D_TILE*)(BG_GFX))

#define BG_GFX_SUB_S_TILE ((S_TILE*)(BG_GFX_SUB))
#define BG_GFX_SUB_D_TILE ((D_TILE*)(BG_GFX_SUB))

#define S_TILE_SIZE_u16 ((sizeof(S_TILE))>>1)
#define D_TILE_SIZE_u16 ((sizeof(D_TILE))>>1)

// Window 0
#define WIN0X        (*(vuint16*)0x04000040)
#define WIN0Y        (*(vuint16*)0x04000044)

// Window 1
#define WIN1X        (*(vuint16*)0x04000042)
#define WIN1Y        (*(vuint16*)0x04000046)
                             
#ifdef __cplusplus
}
#endif

#endif // __DS_MEMORY_H__

