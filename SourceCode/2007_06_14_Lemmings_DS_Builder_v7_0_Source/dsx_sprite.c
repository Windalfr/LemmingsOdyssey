// Fake DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#include "graphicsglobals.h"
#include "dsx_sprite.h"
#include "dsx_backbuff.h"

void DSX_DrawSprite(const DSX_SPRITE *sprite, int d_x, int d_y, const u16 *pal, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE]) {
   if (pal == 0) pal = sprite->palette;

   if (d_x >= LEVEL_DISPLAY_X_SIZE)       return;
   if (d_y >= LEVEL_DISPLAY_Y_SIZE)      return;
   if (d_x <  (0-sprite->width))  return;
   if (d_y <  (0-sprite->height)) return;

   u16 d;

   // These are the start coordinates (on the sprite for source)
   int start_src_x = (d_x < 0) ? (0-d_x) : 0;
   int start_src_y = (d_y < 0) ? (0-d_y) : 0;

   // These are the dest coordinates (on the screen for draw)
   int start_dst_x = (d_x < 0) ? 0 : d_x;
   int start_dst_y = (d_y < 0) ? 0 : d_y;

   int src_x, src_y, dst_x, dst_y;

   for (src_y = start_src_y, dst_y = start_dst_y; (src_y < sprite->height) && (dst_y < LEVEL_DISPLAY_Y_SIZE); src_y++, dst_y++) {
      for (src_x = start_src_x, dst_x = start_dst_x; (src_x < sprite->width) && (dst_x < LEVEL_DISPLAY_X_SIZE); src_x++, dst_x++) {
         d = sprite->data[src_x + (src_y*(sprite->width))];
         if (d != 0) {
            DSX_DrawPixel(dst_x, dst_y, pal[d], backbuff_to_use);
         }
      }
   }
}

// This draws a sprite of a sprite to the screen.
// The x and y are in camera coordinates.
void DSX_DrawSpriteStrip(const DSX_SPRITE *sprite, int x, int y, int column, const u16* pal, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE]) {
   if (x >= LEVEL_DISPLAY_X_SIZE) return;
   if (x <  0                   ) return;                       

   if ((y                       ) >= LEVEL_DISPLAY_Y_SIZE) return;
   if ((y + (sprite->height - 1)) <  0                   ) return;

   if (pal == 0) pal = sprite->palette;

   int yd;
   int yl = sprite->height;

   int dst_x, dst_y;

   u32 d;
   //u8 r, g, b;

   for (yd = 0; yd < yl; yd++) {
      d = sprite->data[column + (yd*(sprite->width))];

      if (d != 0) {
         dst_y = y + yd;
         dst_x = x;
         DSX_DrawPixel(dst_x, dst_y, pal[d], backbuff_to_use);
      }
   }
}



