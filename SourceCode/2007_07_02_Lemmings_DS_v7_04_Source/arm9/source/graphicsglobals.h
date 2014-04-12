// Globals for lemmings DS like magnification, etc.
//  by Mathew Carr

#ifndef GRAPHICSGLOBALS_H
#define GRAPHICSGLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

#define DISPLAY_X_SIZE 256
#define DISPLAY_Y_SIZE 192

#define GAME_DISPLAY_X_SIZE  DISPLAY_X_SIZE
#define GAME_DISPLAY_Y_SIZE  168

#define GAME_DISPLAY_X_SIZE2 (GAME_DISPLAY_X_SIZE >> 1)
#define GAME_DISPLAY_Y_SIZE2 (GAME_DISPLAY_Y_SIZE >> 1)

#define GAME_DISPLAY_X_SIZE4 (GAME_DISPLAY_X_SIZE >> 2)
#define GAME_DISPLAY_Y_SIZE4 (GAME_DISPLAY_Y_SIZE >> 2)

extern s32 camera_x_inset;
extern s32 camera_y_inset;
extern s32 magnification;
extern s32 log2magnification;        
                             
#ifdef __cplusplus
}
#endif

#endif // GRAPHICSGLOBALS_H
