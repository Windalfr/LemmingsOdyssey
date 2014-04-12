// DS Sprite thingy that will run way too slow.
//  by Mathew Carr

#ifndef DS_SPRITE_H
#define DS_SPRITE_H
         
#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
#include <nds.h>
#else
#include "ds_types.h"
#endif

typedef struct tagDS_SPRITE {
   const u8   *data;
   s32   width, height;
   const u16  *palette;
} DS_SPRITE;

#ifdef ARM9
void DS_DrawDBSprite(const DS_SPRITE *sprite, int dest_x, int dest_y, const u16 *pal);
void DS_DrawSprite(const DS_SPRITE *sprite, int dest_x, int dest_y, const u16 *pal);

void DS_DrawDBSpriteNT(const DS_SPRITE *sprite, int dest_x, int dest_y, const u16 *pal);
void DS_DrawSpriteNT(const DS_SPRITE *sprite, int dest_x, int dest_y, const u16 *pal);

void DS_DrawSpriteWMag(const DS_SPRITE *sprite, int x, int y, const u16 *pal, int hflip);
void DS_DrawDBSpriteWMag(const DS_SPRITE *sprite, int x, int y, const u16 *pal, int hflip);

void DS_BlitGFX(const DS_SPRITE *sprite, int d_x, int d_y);
void DS_BlitGFXSpecPal(const DS_SPRITE *sprite, int d_x, int d_y, const u16* pal);

void DS_DrawSpriteStripWMag(const DS_SPRITE *sprite, int x, int y, int column, const u16* pal);
#endif

#ifdef __cplusplus
}
#endif

#endif // DS_SPRITE_H
