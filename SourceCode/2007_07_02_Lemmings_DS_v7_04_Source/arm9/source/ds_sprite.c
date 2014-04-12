// DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#include "graphicsglobals.h"
#include "ds_sprite.h"
#include "ds_backbuff.h"

void DS_DrawDBSprite(const DS_SPRITE *sprite, int d_x, int d_y, const u16 *pal) {
   if (pal == 0) pal = sprite->palette;

   if (d_x >= SCREEN_WIDTH)       return;
   if (d_y >= SCREEN_HEIGHT)      return;
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

   for (src_y = start_src_y, dst_y = start_dst_y; (src_y < sprite->height) && (dst_y < SCREEN_HEIGHT); src_y++, dst_y++) {
      for (src_x = start_src_x, dst_x = start_dst_x; (src_x < sprite->width) && (dst_x < SCREEN_WIDTH); src_x++, dst_x++) {
         d = sprite->data[src_x + (src_y*(sprite->width))];
         if (d != 0) {
            DrawDBPixel(dst_x, dst_y, pal[d]);
         }
      }
   }
}

void DS_DrawSprite(const DS_SPRITE *sprite, int d_x, int d_y, const u16 *pal) {
   if (pal == 0) pal = sprite->palette;

   if (d_x >= SCREEN_WIDTH)       return;
   if (d_y >= SCREEN_HEIGHT)      return;
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

   for (src_y = start_src_y, dst_y = start_dst_y; (src_y < sprite->height) && (dst_y < SCREEN_HEIGHT); src_y++, dst_y++) {
      for (src_x = start_src_x, dst_x = start_dst_x; (src_x < sprite->width) && (dst_x < SCREEN_WIDTH); src_x++, dst_x++) {
         d = sprite->data[src_x + (src_y*(sprite->width))];
         if (d != 0) {
            DrawPixel(dst_x, dst_y, pal[d]);
         }
      }
   }
}

void DS_DrawDBSpriteNT(const DS_SPRITE *sprite, int d_x, int d_y, const u16 *pal) {
   if (pal == 0) pal = sprite->palette;

   if (d_x >= SCREEN_WIDTH)       return;
   if (d_y >= SCREEN_HEIGHT)      return;
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

   for (src_y = start_src_y, dst_y = start_dst_y; (src_y < sprite->height) && (dst_y < SCREEN_HEIGHT); src_y++, dst_y++) {
      for (src_x = start_src_x, dst_x = start_dst_x; (src_x < sprite->width) && (dst_x < SCREEN_WIDTH); src_x++, dst_x++) {
         d = sprite->data[src_x + (src_y*(sprite->width))];
         //if (d != 0) {
            DrawDBPixel(dst_x, dst_y, pal[d]);
         //}
      }
   }
}

void DS_DrawSpriteNT(const DS_SPRITE *sprite, int d_x, int d_y, const u16 *pal) {
   if (pal == 0) pal = sprite->palette;

   if (d_x > SCREEN_WIDTH)       return;
   if (d_y > SCREEN_HEIGHT)      return;
   if (d_x < (0-sprite->width))  return;
   if (d_y < (0-sprite->height)) return;

   u16 d;

   // These are the start coordinates (on the sprite for source)
   int start_src_x = (d_x < 0) ? (0-d_x) : 0;
   int start_src_y = (d_y < 0) ? (0-d_y) : 0;

   // These are the dest coordinates (on the screen for draw)
   int start_dst_x = (d_x < 0) ? 0 : d_x;
   int start_dst_y = (d_y < 0) ? 0 : d_y;

   int src_x, src_y, dst_x, dst_y;

   for (src_y = start_src_y, dst_y = start_dst_y; (src_y < sprite->height) && (dst_y < SCREEN_HEIGHT); src_y++, dst_y++) {
      for (src_x = start_src_x, dst_x = start_dst_x; (src_x < sprite->width) && (dst_x < SCREEN_WIDTH); src_x++, dst_x++) {
         d = sprite->data[src_x + (src_y*(sprite->width))];
         //if (d != 0) {
            DrawPixel(dst_x, dst_y, pal[d]);
         //}
      }
   }
}

void DS_BlitGFX(const DS_SPRITE *sprite, int d_x, int d_y) {
   int src_x, src_y, dst_x, dst_y;

   u8 *mem_loc = sprite->data;
   
   u16 *pal = sprite->palette;

   for (src_y = 0, dst_y = d_y; (src_y < sprite->height); src_y++, dst_y++) {
      for (src_x = 0, dst_x = d_x; (src_x < sprite->width); src_x++, dst_x++) {
         DrawPixel(dst_x, dst_y, pal[*mem_loc++]);
      }
   }
}

void DS_BlitGFXSpecPal(const DS_SPRITE *sprite, int d_x, int d_y, const u16* pal) {
   if (pal == 0) pal = sprite->palette;

   int src_x, src_y, dst_x, dst_y;

   u8 *mem_loc = sprite->data;

   for (src_y = 0, dst_y = d_y; (src_y < sprite->height); src_y++, dst_y++) {
      for (src_x = 0, dst_x = d_x; (src_x < sprite->width); src_x++, dst_x++) {
         DrawPixel(dst_x, dst_y, pal[*mem_loc++]);
      }
   }
}

