// DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#ifndef DS_BACKBUFF_H
#define DS_BACKBUFF_H
   
#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>         

// These are silly. We should be writing to specific VRAM banks if we're going to make any sense of this mess.
#define VRAM_MAIN ((uint16 *)0x06000000)
#define VRAM_SUB ((uint16 *)0x06200000)

#define DrawPixelA(x, y, c) drawbuffer[(x) + ((y)<<8)] = ((c) | BIT(15))
#define DrawPixel( x, y, c) drawbuffer[(x) + ((y)<<8)] = (c)

#define DrawDBPixelA(x, y, c) backbuff[(x) + ((y)<<8)] = ((c) | BIT(15))
#define DrawDBPixel( x, y, c) backbuff[(x) + ((y)<<8)] = (c)

extern u16 *drawbuffer;

extern u16 backbuff[SCREEN_WIDTH * SCREEN_HEIGHT];
                                
#ifdef __cplusplus
}
#endif

#endif // DS_BACKBUFF_H

