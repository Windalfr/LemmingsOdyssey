// Fake DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#ifndef __DSX_BACKBUFF_H__
#define __DSX_BACKBUFF_H__                               

#include "types.h"

#define DSX_BACKBUFFER_X_SIZE (594/2) // That's BITMAP_PANE_LEVEL_X_SIZE/2
#define DSX_BACKBUFFER_Y_SIZE  168
                                                              
void DSX_DrawPixel(int x, int y, int c, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE]);
                        
extern u16 DSX_backbuff[DSX_BACKBUFFER_X_SIZE][DSX_BACKBUFFER_Y_SIZE];

#endif // __DSX_BACKBUFF_H__


