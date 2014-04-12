// Globals for lemmings DS like magnification, etc.
//  by Mathew Carr

#ifndef __GRAPHICSGLOBALS_H__
#define __GRAPHICSGLOBALS_H__  

#include "ds_types.h"

#define BITMAP_PANE_LEVEL_X_SIZE 594
#define BITMAP_PANE_LEVEL_Y_SIZE 336

#define LEVEL_DISPLAY_X_SIZE  (BITMAP_PANE_LEVEL_X_SIZE/2)
#define LEVEL_DISPLAY_Y_SIZE  (BITMAP_PANE_LEVEL_Y_SIZE/2)

#define LEVEL_DISPLAY_X_SIZE2 (BITMAP_PANE_LEVEL_X_SIZE/4)
#define LEVEL_DISPLAY_Y_SIZE2 (BITMAP_PANE_LEVEL_Y_SIZE/4)

extern s32 camera_x_inset;
extern s32 camera_y_inset;

#endif // __GRAPHICSGLOBALS_H__
