// DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#include "dsx_backbuff.h"
//                         A R G B
// 2d array containing the 1-5-5-5 colours that would be displayed on the DS.
u16 DSX_backbuff[DSX_BACKBUFFER_X_SIZE][DSX_BACKBUFFER_Y_SIZE];
              
void DSX_DrawPixel(int x, int y, int c, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE]) {
   if (x >= 0) {
      if (y >= 0) {
         if (x < DSX_BACKBUFFER_X_SIZE) {
            if (y < DSX_BACKBUFFER_Y_SIZE) {
               backbuff_to_use[x][y] = c | 0x8000;
            }
         }
      }
   }  
}