void DS_DrawSpriteWMag(const DS_SPRITE *sprite, int x, int y, const u16* pal, int hflip) {
   if ((y                                         ) >= SCREEN_HEIGHT) return;
   if ((y + ((sprite->height * magnification) - 1)) <  0            ) return;

   if (pal == 0) pal = sprite->palette;

   int xd, yd;
   int xm, ym;
   int xl = sprite->width, yl = sprite->height;
   
   int dst_x, dst_y;
   
   u32 d;
   //u8 r, g, b;
   
   if (hflip == 0) {
      for (yd = 0; yd < yl; yd++) {
         for (xd = 0; xd < xl; xd++) {
            d = sprite->data[xd + (yd*(sprite->width))];

            if (d != 0) {
               for (ym = 0; ym < (s32)magnification; ym++) {    
                  dst_y = y + (yd << log2magnification) + ym;
                  if ((dst_y >= 0) && (dst_y < SCREEN_HEIGHT)) {
                     for (xm = 0; xm < (s32)magnification; xm++) {
                        dst_x = x + (xd << log2magnification) + xm;
                        if ((dst_x >= 0) && (dst_x < SCREEN_WIDTH)) {
                           DrawPixel(dst_x, dst_y, pal[d]);
                        }
                     }
                  }    
               }
            }
         }
      }            
   } else {
      for (yd = 0; yd < yl; yd++) {
         for (xd = 0; xd < xl; xd++) {
            d = sprite->data[xd + (yd*(sprite->width))];

            if (d != 0) {
               for (ym = 0; ym < (s32)magnification; ym++) {    
                  dst_y = y + (yd << log2magnification) + ym;
                  if ((dst_y >= 0) && (dst_y < SCREEN_HEIGHT)) {
                     for (xm = 0; xm < (s32)magnification; xm++) {
                        dst_x = x - ((xd << log2magnification) + xm - ((xl << log2magnification) - 1));
                        if ((dst_x >= 0) && (dst_x < SCREEN_WIDTH)) {
                           DrawPixel(dst_x, dst_y, pal[d]);
                        }
                     }
                  }    
               }
            }
         }
      }    
   }
}


void DS_DrawDBSpriteWMag(const DS_SPRITE *sprite, int x, int y, const u16* pal, int hflip) {   
   if ((y                                         ) >= SCREEN_HEIGHT) return;
   if ((y + ((sprite->height * magnification) - 1)) <  0            ) return;

   if (pal == 0) pal = sprite->palette;

   int xd, yd;
   int xm, ym;
   int xl = sprite->width, yl = sprite->height;
   
   int dst_x, dst_y;
   
   u32 d;
   //u8 r, g, b;
   
   if (hflip == 0) {
      for (yd = 0; yd < yl; yd++) {
         for (xd = 0; xd < xl; xd++) {
            d = sprite->data[xd + (yd*(sprite->width))];

            if (d != 0) {
               for (ym = 0; ym < (s32)magnification; ym++) {    
                  dst_y = y + (yd << log2magnification) + ym;
                  if ((dst_y >= 0) && (dst_y < SCREEN_HEIGHT)) {
                     for (xm = 0; xm < (s32)magnification; xm++) {
                        dst_x = x + (xd << log2magnification) + xm;
                        if ((dst_x >= 0) && (dst_x < SCREEN_WIDTH)) {
                           DrawDBPixel(dst_x, dst_y, d);
                        }
                     }
                  }    
               }
            }
         }
      }            
   } else {
      for (yd = 0; yd < yl; yd++) {
         for (xd = 0; xd < xl; xd++) {
            d = sprite->data[xd + (yd*(sprite->width))];

            if (d != 0) {
               for (ym = 0; ym < (s32)magnification; ym++) {    
                  dst_y = y + (yd << log2magnification) + ym;
                  if ((dst_y >= 0) && (dst_y < SCREEN_HEIGHT)) {
                     for (xm = 0; xm < (s32)magnification; xm++) {
                        dst_x = x - ((xd << log2magnification) + xm - ((xl << log2magnification) - 1));
                        if ((dst_x >= 0) && (dst_x < SCREEN_WIDTH)) {
                           DrawDBPixel(dst_x, dst_y, d);
                        }
                     }
                  }    
               }
            }
         }
      }    
   }
}

// This draws a sprite of a sprite to the screen.
// The x and y are in camera coordinates.
void DS_DrawSpriteStripWMag(const DS_SPRITE *sprite, int x, int y, int column, const u16* pal) {
   if ((x                    ) >= SCREEN_WIDTH) return;
   if ((x + magnification - 1) <  0           ) return;                       

   if ((y                                         ) >= SCREEN_HEIGHT) return;
   if ((y + ((sprite->height * magnification) - 1)) <  0            ) return;

   if (pal == 0) pal = sprite->palette;

   int yd;
   int xm, ym;
   int yl = sprite->height;

   int dst_x, dst_y;

   u32 d;
   //u8 r, g, b;

   for (yd = 0; yd < yl; yd++) {
      d = sprite->data[column + (yd*(sprite->width))];

      if (d != 0) {
         for (ym = 0; ym < (s32)magnification; ym++) {
            dst_y = y + (yd << log2magnification) + ym;
            if ((dst_y >= 0) && (dst_y < SCREEN_HEIGHT)) {
               for (xm = 0; xm < (s32)magnification; xm++) {
                  dst_x = x + xm;
                  if ((dst_x >= 0) && (dst_x < SCREEN_WIDTH)) {
                     DrawPixel(dst_x, dst_y, pal[d]);
                  }    
               }
            }
         }
      }
   }
}


