// Fake DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#ifndef __DSX_SPRITE_H__
#define __DSX_SPRITE_H__

#include "dsx_backbuff.h"

typedef struct tagDSX_SPRITE {
   const u8   *data;
   s32   width, height;
   const u16  *palette;
} DSX_SPRITE;

void DSX_DrawSprite(const DSX_SPRITE *sprite, int dest_x, int dest_y, const u16 *pal, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff);   
void DSX_DrawSpriteStrip(const DSX_SPRITE *sprite, int x, int y, int column, const u16* pal, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff);
#endif // __DSX_SPRITE_H__
