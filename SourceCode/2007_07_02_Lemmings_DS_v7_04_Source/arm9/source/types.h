// File containing definitions of various structs used in LLS.

#ifndef __TYPES_H__
#define __TYPES_H__
             
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagPOINT {
   s32 x, y;    
} POINT;

//typedef struct tagRECT {
//   s32 x1, y1;
//   s32 x2, y2;
//} RECT;

typedef struct tagS_TILE {
   u16 data[8*2]; // S tiles are 8 by 8, in -nibbles-.
} S_TILE;

typedef struct tagD_TILE {
   u16 data[8*4]; // D tiles are 8 by 8, in -bytes-.
} D_TILE;
                   
#ifdef __cplusplus
}
#endif

#endif // __TYPES_H__         

