//                                                     
// Lemmings DS Builder, an application for editing Lemmings DS *.lds files.
//
// (c) May 2006, June 2006, July 2006, August 2006
//     March, April, May 2007
//
// main.cpp
//   The main source file for the Lemmings DS Builder application.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//
#include <allegro.h>

#include <stdio.h>
#include <string.h>
#include <math.h>    

#include <vector>
#include <string>

static int    global_argc;
static char **global_argv;

#include "types.h"
#include "ds_types.h"
#include "graphicsglobals.h"
#include "dsx_backbuff.h"             
#include "dsx_sprite.h"
#include "lemmings_graphical_object.h"
#include "lemmings_level.h"
#include "lemmings_level_appended_texture.h"
#include "lemmings_texture_archive.h"

#include "bmp_loader.h"     

#define WINDOW_X_SIZE 800
#define WINDOW_Y_SIZE 600
BITMAP *bitmap_backbuffer;

#define GLOBAL_FILE_SELECTOR_WIDTH  500
#define GLOBAL_FILE_SELECTOR_HEIGHT 400

#define PI 3.1415926535     
                                                             
// Returns the argument a incrased to the next d.
int to_next(int a, int d) {    
   return ((a + (d-1)) / d) * d;
}


// Does the file exist?
bool file_exists(const char *filename) {
   FILE *input_file;

   input_file = fopen(filename, "rb");

   if (input_file == NULL) {
      return false;
   } else {
      fclose(input_file);
      return true;
   }
}

void draw_bitmap(BITMAP *source, BITMAP *dest, int dest_x, int dest_y) {
   blit(source, dest, 0, 0, dest_x, dest_y, source->w, source->h);     
}   
                                                                                
#define dump(src, dest)    acquire_bitmap(dest);            \
                           draw_bitmap(src, dest, 0, 0);    \
                           release_bitmap(dest);
                           
                           
void pageflip() {     
   dump(bitmap_backbuffer, screen);     
   //vsync();
   //clear_bitmap(bitmap_backbuffer);
}

char oldkey[KEY_MAX];
void DoKeyInput() {
   for (int n = 0; n < KEY_MAX; n++) {
      oldkey[KEY_MAX] = key[KEY_MAX]; 
   } 
   poll_keyboard(); 
}
#define KeyIsDown(x)  ( key[x])
#define KeyDown(x)    ( key[x] && !oldkey[x])
#define KeyHeld(x)    ( key[x] &&  oldkey[x])
#define KeyRelease(x) (!key[x] &&  oldkey[x])
#define KeyIdle(x)    (!key[x] && !oldkey[x])

#define MOUSE_LMB 0
#define MOUSE_RMB 1
#define MOUSE_MMB 2

int oldmouse_b;

int mouse_delta_x, mouse_delta_y;
void DoMouseInput() {
   oldmouse_b = mouse_b;
   poll_mouse();
   get_mouse_mickeys(&mouse_delta_x, &mouse_delta_y);
}                                     
#define MouseIsDown_oldmacro(x)  ( (mouse_b & (1 << x)))
#define MouseDown_oldmacro(x)    ( (mouse_b & (1 << x)) && (!(oldmouse_b & (1 << x))))
#define MouseHeld_oldmacro(x)    ( (mouse_b & (1 << x)) && ( (oldmouse_b & (1 << x))))
#define MouseRelease_oldmacro(x) (!(mouse_b & (1 << x)) && ( (oldmouse_b & (1 << x))))
#define MouseIdle_oldmacro(x)    (!(mouse_b & (1 << x)) && (!(oldmouse_b & (1 << x))))
             
int oldrmb_special_b, rmb_special_b;             
                         
void DoRMBConjunctionInput() {
   oldrmb_special_b = rmb_special_b;
   rmb_special_b = MouseIsDown_oldmacro(MOUSE_RMB) | KeyIsDown(KEY_0_PAD);  
} 

#define RMBConjunctionIsDown(x)  ( rmb_special_b)
#define RMBConjunctionDown(x)    ( rmb_special_b && !oldrmb_special_b)
#define RMBConjunctionHeld(x)    ( rmb_special_b &&  oldrmb_special_b)
#define RMBConjunctionRelease(x) (!rmb_special_b &&  oldrmb_special_b)
#define RMBConjunctionIdle(x)    (!rmb_special_b && !oldrmb_special_b)

static inline bool MouseIsDown(int x);
static inline bool MouseDown(int x);
static inline bool MouseHeld(int x);
static inline bool MouseRelease(int x);
static inline bool MouseIdle(int x);         
         
static inline bool MouseIsDown(int x) {
   return (x != MOUSE_RMB) 
             ? MouseIsDown_oldmacro(x)
             : RMBConjunctionIsDown();
}  
static inline bool MouseDown(int x) {
   return (x != MOUSE_RMB) 
             ? MouseDown_oldmacro(x)
             : RMBConjunctionDown();
}  
static inline bool MouseHeld(int x) {
   return (x != MOUSE_RMB) 
             ? MouseHeld_oldmacro(x)
             : RMBConjunctionHeld();
}  
static inline bool MouseRelease(int x) {
   return (x != MOUSE_RMB) 
             ? MouseRelease_oldmacro(x)
             : RMBConjunctionRelease();
}  
static inline bool MouseIdle(int x) {
   return (x != MOUSE_RMB) 
             ? MouseIdle_oldmacro(x)
             : RMBConjunctionIdle();
}  

// #define BITMAP_PANE_LEVEL_X_SIZE 594
// #define BITMAP_PANE_LEVEL_Y_SIZE 336
BITMAP *bitmap_pane_level;  // Used for the DSX display and associated level gadgets and overlays.

#define BITMAP_PANE_RIGHT_X_SIZE 206
#define BITMAP_PANE_RIGHT_Y_SIZE 800
BITMAP *bitmap_pane_right;

BITMAP *bitmap_pane_right_panels;
BITMAP *bitmap_pane_right_palette_mapper;
BITMAP *bitmap_pane_right_palette_editor;

//////////////////////////////////////////////////////////////
BITMAP *bitmap_lower_left_lemming_tool_interface;
BITMAP *bitmap_lower_left_lemming_tool_numbers;

BITMAP *bitmap_lower_left_lemming_tool_number[10];

BITMAP *bitmap_lower_left_lemming_tool_number_infinity;

BITMAP *bitmap_cursor;

BITMAP *bitmap_slider_marker;

BITMAP *Load24BitmapWithError(const char *filename, const char *error = "Resource loading error", int error_code = 10) {
   BITMAP *ret = load_bitmap(filename, NULL);
   if (ret == NULL) {
      allegro_message("Resource loading error.");
      exit(error_code);
   }     
   return ret;
}

int WHITE  ;
int BLACK  ;
int RED    ;
int GREEN  ;
int BLUE   ;
int CYAN   ;
int MAGENTA;
int YELLOW ;
int DGREY  ;
int LGREY  ;

int COLOUR_SELECTED_ITEM;
int COLOUR_SELECTED_WATER;  
int COLOUR_SELECTED_ACTIVE_ZONE;

int COLOUR_STEEL_AREA;            
int COLOUR_SELECTED_STEEL_AREA;
int COLOUR_ONE_WAY_AREA;          
int COLOUR_SELECTED_ONE_WAY_AREA;

void MakeColours() {
   WHITE   = makecol(255,255,255); 
   BLACK   = makecol(  0,  0,  0);
   RED     = makecol(255,  0,  0);
   GREEN   = makecol(  0,255,  0);
   BLUE    = makecol(  0,  0,255);  
   CYAN    = makecol(  0,255,255);
   MAGENTA = makecol(255,  0,255);
   YELLOW  = makecol(255,255,  0);
   DGREY   = makecol( 55, 55, 55);
   LGREY   = makecol(170,170,170);
   
   COLOUR_SELECTED_ITEM        = makecol(255,255,127);
   COLOUR_SELECTED_WATER       = makecol(155,205,255);
   COLOUR_SELECTED_ACTIVE_ZONE = makecol( 50,255, 50);
   
   COLOUR_STEEL_AREA            = makecol(190,190,190);
   COLOUR_SELECTED_STEEL_AREA   = makecol(255,255,255);
   COLOUR_ONE_WAY_AREA          = makecol( 30,200, 30);
   COLOUR_SELECTED_ONE_WAY_AREA = makecol( 90,255, 90);
}

void LoadLevelEditorResources() {
   bitmap_lower_left_lemming_tool_interface       = Load24BitmapWithError("resources/gfx/lemming tool interface.bmp");
   bitmap_lower_left_lemming_tool_numbers         = Load24BitmapWithError("resources/gfx/lemming tool interface numbers.bmp");
   bitmap_lower_left_lemming_tool_number_infinity = Load24BitmapWithError("resources/gfx/lemming tool interface infinity.bmp");
   
   for (int n = 0; n < 10; n++) {
      // Let's get some individual number sub bitmaps.
      bitmap_lower_left_lemming_tool_number[n] = create_sub_bitmap(bitmap_lower_left_lemming_tool_numbers,
                                                                                            0, n*16, 8, 16); 
   }
   
   bitmap_cursor = Load24BitmapWithError("resources/gfx/cursor.bmp");
   
   bitmap_slider_marker = Load24BitmapWithError("resources/gfx/slider marker.bmp");
}

void UnloadLevelEditorResources() {
   destroy_bitmap(bitmap_lower_left_lemming_tool_interface);   
   destroy_bitmap(bitmap_lower_left_lemming_tool_numbers);
   destroy_bitmap(bitmap_lower_left_lemming_tool_number_infinity);
   
   for (int n = 0; n < 10; n++) {
      // Let's get some individual number sub bitmaps.
      destroy_bitmap(bitmap_lower_left_lemming_tool_number[n]); 
   }
   
   destroy_bitmap(bitmap_cursor);
   
   destroy_bitmap(bitmap_slider_marker);
}
                                                
//////////////////////////////////////////////////////////////// 

#define MAX_NO_SELECTED_OBJECTS 2000

int currently_selected_object_array[MAX_NO_SELECTED_OBJECTS];

int currently_selected_object_handles_x1[MAX_NO_SELECTED_OBJECTS];
int currently_selected_object_handles_x2[MAX_NO_SELECTED_OBJECTS];
int currently_selected_object_handles_y1[MAX_NO_SELECTED_OBJECTS];
int currently_selected_object_handles_y2[MAX_NO_SELECTED_OBJECTS];

int currently_selected_object_count;

void SelectionRemoveItemNumber(int number) {
   bool found_yet = false;  
     
   for (int s = 0; s < currently_selected_object_count; s++) {
      if (!found_yet) {
         if (currently_selected_object_array[s] == number) {
            found_yet = true;
         }
      } else {
         currently_selected_object_array[s-1] = currently_selected_object_array[s];
      }
   }     
   if (found_yet) --currently_selected_object_count;                      
}

void SelectionAddItemNumber(int number) {
   if (currently_selected_object_count == MAX_NO_SELECTED_OBJECTS) return;  
     
   int insertion_location = -1;  
     
   for (int s = 0; s < currently_selected_object_count; s++) {
      if (currently_selected_object_array[s] < number) {
         insertion_location = s;
         break;
      }
   } 
   
   if (insertion_location != -1) {
      for (int s = currently_selected_object_count-1; s >= insertion_location; s--) {
         currently_selected_object_array[s+1] = currently_selected_object_array[s]; 
      }
      
      currently_selected_object_array[insertion_location] = number;
   } else {
      currently_selected_object_array[currently_selected_object_count] = number;
   }   
    
   ++currently_selected_object_count;  
}

void SelectionClear() {
   for (int s = 0; s < MAX_NO_SELECTED_OBJECTS; s++) {
      currently_selected_object_array[s] = 0;
   }
   currently_selected_object_count = 0;
}             

bool SelectionIsItemSelected(int number) {
   for (int s = 0; s < currently_selected_object_count; s++) {
      if (currently_selected_object_array[s] == number) return true; 
   }
   return false;
}              

#define MAX_NO_SELECTED_SPECIAL_OBJECTS (MAX_NO_ENTRANCES      \
                                       + MAX_NO_EXITS          \
                                       + MAX_NO_TRAPS          \
                                       + MAX_NO_HAZARDS        \
                                       + MAX_NO_UNINTERACTIVES \
                                       + MAX_NO_WATERS)

#define SELECTED_SPECIAL_OBJECTS_ENTRANCES_START      0                                          
#define SELECTED_SPECIAL_OBJECTS_ENTRANCES_END        (SELECTED_SPECIAL_OBJECTS_ENTRANCES_START      + MAX_NO_ENTRANCES      - 1)
#define SELECTED_SPECIAL_OBJECTS_EXITS_START          (SELECTED_SPECIAL_OBJECTS_ENTRANCES_END        + 1)
#define SELECTED_SPECIAL_OBJECTS_EXITS_END            (SELECTED_SPECIAL_OBJECTS_EXITS_START          + MAX_NO_EXITS          - 1)
#define SELECTED_SPECIAL_OBJECTS_TRAPS_START          (SELECTED_SPECIAL_OBJECTS_EXITS_END            + 1)
#define SELECTED_SPECIAL_OBJECTS_TRAPS_END            (SELECTED_SPECIAL_OBJECTS_TRAPS_START          + MAX_NO_TRAPS          - 1)
#define SELECTED_SPECIAL_OBJECTS_HAZARDS_START        (SELECTED_SPECIAL_OBJECTS_TRAPS_END            + 1)
#define SELECTED_SPECIAL_OBJECTS_HAZARDS_END          (SELECTED_SPECIAL_OBJECTS_HAZARDS_START        + MAX_NO_HAZARDS        - 1)
#define SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START (SELECTED_SPECIAL_OBJECTS_HAZARDS_END          + 1)                   
#define SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END   (SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START + MAX_NO_UNINTERACTIVES - 1)
#define SELECTED_SPECIAL_OBJECTS_WATERS_START         (SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END   + 1)
#define SELECTED_SPECIAL_OBJECTS_WATERS_END           (SELECTED_SPECIAL_OBJECTS_WATERS_START         + MAX_NO_WATERS         - 1)

// Numbers 0                               to                                    (MAX_NO_ENTRANCES  - 1)  are entrances
// Numbers MAX_NO_ENTRANCES                to (MAX_NO_ENTRANCES                + (MAX_NO_EXITS      - 1)) are exits
// Numbers MAX_NO_ENTRANCES + MAX_NO_EXITS to (MAX_NO_ENTRANCES + MAX_NO_EXITS + (MAX_NO_FLAMETRAPS - 1)) are flametraps
// Numbers 0 to (MAX_NO_ENTRANCES - 1) are entrances  

// ETC.

int currently_selected_special_object_array[MAX_NO_SELECTED_SPECIAL_OBJECTS];  

int currently_selected_special_object_handles_x[MAX_NO_SELECTED_SPECIAL_OBJECTS];
int currently_selected_special_object_handles_x2[MAX_NO_SELECTED_SPECIAL_OBJECTS];  // for water
int currently_selected_special_object_handles_y[MAX_NO_SELECTED_SPECIAL_OBJECTS];

int currently_selected_special_object_count;


void SelectionSpecialRemoveItemNumber(int number) {
   bool found_yet = false;  
     
   for (int s = 0; s < currently_selected_special_object_count; s++) {
      if (!found_yet) {
         if (currently_selected_special_object_array[s] == number) {
            found_yet = true;
         }
      } else {
         currently_selected_special_object_array[s-1] = currently_selected_special_object_array[s];
      }
   }     
   if (found_yet) --currently_selected_special_object_count;                      
}

void SelectionSpecialAddItemNumber(int number) {
   if (currently_selected_special_object_count == MAX_NO_SELECTED_OBJECTS) return;  
     
   int insertion_location = -1;  
     
   for (int s = 0; s < currently_selected_special_object_count; s++) {
      if (currently_selected_special_object_array[s] < number) {
         insertion_location = s;
         break;
      }
   } 
   
   if (insertion_location != -1) {
      for (int s = currently_selected_special_object_count-1; s >= insertion_location; s--) {
         currently_selected_special_object_array[s+1] = currently_selected_special_object_array[s]; 
      }
      
      currently_selected_special_object_array[insertion_location] = number;
   } else {
      currently_selected_special_object_array[currently_selected_special_object_count] = number;
   }   
    
   ++currently_selected_special_object_count;                      
}

void SelectionSpecialClear() {
   for (int s = 0; s < MAX_NO_SELECTED_SPECIAL_OBJECTS; s++) {
      currently_selected_special_object_array[s] = 0;
   }
   currently_selected_special_object_count = 0;
}             

bool SelectionSpecialIsItemSelected(int number) {
   for (int s = 0; s < currently_selected_special_object_count; s++) {
      if (currently_selected_special_object_array[s] == number) return true; 
   }
   return false;
}                

#define MAX_NO_SELECTED_AREA_OBJECTS (MAX_NO_STEEL_AREAS + MAX_NO_ONE_WAY_AREAS)

#define SELECTED_AREA_OBJECTS_STEEL_AREAS_START   (0)
#define SELECTED_AREA_OBJECTS_STEEL_AREAS_END     (MAX_NO_STEEL_AREAS   - 1)
#define SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START (SELECTED_AREA_OBJECTS_STEEL_AREAS_END + 1)
#define SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END   (SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START + MAX_NO_ONE_WAY_AREAS - 1)

int currently_selected_area_object_array[MAX_NO_SELECTED_AREA_OBJECTS];
                                                                             
int currently_selected_area_object_handles_x1[MAX_NO_SELECTED_AREA_OBJECTS];
int currently_selected_area_object_handles_x2[MAX_NO_SELECTED_AREA_OBJECTS]; 
int currently_selected_area_object_handles_y1[MAX_NO_SELECTED_AREA_OBJECTS];
int currently_selected_area_object_handles_y2[MAX_NO_SELECTED_AREA_OBJECTS];

int currently_selected_area_object_count;

void SelectionAreaRemoveItemNumber(int number) {
   bool found_yet = false;  
     
   for (int s = 0; s < currently_selected_area_object_count; s++) {
      if (!found_yet) {
         if (currently_selected_area_object_array[s] == number) {
            found_yet = true;
         }
      } else {
         currently_selected_area_object_array[s-1] = currently_selected_area_object_array[s];
      }
   }     
   if (found_yet) --currently_selected_area_object_count;                      
}

void SelectionAreaAddItemNumber(int number) {
   if (currently_selected_area_object_count == MAX_NO_SELECTED_AREA_OBJECTS) return;  
     
   int insertion_location = -1;  
     
   for (int s = 0; s < currently_selected_area_object_count; s++) {
      if (currently_selected_area_object_array[s] < number) {
         insertion_location = s;
         break;
      }
   } 
   
   if (insertion_location != -1) {
      for (int s = currently_selected_area_object_count-1; s >= insertion_location; s--) {
         currently_selected_area_object_array[s+1] = currently_selected_area_object_array[s]; 
      }
      
      currently_selected_area_object_array[insertion_location] = number;
   } else {
      currently_selected_area_object_array[currently_selected_area_object_count] = number;
   }   
    
   ++currently_selected_area_object_count;                      
}

void SelectionAreaClear() {
   for (int s = 0; s < MAX_NO_SELECTED_AREA_OBJECTS; s++) {
      currently_selected_area_object_array[s] = 0;
   }
   currently_selected_area_object_count = 0;
}             

bool SelectionAreaIsItemSelected(int number) {
   for (int s = 0; s < currently_selected_area_object_count; s++) {
      if (currently_selected_area_object_array[s] == number) return true; 
   }
   return false;
}                                 
                                                
//////////////////////////////////////////////////////////////// 
// These are used when rendering the DS emulated level

// The main memory chunk represents the entire size of any level.
// It's 4 megabytes big. If you have a level bigger than that... you're weird.
#define MAIN_MEMORY_CHUNK_SIZE (4*1024*1024)
u8 *main_memory_chunk;        
                         
LEMMINGS_LEVEL_LDS_FILE_V7 *level_working_on;  

LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *appended_texture_256s_begin; // This should always point to the beginning of the
                                                                  // memory where the appended_texture_256s are held.
                                                                  // (After the appended_texture_16s).
                                  
LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *terrain_objects_begin; // This should always point to the beginning of the
                                                             // memory where the level_objects are held.
                                                             // (After the texture objects).
                                                                  
// Returns a pointer to the nth (from zero) appended texture 16 in the level memory.
// If force is true, then bounding errors are ignored.
LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *GetAppendedTexture16(int n, bool force = false) {
   if (!force) {                                
      if (n >= level_working_on->no_appended_texture_16s) {
         return NULL;
      }  
   }                                           
   
   // This pointer is going to lead us to the correct texture.
   u8 *ptr_to_part = ((u8*)level_working_on)+sizeof(LEMMINGS_LEVEL_LDS_FILE_V7);
                           
   // Loop through all of the appended texture 16s after the lemmings level chunk.
   for (int object_extra_16 = 0; object_extra_16 < n; object_extra_16++) {
      ptr_to_part += ((LEMMINGS_LEVEL_APPENDED_TEXTURE_16*)ptr_to_part)->my_mem_size;
   }                   
   
   return (LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *)ptr_to_part;        
}        

void RefreshAppendedTexture256sBegin() {
   appended_texture_256s_begin = (LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *)GetAppendedTexture16(level_working_on->no_appended_texture_16s, true);  
}

LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *GetAppendedTexture256(int n, bool force = false) {
   if (!force) {                                
      if (n >= level_working_on->no_appended_texture_256s) {
         return NULL;
      }  
   }                                           
   
   // This pointer is going to lead us to the correct texture.
   u8 *ptr_to_part = (u8 *)appended_texture_256s_begin;
                           
   // Loop through all of the appended texture 256s after the lemmings level chunk.
   for (int object_extra_256 = 0; object_extra_256 < n; object_extra_256++) {
      ptr_to_part += ((LEMMINGS_LEVEL_APPENDED_TEXTURE_256*)ptr_to_part)->my_mem_size;
   }                   
   
   return (LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *)ptr_to_part;        
}
     
void RefreshTerrainObjectsBegin() {
   terrain_objects_begin = (LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *)GetAppendedTexture256(level_working_on->no_appended_texture_256s, true);   
}

LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *GetTerrainObject(int n, bool force = false) {      
   if (!force) { 
      if (n >= level_working_on->no_terrain_objects) {
         return NULL;
      }
   }
   
   u8 *ptr_to_part = (u8*)terrain_objects_begin;

   for (int level_object = 0; level_object < n; level_object++) {
             if (((LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER*)ptr_to_part)->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16 ) {
         ptr_to_part += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16);

      } else if (((LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER*)ptr_to_part)->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
         ptr_to_part += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256);

      } else if (((LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER*)ptr_to_part)->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1  ) {
         ptr_to_part += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1);
      }
   }                     
   
   return (LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *)ptr_to_part;        
}

// Right is referring to the further away memory locations

#define SHIFT_DISTANCE_NEW_LEVEL_TERRAIN_OBJECT_1                   (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  ))
#define SHIFT_DISTANCE_NEW_LEVEL_TERRAIN_OBJECT_16                  (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 ))
#define SHIFT_DISTANCE_NEW_LEVEL_TERRAIN_OBJECT_256                 (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256))
#define SHIFT_DISTANCE_DELETE_LEVEL_TERRAIN_OBJECT_1               (0-(sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  )))
#define SHIFT_DISTANCE_DELETE_LEVEL_TERRAIN_OBJECT_16              (0-(sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 )))
#define SHIFT_DISTANCE_DELETE_LEVEL_TERRAIN_OBJECT_256             (0-(sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_1_TO_LEVEL_TERRAIN_OBJECT_16   ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 )) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  )))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_1_TO_LEVEL_TERRAIN_OBJECT_256  ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  )))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_16_TO_LEVEL_TERRAIN_OBJECT_1   ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  )) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 )))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_16_TO_LEVEL_TERRAIN_OBJECT_256 ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 )))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_256_TO_LEVEL_TERRAIN_OBJECT_1  ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1  )) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)))
#define SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_256_TO_LEVEL_TERRAIN_OBJECT_16 ((sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16 )) - (sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)))
void ShiftLevelDataRight(u8 *starting_position, int spaces, u32 &chunk_size) {
   u8 *from, *to;
   
   from = main_memory_chunk + chunk_size - 1;  
   to   = from + spaces;  
   
   while (from >= starting_position){
      *(to--) = *(from--); 
   }  
   
   chunk_size += spaces;
}

void ShiftLevelDataLeft(u8 *starting_position, int spaces, u32 &chunk_size) {
   u8 *from, *to;
    
   from = starting_position;
   to   = starting_position - spaces;  
   
   while (from < (chunk_size + main_memory_chunk)){
      *(to++) = *(from++); 
   }  
   
   chunk_size -= spaces;
}      
    
void ShiftLevelData(u8 *starting_position, int spaces, u32 &chunk_size) {
   if (spaces == 0) return;
   
   int original_size = chunk_size;
   
   if (spaces < 0) {
      ShiftLevelDataLeft(starting_position, 0-spaces, chunk_size);
   } else {
      ShiftLevelDataRight(starting_position, spaces, chunk_size);
   }          
                  
   //allegro_message("chunk shift by %d.\nfrom %d to %d", spaces, original_size, chunk_size);
}           

void DeleteTerrainObject(int object) {  
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected = GetTerrainObject(object);
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_ahead    = GetTerrainObject(object + 1, true);
                                                                                              
   //allegro_message("Deleting object %d", currently_selected_object_array[o]);
                                    
   int shift_distance = (int)object_selected - (int)object_ahead;
   ShiftLevelData((u8*)object_ahead, shift_distance, level_working_on->lemmings_level_file_size);
                                                                        
   --level_working_on->no_terrain_objects;        
   
   for (int o = 0; o < currently_selected_object_count; o++) {
      if (object < currently_selected_object_array[o]) {
         --currently_selected_object_array[o];
      }
   }
}  

LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *AddTerrainObject(int type, LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *source_type = NULL) {  
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *new_object = GetTerrainObject(level_working_on->no_terrain_objects, true);
                                                                        
   ++level_working_on->no_terrain_objects;        
   
   if (type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
      if (source_type != NULL) memcpy(new_object, source_type, sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16));   
      level_working_on->lemmings_level_file_size += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16);      
   } else
   if (type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {              
      if (source_type != NULL) memcpy(new_object, source_type, sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256)); 
      level_working_on->lemmings_level_file_size += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256);  
   } else
   if (type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {                   
      if (source_type != NULL) memcpy(new_object, source_type, sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1));
      level_working_on->lemmings_level_file_size += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1);            
   }
   
   return new_object;
}

void SwapLevelObjects(int a, int b) { 
   if (abs(a - b) != 1) return;                           //allegro_message("going to swap %d and %d", a, b);
     
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_a = GetTerrainObject(a);
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_b = GetTerrainObject(b);
   
   int size_a, size_b;
   
   if (object_a->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
      size_a = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16);
   if (object_a->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256)
      size_a = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256);
   if (object_a->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1)
      size_a = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1);
   
   if (object_b->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
      size_b = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16);
   if (object_b->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256)
      size_b = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256);
   if (object_b->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1)
      size_b = sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1);
   
   u8 *object_a_copy = new u8[size_a];
   memcpy((void *)object_a_copy, (const void *)object_a, size_a);
   u8 *object_b_copy = new u8[size_b];     
   memcpy((void *)object_b_copy, (const void *)object_b, size_b);
   
   if (object_b > object_a) {
      memcpy((void *)((u8*)object_a         ), (const void *)object_b_copy, size_b);
      memcpy((void *)((u8*)object_a + size_b), (const void *)object_a_copy, size_a);
   } else {
      memcpy((void *)((u8*)object_b         ), (const void *)object_a_copy, size_a);
      memcpy((void *)((u8*)object_b + size_a), (const void *)object_b_copy, size_b);
   }
   
   delete[] object_a_copy;
   delete[] object_b_copy;
} 
      
// This swaps objects and b, which are of the same type.
void SwapLevelSpecialObjects(int absolute_id_a, int absolute_id_b) {
   if (absolute_id_a > absolute_id_b) {
      absolute_id_a ^= absolute_id_b ^= absolute_id_a ^= absolute_id_b;
   }  
   
   int category_id_a, category_id_b;
     
   if ((absolute_id_a >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START) 
    && (absolute_id_a <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {
      category_id_a = absolute_id_a - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START; 
      category_id_b = absolute_id_b - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START; 
       
      int temp_entrance_x, temp_entrance_y, temp_entrance_d;
      
      temp_entrance_x = level_working_on->runtime_stats.entrance_x[category_id_a];
      temp_entrance_y = level_working_on->runtime_stats.entrance_y[category_id_a];
      temp_entrance_d = level_working_on->runtime_stats.entrance_d[category_id_a];
      
      level_working_on->runtime_stats.entrance_x[category_id_a] = level_working_on->runtime_stats.entrance_x[category_id_b];
      level_working_on->runtime_stats.entrance_y[category_id_a] = level_working_on->runtime_stats.entrance_y[category_id_b];
      level_working_on->runtime_stats.entrance_d[category_id_a] = level_working_on->runtime_stats.entrance_d[category_id_b];
      
      level_working_on->runtime_stats.entrance_x[category_id_b] = temp_entrance_x;
      level_working_on->runtime_stats.entrance_y[category_id_b] = temp_entrance_y;
      level_working_on->runtime_stats.entrance_d[category_id_b] = temp_entrance_d;
   } else 
   if ((absolute_id_a >= SELECTED_SPECIAL_OBJECTS_EXITS_START) 
    && (absolute_id_a <= SELECTED_SPECIAL_OBJECTS_EXITS_END  )) {     
      category_id_a = absolute_id_a - SELECTED_SPECIAL_OBJECTS_EXITS_START; 
      category_id_b = absolute_id_b - SELECTED_SPECIAL_OBJECTS_EXITS_START; 
      
      int temp_exit_x, temp_exit_y;
      
      temp_exit_x = level_working_on->runtime_stats.exit_x[category_id_a];
      temp_exit_y = level_working_on->runtime_stats.exit_y[category_id_a];
      
      level_working_on->runtime_stats.exit_x[category_id_a] = level_working_on->runtime_stats.exit_x[category_id_b];
      level_working_on->runtime_stats.exit_y[category_id_a] = level_working_on->runtime_stats.exit_y[category_id_b];
      
      level_working_on->runtime_stats.exit_x[category_id_b] = temp_exit_x;
      level_working_on->runtime_stats.exit_y[category_id_b] = temp_exit_y;
   } else 
   if ((absolute_id_a >= SELECTED_SPECIAL_OBJECTS_TRAPS_START) 
    && (absolute_id_a <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) {    
      category_id_a = absolute_id_a - SELECTED_SPECIAL_OBJECTS_TRAPS_START; 
      category_id_b = absolute_id_b - SELECTED_SPECIAL_OBJECTS_TRAPS_START; 
      
      int temp_trap_x, temp_trap_y, temp_trap_z, temp_trap_genus;
      
      temp_trap_x     = level_working_on->runtime_stats.trap_x[category_id_a];
      temp_trap_y     = level_working_on->runtime_stats.trap_y[category_id_a];
      temp_trap_z     = level_working_on->runtime_stats.trap_z[category_id_a];
      temp_trap_genus = level_working_on->runtime_stats.trap_genus[category_id_a];
                                            
      level_working_on->runtime_stats.trap_x[    category_id_a] = level_working_on->runtime_stats.trap_x[    category_id_b];
      level_working_on->runtime_stats.trap_y[    category_id_a] = level_working_on->runtime_stats.trap_y[    category_id_b];
      level_working_on->runtime_stats.trap_z[    category_id_a] = level_working_on->runtime_stats.trap_z[    category_id_b];
      level_working_on->runtime_stats.trap_genus[category_id_a] = level_working_on->runtime_stats.trap_genus[category_id_b];
      
      level_working_on->runtime_stats.trap_x[    category_id_b] = temp_trap_x;
      level_working_on->runtime_stats.trap_y[    category_id_b] = temp_trap_y;
      level_working_on->runtime_stats.trap_z[    category_id_b] = temp_trap_z;
      level_working_on->runtime_stats.trap_genus[category_id_b] = temp_trap_genus;
   } else 
   if ((absolute_id_a >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START) 
    && (absolute_id_a <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) {    
      category_id_a = absolute_id_a - SELECTED_SPECIAL_OBJECTS_HAZARDS_START; 
      category_id_b = absolute_id_b - SELECTED_SPECIAL_OBJECTS_HAZARDS_START; 
      
      int temp_hazard_x, temp_hazard_y, temp_hazard_z, temp_hazard_genus;
      
      temp_hazard_x     = level_working_on->runtime_stats.hazard_x[    category_id_a];
      temp_hazard_y     = level_working_on->runtime_stats.hazard_y[    category_id_a];
      temp_hazard_z     = level_working_on->runtime_stats.hazard_z[    category_id_a];
      temp_hazard_genus = level_working_on->runtime_stats.hazard_genus[category_id_a];
                                            
      level_working_on->runtime_stats.hazard_x[    category_id_a] = level_working_on->runtime_stats.hazard_x[    category_id_b];
      level_working_on->runtime_stats.hazard_y[    category_id_a] = level_working_on->runtime_stats.hazard_y[    category_id_b];
      level_working_on->runtime_stats.hazard_z[    category_id_a] = level_working_on->runtime_stats.hazard_z[    category_id_b];
      level_working_on->runtime_stats.hazard_genus[category_id_a] = level_working_on->runtime_stats.hazard_genus[category_id_b];
      
      level_working_on->runtime_stats.hazard_x[    category_id_b] = temp_hazard_x;
      level_working_on->runtime_stats.hazard_y[    category_id_b] = temp_hazard_y;
      level_working_on->runtime_stats.hazard_z[    category_id_b] = temp_hazard_z;
      level_working_on->runtime_stats.hazard_genus[category_id_b] = temp_hazard_genus;
   } else 
   if ((absolute_id_a >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START) 
    && (absolute_id_a <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) {    
      category_id_a = absolute_id_a - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START; 
      category_id_b = absolute_id_b - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START; 
      
      int temp_uninteractive_x, temp_uninteractive_y, temp_uninteractive_z, temp_uninteractive_genus;
      
      temp_uninteractive_x     = level_working_on->runtime_stats.uninteractive_x[    category_id_a];
      temp_uninteractive_y     = level_working_on->runtime_stats.uninteractive_y[    category_id_a];
      temp_uninteractive_z     = level_working_on->runtime_stats.uninteractive_z[    category_id_a];
      temp_uninteractive_genus = level_working_on->runtime_stats.uninteractive_genus[category_id_a];
                                            
      level_working_on->runtime_stats.uninteractive_x[    category_id_a] = level_working_on->runtime_stats.uninteractive_x[    category_id_b];
      level_working_on->runtime_stats.uninteractive_y[    category_id_a] = level_working_on->runtime_stats.uninteractive_y[    category_id_b];
      level_working_on->runtime_stats.uninteractive_z[    category_id_a] = level_working_on->runtime_stats.uninteractive_z[    category_id_b];
      level_working_on->runtime_stats.uninteractive_genus[category_id_a] = level_working_on->runtime_stats.uninteractive_genus[category_id_b];
      
      level_working_on->runtime_stats.uninteractive_x[    category_id_b] = temp_uninteractive_x;
      level_working_on->runtime_stats.uninteractive_y[    category_id_b] = temp_uninteractive_y;
      level_working_on->runtime_stats.uninteractive_z[    category_id_b] = temp_uninteractive_z;
      level_working_on->runtime_stats.uninteractive_genus[category_id_b] = temp_uninteractive_genus;
   } 
}                                   

int TextureTopAtPoint(int x, int y) {
   const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info; 
    
   for (int o = level_working_on->no_terrain_objects - 1; o >= 0; o--) {
      generic_object_info = (const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(o);
      
      if ((x >= generic_object_info->x1)
       && (x <= generic_object_info->x2)
       && (y >= generic_object_info->y1)
       && (y <= generic_object_info->y2)) {
         return o;
      }
   } 
    
   return -1; // no texture 
}                             

int AreaTopAtPoint(int x, int y) {    
   for (int o = level_working_on->runtime_stats.no_steel_areas - 1; o >= 0; o--) {      
      if ((x >= level_working_on->runtime_stats.steel_area_x1[o])
       && (x <= level_working_on->runtime_stats.steel_area_x2[o])
       && (y >= level_working_on->runtime_stats.steel_area_y1[o])
       && (y <= level_working_on->runtime_stats.steel_area_y2[o])) {
         return o + SELECTED_AREA_OBJECTS_STEEL_AREAS_START;
      }
   } 
   for (int o = level_working_on->runtime_stats.no_one_way_areas - 1; o >= 0; o--) {      
      if ((x >= level_working_on->runtime_stats.one_way_area_x1[o])
       && (x <= level_working_on->runtime_stats.one_way_area_x2[o])
       && (y >= level_working_on->runtime_stats.one_way_area_y1[o])
       && (y <= level_working_on->runtime_stats.one_way_area_y2[o])) {
         return o + SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START;
      }
   } 
    
   return -1; // no texture 
}  

// Truncation functions make sure that all level pieces lie within the level.
void TruncateLevelObjectsToLevel() { return;
   LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info;
     
   for (int o = level_working_on->no_terrain_objects - 1; o >= 0; o--) {      //allegro_message("analysing object %d for error", o);
      generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(o);
                                                                                                  
      if (generic_object_info->x1 < (0               )) generic_object_info->x1 = 0; 
      if (generic_object_info->y1 < (0               )) generic_object_info->y1 = 0; 
      if (generic_object_info->x2 > (LEVEL_X_SIZE - 1)) generic_object_info->x2 = LEVEL_X_SIZE - 1; 
      if (generic_object_info->y2 > (LEVEL_Y_SIZE - 1)) generic_object_info->y2 = LEVEL_Y_SIZE - 1; 
      
      if ((generic_object_info->x1 > generic_object_info->x2)
       || (generic_object_info->y1 > generic_object_info->y2)) {      //allegro_message("object %d fail test:\nx1: %d, x2: %d\ny1: %d, y2: %d", o, generic_object_info->x1, generic_object_info->x2, generic_object_info->y1, generic_object_info->y2);
         SelectionRemoveItemNumber(o);         
         DeleteTerrainObject(o); 
      } 
   }  
}
              
void TruncateLevelAreaObjectsToLevel() {  
   for (int a = 0; a < level_working_on->runtime_stats.no_steel_areas; a++) {
      if (level_working_on->runtime_stats.steel_area_x1[a] < (0               )) level_working_on->runtime_stats.steel_area_x1[a] = 0;
      if (level_working_on->runtime_stats.steel_area_y1[a] < (0               )) level_working_on->runtime_stats.steel_area_y1[a] = 0;
      if (level_working_on->runtime_stats.steel_area_x2[a] > (LEVEL_X_SIZE - 1)) level_working_on->runtime_stats.steel_area_x2[a] = LEVEL_X_SIZE - 1;
      if (level_working_on->runtime_stats.steel_area_y2[a] > (LEVEL_Y_SIZE - 1)) level_working_on->runtime_stats.steel_area_y2[a] = LEVEL_Y_SIZE - 1;
   }  
   
   for (int a = 0; a < level_working_on->runtime_stats.no_one_way_areas; a++) {
      if (level_working_on->runtime_stats.one_way_area_x1[a] < (0               )) level_working_on->runtime_stats.one_way_area_x1[a] = 0;
      if (level_working_on->runtime_stats.one_way_area_y1[a] < (0               )) level_working_on->runtime_stats.one_way_area_y1[a] = 0;
      if (level_working_on->runtime_stats.one_way_area_x2[a] > (LEVEL_X_SIZE - 1)) level_working_on->runtime_stats.one_way_area_x2[a] = LEVEL_X_SIZE - 1;
      if (level_working_on->runtime_stats.one_way_area_y2[a] > (LEVEL_Y_SIZE - 1)) level_working_on->runtime_stats.one_way_area_y2[a] = LEVEL_Y_SIZE - 1;
   }  
}
                        
// The raw data for the level.
u8 level_data[LEVEL_X_SIZE][LEVEL_Y_SIZE];

void DSX_DrawLevel() {
   u16 *level_palette_to_use = level_working_on->runtime_stats.level_palette;

   int xp, yp;
   int xd, yd;

   int ld;         

   for (xp = 0; xp < LEVEL_DISPLAY_X_SIZE; xp++) {
      for (yp = 0; yp < LEVEL_DISPLAY_Y_SIZE; yp++) {
         xd = (camera_x_inset + xp);
         yd = (camera_y_inset + yp);

         ld = level_data[xd][yd];
         if (ld != 0) {
            DSX_DrawPixel(xp, yp, level_palette_to_use[ld], DSX_backbuff);
         }
      }
   }
}                        
                                            
#define RELEASE_RATE_MINIMUM  1
#define RELEASE_RATE_MAXIMUM 99  

// These hold the numbers of standard and custom exits respectively.
int no_standard_exits = 0;
int   no_custom_exits = 0;

// These are vectors of pointers to the graphical objects containing the exits.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_exit_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_exit_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_exit_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_exit_graphical_object_sprites;

// These hold the numbers of standard and custom entrances respectively.
int no_standard_entrances = 0;
int   no_custom_entrances = 0;

// These are vectors of pointers to the graphical objects containing the entrances.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_entrance_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_entrance_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_entrance_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_entrance_graphical_object_sprites;     

// These hold the numbers of standard and custom traps respectively.
int no_standard_traps = 0;
int   no_custom_traps = 0;

// These are vectors of pointers to the graphical objects containing the traps.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_trap_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_trap_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_trap_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_trap_graphical_object_sprites;

// These hold the numbers of standard and custom hazards respectively.
int no_standard_hazards = 0;
int   no_custom_hazards = 0;

// These are vectors of pointers to the graphical objects containing the hazards.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_hazard_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_hazard_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_hazard_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_hazard_graphical_object_sprites;

// These hold the numbers of standard and custom uninteractives respectively.
int no_standard_uninteractives = 0;
int   no_custom_uninteractives = 0;

// These are vectors of pointers to the graphical objects containing the uninteractives.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_uninteractive_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_uninteractive_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_uninteractive_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_uninteractive_graphical_object_sprites;

// These hold the numbers of standard and custom waters respectively.
int no_standard_waters = 0;
int   no_custom_waters = 0;

// These are vectors of pointers to the graphical objects containing the waters.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_water_graphical_objects;
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *>   custom_water_graphical_objects;

// These are the DSX_SPRITE counterparts generated from the above graphical objects.
std::vector<DSX_SPRITE *> standard_water_graphical_object_sprites;
std::vector<DSX_SPRITE *>   custom_water_graphical_object_sprites;

// This function generalises the loading of graphical objects of a specific type from a specific
// location into a specific vector.
void DSX_GreedyGatherObjectCategory(std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> *destination_go_vector,     // Where will the pointer to this new memory go?
                                    std::vector<DSX_SPRITE *>                       *destination_sprite_vector, // Where will the sprites generated go?
                                    int *quantity_int,                                                          // Where's an int to keep track of this stuff?
                                    const char *source_directory,                                               // Where's the directory?
                                    const char *source_graphical_object_type,                                   // Whats the filename start?
                                    u16 *palette_to_set);                                                       // What palette are these sprites going to be using after they're generated?
   
// This function will gather all of the graphical objects it can from the standard and custom directories.
void DSX_GreedyGatherAllAvailableGraphicalObjects() {
   const char *standard_loading_location = "standard_graphical_objects/"; 
   const char *custom_loading_location   = "custom_graphical_objects/";  
   
   const char *object_category_strings[] = {"exit",
                                            "entrance",
                                            "trap",
                                            "hazard",
                                            "uninteractive",
                                            "water"};
     
   DSX_GreedyGatherObjectCategory(&standard_exit_graphical_objects,
                                  &standard_exit_graphical_object_sprites,
                                  &no_standard_exits,
                                  standard_loading_location,
                                  object_category_strings[0],
                                  level_working_on->runtime_stats.exit_palette);  
     
   DSX_GreedyGatherObjectCategory(&standard_entrance_graphical_objects,
                                  &standard_entrance_graphical_object_sprites,
                                  &no_standard_entrances,
                                  standard_loading_location,
                                  object_category_strings[1],
                                  level_working_on->runtime_stats.entrance_palette);  
     
   DSX_GreedyGatherObjectCategory(&standard_trap_graphical_objects,
                                  &standard_trap_graphical_object_sprites,
                                  &no_standard_traps,
                                  standard_loading_location,
                                  object_category_strings[2],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&standard_hazard_graphical_objects,
                                  &standard_hazard_graphical_object_sprites,
                                  &no_standard_hazards,
                                  standard_loading_location,
                                  object_category_strings[3],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&standard_uninteractive_graphical_objects,
                                  &standard_uninteractive_graphical_object_sprites,
                                  &no_standard_uninteractives,
                                  standard_loading_location,
                                  object_category_strings[4],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&standard_water_graphical_objects,
                                  &standard_water_graphical_object_sprites,
                                  &no_standard_waters,
                                  standard_loading_location,
                                  object_category_strings[5],
                                  level_working_on->runtime_stats.water_palette);       
     
   DSX_GreedyGatherObjectCategory(&custom_exit_graphical_objects,
                                  &custom_exit_graphical_object_sprites,
                                  &no_custom_exits,
                                  custom_loading_location,
                                  object_category_strings[0],
                                  level_working_on->runtime_stats.exit_palette);  
     
   DSX_GreedyGatherObjectCategory(&custom_entrance_graphical_objects,
                                  &custom_entrance_graphical_object_sprites,
                                  &no_custom_entrances,
                                  custom_loading_location,
                                  object_category_strings[1],
                                  level_working_on->runtime_stats.entrance_palette);  
     
   DSX_GreedyGatherObjectCategory(&custom_trap_graphical_objects,
                                  &custom_trap_graphical_object_sprites,
                                  &no_custom_traps,
                                  custom_loading_location,
                                  object_category_strings[2],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&custom_hazard_graphical_objects,
                                  &custom_hazard_graphical_object_sprites,
                                  &no_custom_hazards,
                                  custom_loading_location,
                                  object_category_strings[3],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&custom_uninteractive_graphical_objects,
                                  &custom_uninteractive_graphical_object_sprites,
                                  &no_custom_uninteractives,
                                  custom_loading_location,
                                  object_category_strings[4],
                                  0);  
     
   DSX_GreedyGatherObjectCategory(&custom_water_graphical_objects,
                                  &custom_water_graphical_object_sprites,
                                  &no_custom_waters,
                                  custom_loading_location,
                                  object_category_strings[5],
                                  level_working_on->runtime_stats.water_palette); 
}                                      

void DSX_GreedyGatherObjectCategory(std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> *destination_go_vector,     // Where will the pointer to this new memory go?
                                    std::vector<DSX_SPRITE *>                       *destination_sprite_vector, // Where will the sprites generated go?
                                    int *quantity_int,                                                          // Where's an int to keep track of this stuff?
                                    const char *source_directory,                                               // Where's the directory?
                                    const char *source_graphical_object_type,                                   // Whats the filename start?
                                    u16 *palette_to_set) {                                                      // What palette are these sprites going to be using after they're generated?
   // Reset counter                                 
   *quantity_int = 0;                                 
                                    
   // Grab all you can                               
   do {         
      // This holds the filename of the file we're going to load
      char incoming_graphical_object_filename[16384];
      sprintf(incoming_graphical_object_filename, "%s%s_%d.lgo", source_directory, source_graphical_object_type, *quantity_int);
                                                    
      if (!file_exists(incoming_graphical_object_filename)) {
         //allegro_message("found %d %ss in %s.", *quantity_int, source_graphical_object_type, source_directory);
         return;
      }
      
      unsigned int graphical_object_filesize;
      
      // Open the file
      FILE *graphical_object_file = fopen(incoming_graphical_object_filename, "rb");
                            
      // Grab the expected filesize                                                
      fread(&graphical_object_filesize, 4, 1, graphical_object_file);
      
      rewind(graphical_object_file);                                

      // Allocate memory for a memory instance of the file
      u8 *graphical_object_memory_instance = new u8[graphical_object_filesize];
                                
      // Load the file into memory
      fread(graphical_object_memory_instance, graphical_object_filesize, 1, graphical_object_file); 
      
      // Close the file.
      fclose(graphical_object_file);
      
      // Clonk this pointer onto the graphical object pointer vector.
      destination_go_vector->push_back((LEMMINGS_GRAPHICAL_OBJECT_HEADER *)graphical_object_memory_instance);
      
      DSX_SPRITE *generated_sprite_array = GraphicalObject_ConstructDSSpriteArray((LEMMINGS_GRAPHICAL_OBJECT_HEADER *)graphical_object_memory_instance,
                                                                                  palette_to_set);     
      
      // Clonk this pointer onto the graphical object pointer vector.
      destination_sprite_vector->push_back(generated_sprite_array); 
      
      (*quantity_int)++;
   } while (1);   
}

void DSX_DestroyGraphicalObjectsAndSprites() {
   for (int o = 0; o < no_standard_exits; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_exit_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(standard_exit_graphical_object_sprites.at(o));
   }  
                             
   for (int o = 0; o < no_standard_entrances; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_entrance_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(standard_entrance_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_standard_traps; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_trap_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(standard_trap_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_standard_hazards; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_hazard_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(standard_hazard_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_standard_uninteractives; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_uninteractive_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(standard_uninteractive_graphical_object_sprites.at(o));
   }  
       
   for (int o = 0; o < no_custom_exits; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] custom_exit_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(custom_exit_graphical_object_sprites.at(o));
   }  
                             
   for (int o = 0; o < no_custom_entrances; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] custom_entrance_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(custom_entrance_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_custom_traps; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] custom_trap_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(custom_trap_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_custom_hazards; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] custom_hazard_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(custom_hazard_graphical_object_sprites.at(o));
   }  
   
   for (int o = 0; o < no_custom_uninteractives; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] custom_uninteractive_graphical_objects.at(o);
      // Free the malloced memory containing the DSX_SPRITE
      free(custom_uninteractive_graphical_object_sprites.at(o));
   }  
}

void DSX_DrawExit(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, bool ignore_handles_and_centre, int x, int y,  bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd = (x - ((!ignore_handles_and_centre) ? graphical_object->handle_x : (graphical_object->graphic_width  /2))) - (camera_motion_effect ? camera_x_inset : 0);
   int yd = (y - ((!ignore_handles_and_centre) ? graphical_object->handle_y : (graphical_object->graphic_height /2))) - (camera_motion_effect ? camera_y_inset : 0);
   
   DSX_DrawSprite(&sprite_array[frame], xd, yd, 0, backbuff_to_use);
}

void DSX_DrawEntrance(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, bool ignore_handles_and_centre, int x, int y, bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd = (x - ((!ignore_handles_and_centre) ? graphical_object->handle_x : (graphical_object->graphic_width  /2))) - (camera_motion_effect ? camera_x_inset : 0);
   int yd = (y - ((!ignore_handles_and_centre) ? graphical_object->handle_y : (graphical_object->graphic_height /2))) - (camera_motion_effect ? camera_y_inset : 0);
   
   DSX_DrawSprite(&sprite_array[frame], xd, yd, 0, backbuff_to_use);
}

void DSX_DrawTrap(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, bool ignore_handles_and_centre, int x, int y, u16 *palette_to_use, bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd = (x - ((!ignore_handles_and_centre) ? graphical_object->handle_x : (graphical_object->graphic_width  /2))) - (camera_motion_effect ? camera_x_inset : 0);
   int yd = (y - ((!ignore_handles_and_centre) ? graphical_object->handle_y : (graphical_object->graphic_height /2))) - (camera_motion_effect ? camera_y_inset : 0);
   
   DSX_DrawSprite(&sprite_array[frame], xd, yd, palette_to_use, backbuff_to_use);
}

void DSX_DrawHazard(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, bool ignore_handles_and_centre, int x, int y, u16 *palette_to_use, bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd = (x - ((!ignore_handles_and_centre) ? graphical_object->handle_x : (graphical_object->graphic_width  /2))) - (camera_motion_effect ? camera_x_inset : 0);
   int yd = (y - ((!ignore_handles_and_centre) ? graphical_object->handle_y : (graphical_object->graphic_height /2))) - (camera_motion_effect ? camera_y_inset : 0);
   
   DSX_DrawSprite(&sprite_array[frame], xd, yd, palette_to_use, backbuff_to_use);
}

void DSX_DrawUninteractive(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, bool ignore_handles_and_centre, int x, int y, u16 *palette_to_use, bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd = (x - ((!ignore_handles_and_centre) ? graphical_object->handle_x : (graphical_object->graphic_width  /2))) - (camera_motion_effect ? camera_x_inset : 0);
   int yd = (y - ((!ignore_handles_and_centre) ? graphical_object->handle_y : (graphical_object->graphic_height /2))) - (camera_motion_effect ? camera_y_inset : 0);
   
   DSX_DrawSprite(&sprite_array[frame], xd, yd, palette_to_use, backbuff_to_use);
}

void DSX_DrawWaterArea(LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, DSX_SPRITE *sprite_array, int frame, int x1, int x2, int y, bool camera_motion_effect = true, u16 backbuff_to_use[][DSX_BACKBUFFER_Y_SIZE] = DSX_backbuff) {
   int xd, yd;

   int water_width = (x2 - x1);  

   yd = ((y - graphical_object->handle_y)) - (camera_motion_effect ? camera_y_inset : 0);      
                  
   int lower_edge = yd - (graphical_object->graphic_height - 1);                
                                                          
   if ((yd > 0) && (lower_edge < LEVEL_DISPLAY_Y_SIZE)) {       
      for (int x = 0; x <= water_width; x++) {
         xd = ((x + x1)) - (camera_motion_effect ? camera_x_inset : 0);
         DSX_DrawSpriteStrip(&sprite_array[frame], xd, yd, x % graphical_object->graphic_width, 0, backbuff_to_use);
      }
   }
}

#define TOOL_CLIMBER  0
#define TOOL_FLOATER  1
#define TOOL_EXPLODER 2
#define TOOL_BLOCKER  3
#define TOOL_BUILDER  4
#define TOOL_BASHER   5
#define TOOL_MINER    6
#define TOOL_DIGGER   7

#define DSX_COLOUR_BLUE_PART  (31 << 10)
#define DSX_COLOUR_GREEN_PART (31 <<  5)
#define DSX_COLOUR_RED_PART   (31 <<  0)

void DSX_MagicDSXScreenToLevelPane2X(BITMAP *destination_bitmap = bitmap_pane_level) {
   int colour;  
   acquire_bitmap(destination_bitmap);
   for (int src_y = 0; src_y < LEVEL_DISPLAY_Y_SIZE; src_y++) {
      for (int src_x = 0; src_x < LEVEL_DISPLAY_X_SIZE; src_x++) {
         colour = DSX_backbuff[src_x][src_y];
         
         colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));
                             
         _putpixel24(destination_bitmap, src_x*2+0, src_y*2+0, colour);
         _putpixel24(destination_bitmap, src_x*2+1, src_y*2+0, colour);
         _putpixel24(destination_bitmap, src_x*2+0, src_y*2+1, colour);
         _putpixel24(destination_bitmap, src_x*2+1, src_y*2+1, colour);  
      } 
   }                
   release_bitmap(bitmap_pane_level);
}

void DSX_MagicDSXScreenToLevelPane1X(BITMAP *destination_bitmap = bitmap_pane_level) {
   int colour;  
   acquire_bitmap(destination_bitmap);
   for (int src_y = 0; src_y < LEVEL_DISPLAY_Y_SIZE; src_y++) {
      for (int src_x = 0; src_x < LEVEL_DISPLAY_X_SIZE; src_x++) {
         colour = DSX_backbuff[src_x][src_y];
         
         colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));
                             
         _putpixel24(destination_bitmap, src_x, src_y, colour); 
      } 
   }                
   release_bitmap(bitmap_pane_level);
}

void CameraFocusTo(int x, int y) {
   camera_x_inset = x - (LEVEL_DISPLAY_X_SIZE >> (1));
   camera_y_inset = y - (LEVEL_DISPLAY_Y_SIZE >> (1));

   if (camera_x_inset < 0) {
      camera_x_inset = 0;
   }
   if (camera_x_inset > ((s32)((LEVEL_X_SIZE)) - LEVEL_DISPLAY_X_SIZE)) {
      camera_x_inset = ((LEVEL_X_SIZE)) - LEVEL_DISPLAY_X_SIZE;
   }
   if (camera_y_inset < 0) {
      camera_y_inset = 0;
   }
   if (camera_y_inset > ((s32)((LEVEL_Y_SIZE)) - LEVEL_DISPLAY_Y_SIZE)) {
      camera_y_inset = ((LEVEL_Y_SIZE)) - LEVEL_DISPLAY_Y_SIZE;
   }
}   

void DrawLowerLeftToolInterfaceNumber(int number, int x, int y) {
   draw_bitmap(bitmap_lower_left_lemming_tool_number[number], bitmap_backbuffer, x, y);
}


int SpecialObjectTopAtPoint(int x, int y) {    
   // Get the raw number for the entrance genus junction 
   int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                      & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
    
   // Return the correct graphical object for the entrance based on the genus junction value.   
   LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                             & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                               ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                               :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);
    
   for (int o = level_working_on->runtime_stats.no_entrances - 1; o >= 0; o--) {      
      if ((x >= (level_working_on->runtime_stats.entrance_x[o] - active_entrance_graphical_object->handle_x                                                       ))
       && (x <= (level_working_on->runtime_stats.entrance_x[o] - active_entrance_graphical_object->handle_x + active_entrance_graphical_object->graphic_width  - 1))
       && (y >= (level_working_on->runtime_stats.entrance_y[o] - active_entrance_graphical_object->handle_y                                                       ))
       && (y <= (level_working_on->runtime_stats.entrance_y[o] - active_entrance_graphical_object->handle_y + active_entrance_graphical_object->graphic_height - 1))) {
         return o + SELECTED_SPECIAL_OBJECTS_ENTRANCES_START;
      }
   } 
       
   // Get the raw number for the exit genus junction 
   int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                      & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
    
   // Return the correct graphical object for the exit based on the genus junction value.   
   LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                             & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                               ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                               :   custom_exit_graphical_objects.at(exit_genus_junction_id);
    
   for (int o = level_working_on->runtime_stats.no_exits - 1; o >= 0; o--) {      
      if ((x >= (level_working_on->runtime_stats.exit_x[o] - active_exit_graphical_object->handle_x                                                       ))
       && (x <= (level_working_on->runtime_stats.exit_x[o] - active_exit_graphical_object->handle_x + active_exit_graphical_object->graphic_width  - 1))
       && (y >= (level_working_on->runtime_stats.exit_y[o] - active_exit_graphical_object->handle_y                                                       ))
       && (y <= (level_working_on->runtime_stats.exit_y[o] - active_exit_graphical_object->handle_y + active_exit_graphical_object->graphic_height - 1))) {
         return o + SELECTED_SPECIAL_OBJECTS_EXITS_START;
      }
   } 
       
   for (int o = level_working_on->runtime_stats.no_traps - 1; o >= 0; o--) {      
      // Get the raw number for this trap genus junction 
      int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[o]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the trap based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[o]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                  :   custom_trap_graphical_objects.at(trap_genus_junction_id);
       
      if ((x >= (level_working_on->runtime_stats.trap_x[o] - active_trap_graphical_object->handle_x                                                       ))
       && (x <= (level_working_on->runtime_stats.trap_x[o] - active_trap_graphical_object->handle_x + active_trap_graphical_object->graphic_width  - 1))
       && (y >= (level_working_on->runtime_stats.trap_y[o] - active_trap_graphical_object->handle_y                                                       ))
       && (y <= (level_working_on->runtime_stats.trap_y[o] - active_trap_graphical_object->handle_y + active_trap_graphical_object->graphic_height - 1))) {
         return o + SELECTED_SPECIAL_OBJECTS_TRAPS_START;
      }
   } 
                         
   for (int o = level_working_on->runtime_stats.no_hazards - 1; o >= 0; o--) {      
      // Get the raw number for this hazard genus junction 
      int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[o]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the hazard based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[o]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                  :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);
       
      if ((x >= (level_working_on->runtime_stats.hazard_x[o] - active_hazard_graphical_object->handle_x                                                       ))
       && (x <= (level_working_on->runtime_stats.hazard_x[o] - active_hazard_graphical_object->handle_x + active_hazard_graphical_object->graphic_width  - 1))
       && (y >= (level_working_on->runtime_stats.hazard_y[o] - active_hazard_graphical_object->handle_y                                                       ))
       && (y <= (level_working_on->runtime_stats.hazard_y[o] - active_hazard_graphical_object->handle_y + active_hazard_graphical_object->graphic_height - 1))) {
         return o + SELECTED_SPECIAL_OBJECTS_HAZARDS_START;
      }
   } 
   
   for (int o = level_working_on->runtime_stats.no_uninteractives - 1; o >= 0; o--) {      
      // Get the raw number for this uninteractive genus junction 
      int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[o]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the uninteractive based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[o]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                  :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);
       
      if ((x >= (level_working_on->runtime_stats.uninteractive_x[o] - active_uninteractive_graphical_object->handle_x                                                       ))
       && (x <= (level_working_on->runtime_stats.uninteractive_x[o] - active_uninteractive_graphical_object->handle_x + active_uninteractive_graphical_object->graphic_width  - 1))
       && (y >= (level_working_on->runtime_stats.uninteractive_y[o] - active_uninteractive_graphical_object->handle_y                                                       ))
       && (y <= (level_working_on->runtime_stats.uninteractive_y[o] - active_uninteractive_graphical_object->handle_y + active_uninteractive_graphical_object->graphic_height - 1))) {
         return o + SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START;
      }
   }    
   
   // Get the raw number for the water genus junction 
   int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                      & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
    
   // Return the correct graphical object for the water based on the genus junction value.   
   LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_water_graphical_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                             & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                               ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                               :   custom_water_graphical_objects.at(water_genus_junction_id);

   int water_y_further = ((active_water_graphical_object->graphic_height - active_water_graphical_object->handle_y) - 1);                                                                               
   
   for (int o = level_working_on->runtime_stats.no_waters - 1; o >= 0; o--) {      
      if ((x >= (level_working_on->runtime_stats.water_x1[o]))
       && (x <= (level_working_on->runtime_stats.water_x2[o]))
       && (y >= (level_working_on->runtime_stats.water_y[o]))
       && (y <= (level_working_on->runtime_stats.water_y[o] + water_y_further))) {
         return o + SELECTED_SPECIAL_OBJECTS_WATERS_START;
      }
   } 

   return -1; // no object 
}

// -----------------------------------------------------------------------------

// This needs to be some kind of method to store
// loaded textures from the USING archive.

// This pointer holds the location of a loaded lemmings texture archive file.
LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_active_texture_archive;

// -----------------------------------------------------------------------------

#define CURRENTLY_SELECTED_OBJECT_BLINK_RATE 20

int currently_selected_object_blink_frame = 0;   

#define TRANSPARENT_COLOUR_BLINK_RATE 30

int transparent_colour_blink_frame = 0;

void DrawSelectedBoxOnItem(int item) {
   int x_left, x_right, y_top, y_bottom;
   
   const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(item);
   
   x_left   = (2 * (generic_object_info->x1 - camera_x_inset)) - 1;
   x_right  = (2 * (generic_object_info->x2 - camera_x_inset)) + 2;
   y_top    = (2 * (generic_object_info->y1 - camera_y_inset)) - 1;
   y_bottom = (2 * (generic_object_info->y2 - camera_y_inset)) + 2;
   
   rect(bitmap_pane_level, x_left, y_top, x_right, y_bottom, COLOUR_SELECTED_ITEM);
}                     

void DrawSelectedBoxOnSpecialItem(int item) {
   int x_left, x_right, y_top, y_bottom;
   int x_s_left = -1, x_s_right = -1, y_s_top = -1, y_s_bottom = -1;
   
   if ((item >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {            
      // Get the raw number for the entrance genus junction 
      int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the entrance based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                  :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);
                          
      int entrance = item - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START; 
      x_left   = (2 * (level_working_on->runtime_stats.entrance_x[entrance] - active_entrance_graphical_object->handle_x - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.entrance_x[entrance] - active_entrance_graphical_object->handle_x + active_entrance_graphical_object->graphic_width  - 1 - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.entrance_y[entrance] - active_entrance_graphical_object->handle_y - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.entrance_y[entrance] - active_entrance_graphical_object->handle_y + active_entrance_graphical_object->graphic_height - 1 - camera_y_inset)) + 2;

      int entrance_direction_triangle_points[6] = {(2 * ((level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset) + 0)),
                                                   (2 * ((level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset) - 5 + 3)),
                                                   (2 * ((level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset) + 0)),
                                                   (2 * ((level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset) + 5 + 3)),
                                                   (2 * ((level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset) + ((level_working_on->runtime_stats.entrance_d[entrance] == 1)
                                                                                                                                 ? 5 : (0-5)))),   
                                                   (2 * ((level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset) + 0 + 3)),};
                           
      polygon(bitmap_pane_level, 3, entrance_direction_triangle_points, WHITE);  
      
      x_s_left   = (2 * (level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset));
      x_s_right  = (2 * (level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset)) + 1;
      y_s_top    = (2 * (level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset));
      y_s_bottom = (2 * (level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset)) + 1;
      
      textprintf_centre(bitmap_pane_level, font, (2*(level_working_on->runtime_stats.entrance_x[entrance] - camera_x_inset)),
                                                 (2*(level_working_on->runtime_stats.entrance_y[entrance] - camera_y_inset)) -14,
                                                 WHITE,
                                                 "%d",
                                                 entrance);                          
   } else 
   if ((item >= SELECTED_SPECIAL_OBJECTS_EXITS_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_EXITS_END  )) {
      // Get the raw number for the exit genus junction 
      int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the exit based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                  :   custom_exit_graphical_objects.at(exit_genus_junction_id);
                          
      int exit = item - SELECTED_SPECIAL_OBJECTS_EXITS_START; 
      x_left   = (2 * (level_working_on->runtime_stats.exit_x[exit] - active_exit_graphical_object->handle_x - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.exit_x[exit] - active_exit_graphical_object->handle_x + active_exit_graphical_object->graphic_width  - 1 - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.exit_y[exit] - active_exit_graphical_object->handle_y - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.exit_y[exit] - active_exit_graphical_object->handle_y + active_exit_graphical_object->graphic_height - 1 - camera_y_inset)) + 2;

      x_s_left   = (2 * (level_working_on->runtime_stats.exit_x[exit] + active_exit_graphical_object->active_zone_x1 - camera_x_inset));
      x_s_right  = (2 * (level_working_on->runtime_stats.exit_x[exit] + active_exit_graphical_object->active_zone_x2 - camera_x_inset)) + 1;
      y_s_top    = (2 * (level_working_on->runtime_stats.exit_y[exit] + active_exit_graphical_object->active_zone_y1 - camera_y_inset));
      y_s_bottom = (2 * (level_working_on->runtime_stats.exit_y[exit] + active_exit_graphical_object->active_zone_y2 - camera_y_inset)) + 1;
   } else
   if ((item >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) {
      int trap = item - SELECTED_SPECIAL_OBJECTS_TRAPS_START; 
      
      // Get the raw number for the trap genus junction 
      int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[trap]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the trap based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[trap]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                  :   custom_trap_graphical_objects.at(trap_genus_junction_id);
                          
      x_left   = (2 * (level_working_on->runtime_stats.trap_x[trap] - active_trap_graphical_object->handle_x - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.trap_x[trap] - active_trap_graphical_object->handle_x + active_trap_graphical_object->graphic_width  - 1 - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.trap_y[trap] - active_trap_graphical_object->handle_y - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.trap_y[trap] - active_trap_graphical_object->handle_y + active_trap_graphical_object->graphic_height - 1 - camera_y_inset)) + 2;

      x_s_left   = (2 * (level_working_on->runtime_stats.trap_x[trap] + active_trap_graphical_object->active_zone_x1 - camera_x_inset));
      x_s_right  = (2 * (level_working_on->runtime_stats.trap_x[trap] + active_trap_graphical_object->active_zone_x2 - camera_x_inset)) + 1;
      y_s_top    = (2 * (level_working_on->runtime_stats.trap_y[trap] + active_trap_graphical_object->active_zone_y1 - camera_y_inset));
      y_s_bottom = (2 * (level_working_on->runtime_stats.trap_y[trap] + active_trap_graphical_object->active_zone_y2 - camera_y_inset)) + 1;
   } else
   if ((item >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) {
      int hazard = item - SELECTED_SPECIAL_OBJECTS_HAZARDS_START; 
      
      // Get the raw number for the hazard genus junction 
      int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[hazard]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the hazard based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[hazard]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                  :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);
                          
      x_left   = (2 * (level_working_on->runtime_stats.hazard_x[hazard] - active_hazard_graphical_object->handle_x - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.hazard_x[hazard] - active_hazard_graphical_object->handle_x + active_hazard_graphical_object->graphic_width  - 1 - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.hazard_y[hazard] - active_hazard_graphical_object->handle_y - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.hazard_y[hazard] - active_hazard_graphical_object->handle_y + active_hazard_graphical_object->graphic_height - 1 - camera_y_inset)) + 2;
   
      x_s_left   = (2 * (level_working_on->runtime_stats.hazard_x[hazard] + active_hazard_graphical_object->active_zone_x1 - camera_x_inset));
      x_s_right  = (2 * (level_working_on->runtime_stats.hazard_x[hazard] + active_hazard_graphical_object->active_zone_x2 - camera_x_inset)) + 1;
      y_s_top    = (2 * (level_working_on->runtime_stats.hazard_y[hazard] + active_hazard_graphical_object->active_zone_y1 - camera_y_inset));
      y_s_bottom = (2 * (level_working_on->runtime_stats.hazard_y[hazard] + active_hazard_graphical_object->active_zone_y2 - camera_y_inset)) + 1;
   } else
   if ((item >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) {
      int uninteractive = item - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START; 
      
      // Get the raw number for the uninteractive genus junction 
      int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[uninteractive]]
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the uninteractive based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[uninteractive]]
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                  :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);
                          
      x_left   = (2 * (level_working_on->runtime_stats.uninteractive_x[uninteractive] - active_uninteractive_graphical_object->handle_x - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.uninteractive_x[uninteractive] - active_uninteractive_graphical_object->handle_x + active_uninteractive_graphical_object->graphic_width  - 1 - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.uninteractive_y[uninteractive] - active_uninteractive_graphical_object->handle_y - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.uninteractive_y[uninteractive] - active_uninteractive_graphical_object->handle_y + active_uninteractive_graphical_object->graphic_height - 1 - camera_y_inset)) + 2;
   } else   
   if ((item >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
    && (item <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {
      // Get the raw number for the water genus junction 
      int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the water based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_water_graphical_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                                  :   custom_water_graphical_objects.at(water_genus_junction_id);
   
      int water_y_further = ((active_water_graphical_object->graphic_height - active_water_graphical_object->handle_y) - 1);                                                                               
      
      int water = item - SELECTED_SPECIAL_OBJECTS_WATERS_START; 
      x_left   = (2 * (level_working_on->runtime_stats.water_x1[water] - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.water_x2[water] - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.water_y[water]  - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.water_y[water]  + water_y_further - camera_y_inset)) + 2;
   
      x_s_left   = x_left  + 1;
      x_s_right  = x_right - 1;
      y_s_top    = (2 * (level_working_on->runtime_stats.water_y[water] + active_water_graphical_object->active_zone_y1 - camera_y_inset));
      y_s_bottom = (2 * (level_working_on->runtime_stats.water_y[water] + active_water_graphical_object->active_zone_y2 - camera_y_inset)) + 1;
   }  
                  
   if (item >= SELECTED_SPECIAL_OBJECTS_WATERS_START) {
      rect(bitmap_pane_level, x_left, y_top, x_right, y_bottom, COLOUR_SELECTED_WATER);
   } else {
      rect(bitmap_pane_level, x_left, y_top, x_right, y_bottom, COLOUR_SELECTED_ITEM);
   }
   
   if ((x_s_left != x_s_right) &&  (y_s_top != y_s_bottom)) {
      rect(bitmap_pane_level, x_s_left, y_s_top, x_s_right, y_s_bottom, COLOUR_SELECTED_ACTIVE_ZONE);
   }
}                  

void DrawSteelArea(int area, bool selected, bool outside) {
   int x_left, x_right, y_top, y_bottom;     
   
   if (outside) {
      x_left   = (2 * (level_working_on->runtime_stats.steel_area_x1[area] - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.steel_area_x2[area] - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.steel_area_y1[area] - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.steel_area_y2[area] - camera_y_inset)) + 2;
   } else {
      x_left   = (2 * (level_working_on->runtime_stats.steel_area_x1[area] - camera_x_inset));
      x_right  = (2 * (level_working_on->runtime_stats.steel_area_x2[area] - camera_x_inset)) + 1;
      y_top    = (2 * (level_working_on->runtime_stats.steel_area_y1[area] - camera_y_inset));
      y_bottom = (2 * (level_working_on->runtime_stats.steel_area_y2[area] - camera_y_inset)) + 1;
   }
   
   int colour = selected ? COLOUR_SELECTED_STEEL_AREA : COLOUR_STEEL_AREA;
   
   line(bitmap_pane_level,  x_left, y_top, x_right, y_bottom, colour);
   line(bitmap_pane_level, x_right, y_top,  x_left, y_bottom, colour);
   
   rect(bitmap_pane_level, x_left, y_top, x_right, y_bottom, colour);
}      

void DrawOneWayArea(int area, bool selected, bool outside) {
   int x_left, x_right, y_top, y_bottom;     
   
   if (outside) {
      x_left   = (2 * (level_working_on->runtime_stats.one_way_area_x1[area] - camera_x_inset)) - 1;
      x_right  = (2 * (level_working_on->runtime_stats.one_way_area_x2[area] - camera_x_inset)) + 2;
      y_top    = (2 * (level_working_on->runtime_stats.one_way_area_y1[area] - camera_y_inset)) - 1;
      y_bottom = (2 * (level_working_on->runtime_stats.one_way_area_y2[area] - camera_y_inset)) + 2;
   } else {
      x_left   = (2 * (level_working_on->runtime_stats.one_way_area_x1[area] - camera_x_inset));
      x_right  = (2 * (level_working_on->runtime_stats.one_way_area_x2[area] - camera_x_inset)) + 1;
      y_top    = (2 * (level_working_on->runtime_stats.one_way_area_y1[area] - camera_y_inset));
      y_bottom = (2 * (level_working_on->runtime_stats.one_way_area_y2[area] - camera_y_inset)) + 1;
   }  
              
   int colour = selected ? COLOUR_SELECTED_ONE_WAY_AREA : COLOUR_ONE_WAY_AREA;           
                                                 
   line(bitmap_pane_level,  x_left, y_top, x_right, y_bottom, colour);
   line(bitmap_pane_level, x_right, y_top,  x_left, y_bottom, colour);
   
   rect(bitmap_pane_level,  x_left, y_top, x_right, y_bottom, colour);
   
   int mid_x = (level_working_on->runtime_stats.one_way_area_x1[area] + level_working_on->runtime_stats.one_way_area_x2[area]) >> 1;
   int mid_y = (level_working_on->runtime_stats.one_way_area_y1[area] + level_working_on->runtime_stats.one_way_area_y2[area]) >> 1;
   
   int one_way_area_direction_triangle_points[6] = {(2 * ((mid_x - camera_x_inset) + 0)),
                                                    (2 * ((mid_y - camera_y_inset) - 5)),
                                                    (2 * ((mid_x - camera_x_inset) + 0)),
                                                    (2 * ((mid_y - camera_y_inset) + 5)),
                                                    (2 * ((mid_x - camera_x_inset) + ((level_working_on->runtime_stats.one_way_area_d[area] == 1)
                                                                                                                              ? 5 : (0-5)))),   
                                                    (2 * ((mid_y - camera_y_inset) + 0)),};
                           
   polygon(bitmap_pane_level, 3, one_way_area_direction_triangle_points, WHITE);  
}            

void DrawCameraFocus() {
   int x, y;     
   
   x = (2 * (level_working_on->runtime_stats.camera_x - camera_x_inset)) - 1;
   y = (2 * (level_working_on->runtime_stats.camera_y - camera_y_inset)) - 1;
                                              
   rect(bitmap_pane_level, x - 4,
                           y - 4,
                           x + 4,
                           y + 4, WHITE); 
                               
   rectfill(bitmap_pane_level, x - 3,
                               y - 3,
                               x + 3,  
                               y + 3, RED);
}   

void rectfill15a(BITMAP *destination, int x1, int y1, int x2, int y2, int colour) {
   colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                      (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                      (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));     
                      
   rectfill(destination, x1, y1, x2, y2, colour);      
}

//////////////////////////////////////////////////////////////// 

#define EDITOR_TOOL_SELECT_TEXTURE                0
#define EDITOR_TOOL_SELECT_SPEC_OBJ               1

#define EDITOR_TOOL_NEW_TEXTURE_ON_CLICK          2 // You've clicked it, but nothing has happened yet.
#define EDITOR_TOOL_NEW_SOLID_TEXTURE_ON_CLICK    3 // You've gotta stab the level pane for this to have any effect.
                                            
#define EDITOR_TOOL_PLACE_DUPE_TEXTURE_ON_CLICK   4 // You've clicked it, but not placed it.
                                                    // A 'moving' tool is redundant with respect to
                                                    // EDITOR_TOOL_MOVING_TEXTURE
                                                    
#define EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK       5                                                     
                                                    
#define EDITOR_TOOL_PLACING_TEXTURE               6 // Okay, you've clicked and you're wrestling around with it on the screen.

#define EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK        7 
#define EDITOR_TOOL_SCALING_TEXTURE_LMB           8
#define EDITOR_TOOL_SCALING_TEXTURE_RMB           9
                                                     
#define EDITOR_TOOL_GRAB_TEXTURE                 99 // not used   - These tools don't have a persistent mode:
#define EDITOR_TOOL_APPLY_TEXTURE                98 // not used      They only have an immediate effect on selected items.
                                                 
#define EDITOR_TOOL_MOVING_TEXTURE               11
#define EDITOR_TOOL_MOVING_SPEC_OBJ              12

#define EDITOR_TOOL_PLACE_WATER_ON_CLICK    13
#define EDITOR_TOOL_PLACING_WATER           14    
#define EDITOR_TOOL_SCALING_WATER_LMB       15
#define EDITOR_TOOL_SCALING_WATER_RMB       16

// --------------------------------------------
                                                                                                      
#define EDITOR_TOOL_SELECT_AREA                  32

#define EDITOR_TOOL_NEW_STEEL_AREA_ON_CLICK      33 // You've clicked it, but nothing has happened yet.
#define EDITOR_TOOL_NEW_ONE_WAY_AREA_ON_CLICK    34 // You've clicked it, but nothing has happened yet.                                               
                                                    
#define EDITOR_TOOL_PLACING_AREA                 35 // Okay, you've clicked and you're wrestling around with it on the screen.

#define EDITOR_TOOL_SCALE_AREA_ON_CLICK          36 
#define EDITOR_TOOL_SCALING_AREA_LMB             37
#define EDITOR_TOOL_SCALING_AREA_RMB             38  

#define EDITOR_TOOL_MOVING_AREA                  39

// --------------------------------------------

#define EDITOR_TOOL_CAMERA_FOCUS_POSITION_ON_CLICK  100 
#define EDITOR_TOOL_CAMERA_FOCUS_POSITIONING_LMB    101

// --------------------------------------------
                                                     
int editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;

#define SCREEN_LAYOUT_LEVEL_PANE_X_POSITION   0 
#define SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION  70 

void BlitLevelPaneToBackbuffer() {
   blit(bitmap_pane_level, bitmap_backbuffer, 0,
                                              0, SCREEN_LAYOUT_LEVEL_PANE_X_POSITION,
                                                 SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION, BITMAP_PANE_LEVEL_X_SIZE,
                                                                                      BITMAP_PANE_LEVEL_Y_SIZE);  
}
                                     
#define SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION SCREEN_LAYOUT_LEVEL_PANE_X_POSITION
#define SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION
#define SCREEN_LAYOUT_LEVEL_PANE_END_X_POSITION   (SCREEN_LAYOUT_LEVEL_PANE_X_POSITION + BITMAP_PANE_LEVEL_X_SIZE - 1)
#define SCREEN_LAYOUT_LEVEL_PANE_END_Y_POSITION   (SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION + BITMAP_PANE_LEVEL_Y_SIZE - 1)
                          
void ConvertScreenCoordsToLevelCoords(int src_x, int src_y, int &dst_x, int &dst_y) {           
   dst_x = (((src_x - SCREEN_LAYOUT_LEVEL_PANE_X_POSITION) >> 1) + camera_x_inset);  
   dst_y = (((src_y - SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION) >> 1) + camera_y_inset);        
   
   //allegro_message("Detected click at: %d, %d", src_x, src_y);
   //allegro_message("Click resolved to: %d, %d", dst_x, dst_y);  
}

#define LOWER_INTERFACE_OPTION_SET_DEFAULT    0
#define LOWER_INTERFACE_OPTION_SET_AREA_SETUP 1 // For steel and one way and stuff like that.

int lower_interface_current_option_set = LOWER_INTERFACE_OPTION_SET_DEFAULT;

int lower_interface_currently_selected_texture_adjustment_tox_value = 0;
int lower_interface_currently_selected_texture_adjustment_toy_value = 0;
int lower_interface_currently_selected_texture_adjustment_tsx_value = 64;
int lower_interface_currently_selected_texture_adjustment_tsy_value = 64;

int lower_interface_currently_selected_subtractive_value    = 0;

#define LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING          0
#define LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_MASKING_ENABLED     1
#define LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_ONLY_DRAW_ON_BLANKS 2
int lower_interface_currently_selected_masking_behaviour_value = LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING;

void BlitLowerInterfaceToBackbuffer() {     
   const int COLOUR_LOWER_INTERFACE_BACKDROP = 10 << 16 | 10 << 8 | 50;
      
#define SCREEN_LAYOUT_LOWER_INTERFACE_START_X_POSITION 0
#define SCREEN_LAYOUT_LOWER_INTERFACE_START_Y_POSITION 406
#define SCREEN_LAYOUT_LOWER_INTERFACE_END_X_POSITION   593
#define SCREEN_LAYOUT_LOWER_INTERFACE_END_Y_POSITION   599
      
   rectfill(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_START_X_POSITION,
                               SCREEN_LAYOUT_LOWER_INTERFACE_START_Y_POSITION, 
                               SCREEN_LAYOUT_LOWER_INTERFACE_END_X_POSITION,
                               SCREEN_LAYOUT_LOWER_INTERFACE_END_Y_POSITION,
                               COLOUR_LOWER_INTERFACE_BACKDROP);
                               

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START            40
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END             560
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_LENGTH                 \
         ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END                 \
                 - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START) + 1)
         
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y                415
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y                435
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y                455
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y                475
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE                8  
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_TEXT                 4
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_VALUE_TEXT        (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END + 34)
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_X_DIFFERENCE    3
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_Y_DIFFERENCE    3
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE      3
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_TOP        8192
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_BOTTOM     2048

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_TOP(n)                     \
              (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START                         \
                 + ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_LENGTH * (n + (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_TOP   /2)))/(SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_TOP   )))
                 
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_BOTTOM(n)                  \
              (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START                         \
                 + ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_LENGTH * (n + (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_BOTTOM/2)))/(SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_BOTTOM)))

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_TOP(n)                       \
              ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_TOP(n)                 \
                 + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_TOP(n+1))/2) 

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_BOTTOM(n)                    \
              ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_BOTTOM(n)              \
                 + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_BOTTOM(n+1))/2) 

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_RIGHT_TOP(n)                        \
              ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_TOP(n+1))-1)

#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_RIGHT_BOTTOM(n)                     \
              ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_LEFT_BOTTOM(n+1))-1)

   hline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END,
                            LGREY);
                               
   textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_TEXT,
                                       SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y
                                          -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                       WHITE,
                                       "TOX:");
                                               
   if (currently_selected_object_count != 0) {   
      textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_VALUE_TEXT,
                                          SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y
                                             -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "%d",
                                          lower_interface_currently_selected_texture_adjustment_tox_value);
                                           
      draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_TOP(lower_interface_currently_selected_texture_adjustment_tox_value)
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_X_DIFFERENCE,
                                                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_Y_DIFFERENCE);
   }

   hline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END,
                            LGREY);               
                               
   textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_TEXT,
                                       SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y
                                          -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                       WHITE,
                                       "TOY:");                
                                               
   if (currently_selected_object_count != 0) { 
      textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_VALUE_TEXT,
                                          SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y
                                             -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "%d",
                                          lower_interface_currently_selected_texture_adjustment_toy_value);
                                          
      draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_TOP(lower_interface_currently_selected_texture_adjustment_toy_value)
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_X_DIFFERENCE,
                                                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_Y_DIFFERENCE);
   }                                                       

   hline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END,
                            LGREY);               
                               
   textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_TEXT,
                                       SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y
                                          -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                       WHITE,
                                       "TSX:");            
                                              
   if (currently_selected_object_count != 0) { 
      textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_VALUE_TEXT,
                                          SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y
                                             -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "%d",
                                          lower_interface_currently_selected_texture_adjustment_tsx_value); 
                                            
      draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_BOTTOM(lower_interface_currently_selected_texture_adjustment_tsx_value)
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_X_DIFFERENCE,
                                                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_Y_DIFFERENCE);
   }
                                                             
   hline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y,
                            SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END,
                            LGREY);               
                               
   textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_TEXT,
                                       SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y
                                          -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                       WHITE,
                                       "TSY:");            
                                                
   if (currently_selected_object_count != 0) {
      textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_VALUE_TEXT,
                                          SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y
                                             -  SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "%d",
                                          lower_interface_currently_selected_texture_adjustment_tsy_value); 
                                            
      draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_ZONE_X_CENTRE_BOTTOM(lower_interface_currently_selected_texture_adjustment_tsy_value)
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_X_DIFFERENCE,
                                                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y
                                                             - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_SLIDER_Y_DIFFERENCE);
   }                              
 

      
#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_START_X_POSITION   10     
#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_START_Y_POSITION  490  

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_SIZE       143     
#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_SIZE        17   

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT        4
#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT        3 

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(n) + 5)
    
   for (int xline = 0; xline <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT; xline++) {
      hline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(0),
                               SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(xline),     
                               SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT),
                               LGREY); 
   }
   
   for (int yline = 0; yline <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT; yline++) {
      vline(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(yline),     
                               SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                               SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT),
                               LGREY); 
   }                                
      
   if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_DEFAULT) {
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Select Textures");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Select Spec. Objs.");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "New Cur Texture");  
                                                                         
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "New Solid Texture");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(2),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Grab Texture");  
                                                                         
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(3),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Apply Texture");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(2),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Push Backwards"); 
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(3),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Pull Forwards");     
                                                                         
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Flip Selected");  
                                                                         
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Scale Texture");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(2),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Dupe Selected"); 
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(3),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Delete Selected");                                                                                
                                               
      switch (editor_currently_selected_tool) {
         case (EDITOR_TOOL_SELECT_TEXTURE) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(0)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_SELECT_SPEC_OBJ) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(1)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_NEW_TEXTURE_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(0)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(1)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_NEW_SOLID_TEXTURE_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(1)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(1)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_GRAB_TEXTURE) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(2),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(2)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;      
         case (EDITOR_TOOL_APPLY_TEXTURE) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(3),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(3)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;                      
         case (EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(2),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(1)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(2)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_PLACE_DUPE_TEXTURE_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(2),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(2),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(2)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(2)+1,
                                    RED);  
         } break;    
      }
       
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X1 297
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X2 437
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_CAPTION_X (1 + ((SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X1 + SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y1 550
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y2 566
#define SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_CAPTION_Y (SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y1 + 5)
   
      rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X1,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y1,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X2,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y2,
                              (currently_selected_object_count == 0)
                                 ? DGREY
                                 : ((lower_interface_currently_selected_subtractive_value == 1)
                                     ? WHITE
                                     : DGREY));
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_CAPTION_X,
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_CAPTION_Y,
                                                 (currently_selected_object_count == 0)
                                                    ? DGREY
                                                    : ((lower_interface_currently_selected_subtractive_value == 1)
                                                        ? WHITE
                                                        : LGREY),
                                                 "Subtractive");  
       
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X1 438
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X2 578
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_CAPTION_X (1 + ((SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X1 + SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y1 550
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y2 566
#define SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_CAPTION_Y (SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y1 + 5)
      
      rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X1,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y1,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X2,
                              SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y2,
                              (currently_selected_object_count == 0)
                                 ? DGREY
                                 : (lower_interface_currently_selected_masking_behaviour_value != LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING)
                                     ? WHITE
                                     : DGREY);
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_CAPTION_X,
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_CAPTION_Y,
                                                 ((currently_selected_object_count == 0)
                                                     ? DGREY
                                                     : (lower_interface_currently_selected_masking_behaviour_value == LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING)
                                                         ? LGREY
                                                         : (lower_interface_currently_selected_masking_behaviour_value == LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_MASKING_ENABLED)
                                                            ? WHITE    
                                                            : RED),
                                                 "Mask Behaviour");  
   } else
   if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_AREA_SETUP) {
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Select Area");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "New Steel Area");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Steel: %2d/%2d",
                                                 level_working_on->runtime_stats.no_steel_areas,
                                                 MAX_NO_STEEL_AREAS);  
                                                                         
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "New Oneway Area");      
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Oneway: %2d/%2d",
                                                 level_working_on->runtime_stats.no_one_way_areas,
                                                 MAX_NO_ONE_WAY_AREAS);  
                                                  
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(3),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Reposition Focus");  
                                                                          
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(2),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Scale Area");  
                                                 
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(2),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Flip Oneway");  
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_CENTRE(3),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Delete Selected");                                                                                
                                               
      switch (editor_currently_selected_tool) {
         case (EDITOR_TOOL_SELECT_AREA) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(0)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;                             
         case (EDITOR_TOOL_CAMERA_FOCUS_POSITION_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(3),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(3)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(0)+1,
                                    RED);  
         } break;                               
         case (EDITOR_TOOL_NEW_STEEL_AREA_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(0),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(0)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(1)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_NEW_ONE_WAY_AREA_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(1)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(1)+1,
                                    RED);  
         } break;    
         case (EDITOR_TOOL_SCALE_AREA_ON_CLICK) : {
            rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(2),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(1),
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(2)+1,
                                    SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(1)+1,
                                    RED);  
         } break;   
      }
   }        
      
#define SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_OBJECTS_COUNT_CAPTION_X 295
#define SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_OBJECTS_COUNT_CAPTION_Y 576
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_OBJECTS_COUNT_CAPTION_X,
                                          SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_OBJECTS_COUNT_CAPTION_Y,
                                          WHITE,
                                          "Objects: %d",
                                          level_working_on->no_terrain_objects); 
                                                                      
#define SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_X 295
#define SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_Y 586
      if (level_working_on->lemmings_level_file_size < 1024) {
         textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_X,
                                             SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_Y,
                                             WHITE,
                                             "Filesize: %6.3f bytes",
                                             (((float)(level_working_on->lemmings_level_file_size))/(1.0f))); 
      } else                                    
      if (level_working_on->lemmings_level_file_size < 1024*1024) {
         textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_X,
                                             SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_Y,
                                             WHITE,
                                             "Filesize: %6.3fkbytes",
                                             (((float)(level_working_on->lemmings_level_file_size))/(1024.0f))); 
      } else                                   
      if (level_working_on->lemmings_level_file_size < 1024*1024*1024) {
         textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_X,
                                             SCREEN_LAYOUT_LOWER_INTERFACE_CURRENT_FILESIZE_CAPTION_Y,
                                             WHITE,
                                             "Filesize: %6.3fmbytes",
                                             (((float)(level_working_on->lemmings_level_file_size))/(1024.0f*1024.0f))); 
      }                                    
                                                  
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X1 483
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X2 583
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_CAPTION_X (1 + ((SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X1 + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y1 577
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y2 593
#define SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_CAPTION_Y (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y1 + 5)
   
   rect(bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X1,
                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y1,
                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X2,
                           SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y2,
                           WHITE);
                              
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_CAPTION_X,
                                              SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_CAPTION_Y,
                                              WHITE,
                                              "Tex / Area");  

#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION                (0)
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION (WINDOW_Y_SIZE-48)
   draw_bitmap(bitmap_lower_left_lemming_tool_interface, bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION,
                                                                            SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION);
                                                                            
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_END (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION + (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH * 9) - 1)
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_END (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION + 48 - 1)
                                                                          
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH     32
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X    9                                                                     
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y    2

#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_NUMBER_SPACING 8
                                                                   
   // Vairables for parsing the integer variables into their component parts.
   u32 tens, units;

   ParseIntIntoComponents(level_working_on->runtime_stats.release_rate, &tens, &units);
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_RELEASE_RATE_LEFT_X  (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION + (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH * 0))
   // Draw the numbers for the release rate now.
   DrawLowerLeftToolInterfaceNumber( tens, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_RELEASE_RATE_LEFT_X + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X,
                                           SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION   + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y);
   DrawLowerLeftToolInterfaceNumber(units, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_RELEASE_RATE_LEFT_X + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_NUMBER_SPACING,
                                           SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION   + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y);

#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_RELEASE_RATE_RIGHT_X (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION + (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH * 1))

   // Draw the numbers for all of the tools.
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_FAR_LEFT_X      (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION + (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH * 1))
#define SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_FAR_RIGHT_X_O   (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION + (SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH * 9))

   for (int tool = 0; tool < 8; tool++) {
      if (level_working_on->runtime_stats.tool_complement[tool] == 0) {
      } else   
      if (level_working_on->runtime_stats.tool_complement[tool] == 100) {
         draw_bitmap(bitmap_lower_left_lemming_tool_number_infinity, bitmap_backbuffer, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_FAR_LEFT_X   + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X + (tool * SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH),
                                                                                        SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y);
      } else {
         ParseIntIntoComponents(level_working_on->runtime_stats.tool_complement[tool], &tens, &units);
         // Draw the numbers for the release rate now.
         DrawLowerLeftToolInterfaceNumber( tens, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_FAR_LEFT_X   + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X + (tool * SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH),
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION        + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y);
         DrawLowerLeftToolInterfaceNumber(units, SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_FAR_LEFT_X   + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_X + (tool * SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH) + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_TOOL_NUMBER_SPACING,
                                                 SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION        + SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INSET_Y);
      }
   }
}

void BlitUpperInterfaceToBackbuffer() {                           
   const int COLOUR_UPPER_INTERFACE_BACKDROP = 50 << 16 | 50 << 8 | 50;
                                                                         
#define SCREEN_LAYOUT_UPPER_INTERFACE_START_X_POSITION   0
#define SCREEN_LAYOUT_UPPER_INTERFACE_START_Y_POSITION   0
#define SCREEN_LAYOUT_UPPER_INTERFACE_END_X_POSITION   593
#define SCREEN_LAYOUT_UPPER_INTERFACE_END_Y_POSITION    69

   rectfill(bitmap_backbuffer, SCREEN_LAYOUT_UPPER_INTERFACE_START_X_POSITION,
                               SCREEN_LAYOUT_UPPER_INTERFACE_START_Y_POSITION, 
                               SCREEN_LAYOUT_UPPER_INTERFACE_END_X_POSITION,
                               SCREEN_LAYOUT_UPPER_INTERFACE_END_Y_POSITION,
                               COLOUR_UPPER_INTERFACE_BACKDROP);

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_START_X_POSITION  4     
#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_START_Y_POSITION  5

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_SIZE     117     
#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_SIZE      15   

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT      5
#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT      4   

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_SIZE) - 1)      

#define SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(n) + 5)
    
   for (int xline = 0; xline <= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT; xline++) {
      hline(bitmap_backbuffer, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(0),
                               SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(xline),     
                               SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT),
                               WHITE); 
   }
   
   for (int yline = 0; yline <= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT; yline++) {
      vline(bitmap_backbuffer, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(yline),     
                               SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(0),
                               SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT),
                               WHITE); 
   }
        
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(0),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(0),
                                              WHITE,
                                              "New Level");   
                                                              
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(0),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(1),
                                              WHITE,
                                              "Load Level");   
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(0),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(2),
                                              WHITE,
                                              "Save Level");    
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(1),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(0),
                                              WHITE,
                                              "Level Info");    
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(2),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(0),
                                              WHITE,
                                              "Save Image");    
                                                                                                            
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(2),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT - 1),
                                              WHITE,
                                              "Scroll: %3d%%", (int)(100 * (((float)(camera_x_inset)) / ((float)((LEVEL_X_SIZE) - (DSX_BACKBUFFER_X_SIZE))))));   
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-2),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(0),
                                              WHITE,
                                              "Import 16 Pal");    
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-2),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(1),
                                              WHITE,
                                              "Import 256 Pal");
                                              
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-1),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(0),
                                              WHITE,
                                              "Import 16 Img");    
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-1),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(1),
                                              WHITE,
                                              "Import 256 Img");    
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-1),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT-2),
                                              WHITE,
                                              "About");  
                                                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_CENTRE(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT-1),
                                              SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_TEXT_Y_POSITION(SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT-1),
                                              WHITE,
                                              "Quit Editor");    
}  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X 3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y 6

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE 64
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE 64

BITMAP *bitmap_right_interface_texture_preview[SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X][SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y];

void SetUpRightInterfaceTexturePreviewCells() {
   for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y; ycell++) {
      for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X; xcell++) {
         bitmap_right_interface_texture_preview[xcell][ycell] = create_bitmap(SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE);
      }
   }        
}

// This memory is used for drawing the texture previews into.
u8 fake_level_data_memory_for_texture_preview[LEVEL_X_SIZE][LEVEL_Y_SIZE];

u8  right_interface_currently_held_pal_map[256]; // This holds the palette mappings for the 'current texture'  
u32 right_interface_currently_selected_texture_swap_axes_value = 0;

void DrawTexturePreviewInto(BITMAP *destination_cell, int object_type, int object_id, u8 *pal_map_to_use = NULL) { 
   // This will hold the terrain object (placed terrain) data used to render the object.          
   LEMMINGS_LEVEL_TERRAIN_OBJECT_256 temporary_level_object;  
   // This will holds the size and location of the image data to use.
   LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE image_source;  
   
   if (!(object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) {
      if (object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
         LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *texture_archive_texture = GetTextureArchiveTexture16(loaded_active_texture_archive, object_id);
         
         image_source.xs   = texture_archive_texture->xs;
         image_source.ys   = texture_archive_texture->ys;
         
         image_source.data = texture_archive_texture->data;
      } else {                                                         
         LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *texture_archive_texture = GetTextureArchiveTexture256(loaded_active_texture_archive, object_id);
         
         image_source.xs   = texture_archive_texture->xs;
         image_source.ys   = texture_archive_texture->ys;
         
         image_source.data = texture_archive_texture->data;
      }
   } else {                         
      object_id &= ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
          
      if (object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
         LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *original_appended_texture_16 = GetAppendedTexture16(object_id);
      
         image_source.xs   = original_appended_texture_16->xs;
         image_source.ys   = original_appended_texture_16->ys;            
         image_source.data = original_appended_texture_16->data;        
      } else {                                                                 
         LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *original_appended_texture_256 = GetAppendedTexture256(object_id);
      
         image_source.xs   = original_appended_texture_256->xs;
         image_source.ys   = original_appended_texture_256->ys;            
         image_source.data = original_appended_texture_256->data; 
      }
   }
                                               
   // Formulate a terrain placement that will draw the terrain in the small terain preview box.                                                
   temporary_level_object.object_header.object_type  = object_type;
   temporary_level_object.object_header.object_flags = right_interface_currently_selected_texture_swap_axes_value;
   
   temporary_level_object.tox = (image_source.xs/2 - 32) << 8;
   temporary_level_object.toy = (image_source.ys/2 - 32) << 8;
   
   temporary_level_object.tsx = 1 << 8;
   temporary_level_object.tsy = 1 << 8;
   
   temporary_level_object.x1 = 0;
   temporary_level_object.y1 = 0;
   temporary_level_object.x2 = 63;
   temporary_level_object.y2 = 63;     
   
   if (pal_map_to_use != NULL) {
      for (int p = 0; p < 256; p++) {
         temporary_level_object.pal_map[p] = pal_map_to_use[p]; 
      }  
   } else {
      for (int p = 0; p < 256; p++) {
         temporary_level_object.pal_map[p] = p; 
      }  
   }
   
   for (int texture_pixel_y = 0; texture_pixel_y < 64; texture_pixel_y++) {
      for (int texture_pixel_x = 0; texture_pixel_x < 64; texture_pixel_x++) {
         fake_level_data_memory_for_texture_preview[texture_pixel_x][texture_pixel_y] = 0; 
      }
   } 
   
   if (object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT) {
      if (object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
         RenderLevelObject16ToLevel( fake_level_data_memory_for_texture_preview, (const LEMMINGS_LEVEL_TERRAIN_OBJECT_16*) &temporary_level_object, &image_source);
      } else {
         RenderLevelObject256ToLevel(fake_level_data_memory_for_texture_preview, (const LEMMINGS_LEVEL_TERRAIN_OBJECT_256*)&temporary_level_object, &image_source);
      }
   } else {
      if (object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
         RenderLevelObject16ToLevel( fake_level_data_memory_for_texture_preview, (const LEMMINGS_LEVEL_TERRAIN_OBJECT_16*) &temporary_level_object, &image_source);
      } else {
         RenderLevelObject256ToLevel(fake_level_data_memory_for_texture_preview, (const LEMMINGS_LEVEL_TERRAIN_OBJECT_256*)&temporary_level_object, &image_source);
      }
   }                                                                                          
   
   u16 *level_palette_to_use = level_working_on->runtime_stats.level_palette;

   int xp, yp;

   int colour;

   acquire_bitmap(destination_cell);

   for (xp = 0; xp < 64; xp++) {
      for (yp = 0; yp < 64; yp++) {
         colour = level_palette_to_use[fake_level_data_memory_for_texture_preview[xp][yp]];
         colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));
                             
         _putpixel24(destination_cell, xp, yp, colour);
      }
   }    

   release_bitmap(destination_cell);
}

void DrawTexturePreviewIntoWZoomHack(BITMAP *destination_cell, int object_type, int object_id, u8 *pal_map_to_use = NULL, int zoom = 1) {
   BITMAP *temporary_bitmap = create_bitmap(64, 64);
   
   DrawTexturePreviewInto(temporary_bitmap, object_type, object_id, pal_map_to_use);  
   
   stretch_blit(temporary_bitmap, destination_cell, 32 - 32/zoom, 32 - 32/zoom, 64/zoom, 64/zoom, 0, 0, 64, 64);
   
   destroy_bitmap(temporary_bitmap);
} 

u16 fake_DSX_backbuffer_for_special_object_preview[DSX_BACKBUFFER_X_SIZE][DSX_BACKBUFFER_Y_SIZE];

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE 100
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE 100

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_X_SIZE 100
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_Y_SIZE 100
                                                            
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT          0
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE      1
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP          2
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD        3
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE 4
#define RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_WATER         5
void DrawSpecialObjectPreviewInto(BITMAP *destination_cell, int object_type, int genus) {         
   int xp, yp;
   
   for (xp = 0; xp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/2; xp++) {
      for (yp = 0; yp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/2; yp++) {
         fake_DSX_backbuffer_for_special_object_preview[xp][yp] = 0; 
      }
   }                     
   
   switch (object_type) {
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) : {
         // Get the raw number for the entrance genus junction 
         int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the entrance based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                     :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);    
           
         // Return the correct graphical object sprite for the entrance based on the genus junction value.   
         DSX_SPRITE *active_entrance_graphical_object_sprite = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_entrance_graphical_object_sprites.at(entrance_genus_junction_id)
                                                                                     :   custom_entrance_graphical_object_sprites.at(entrance_genus_junction_id);    
           
         DSX_DrawEntrance(active_entrance_graphical_object,
                          active_entrance_graphical_object_sprite,
                          active_entrance_graphical_object->representing_frame,
                          false,
                          SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/4,
                          SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/4,
                          false,
                          fake_DSX_backbuffer_for_special_object_preview);
      } break;    
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) : {
         // Get the raw number for the exit genus junction 
         int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the exit based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                     :   custom_exit_graphical_objects.at(exit_genus_junction_id);    
           
         // Return the correct graphical object sprite for the exit based on the genus junction value.   
         DSX_SPRITE *active_exit_graphical_object_sprite = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_exit_graphical_object_sprites.at(exit_genus_junction_id)
                                                                                     :   custom_exit_graphical_object_sprites.at(exit_genus_junction_id);    
           
         DSX_DrawExit(active_exit_graphical_object,
                      active_exit_graphical_object_sprite,
                      active_exit_graphical_object->representing_frame,      
                      true,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/4,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/4,
                      false,
                      fake_DSX_backbuffer_for_special_object_preview);
      } break;  
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) : {
         // Get the raw number for the trap genus junction 
         int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[genus]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the trap based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_objects.at(trap_genus_junction_id);    
           
         // Return the correct graphical object sprite for the trap based on the genus junction value.   
         DSX_SPRITE *active_trap_graphical_object_sprite = (!(level_working_on->runtime_stats.trap_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_object_sprites.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_object_sprites.at(trap_genus_junction_id);    
                                 
         DSX_DrawTrap(active_trap_graphical_object,
                      active_trap_graphical_object_sprite,
                      active_trap_graphical_object->representing_frame, 
                      true,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/4,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/4,
                      level_working_on->runtime_stats.trap_genus_palettes[genus],
                      false,
                      fake_DSX_backbuffer_for_special_object_preview);
      } break;  
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) : {
         // Get the raw number for the hazard genus junction 
         int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[genus]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the hazard based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);    
           
         // Return the correct graphical object sprite for the hazard based on the genus junction value.   
         DSX_SPRITE *active_hazard_graphical_object_sprite = (!(level_working_on->runtime_stats.hazard_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_object_sprites.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_object_sprites.at(hazard_genus_junction_id);    
           
         DSX_DrawHazard(active_hazard_graphical_object,
                      active_hazard_graphical_object_sprite,
                      active_hazard_graphical_object->representing_frame,
                      true,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/4,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/4,
                      level_working_on->runtime_stats.hazard_genus_palettes[genus],
                      false,
                      fake_DSX_backbuffer_for_special_object_preview);
      } break;  
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) : {
         // Get the raw number for the uninteractive genus junction 
         int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[genus]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the uninteractive based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);    
           
         // Return the correct graphical object sprite for the uninteractive based on the genus junction value.   
         DSX_SPRITE *active_uninteractive_graphical_object_sprite = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[genus]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id);    
           
         DSX_DrawUninteractive(active_uninteractive_graphical_object,
                      active_uninteractive_graphical_object_sprite,
                      active_uninteractive_graphical_object->representing_frame,  
                      true,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/4,
                      SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/4,
                      level_working_on->runtime_stats.uninteractive_genus_palettes[genus],
                      false,
                      fake_DSX_backbuffer_for_special_object_preview);
      } break;      
   }           
                                
   int colour;
   
   acquire_bitmap(destination_cell);

   for (xp = 0; xp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/2; xp++) {
      for (yp = 0; yp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/2; yp++) {
         colour = fake_DSX_backbuffer_for_special_object_preview[xp][yp];
         colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));
                             
         _putpixel24(destination_cell, xp*2+0, yp*2+0, colour);
         _putpixel24(destination_cell, xp*2+1, yp*2+0, colour);
         _putpixel24(destination_cell, xp*2+0, yp*2+1, colour);
         _putpixel24(destination_cell, xp*2+1, yp*2+1, colour);
      }
   }                            

   release_bitmap(destination_cell);
}

void DrawWaterObjectPreviewInto(BITMAP *destination_cell) {
   int xp, yp;        
                    
   for (xp = 0; xp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/2; xp++) {
      for (yp = 0; yp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/2; yp++) {
         fake_DSX_backbuffer_for_special_object_preview[xp][yp] = 0; 
      }
   }                                                       
   
   // Get the raw number for the water genus junction 
   int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                      & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
    
   // Return the correct graphical object for the water based on the genus junction value.   
   LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_water_graphical_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                             & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                               ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                               :   custom_water_graphical_objects.at(water_genus_junction_id);

   // Return the correct graphical object sprite for the water based on the genus junction value.   
   DSX_SPRITE *active_water_graphical_object_sprite = (!(level_working_on->runtime_stats.water_genus_junction
                                                                             & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                               ? standard_water_graphical_object_sprites.at(water_genus_junction_id)
                                                                               :   custom_water_graphical_object_sprites.at(water_genus_junction_id);

   int water_y_further = ((active_water_graphical_object->graphic_height - active_water_graphical_object->handle_y) - 1);                                                                               
      
   DSX_DrawWaterArea(active_water_graphical_object,
                     active_water_graphical_object_sprite,
                     active_water_graphical_object->representing_frame,
                     0,
                     (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/2)-1,
                     (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/2) - water_y_further - 1,
                     false,
                     fake_DSX_backbuffer_for_special_object_preview);
                                
   int colour;
   
   acquire_bitmap(destination_cell);

   for (xp = 0; xp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE/2; xp++) {
      for (yp = 0; yp < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE/2; yp++) {
         colour = fake_DSX_backbuffer_for_special_object_preview[xp][yp];
         colour = makecol24((int)((((colour & DSX_COLOUR_RED_PART)   >>  0) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_GREEN_PART) >>  5) * 255.0f)/31.0f),
                            (int)((((colour & DSX_COLOUR_BLUE_PART)  >> 10) * 255.0f)/31.0f));
                             
         _putpixel24(destination_cell, xp*2+0, yp*2+0, colour);
         _putpixel24(destination_cell, xp*2+1, yp*2+0, colour);
         _putpixel24(destination_cell, xp*2+0, yp*2+1, colour);
         _putpixel24(destination_cell, xp*2+1, yp*2+1, colour);
      }
   }                            

   release_bitmap(destination_cell);                                 
}

void Split15BitColour(int colour, unsigned int *r, unsigned int *g, unsigned int *b) {
   *r = (colour & DSX_COLOUR_RED_PART)   >>  0;
   *g = (colour & DSX_COLOUR_GREEN_PART) >>  5;
   *b = (colour & DSX_COLOUR_BLUE_PART)  >> 10;
}

void Create15BitColour(int *out, int r, int g, int b) {
   *out = RGB15A((int)((((float)(r)) / 255.0f) * 31.0f),
                 (int)((((float)(g)) / 255.0f) * 31.0f),
                 (int)((((float)(b)) / 255.0f) * 31.0f));  
}

unsigned int right_interface_texture_selection_database_size = 0;

void RecalculateRightInterfaceTextureSelectionDatabaseSize();

#define RIGHT_INTERFACE_TAB_TEXTURE_SELECTION 0
#define RIGHT_INTERFACE_TAB_PALETTE_MAPPER    1
#define RIGHT_INTERFACE_TAB_PALETTE_EDITOR    2
#define RIGHT_INTERFACE_TAB_SPECIAL_OBJECTS   3

unsigned int right_interface_current_tab = RIGHT_INTERFACE_TAB_TEXTURE_SELECTION;   

const char *text_right_interface_tab_selector_caption_rectangle_caption[] = {
                              "Texture Selection",
                              "Palette Mapper",
                              "Palette Editor",
                              "Special Objects",};
                                                             
#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_16    0
#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_256   1  
#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_16      2
#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_256     3    

#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_256_COLOUR_BIT 1
#define RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT     2
int right_interface_texture_selection_current_set  = RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_16; 
                                                                     
const char *text_right_interface_texture_selection_title_rectangle_caption[] = {
                             "Standard 16",
                             "Standard 256",
                             "Custom 16",
                             "Custom 256",};                
                                
unsigned int right_interface_texture_selection_current_page = 0;                                        

unsigned int right_interface_currently_selected_texture      = 0;
unsigned int right_interface_currently_selected_texture_type = 0;

unsigned int right_interface_currently_selected_texture_window_zoom = 1;

unsigned int right_interface_currently_selected_palette_mapper_palette_entry = 0;

unsigned int right_interface_currently_selected_palette_editor_left_colour  = 0;
unsigned int right_interface_currently_selected_palette_editor_right_colour = 1;

unsigned int right_interface_currently_selected_palette_editor_colour_r_value = 0;
unsigned int right_interface_currently_selected_palette_editor_colour_g_value = 0;
unsigned int right_interface_currently_selected_palette_editor_colour_b_value = 0;

unsigned int right_interface_special_objects_current_object = RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE; 

int right_interface_special_objects_current_genus = 0;  
              
const char *right_interface_special_objects_object_category_strings[] = {"Exit",
                                                                         "Entrance",
                                                                         "Trap",
                                                                         "Hazard",
                                                                         "Uninteractive",
                                                                         "Water"};

// This comes in REAL handy when managing the tab control logic.                                                                         
unsigned int no_genuses_for_special_object[5] = {1, 1, NO_TRAP_GENUSES, NO_HAZARD_GENUSES, NO_UNINTERACTIVE_GENUSES}; 

u16 *right_interface_current_special_object_active_palette;      
                                   
void ResetSpecialObjectPalettePointerBasedOnGenus() {   
   switch (right_interface_special_objects_current_object) {
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) : {
         right_interface_current_special_object_active_palette = level_working_on->runtime_stats.entrance_palette;
      } break;    
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) : {
         right_interface_current_special_object_active_palette = level_working_on->runtime_stats.exit_palette;
      } break;    
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) : {
         right_interface_current_special_object_active_palette = level_working_on->runtime_stats.trap_genus_palettes[right_interface_special_objects_current_genus];
      } break;    
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) : {
         right_interface_current_special_object_active_palette = level_working_on->runtime_stats.hazard_genus_palettes[right_interface_special_objects_current_genus];
      } break;    
      case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) : {
         right_interface_current_special_object_active_palette = level_working_on->runtime_stats.uninteractive_genus_palettes[right_interface_special_objects_current_genus];
      } break;    
   }        
}                                                                                                                                                        
   
                             
unsigned int right_interface_currently_selected_special_objects_left_colour  = 0; 
unsigned int right_interface_currently_selected_special_objects_right_colour = 1;                             
                             
unsigned int right_interface_currently_selected_special_objects_left_colour_r_value = 0;
unsigned int right_interface_currently_selected_special_objects_left_colour_g_value = 0;
unsigned int right_interface_currently_selected_special_objects_left_colour_b_value = 0;   

#define COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(n) (((n) < 16) ? (right_interface_current_special_object_active_palette[n]) \
                                                           : (level_working_on->runtime_stats.water_palette[(n) & 15])) 

void BlitRightInterfaceToBackbuffer() {
   acquire_bitmap(bitmap_backbuffer);  
   if (right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION) {          
      const int COLOUR_RIGHT_INTERFACE_TEXTURE_SELECTION_BACKDROP = 60 << 16 | 20 << 8 | 20;

#define SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION 594
#define SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION   0
#define SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION   799
#define SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION   599
      
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION, 
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION,
                                  COLOUR_RIGHT_INTERFACE_TEXTURE_SELECTION_BACKDROP);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1 650
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2 750
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X_TEXT (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1  88
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2 113
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1 + 4)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y_TEXT_1 + 10)

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2-1,
                                  BLACK);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1    - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_RIGHT - 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_Y_LEFT  ((SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2)/2)
      const int right_interface_texture_selection_left_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_RIGHT,
                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1,
                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_RIGHT,
                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2+1,
                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_LEFT,
                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_Y_LEFT+1,};
                                   
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2    + 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_LEFT + 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_Y_RIGHT ((SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2)/2)
      const int right_interface_texture_selection_right_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_LEFT,
                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1,
                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_LEFT,
                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2+1,
                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_RIGHT,
                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_Y_RIGHT+1,};

      polygon(bitmap_backbuffer, 3, right_interface_texture_selection_left_triangle_points,  WHITE);  
      polygon(bitmap_backbuffer, 3, right_interface_texture_selection_right_triangle_points, WHITE);                       
                             
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 text_right_interface_texture_selection_title_rectangle_caption[right_interface_texture_selection_current_set]);    

      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y_TEXT_2,
                                                 WHITE,
                                                 "Page %d",
                                                 right_interface_texture_selection_current_page + 1);
      
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_START_X_POSITION    599     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_START_Y_POSITION    128  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE+1)     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE+1)   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(n) + 5)   

      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_COUNT),
                                  WHITE); 
      }
   
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }      
         
      // ----------------------------------------------
      
      int this_page_cell, logical_real_cell;  
                                                             
      for (int cell = 0; cell < (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                               * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y); cell++) {
         logical_real_cell = cell + right_interface_texture_selection_current_page * (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                    * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y);          
         if (logical_real_cell < right_interface_texture_selection_database_size) {
            DrawTexturePreviewInto(bitmap_right_interface_texture_preview[cell % SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X][cell / SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X],
                                     ((right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_256_COLOUR_BIT)
                                       ? LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256
                                       : LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16),
                                     logical_real_cell | ((right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT)
                                                         ? LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT
                                                         : 0),
                                     MouseIsDown(MOUSE_RMB) ? NULL : right_interface_currently_held_pal_map);     
         } else {
            clear_bitmap(bitmap_right_interface_texture_preview[cell % SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X][cell / SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X]);
         }
      }                                                                
                                                                        
      // ----------------------------------------------
      
      int xcell, ycell;
      
      char custom_caption[9];
                                                        
      const char *texture_cell_caption;

      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X; xcell++) {
            this_page_cell    = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X);
            logical_real_cell = this_page_cell + right_interface_texture_selection_current_page * (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                                 * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y);   
            
            draw_bitmap(bitmap_right_interface_texture_preview[xcell][ycell], bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(xcell)+1,
                                                                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(ycell)+1);
                                              
            if (logical_real_cell < right_interface_texture_selection_database_size) {
               if (!(right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT)) {
                  if (right_interface_texture_selection_current_set == RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_16) {
                     texture_cell_caption = GetTextureArchiveTexture16(loaded_active_texture_archive, logical_real_cell)->name;
                  } else {                                                                         
                     texture_cell_caption = GetTextureArchiveTexture256(loaded_active_texture_archive, logical_real_cell)->name;
                  }
               } else {
                  sprintf(custom_caption, "cust %3d", logical_real_cell);
              
                  texture_cell_caption = custom_caption;
               }                                                                 
                                                                                            
               textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_CENTRE(xcell)+1,
                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_BOTTOM(ycell)-10,
                                                          WHITE, 
                                                          texture_cell_caption);
            }
         }                                                                                           
      }         
   } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER) {
      const int COLOUR_RIGHT_INTERFACE_PALETTE_MAPPER_BACKDROP = 10 << 16 | 55 << 8 | 10;
      
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION, 
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION,
                                  COLOUR_RIGHT_INTERFACE_PALETTE_MAPPER_BACKDROP);
                                           
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X 16
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y 16

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_X_SIZE 10
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_Y_SIZE 10
                                         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION    609     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION    108  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_X_SIZE+1)     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_Y_SIZE+1)   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_SIZE) - 1) 
                                          
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION - 2,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION - 12,
                                          WHITE,
                                          "Mapping:");
                                          
      if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {     
         for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT; yline++) {
            vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(yline),     
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(0),
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_COUNT),
                                     WHITE); 
         } 
                
         for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_COUNT; xline++) {
            hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(0),
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(xline),     
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT),
                                     WHITE); 
         }
      } else {     
         for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT; yline++) {
            vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(yline),     
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(0),
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(1),
                                     WHITE); 
            vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(yline),     
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(1),
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_COUNT),
                                     BLUE); 
         }

         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(0),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT),
                                  WHITE);
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(1),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT),
                                  WHITE);      
                                             
         for (int xline = 2; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_Y_COUNT; xline++) {
            hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(0),
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(xline),     
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_CELL_X_COUNT),
                                     BLUE); 
         }
      }
      
      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; xcell++) {
            rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(xcell)+1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(ycell) +1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_RIGHT(xcell),
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_BOTTOM(ycell),
                                           level_working_on->runtime_stats.level_palette[right_interface_currently_held_pal_map[xcell + ycell*SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X]]);
         }
      }                                                                           
                        
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1 674
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2 785
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_CAPTION_X (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y1 290
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y2 305
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_CAPTION_Y (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y1 + 4)

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y2,
                              WHITE);
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_CAPTION_X,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_CAPTION_Y,
                                                 WHITE,
                                                 "Reset Mapping");
                                         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_X_POSITION    609     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_Y_POSITION    340  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_X_SIZE+1)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_Y_SIZE+1)   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_SIZE) - 1)       
                                           
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_X_POSITION - 2,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_Y_POSITION - 12,
                                          WHITE,
                                          "Palette:");  
                                          
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_COUNT),
                                  WHITE); 
      }   
      
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_CELL_X_COUNT),
                                  WHITE);             
      }
      
      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; xcell++) {
            rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(xcell)+1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(ycell) +1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_RIGHT(xcell),
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_BOTTOM(ycell),
                                           level_working_on->runtime_stats.level_palette[xcell + ycell*SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X]);
         }
      }                                   
      
      const int COLOUR_RIGHT_INTERFACE_PALETTE_MAPPER_LAST_APPLIED_HIGHLIGHT_COLOUR = RED;//255 << 16 | 0 << 8 | 0;
                  
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(right_interface_currently_selected_palette_mapper_palette_entry & 15),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(right_interface_currently_selected_palette_mapper_palette_entry / 16),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_RIGHT(right_interface_currently_selected_palette_mapper_palette_entry & 15)+1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_BOTTOM(right_interface_currently_selected_palette_mapper_palette_entry / 16)+1,
                              COLOUR_RIGHT_INTERFACE_PALETTE_MAPPER_LAST_APPLIED_HIGHLIGHT_COLOUR);  
            
   } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR) {
      const int COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_BACKDROP = 20 << 16 | 20 << 8 | 60;
      
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION, 
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION,
                                  COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_BACKDROP);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION    609     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION    108  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_X_SIZE+1)     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELL_Y_SIZE+1)   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_SIZE) - 1)       
                                           
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION - 2,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION - 12,
                                          WHITE,
                                          "Palette:");  
                                          
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }   
      
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT),
                                  WHITE);             
      }
      
      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; xcell++) {
            rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(xcell)+1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(ycell) +1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(xcell),
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(ycell),
                                           level_working_on->runtime_stats.level_palette[xcell + ycell*SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT]);
         }
      }                                   
      
      const int COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR  = RED;//255 << 16 | 0 << 8 | 0;
                  
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(right_interface_currently_selected_palette_editor_left_colour & 15),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(right_interface_currently_selected_palette_editor_left_colour / 16),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(right_interface_currently_selected_palette_editor_left_colour & 15)+1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(right_interface_currently_selected_palette_editor_left_colour / 16)+1,
                              COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR);   
                              
      const int COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR = BLUE;//255 << 16 | 0 << 8 | 0;   
                  
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(right_interface_currently_selected_palette_editor_right_colour & 15),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(right_interface_currently_selected_palette_editor_right_colour / 16),
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(right_interface_currently_selected_palette_editor_right_colour & 15)+1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(right_interface_currently_selected_palette_editor_right_colour / 16)+1,
                              COLOUR_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR);   
                              
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X1  636
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y1  296
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X2  691
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y2  316

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_CAPTION_X     612
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_CAPTION_Y     303

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y2,
                              WHITE);
                              
      rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X1 + 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y1 + 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_X2 - 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_RECTANGLE_Y2 - 1,
                                     level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour]);
                                    
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_CAPTION_X,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_LEFT_COLOUR_CAPTION_Y,
                                          WHITE,
                                          "A:");                              
                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X1  728
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y1  296
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X2  783
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y2  316   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_CAPTION_X     704
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_CAPTION_Y     303

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y2,
                              WHITE);
                              
      rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X1 + 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y1 + 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_X2 - 1,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_RECTANGLE_Y2 - 1,
                                     level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour]);
                                    
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_CAPTION_X,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_RIGHT_COLOUR_CAPTION_Y,
                                          WHITE,
                                          "B:");  
                                                
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START           620
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END             770
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_LENGTH                 \
         ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END                 \
                 - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START) + 1)
         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y               335
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y               360
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y               385
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE             8  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_TEXT            605
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_VALUE_TEXT      794
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE 3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE 3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE   3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_NO_ZONES           32

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_LEFT(n)                         \
              (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START                         \
                 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_LENGTH * (n))/SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_NO_ZONES))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_CENTRE(n)                       \
              ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_LEFT(n)                 \
                 + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_LEFT(n+1))/2) 

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_RIGHT(n)                        \
              ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_LEFT(n+1))-1)

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END,
                               WHITE);
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "R");
                               
      if (right_interface_currently_selected_palette_editor_left_colour != 0) {                         
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_palette_editor_colour_r_value);
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_palette_editor_colour_r_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);
      }

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END,
                               WHITE);               
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "G");                
                               
      if (right_interface_currently_selected_palette_editor_left_colour != 0) {  
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_palette_editor_colour_g_value);
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_palette_editor_colour_g_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);
      }

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END,
                               WHITE);               
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "B");            
                             
      if (right_interface_currently_selected_palette_editor_left_colour != 0) {    
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_palette_editor_colour_b_value); 
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_palette_editor_colour_b_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);
      }                                                          
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_START_X_POSITION  605     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_START_Y_POSITION  406  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE        92     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE        17   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT        2
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT        5   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) + 5)
    
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT),
                                  WHITE); 
      }
   
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Copy A to B");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Copy B to A");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Light Swap");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Deep Swap");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Grad A to B");   
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(4),
                                                 WHITE,
                                                 "A Line to B");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(4),
                                                 WHITE,
                                                 "B Line to A");
                                                 
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_SIZE     150
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_SIZE 185
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_SIZE      17

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT     605
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP      520

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_CENTRE         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_RIGHT          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_RIGHT      \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_CENTRE         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_BOTTOM         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_TEXT_Y_POSITION  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP + 5)                                     
                                                 
      rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_RIGHT,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_RIGHT,
                                     SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_BOTTOM,
                                     level_working_on->runtime_stats.level_palette[level_working_on->one_way_colour]);
                                                    
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP, 
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_RIGHT,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_BOTTOM,
                              WHITE);                               
                                                 
      vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_RIGHT,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP, 
                               SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_BOTTOM,
                               WHITE);                      
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_CENTRE,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_TEXT_Y_POSITION,
                                                 WHITE,
                                                 "Use A as One Way:"); 
                                                 
   } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_SPECIAL_OBJECTS) {
      const int COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_BACKDROP = 55 << 16 | 40 << 8 | 5;
      
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION, 
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION,
                                  COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_BACKDROP);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X1 640
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X2 760
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X_TEXT (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1  12
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2  37
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1 + 5)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y_TEXT_1 + 9)

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2-1,
                                  BLACK);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X1    - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_RIGHT - 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_Y_LEFT  ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2)/2)
      const int right_interface_special_objects_left_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_RIGHT,
                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1,
                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_RIGHT,
                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2+1,
                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_LEFT,
                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_Y_LEFT+1,};
                                   
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X2    + 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_LEFT + 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_Y_RIGHT ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2)/2)
      const int right_interface_special_objects_right_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_LEFT,
                                                                            SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1,
                                                                            SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_LEFT,
                                                                            SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2+1,
                                                                            SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_RIGHT,
                                                                            SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_Y_RIGHT+1,};
      
      polygon(bitmap_backbuffer, 3, right_interface_special_objects_left_triangle_points,  WHITE);  
      polygon(bitmap_backbuffer, 3, right_interface_special_objects_right_triangle_points, WHITE);                       
                            
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 right_interface_special_objects_object_category_strings[right_interface_special_objects_current_object]);    
                                                 
      if ((right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP         )
       || (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD       )
       || (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE)) {                                            
         textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_X_TEXT,
                                                    SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y_TEXT_2,
                                                    WHITE,
                                                    "Genus %2d/%2d",
                                                    right_interface_special_objects_current_genus + 1,
                                                    no_genuses_for_special_object[right_interface_special_objects_current_object]);    
      }
                                                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_X_POSITION 601
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_Y_POSITION  45   
                                                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_TEXT_CAPTION_X_POSITION  (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_X_POSITION + (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE  / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_TEXT_CAPTION_Y_POSITION ((SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_Y_POSITION +  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE) - 12)      
     
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_X_POSITION,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_Y_POSITION,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE + 1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE + 1,
                              WHITE);        
                              
      BITMAP *bitmap_right_interface_special_object_preview = create_sub_bitmap(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_X_POSITION + 1,
                                                                                                   SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_Y_POSITION + 1,
                                                                                                   SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_X_SIZE,
                                                                                                   SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PREVIEW_CELL_Y_SIZE);

      DrawSpecialObjectPreviewInto(bitmap_right_interface_special_object_preview, right_interface_special_objects_current_object, right_interface_special_objects_current_genus);                                   
                                                                                   
      destroy_bitmap(bitmap_right_interface_special_object_preview);
      
      const char *right_interface_special_object_preview_box_caption = "";
      
      switch (right_interface_special_objects_current_object) {
         case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) : {
            // Get the raw number for the entrance genus junction 
            int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                               & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
            // Return the correct graphical object for the entrance based on the genus junction value.   
            LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                      & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                        ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                        :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);    
              
            right_interface_special_object_preview_box_caption = active_entrance_graphical_object->graphical_object_name;
         } break;    
         case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) : {
            // Get the raw number for the exit genus junction 
            int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                               & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
            // Return the correct graphical object for the exit based on the genus junction value.   
            LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                      & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                        ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                        :   custom_exit_graphical_objects.at(exit_genus_junction_id);    
              
            right_interface_special_object_preview_box_caption = active_exit_graphical_object->graphical_object_name;
         } break;  
         case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) : {
            // Get the raw number for the trap genus junction 
            int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]
                                               & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
            // Return the correct graphical object for the trap based on the genus junction value.   
            LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]
                                                                                      & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                        ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                        :   custom_trap_graphical_objects.at(trap_genus_junction_id);    
              
            right_interface_special_object_preview_box_caption = active_trap_graphical_object->graphical_object_name;
         } break;  
         case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) : {
            // Get the raw number for the hazard genus junction 
            int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]
                                               & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
            // Return the correct graphical object for the hazard based on the genus junction value.   
            LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]
                                                                                      & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                        ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                        :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);    
              
            right_interface_special_object_preview_box_caption = active_hazard_graphical_object->graphical_object_name;
         } break;  
         case (RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) : {
            // Get the raw number for the uninteractive genus junction 
            int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]
                                               & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
            // Return the correct graphical object for the uninteractive based on the genus junction value.   
            LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]
                                                                                      & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                        ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                        :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);    
              
            right_interface_special_object_preview_box_caption = active_uninteractive_graphical_object->graphical_object_name;
         } break;      
      }                 
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_TEXT_CAPTION_X_POSITION,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_SPECIAL_OBJECT_BOX_TEXT_CAPTION_Y_POSITION,
                                                 WHITE,
                                                 right_interface_special_object_preview_box_caption);      
  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X1 712
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X2 791
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1  50
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2  85
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT   (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 +  8 +  1)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + 19 +  1)
  
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2-1,
                                  BLACK);
                                        
      // These are the arrows on the outside of the junction selector box                 
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X1     + 4)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT     (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2)/2)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP      (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM   (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT + 10)
      const int right_interface_special_objects_junction_selector_left_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP,
                                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM,
                                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT,
                                                                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT,};
                                   
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X2     - 4)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT   (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT + 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT  ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2)/2)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_TOP     (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_BOTTOM  (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT + 10)
      const int right_interface_special_objects_junction_selector_right_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_TOP,
                                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_BOTTOM,
                                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT,
                                                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT,};
                                                                                             
      polygon(bitmap_backbuffer, 3, right_interface_special_objects_junction_selector_left_triangle_points,  WHITE);  
      polygon(bitmap_backbuffer, 3, right_interface_special_objects_junction_selector_right_triangle_points, WHITE);     
                              
      bool current_object_type_is_custom;    
                                
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
         current_object_type_is_custom = level_working_on->runtime_stats.entrance_genus_junction
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
         current_object_type_is_custom = level_working_on->runtime_stats.exit_genus_junction     
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
         current_object_type_is_custom = level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]    
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
         current_object_type_is_custom = level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]      
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
         current_object_type_is_custom = level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]        
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
      }  
                                    
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 (!current_object_type_is_custom)
                                                    ? "Standard"
                                                    : "Custom");
                                                                                                                 
      int no_current_type_of_object;           
      int max_available_type_no_of_object;  
                                                  
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
         no_current_type_of_object = 1 + (level_working_on->runtime_stats.entrance_genus_junction
             & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
             
         max_available_type_no_of_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                 ? no_standard_entrances
                                                 : no_custom_entrances; 
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
         no_current_type_of_object = 1 + (level_working_on->runtime_stats.exit_genus_junction     
             & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);   
             
         max_available_type_no_of_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                 ? no_standard_exits
                                                 : no_custom_exits; 
      } else                                                             
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
         no_current_type_of_object = 1 + (level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]    
             & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT); 
             
         max_available_type_no_of_object = (!(level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]    
                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                 ? no_standard_traps
                                                 : no_custom_traps; 
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
         no_current_type_of_object = 1 + (level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]      
             & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);  
             
         max_available_type_no_of_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]    
                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                 ? no_standard_hazards
                                                 : no_custom_hazards; 
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
         no_current_type_of_object = 1 + (level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]        
             & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);  
             
         max_available_type_no_of_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]    
                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                 ? no_standard_uninteractives
                                                 : no_custom_uninteractives; 
      }         
                                                      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_2,
                                                 WHITE,
                                                 "%2d of %2d",
                                                 no_current_type_of_object,
                                                 max_available_type_no_of_object);
                                                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X1 712
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X2 791
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 105
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y2 140
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 +  8 +  1)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 + 19 +  1)

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y2-1,
                                  BLACK);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X2) / 2))
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 "Placed");
                                 
      int no_of_object;           
      int max_no_of_object;                                              
                                                    
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
         no_of_object = level_working_on->runtime_stats.no_entrances;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
         no_of_object = level_working_on->runtime_stats.no_exits;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
         no_of_object = level_working_on->runtime_stats.no_traps;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
         no_of_object = level_working_on->runtime_stats.no_hazards;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
         no_of_object = level_working_on->runtime_stats.no_uninteractives;
      }         
                                                 
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
         max_no_of_object = MAX_NO_ENTRANCES;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
         max_no_of_object = MAX_NO_EXITS;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
         max_no_of_object = MAX_NO_TRAPS;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
         max_no_of_object = MAX_NO_HAZARDS;
      } else
      if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
         max_no_of_object = MAX_NO_UNINTERACTIVES;
      }                                 
              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_2,
                                                 WHITE,
                                                 "%d of %d",
                                                 no_of_object,
                                                 max_no_of_object);
      
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_START_X_POSITION  605     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_START_Y_POSITION  152  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE        92     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE        17   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT        2
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT        2 

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) + 5)
    
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT),
                                  WHITE); 
      }
   
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }                                
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Add Object");
                                                 
      if (editor_currently_selected_tool == EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK) {
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(1),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(1),
                                 RED);
      }
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Delete All"); 
                                                  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X 16
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_Y 1

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_ENTRY_CELL_X_SIZE 10
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_ENTRY_CELL_X_SIZE 10
       
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_START_X_POSITION    609     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_START_Y_POSITION    193  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_ENTRY_CELL_X_SIZE+1)     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_ENTRY_CELL_X_SIZE+1)   
        
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_SIZE) - 1) 
                                          
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(1),
                                  WHITE); 
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(1),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_Y_COUNT),
                                  BLUE); 
      } 

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(0),
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(0),     
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_COUNT),
                               WHITE);
      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(0),
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(1),     
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELL_X_COUNT),
                               WHITE);      
      
      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_Y; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X; xcell++) {
            rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(xcell)+1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(ycell) +1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(xcell),
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(ycell),
                                           right_interface_current_special_object_active_palette[xcell]);
         }
      }                                      
         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_X_POSITION 601
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_Y_POSITION 212      
                                                                         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_WATER_OBJECT_BOX_TEXT_CAPTION_X_POSITION  (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_X_POSITION + (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_X_SIZE  / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_WATER_OBJECT_BOX_TEXT_CAPTION_Y_POSITION ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_Y_POSITION +  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_Y_SIZE) - 12)      
     
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_X_POSITION,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_Y_POSITION,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_X_SIZE + 1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_Y_SIZE + 1,
                              WHITE);        
                              
      BITMAP *bitmap_right_interface_water_object_preview = create_sub_bitmap(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_X_POSITION + 1,
                                                                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_BOX_Y_POSITION + 1,
                                                                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_X_SIZE,
                                                                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_PREVIEW_CELL_Y_SIZE);
                               
      DrawWaterObjectPreviewInto(bitmap_right_interface_water_object_preview);         
                                                                                   
      destroy_bitmap(bitmap_right_interface_water_object_preview);           
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_WATER_OBJECT_BOX_TEXT_CAPTION_X_POSITION,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_WATER_OBJECT_BOX_TEXT_CAPTION_Y_POSITION,
                                                 WHITE,
                                                (!(level_working_on->runtime_stats.water_genus_junction
                                                    & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                       ? standard_water_graphical_objects.at(level_working_on->runtime_stats.water_genus_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)->graphical_object_name
                                                       :   custom_water_graphical_objects.at(level_working_on->runtime_stats.water_genus_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)->graphical_object_name);  
                                                                                               
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X1 712
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X2 791
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 217
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2 252
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT   (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 +  8 +  1)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + 19 +  1)
  
      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2-1,
                                  BLACK);
                                        
      // These are the arrows on the outside of the junction selector box                 
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X1     + 4)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT     (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT    ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2)/2)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP      (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM   (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT + 10)
      const int right_interface_water_object_junction_selector_left_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP,
                                                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM,
                                                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT,
                                                                                          SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_LEFT,};
                                   
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X2     - 4)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT   (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT + 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT  ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y2)/2)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_TOP     (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_BOTTOM  (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT + 10)
      const int right_interface_water_object_junction_selector_right_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_TOP,
                                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_BOTTOM,
                                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT,
                                                                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT,};
                                                                                             
      polygon(bitmap_backbuffer, 3, right_interface_water_object_junction_selector_left_triangle_points,  WHITE);  
      polygon(bitmap_backbuffer, 3, right_interface_water_object_junction_selector_right_triangle_points, WHITE);     
                              
      bool water_object_type_is_custom = level_working_on->runtime_stats.water_genus_junction
                                            & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
                                          
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 (!water_object_type_is_custom)
                                                    ? "Standard"
                                                    : "Custom");
                                                                                                                 
      int no_current_water_object = 1 + (level_working_on->runtime_stats.water_genus_junction
                                              & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);     
                                                        
      int max_available_water;  
      
      if (!(level_working_on->runtime_stats.water_genus_junction & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {                                        
         max_available_water = no_standard_waters;
      } else {
         max_available_water = no_custom_waters;
      }             
                                                      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_INFO_RECTANGLE_Y_TEXT_2,
                                                 WHITE,
                                                 "%2d of %2d",
                                                 no_current_water_object,
                                                 max_available_water);
                                                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X1 712
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X2 791
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 272
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y2 307
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 +  8 +  1)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y1 + 19 +  1)

      rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y1,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X2,
                              SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y2,
                              WHITE);
      rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y1+1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X2-1,
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y2-1,
                                  BLACK);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X2) / 2))
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_1,
                                                 WHITE,
                                                 "Placed");
              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_X_TEXT,
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_SPECIAL_OBJECT_INFO_RECTANGLE_Y_TEXT_2,
                                                 WHITE,
                                                 "%d of %d",
                                                 level_working_on->runtime_stats.no_waters,
                                                 MAX_NO_WATERS);
      
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_START_X_POSITION  605     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_START_Y_POSITION  321  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE        92     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE        17   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT        2
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT        2 // This can be three without moving stuff downwards. Super top secret! :D   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(n) + 5)
    
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT),
                                  WHITE);
      }
   
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }                                
      
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Add Object");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Delete All"); 
                                                    
      if (editor_currently_selected_tool == EDITOR_TOOL_PLACE_WATER_ON_CLICK) {
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(1),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(1),
                                 RED);
      }    
                                                 
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_X 16
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_Y 1

#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_ENTRY_CELL_X_SIZE 10
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_ENTRY_CELL_X_SIZE 10
       
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_START_X_POSITION    609     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_START_Y_POSITION    365  

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_ENTRY_CELL_X_SIZE+1)     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_SIZE        (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_ENTRY_CELL_X_SIZE+1)   
        
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_X
#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_COUNT        SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_Y   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_SIZE) - 1) 
                                          
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(1),
                                  WHITE); 
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(1),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_Y_COUNT),
                                  BLUE); 
      } 
    
      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(0),
                               SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(0),     
                               SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_COUNT),
                               WHITE);
      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(0),
                               SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(1),     
                               SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELL_X_COUNT),
                               WHITE);      
      
      for (int ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_Y; ycell++) {
         for (int xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_X; xcell++) {
            rectfill15a(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(xcell)+1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(ycell) +1,
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(xcell),
                                           SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(ycell),
                                           level_working_on->runtime_stats.water_palette[xcell]);
         }
      }     
                                                                                                    
      const int COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_LEFT_COLOUR  = RED;//255 << 16 | 0 << 8 | 0;
      const int COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_RIGHT_COLOUR = BLUE;//255 << 16 | 0 << 8 | 0;
      
      if (right_interface_currently_selected_special_objects_left_colour < 16) {
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(right_interface_currently_selected_special_objects_left_colour & 15),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(right_interface_currently_selected_special_objects_left_colour & 15)+1,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(0)+1,
                                 COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_LEFT_COLOUR);  
      } else {       
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(right_interface_currently_selected_special_objects_left_colour & 15),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(right_interface_currently_selected_special_objects_left_colour & 15)+1,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(0)+1,
                                 COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_LEFT_COLOUR);  
      }
                                                                                           
      if (right_interface_currently_selected_special_objects_right_colour < 16 ) {            
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(right_interface_currently_selected_special_objects_right_colour & 15),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(right_interface_currently_selected_special_objects_right_colour & 15)+1,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(0)+1,
                                 COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_RIGHT_COLOUR);
      } else {       
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(right_interface_currently_selected_special_objects_right_colour & 15),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(0),
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(right_interface_currently_selected_special_objects_right_colour & 15)+1,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(0)+1,
                                 COLOUR_RIGHT_INTERFACE_SPECIAL_OBJECTS_HIGHLIGHT_RIGHT_COLOUR); 
      }
                                                                                            
                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START           620
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END             770
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_LENGTH                 \
         ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END                 \
                 - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START) + 1)
         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y               390
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y               (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y + 25)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y               (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y + 25)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE             8  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_TEXT            605
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_VALUE_TEXT      794
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE 3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE 3
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE   3

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_LEFT(n)                         \
              (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START                         \
                 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_LENGTH * (n))/32))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_CENTRE(n)                       \
              ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_LEFT(n)                 \
                 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_LEFT(n+1))/2) 

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_RIGHT(n)                        \
              ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_LEFT(n+1))-1)

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END,
                               WHITE);
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "R");
                               
      if ((right_interface_currently_selected_special_objects_left_colour != 0)
       && (right_interface_currently_selected_special_objects_left_colour != 16)) {
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_special_objects_left_colour_r_value);
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_special_objects_left_colour_r_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);
      }
      
      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END,
                               WHITE);               
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "G");                
                                                 
      if ((right_interface_currently_selected_special_objects_left_colour != 0)
       && (right_interface_currently_selected_special_objects_left_colour != 16)) {
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_special_objects_left_colour_g_value);
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_special_objects_left_colour_g_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);
      }

      hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END,
                               WHITE);               
                               
      textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_TEXT,
                                          SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y
                                             -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                          WHITE,
                                          "B");            
                                  
      if ((right_interface_currently_selected_special_objects_left_colour != 0)
       && (right_interface_currently_selected_special_objects_left_colour != 16)) {
         textprintf_right(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_VALUE_TEXT,
                                             SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y
                                                -  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_TEXT_Y_DIFFERENCE,
                                             WHITE,
                                             "%d",
                                             right_interface_currently_selected_special_objects_left_colour_b_value); 
                                          
         draw_sprite(bitmap_backbuffer, bitmap_slider_marker, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_ZONE_X_CENTRE(right_interface_currently_selected_special_objects_left_colour_b_value)
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_X_DIFFERENCE,
                                                              SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y
                                                                - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_SLIDER_Y_DIFFERENCE);                                                                                               
      }                                                                                                                   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_START_X_POSITION  605     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_START_Y_POSITION  (67 + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y) 

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE        92     
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE        17   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT        2
#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT        5   

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n)           \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_START_X_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_RIGHT(n)          \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(n) +            \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_SIZE) - 1)

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n)            \
    ((SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_START_Y_POSITION) +   \
         (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE * (n)))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_CENTRE(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE/2))

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_BOTTOM(n)         \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) +             \
       (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_SIZE) - 1)       

#define SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(n)  \
    (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(n) + 5)
    
      for (int xline = 0; xline <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT; xline++) {
         hline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(xline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT),
                                  WHITE); 
      }
   
      for (int yline = 0; yline <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT; yline++) {
         vline(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(yline),     
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(0),
                                  SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT),
                                  WHITE); 
      }
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Copy A to B");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(0),
                                                 WHITE,
                                                 "Copy B to A");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Light Swap");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(1),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(1),
                                                 WHITE,
                                                 "Grad A to B");
                              
      textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_CENTRE(0),
                                                 SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_TEXT_Y_POSITION(2),
                                                 WHITE,
                                                 "Default Pal");
   }
   
   if ((right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION)  
    || (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER   )  
    || (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR   )) {
      bool valid_texture_preview = false; 
      if (!(right_interface_currently_selected_texture & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) {
         if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
            if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_16s) {
               valid_texture_preview = true;
            }
         } else {
            if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_256s) {
               valid_texture_preview = true;
            }
         }
      } else {
         int right_interface_currently_selected_texture_id = right_interface_currently_selected_texture & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
         
         if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
            if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_16s) {
               valid_texture_preview = true;
            }
         } else {
            if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_256s) {
               valid_texture_preview = true;
            }
         }         
      }
      
      if (valid_texture_preview) {
          
         // If it's the texture selection panel, or the palette mapping panel
         // we should display the small top box describing the current texture.     
   #define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION 602
   #define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION  10      
        
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE + 1,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE + 1,
                                 WHITE);        
                                 
         BITMAP *bitmap_right_interface_texture_preview = create_sub_bitmap(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION + 1,
                                                                                               SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION + 1,
                                                                                               SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE,
                                                                                               SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE);
                                
                              
         DrawTexturePreviewIntoWZoomHack(bitmap_right_interface_texture_preview, right_interface_currently_selected_texture_type,
                                                                                 right_interface_currently_selected_texture,
                                                                                 right_interface_currently_held_pal_map,
                                                                                 right_interface_currently_selected_texture_window_zoom);                         
                                                                                               
         destroy_bitmap(bitmap_right_interface_texture_preview);
         
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION 688
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION  56 
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_SIZE      98
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_SIZE      17  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_TEXT_X_POSITION  (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION + (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_SIZE/2))        
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_TEXT_Y_POSITION  (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION + 5)        
         
         rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_SIZE,
                                 SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_SIZE,
                                 (right_interface_currently_selected_texture_swap_axes_value != 0) ? WHITE : DGREY);        
          
         textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_TEXT_X_POSITION,
                                                    SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_TEXT_Y_POSITION,
                                                    (right_interface_currently_selected_texture_swap_axes_value != 0) ? WHITE : LGREY,
                                                    "Swap Axes");  
                                                    
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION   677
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_1  12
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_2  21
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_3  30
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_4  39
#define SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_5  48
         
         if (!(right_interface_currently_selected_texture & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) {
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_1,   
                                                WHITE,
                                               "name: %s",
                                                ((right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? GetTextureArchiveTexture16
                                                : GetTextureArchiveTexture256)(loaded_active_texture_archive, right_interface_currently_selected_texture)->name);
      
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_2,
                                                WHITE,
                                               "x sz: %d",
                                                ((right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? GetTextureArchiveTexture16
                                                : GetTextureArchiveTexture256)(loaded_active_texture_archive, right_interface_currently_selected_texture)->xs);
         
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_3,
                                                WHITE,
                                               "y sz: %d",
                                                ((right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? GetTextureArchiveTexture16
                                                : GetTextureArchiveTexture256)(loaded_active_texture_archive, right_interface_currently_selected_texture)->ys);
         
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_4,
                                                WHITE,
                                               "cols: %d",
                                                ((right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? 16
                                                : 256));
         } else {
            int object_id = right_interface_currently_selected_texture & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
           
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_1,   
                                                WHITE,
                                               "cust %3d",
                                                object_id);
         
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_2,
                                                WHITE,
                                               "x sz: %d",
                                                (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? GetAppendedTexture16(object_id)->xs
                                                : GetAppendedTexture256(object_id)->xs);
         
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_3,
                                                WHITE,
                                               "y sz: %d",
                                                (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? GetAppendedTexture16(object_id)->ys
                                                : GetAppendedTexture256(object_id)->ys);
                                                
            textprintf(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_X_POSITION,
                                                SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_DESCRIPTION_TEXT_Y_POSITION_4,
                                                WHITE,
                                               "cols: %d",
                                                ((right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                ? 16
                                                : 256));
         }
      }
   }                     
                        
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X1 624
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X2 770
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X_TEXT (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1 565
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2 590
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y_TEXT_1 (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1 + 9)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y_TEXT_2 (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y_TEXT_1 + 10)
                                            
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X1 SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X2 SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y1 (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1 - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y2 SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION
                                                                                                    
   const int COLOUR_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP        = 110 << 16 | 110 << 8 | 160;
   const int COLOUR_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_BORDER = 255 << 16 | 255 << 8 | 255;
                         
   rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X1,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y1,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X2,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y2,
                           COLOUR_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_BORDER);                            
   rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X1+1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y1+1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_X2-1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP_RECTANGLE_Y2-1,
                               COLOUR_RIGHT_INTERFACE_TAB_SELECTOR_AREA_BACKDROP);
     
   rect(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X1,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X2,
                           SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2,
                           WHITE);
   rectfill(bitmap_backbuffer, SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X1+1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1+1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X2-1,
                               SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2-1,
                               BLACK);
                                  
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X1    - 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_RIGHT - 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_Y_LEFT  ((SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2)/2)
   const int right_interface_tab_selector_left_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                     SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1,
                                                                     SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_RIGHT,
                                                                     SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2+1,
                                                                     SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_LEFT,
                                                                     SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_Y_LEFT+1,};

#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_LEFT  (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X2    + 10)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_RIGHT (SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_LEFT + 13)
#define SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT ((SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1 + SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2)/2)
   const int right_interface_tab_selector_right_triangle_points[6] = {SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                      SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1,
                                                                      SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_LEFT,
                                                                      SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2+1,
                                                                      SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_RIGHT,
                                                                      SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_Y_RIGHT+1,};
      
   polygon(bitmap_backbuffer, 3, right_interface_tab_selector_left_triangle_points,  WHITE);  
   polygon(bitmap_backbuffer, 3, right_interface_tab_selector_right_triangle_points, WHITE);                       
                             
   textprintf_centre(bitmap_backbuffer, font, SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_X_TEXT,
                                              SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y_TEXT_1,
                                              WHITE,
                                              text_right_interface_tab_selector_caption_rectangle_caption[right_interface_current_tab]);     
   
   // ----------------------------------------------
         
   release_bitmap(bitmap_backbuffer);  
}

void RecalculateRightInterfaceTextureSelectionDatabaseSize() { 
   if (!(right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT)) {
      if (right_interface_texture_selection_current_set == RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_16) {
         right_interface_texture_selection_database_size = loaded_active_texture_archive->no_texture_16s;
      } else {
         right_interface_texture_selection_database_size = loaded_active_texture_archive->no_texture_256s;
      }
   } else {
      if (right_interface_texture_selection_current_set == RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_16) {
         right_interface_texture_selection_database_size = level_working_on->no_appended_texture_16s;
      } else {
         right_interface_texture_selection_database_size = level_working_on->no_appended_texture_256s;
      }
   }      
}

////////////////////////////////////////////////////////////////
                                   
void MenuNewLevelSequence();       
void MenuLoadLevelSequence();       
void MenuSaveLevelSequence();       
int MenuQuitEditorSequence();    

void MenuLoadImageToObjectSequence(int destination_slot, int type, bool image_copy = true, bool palette_copy = false);

void MenuLevelInfoSequence(); 

void MenuSaveImageSequence(); 

void MenuAboutDialogSequence();

bool MenuSelectTextureArchiveSequence(char *selected_archive_string_buffer, bool cancel_permitted);

////////////////////////////////////////////////////////////////

void InitialiseGUI();
void InitialiseLevel(const char *texture_archive_without_extension);

int LevelEditorLoop() {
#define LEVEL_EDITOR_STATE_STEADY 0
#define LEVEL_EDITOR_STATE_EXIT   1
   int level_editor_state = LEVEL_EDITOR_STATE_STEADY;
   
#define LEVEL_EDITOR_EDITING_MODE_OBJECT 0
#define LEVEL_EDITOR_EDITING_MODE_STEEL  1
#define LEVEL_EDITOR_EDITING_MODE_WATER  2
   int level_editor_editing_mode = LEVEL_EDITOR_EDITING_MODE_OBJECT; 
   
   bool action_taken_this_frame = false;
                                                                                     
#define LEVEL_EDITOR_CAMERA_MOTION_FLAG_L 0x01                                           
#define LEVEL_EDITOR_CAMERA_MOTION_FLAG_R 0x02
   int camera_motion_flags = 0;            
   
#define LEMMING_TOOL_BONK_LENGTH 1 // Length in ticks for a bonk of the lemming tools.
   int lemming_tool_bonk_timer = 0;             
   
   InitialiseGUI();                              
   
   bool toexit = false;                
                          
   int oldclock, nowclock;
                         
   while (!toexit) {
      DoKeyInput();    
      DoMouseInput();   
      DoRMBConjunctionInput();
      
      //toexit |= key[KEY_X];
      
      // Only one action per frame.
      action_taken_this_frame = false;
      
      // Camera control keys
      camera_motion_flags = 0;
      
      if (key[KEY_LEFT] && !key[KEY_RIGHT]) {
         camera_motion_flags |= LEVEL_EDITOR_CAMERA_MOTION_FLAG_L;
      } else if (key[KEY_RIGHT] && !key[KEY_LEFT]) {
         camera_motion_flags |= LEVEL_EDITOR_CAMERA_MOTION_FLAG_R;
      }
      if (camera_motion_flags & LEVEL_EDITOR_CAMERA_MOTION_FLAG_L) camera_x_inset -= 10;
      if (camera_motion_flags & LEVEL_EDITOR_CAMERA_MOTION_FLAG_R) camera_x_inset += 10;
      
      if (camera_x_inset < 0) {
         camera_x_inset = 0;
      }
      if (camera_x_inset > ((s32)((LEVEL_X_SIZE)) - LEVEL_DISPLAY_X_SIZE)) {
         camera_x_inset = ((LEVEL_X_SIZE)) - LEVEL_DISPLAY_X_SIZE;
      }                                               
      
      do {
         if (KeyDown(KEY_N)) {
            if (currently_selected_object_count != 0) {
               for (int o = 0; o < currently_selected_object_count; o++) {
                  LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                  if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                     int object_id = generic_object_info->object_id;
                     
                     bool custom_object = (object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT) != 0;
                     
                     if (custom_object) object_id &= ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT; 
                                                                                         
                     int target_object_x_size, target_object_y_size;
                     
                     if (!custom_object) {
                        if (generic_object_info->object_header.object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                           target_object_x_size = GetTextureArchiveTexture16(loaded_active_texture_archive, object_id)->xs;
                           target_object_y_size = GetTextureArchiveTexture16(loaded_active_texture_archive, object_id)->ys;
                        } else {
                           target_object_x_size = GetTextureArchiveTexture256(loaded_active_texture_archive, object_id)->xs;
                           target_object_y_size = GetTextureArchiveTexture256(loaded_active_texture_archive, object_id)->ys;
                        }
                     } else { 
                        if (generic_object_info->object_header.object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                           target_object_x_size = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *)GetAppendedTexture16(object_id))->xs;
                           target_object_y_size = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *)GetAppendedTexture16(object_id))->ys;
                        } else {
                           target_object_x_size = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *)GetAppendedTexture256(object_id))->xs;
                           target_object_y_size = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *)GetAppendedTexture256(object_id))->ys;
                        }
                     }
                     
                     // Flip the 'target object size's if the axes are flipped.
                     if (generic_object_info->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES) {
                        target_object_x_size ^= target_object_y_size ^= target_object_x_size ^= target_object_y_size;
                     }
                     
                     // current size
                     int cxs = generic_object_info->x2 - generic_object_info->x1 + 1;
                     int cys = generic_object_info->y2 - generic_object_info->y1 + 1;
                     
                     // texture size
                     int txs = (target_object_x_size << 8) / (generic_object_info->tsx);         
                     int tys = (target_object_y_size << 8) / (generic_object_info->tsy);
                                    
                     // nearest size
                     int nx = NearestXtoY(txs, cxs);        
                     int ny = NearestXtoY(tys, cys);        //allegro_message("tsX = %d, cursizeX = %d, resultX: %d\ntsY = %d, cursizeY = %d, resultY: %d", txs, cxs, nx, tys, cys, ny);
                     
                     if (nx == 0) nx = txs;
                     if (ny == 0) ny = tys;
                     
                     // set the size
                     generic_object_info->x2 = generic_object_info->x1 + nx - 1;
                     generic_object_info->y2 = generic_object_info->y1 + ny - 1;
                  }
               }
               TruncateLevelObjectsToLevel();
            }
         } else
         if (MouseDown(MOUSE_LMB)) {
            if ((mouse_x >= SCREEN_LAYOUT_UPPER_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_UPPER_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_UPPER_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_UPPER_INTERFACE_END_Y_POSITION  )) {  
               // Checking for the editor tool clicking.  
               bool tool_click_successful = false;
                              
               int xcell, ycell;               
                                                                      
               int page_cell;     
                  
               for (ycell = 0; ycell < SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                  for (xcell = 0; xcell < SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                     page_cell = xcell + (ycell * SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT);
                     if ((mouse_x >= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_LEFT(xcell)  )
                      && (mouse_x <= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_X_RIGHT(xcell) )
                      && (mouse_y >= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_TOP(ycell)   ) 
                      && (mouse_y <= SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                        if (page_cell == (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 0)) {
                           MenuNewLevelSequence();
                        } else
                        if (page_cell == (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 1)) {   
                           MenuLoadLevelSequence();
                        } else     
                        if (page_cell == (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 2)) {   
                           MenuSaveLevelSequence();
                        } else     
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 0)
                                        + (                                                         1))) {   
                           MenuLevelInfoSequence();
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 0)
                                        + (                                                         2))) {   
                           MenuSaveImageSequence();
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 0)
                                        + (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT - 2))) {   
                           MenuLoadImageToObjectSequence(-1, LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16, false, true);
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 1)
                                        + (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT - 2))) {  
                           MenuLoadImageToObjectSequence(-1, LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256, false, true);
                        } else        
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 0)
                                        + (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT - 1))) {   
                           MenuLoadImageToObjectSequence(-1, LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16, true, MouseIsDown(MOUSE_RMB));
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * 1)
                                        + (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT - 1))) {  
                           MenuLoadImageToObjectSequence(-1, LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256, true, MouseIsDown(MOUSE_RMB));
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * (SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT - 1)) - 1)) {
                           MenuAboutDialogSequence();
                        } else          
                        if (page_cell == ((SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_X_COUNT * SCREEN_LAYOUT_UPPER_INTERFACE_BUTTON_GRID_CELL_Y_COUNT) - 1)) {
                           if (MenuQuitEditorSequence() == 1) toexit = true;
                        }                        
                        tool_click_successful = true; 
                        action_taken_this_frame = true;  
                     }
                     if (tool_click_successful) break;    
                  }                        
                  if (tool_click_successful) break;
               }                   
               if (action_taken_this_frame) break;    
            } else            
            if ((mouse_x >= SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LEVEL_PANE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_LEVEL_PANE_END_Y_POSITION  )) {  
               bool click_successful = false;                 
                  
               int level_x, level_y;                                                             
               ConvertScreenCoordsToLevelCoords(mouse_x, mouse_y, level_x, level_y);
               
               if (editor_currently_selected_tool == EDITOR_TOOL_SELECT_TEXTURE) {                            
                  int texture_under_cursor = TextureTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", texture_under_cursor);
                  
                  if (!(key_shifts & KB_SHIFT_FLAG)) {                        
                     if (texture_under_cursor == -1) {
                        SelectionClear();
                     } else {
                        if (SelectionIsItemSelected(texture_under_cursor)) {
                           editor_currently_selected_tool = EDITOR_TOOL_MOVING_TEXTURE;
                           
                           // Let's do these handles.
                           for (int o = 0; o < currently_selected_object_count; o++) {
                              LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                              
                              currently_selected_object_handles_x1[o] = generic_object_info->x1 - (mouse_x >> 1) - camera_x_inset; 
                              currently_selected_object_handles_x2[o] = generic_object_info->x2 - (mouse_x >> 1) - camera_x_inset; 
                              currently_selected_object_handles_y1[o] = generic_object_info->y1 - (mouse_y >> 1) - camera_y_inset; 
                              currently_selected_object_handles_y2[o] = generic_object_info->y2 - (mouse_y >> 1) - camera_y_inset;    
                           }
                        } else {
                           SelectionClear();
                           SelectionAddItemNumber(texture_under_cursor);
                        }
                     }
                  } else {                       
                     if (texture_under_cursor == -1) {
                     } else {
                        if (SelectionIsItemSelected(texture_under_cursor)) {
                           SelectionRemoveItemNumber(texture_under_cursor);
                        } else {                                             
                           SelectionAddItemNumber(texture_under_cursor);
                        }
                     }
                  }
                  
                  action_taken_this_frame = true;
               }     
               
               if (currently_selected_object_count == 1) {
                  // gotta refresh the tox, toy, tsx, tsy meters at the bottom of the screen.
                  LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]); 
                  
                  if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                     lower_interface_currently_selected_texture_adjustment_tox_value = generic_object_info->tox;   
                     lower_interface_currently_selected_texture_adjustment_toy_value = generic_object_info->toy;   
                     lower_interface_currently_selected_texture_adjustment_tsx_value = generic_object_info->tsx;   
                     lower_interface_currently_selected_texture_adjustment_tsy_value = generic_object_info->tsy;
                  } else {
                     lower_interface_currently_selected_texture_adjustment_tox_value = 0;   
                     lower_interface_currently_selected_texture_adjustment_toy_value = 0;   
                     lower_interface_currently_selected_texture_adjustment_tsx_value = 1 << 8;   
                     lower_interface_currently_selected_texture_adjustment_tsy_value = 1 << 8;   
                  }

                  if (generic_object_info->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
                     lower_interface_currently_selected_subtractive_value = 1;
                  } else {
                     lower_interface_currently_selected_subtractive_value = 0;
                  }

                  if (generic_object_info->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
                     if (generic_object_info->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
                        lower_interface_currently_selected_masking_behaviour_value = LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_ONLY_DRAW_ON_BLANKS; 
                     } else {
                        lower_interface_currently_selected_masking_behaviour_value = LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_MASKING_ENABLED;
                     }
                  } else {
                     lower_interface_currently_selected_masking_behaviour_value = LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING;
                  }
               }
               
               if (action_taken_this_frame) break; 
                                                                            
               if (editor_currently_selected_tool == EDITOR_TOOL_SELECT_SPEC_OBJ) {                            
                  int object_under_cursor = SpecialObjectTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", texture_under_cursor);
                  
                  if (!(key_shifts & KB_SHIFT_FLAG)) {                        
                     if (object_under_cursor == -1) {
                        SelectionSpecialClear();
                     } else {
                        if (SelectionSpecialIsItemSelected(object_under_cursor)) {
                           editor_currently_selected_tool = EDITOR_TOOL_MOVING_SPEC_OBJ;
                           
                           int item;
                           
                           // Let's do these handles.
                           for (int o = 0; o < currently_selected_special_object_count; o++) {
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START;                                        
                                 currently_selected_special_object_handles_x[o] = level_working_on->runtime_stats.entrance_x[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o] = level_working_on->runtime_stats.entrance_y[item] - (mouse_y >> 1) - camera_y_inset; 
                              } else
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_EXITS_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_EXITS_END  )) {      
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_EXITS_START;                                        
                                 currently_selected_special_object_handles_x[o] = level_working_on->runtime_stats.exit_x[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o] = level_working_on->runtime_stats.exit_y[item] - (mouse_y >> 1) - camera_y_inset; 
                              } else
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) {  
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_TRAPS_START;                                        
                                 currently_selected_special_object_handles_x[o] = level_working_on->runtime_stats.trap_x[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o] = level_working_on->runtime_stats.trap_y[item] - (mouse_y >> 1) - camera_y_inset; 
                              } else
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) {  
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_HAZARDS_START;                                        
                                 currently_selected_special_object_handles_x[o] = level_working_on->runtime_stats.hazard_x[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o] = level_working_on->runtime_stats.hazard_y[item] - (mouse_y >> 1) - camera_y_inset; 
                              } else
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) {  
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START;                                        
                                 currently_selected_special_object_handles_x[o] = level_working_on->runtime_stats.uninteractive_x[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o] = level_working_on->runtime_stats.uninteractive_y[item] - (mouse_y >> 1) - camera_y_inset; 
                              }
                                  
                              if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
                               && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {  
                                 item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_WATERS_START;                                        
                                 currently_selected_special_object_handles_x[o]  = level_working_on->runtime_stats.water_x1[item] - (mouse_x >> 1) - camera_x_inset; 
                                 currently_selected_special_object_handles_x2[o] = level_working_on->runtime_stats.water_x2[item] - (mouse_x >> 1) - camera_x_inset;                              
                                 currently_selected_special_object_handles_y[o]  = level_working_on->runtime_stats.water_y[item]  - (mouse_y >> 1) - camera_y_inset; 
                              }       
                           }                           
                        } else {
                           SelectionSpecialClear();
                           SelectionSpecialAddItemNumber(object_under_cursor);
                        }
                     }
                  } else {                       
                     if (object_under_cursor == -1) {
                     } else {
                        if (SelectionSpecialIsItemSelected(object_under_cursor)) {
                           SelectionSpecialRemoveItemNumber(object_under_cursor);
                        } else {                                             
                           SelectionSpecialAddItemNumber(object_under_cursor);
                        }
                     }
                  }
                  
                  action_taken_this_frame = true;
               }     
               if (action_taken_this_frame) break;    
               
               if (editor_currently_selected_tool == EDITOR_TOOL_NEW_TEXTURE_ON_CLICK) {  
                  bool valid_texture_placement = false; 
                  
                  if (!(right_interface_currently_selected_texture & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) {
                     if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                        if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_16s) {
                           valid_texture_placement = true;
                        }
                     } else {
                        if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_256s) {
                           valid_texture_placement = true;
                        }
                     }
                  } else {
                     int right_interface_currently_selected_texture_id = right_interface_currently_selected_texture & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                     
                     if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                        if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_16s) {
                           valid_texture_placement = true;
                        }
                     } else {
                        if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_256s) {
                           valid_texture_placement = true;
                        }
                     }         
                  }
      
                  if (valid_texture_placement) {
                     editor_currently_selected_tool = EDITOR_TOOL_PLACING_TEXTURE;
                              
                     SelectionClear();
                                                     
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *new_object = (LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)AddTerrainObject(right_interface_currently_selected_texture_type);
                                                                                  
                     SelectionAddItemNumber(level_working_on->no_terrain_objects - 1);   
                                                
                     new_object->object_id = right_interface_currently_selected_texture;
                     new_object->tox = 0;
                     new_object->toy = 0;
                     new_object->tsx = 1 << 8;
                     new_object->tsy = 1 << 8;
                     
                     new_object->object_header.object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                     new_object->object_header.object_type  = right_interface_currently_selected_texture_type;
                                       
                     for (int p = 0; p < ((right_interface_currently_selected_texture_type ==
                                            LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) ? 16 : 256); p++) {
                        new_object->pal_map[p] = right_interface_currently_held_pal_map[p]; 
                     }
                    
                     new_object->x1 = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                     new_object->y1 = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;      
                     new_object->x2 = new_object->x1;     
                     new_object->y2 = new_object->y1; 
                  } 
                     
                  action_taken_this_frame = true;
               }     
               if (action_taken_this_frame) break; 
               
               if (editor_currently_selected_tool == EDITOR_TOOL_NEW_SOLID_TEXTURE_ON_CLICK) {
                  editor_currently_selected_tool = EDITOR_TOOL_PLACING_TEXTURE;
                           
                  SelectionClear();
                                                  
                  LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *new_object = (LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)AddTerrainObject(LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1);
                                                                               
                  SelectionAddItemNumber(level_working_on->no_terrain_objects - 1);   
                  
                  new_object->object_header.object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                  new_object->object_header.object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1;
                                    
                  new_object->colour = right_interface_currently_held_pal_map[0];
                 
                  new_object->x1 = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                  new_object->y1 = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;      
                  new_object->x2 = new_object->x1;     
                  new_object->y2 = new_object->y1; 
                   
                  action_taken_this_frame = true;
               }     
               if (action_taken_this_frame) break;  
               
               if (editor_currently_selected_tool == EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK) {                            
                  int texture_under_cursor = TextureTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", texture_under_cursor);
                                 
                  if (texture_under_cursor != -1) {
                     editor_currently_selected_tool = EDITOR_TOOL_SCALING_TEXTURE_LMB;
                           
                     SelectionAddItemNumber(texture_under_cursor);
                     
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(texture_under_cursor); 
                              
                     currently_selected_object_handles_x1[0] = generic_object_info->x1 - (mouse_x >> 1) - camera_x_inset; 
                     currently_selected_object_handles_y1[0] = generic_object_info->y1 - (mouse_y >> 1) - camera_y_inset; 
                  } else {
                     int object_under_cursor = SpecialObjectTopAtPoint(level_x, level_y); 
                     
                     if ((object_under_cursor >= SELECTED_SPECIAL_OBJECTS_WATERS_START) 
                      && (object_under_cursor <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {
                        editor_currently_selected_tool = EDITOR_TOOL_SCALING_WATER_LMB;
                        
                        SelectionSpecialAddItemNumber(object_under_cursor);  
                        
                        int water = object_under_cursor - SELECTED_SPECIAL_OBJECTS_WATERS_START;
                  
                        currently_selected_special_object_handles_x[0] = level_working_on->runtime_stats.water_x1[water] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_special_object_handles_y[0] = level_working_on->runtime_stats.water_y[water]  - (mouse_y >> 1) - camera_y_inset;                                                                            
                      }    
                  }
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
                
               if (editor_currently_selected_tool == EDITOR_TOOL_PLACE_DUPE_TEXTURE_ON_CLICK) {
                  editor_currently_selected_tool = EDITOR_TOOL_MOVING_TEXTURE;
                           
                  int min_x = LEVEL_X_SIZE, max_x = 0, min_y = LEVEL_Y_SIZE, max_y = 0;
                  
                  int middle_handle_inset_x;
                  int middle_handle_inset_y;
                  
                  for (int o = 0; o < currently_selected_object_count; o++) {
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                     
                     if (generic_object_info->x1 < min_x) {
                        min_x = generic_object_info->x1;
                     }
                     if (generic_object_info->y1 < min_y) {
                        min_y = generic_object_info->y1;
                     }
                     if (generic_object_info->x2 > max_x) {
                        max_x = generic_object_info->x2;
                     }
                     if (generic_object_info->y2 > max_y) {
                        max_y = generic_object_info->y2;
                     }
                  }
                  
                  middle_handle_inset_x = (min_x + max_x)/2;
                  middle_handle_inset_y = (min_y + max_y)/2;
                           
                  int mouse_offset_x = middle_handle_inset_x + (SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION >> 1);
                  int mouse_offset_y = middle_handle_inset_y + (SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION >> 1);        
                           
                  // Let's do these handles.
                  for (int o = currently_selected_object_count - 1; o >= 0; o--) {
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                     
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_new_object  = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)AddTerrainObject(generic_object_info->object_header.object_type, (LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *)generic_object_info);
                     generic_new_object->x1 = 0;
                     generic_new_object->x2 = 0;
                     generic_new_object->y1 = 0;
                     generic_new_object->y2 = 0;
                                        
                     currently_selected_object_array[o] = level_working_on->no_terrain_objects - 1;                   
                                                                      
                     currently_selected_object_handles_x1[o] = generic_object_info->x1 - mouse_offset_x; 
                     currently_selected_object_handles_x2[o] = generic_object_info->x2 - mouse_offset_x; 
                     currently_selected_object_handles_y1[o] = generic_object_info->y1 - mouse_offset_y; 
                     currently_selected_object_handles_y2[o] = generic_object_info->y2 - mouse_offset_y;    
                  }  
                      
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
               
               if (editor_currently_selected_tool == EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK) {
                  editor_currently_selected_tool = EDITOR_TOOL_MOVING_SPEC_OBJ;
                           
                  int item;
                           
                  // Let's do these handles.
                  if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                     item = level_working_on->runtime_stats.no_entrances + SELECTED_SPECIAL_OBJECTS_ENTRANCES_START;
                     level_working_on->runtime_stats.entrance_d[level_working_on->runtime_stats.no_entrances] = 1;
                     level_working_on->runtime_stats.entrance_x[level_working_on->runtime_stats.no_entrances] = -100;
                     level_working_on->runtime_stats.entrance_y[level_working_on->runtime_stats.no_entrances] = -100;
                     ++level_working_on->runtime_stats.no_entrances;
                  } else
                  if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                     item = level_working_on->runtime_stats.no_exits + SELECTED_SPECIAL_OBJECTS_EXITS_START;       
                     level_working_on->runtime_stats.exit_x[level_working_on->runtime_stats.no_exits] = -100;
                     level_working_on->runtime_stats.exit_y[level_working_on->runtime_stats.no_exits] = -100;
                     ++level_working_on->runtime_stats.no_exits;
                  } else
                  if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                     item = level_working_on->runtime_stats.no_traps + SELECTED_SPECIAL_OBJECTS_TRAPS_START;  
                     level_working_on->runtime_stats.trap_x[level_working_on->runtime_stats.no_traps] = -100;
                     level_working_on->runtime_stats.trap_y[level_working_on->runtime_stats.no_traps] = -100;
                     
                     level_working_on->runtime_stats.trap_z[level_working_on->runtime_stats.no_traps] = TRAP_Z_FOREGROUND;
                     
                     level_working_on->runtime_stats.trap_genus[level_working_on->runtime_stats.no_traps] = right_interface_special_objects_current_genus;
                     ++level_working_on->runtime_stats.no_traps;
                  } else
                  if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                     item = level_working_on->runtime_stats.no_hazards + SELECTED_SPECIAL_OBJECTS_HAZARDS_START;  
                     level_working_on->runtime_stats.hazard_x[level_working_on->runtime_stats.no_hazards] = -100;
                     level_working_on->runtime_stats.hazard_y[level_working_on->runtime_stats.no_hazards] = -100;
                     
                     level_working_on->runtime_stats.hazard_z[level_working_on->runtime_stats.no_hazards] = HAZARD_Z_FOREGROUND;
                     
                     level_working_on->runtime_stats.hazard_genus[level_working_on->runtime_stats.no_hazards] = right_interface_special_objects_current_genus;
                     ++level_working_on->runtime_stats.no_hazards;
                  } else
                  if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                     item = level_working_on->runtime_stats.no_uninteractives + SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START;  
                     level_working_on->runtime_stats.uninteractive_x[level_working_on->runtime_stats.no_uninteractives] = -100;
                     level_working_on->runtime_stats.uninteractive_y[level_working_on->runtime_stats.no_uninteractives] = -100;
                     
                     level_working_on->runtime_stats.uninteractive_z[level_working_on->runtime_stats.no_uninteractives] = UNINTERACTIVE_Z_FOREGROUND;
                     
                     level_working_on->runtime_stats.uninteractive_genus[level_working_on->runtime_stats.no_uninteractives] = right_interface_special_objects_current_genus;
                     ++level_working_on->runtime_stats.no_uninteractives;
                  }   
                  
                  SelectionSpecialClear();                                                   
                  currently_selected_special_object_handles_x[0] = -(SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION >> 1);
                  currently_selected_special_object_handles_y[0] = -(SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION >> 1);
                  currently_selected_special_object_array[0] = item;  
                  currently_selected_special_object_count = 1;                                       
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
               
               if (editor_currently_selected_tool == EDITOR_TOOL_PLACE_WATER_ON_CLICK) {
                  editor_currently_selected_tool = EDITOR_TOOL_PLACING_WATER;
                           
                  SelectionClear();
                  SelectionSpecialClear();
                                                              
                  level_working_on->runtime_stats.water_z[level_working_on->runtime_stats.no_waters] = WATER_Z_FOREGROUND;
                 
                  level_working_on->runtime_stats.water_x1[level_working_on->runtime_stats.no_waters] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;    
                  level_working_on->runtime_stats.water_x2[level_working_on->runtime_stats.no_waters]
                     = level_working_on->runtime_stats.water_x1[level_working_on->runtime_stats.no_waters];     
                
                  level_working_on->runtime_stats.water_y[level_working_on->runtime_stats.no_waters]  = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;   
                   
                  ++level_working_on->runtime_stats.no_waters;
                   
                  action_taken_this_frame = true;
               }     
               if (action_taken_this_frame) break;
                                                     
               if (editor_currently_selected_tool == EDITOR_TOOL_SELECT_AREA) {                            
                  int area_under_cursor = AreaTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", texture_under_cursor);
                  
                  if (!(key_shifts & KB_SHIFT_FLAG)) {                        
                     if (area_under_cursor == -1) {
                        SelectionAreaClear();
                     } else {
                        if (SelectionAreaIsItemSelected(area_under_cursor)) {
                           editor_currently_selected_tool = EDITOR_TOOL_MOVING_AREA;
                           
                           // Let's do these handles.
                           for (int o = 0; o < currently_selected_area_object_count; o++) {
                              if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                               && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                                 int steel_area = currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START; 
                                 currently_selected_object_handles_x1[o] = level_working_on->runtime_stats.steel_area_x1[steel_area] - (mouse_x >> 1) - camera_x_inset; 
                                 currently_selected_object_handles_x2[o] = level_working_on->runtime_stats.steel_area_x2[steel_area] - (mouse_x >> 1) - camera_x_inset; 
                                 currently_selected_object_handles_y1[o] = level_working_on->runtime_stats.steel_area_y1[steel_area] - (mouse_y >> 1) - camera_y_inset; 
                                 currently_selected_object_handles_y2[o] = level_working_on->runtime_stats.steel_area_y2[steel_area] - (mouse_y >> 1) - camera_y_inset;                                     
                              } else
                              if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                               && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                                 int one_way_area = currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START; 
                                 currently_selected_object_handles_x1[o] = level_working_on->runtime_stats.one_way_area_x1[one_way_area] - (mouse_x >> 1) - camera_x_inset; 
                                 currently_selected_object_handles_x2[o] = level_working_on->runtime_stats.one_way_area_x2[one_way_area] - (mouse_x >> 1) - camera_x_inset; 
                                 currently_selected_object_handles_y1[o] = level_working_on->runtime_stats.one_way_area_y1[one_way_area] - (mouse_y >> 1) - camera_y_inset; 
                                 currently_selected_object_handles_y2[o] = level_working_on->runtime_stats.one_way_area_y2[one_way_area] - (mouse_y >> 1) - camera_y_inset;                                     
                              } 
                           }
                        } else {
                           SelectionAreaClear();
                           SelectionAreaAddItemNumber(area_under_cursor);
                        }
                     }
                  } else {                       
                     if (area_under_cursor == -1) {
                     } else {
                        if (SelectionAreaIsItemSelected(area_under_cursor)) {
                           SelectionAreaRemoveItemNumber(area_under_cursor);
                        } else {                                             
                           SelectionAreaAddItemNumber(area_under_cursor);
                        }
                     }
                  }
                  
                  action_taken_this_frame = true;
               }     

               if (action_taken_this_frame) break; 
                                                                           
               if ((editor_currently_selected_tool == EDITOR_TOOL_NEW_STEEL_AREA_ON_CLICK  ) 
                || (editor_currently_selected_tool == EDITOR_TOOL_NEW_ONE_WAY_AREA_ON_CLICK)) {
                  SelectionAreaClear();
                  
                  if (editor_currently_selected_tool == EDITOR_TOOL_NEW_STEEL_AREA_ON_CLICK) {
                     if (level_working_on->runtime_stats.no_steel_areas < MAX_NO_STEEL_AREAS) {  
                        SelectionAreaAddItemNumber(level_working_on->runtime_stats.no_steel_areas + SELECTED_AREA_OBJECTS_STEEL_AREAS_START);   
                        
                        level_working_on->runtime_stats.steel_area_x1[level_working_on->runtime_stats.no_steel_areas] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                        level_working_on->runtime_stats.steel_area_y1[level_working_on->runtime_stats.no_steel_areas] = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
                        level_working_on->runtime_stats.steel_area_x2[level_working_on->runtime_stats.no_steel_areas] = level_working_on->runtime_stats.steel_area_x1[level_working_on->runtime_stats.no_steel_areas];
                        level_working_on->runtime_stats.steel_area_y2[level_working_on->runtime_stats.no_steel_areas] = level_working_on->runtime_stats.steel_area_y1[level_working_on->runtime_stats.no_steel_areas];
                                              
                        ++level_working_on->runtime_stats.no_steel_areas; 
                     } else break;
                  } else                  
                  if (editor_currently_selected_tool == EDITOR_TOOL_NEW_ONE_WAY_AREA_ON_CLICK) {
                     if (level_working_on->runtime_stats.no_one_way_areas < MAX_NO_ONE_WAY_AREAS) {     
                        SelectionAreaAddItemNumber(level_working_on->runtime_stats.no_one_way_areas + SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START);   
                        
                        level_working_on->runtime_stats.one_way_area_x1[level_working_on->runtime_stats.no_one_way_areas] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                        level_working_on->runtime_stats.one_way_area_y1[level_working_on->runtime_stats.no_one_way_areas] = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
                        level_working_on->runtime_stats.one_way_area_x2[level_working_on->runtime_stats.no_one_way_areas] = level_working_on->runtime_stats.one_way_area_x1[level_working_on->runtime_stats.no_one_way_areas];
                        level_working_on->runtime_stats.one_way_area_y2[level_working_on->runtime_stats.no_one_way_areas] = level_working_on->runtime_stats.one_way_area_y1[level_working_on->runtime_stats.no_one_way_areas];
                        
                        level_working_on->runtime_stats.one_way_area_d[level_working_on->runtime_stats.no_one_way_areas] = 1;
                                              
                        ++level_working_on->runtime_stats.no_one_way_areas; 
                     } else break;
                  } else break;                               
                            
                  editor_currently_selected_tool = EDITOR_TOOL_PLACING_AREA;
                   
                  action_taken_this_frame = true;
               }     
               if (action_taken_this_frame) break; 
               
               if (editor_currently_selected_tool == EDITOR_TOOL_SCALE_AREA_ON_CLICK) {                            
                  int area_under_cursor = AreaTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", area_under_cursor);
                                 
                  if (area_under_cursor != -1) {
                     editor_currently_selected_tool = EDITOR_TOOL_SCALING_AREA_LMB;
                           
                     SelectionAreaAddItemNumber(area_under_cursor);
                     
                     if ((area_under_cursor >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                      && (area_under_cursor <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                        int steel_area = area_under_cursor - SELECTED_AREA_OBJECTS_STEEL_AREAS_START; 
                                
                        currently_selected_object_handles_x1[0] = level_working_on->runtime_stats.steel_area_x1[steel_area] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_object_handles_y1[0] = level_working_on->runtime_stats.steel_area_y1[steel_area] - (mouse_y >> 1) - camera_y_inset; 
                     } else 
                     if ((area_under_cursor >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                      && (area_under_cursor <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                        int one_way_area = area_under_cursor - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START; 
                                
                        currently_selected_object_handles_x1[0] = level_working_on->runtime_stats.one_way_area_x1[one_way_area] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_object_handles_y1[0] = level_working_on->runtime_stats.one_way_area_y1[one_way_area] - (mouse_y >> 1) - camera_y_inset; 
                     } 
                  }
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
               
               if (editor_currently_selected_tool == EDITOR_TOOL_CAMERA_FOCUS_POSITION_ON_CLICK) {  
                  editor_currently_selected_tool = EDITOR_TOOL_CAMERA_FOCUS_POSITIONING_LMB;
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
            } else            
            if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_END_Y_POSITION  )) {     
               // Checking for the editor tool clicking.  
               bool tool_click_successful = false;
               
               if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_DEFAULT) { 
                  if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X1)
                   && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y1)
                   && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X2)
                   && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y2)) {
                     lower_interface_current_option_set = LOWER_INTERFACE_OPTION_SET_AREA_SETUP; 
                     
                     editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;
                                                                          
                     SelectionClear();                                        
                     SelectionSpecialClear();                                         
                     SelectionAreaClear();   
                                 
                     action_taken_this_frame = true;
                  }                                    
                  if (action_taken_this_frame) break; 
                   
                  if (currently_selected_object_count != 0) {
                     if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X1)
                      && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y1)
                      && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_X2)
                      && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_SUBTRACTIVE_TOGGLE_BUTTON_RECTANGLE_Y2)) {
                        lower_interface_currently_selected_subtractive_value = (lower_interface_currently_selected_subtractive_value == 1)
                                                                                   ? 0 : 1;
                        
                        for (int o = 0; o < currently_selected_object_count; o++) {
                           LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected = GetTerrainObject(currently_selected_object_array[o]);
                        
                           object_selected->object_flags &= ~LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE;
                           
                           if (lower_interface_currently_selected_subtractive_value == 1) {
                              object_selected->object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE;
                           }
                        }
                          
                        action_taken_this_frame = true;
                     }                                    
                     if (action_taken_this_frame) break;  
                  
                     if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X1)
                      && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y1)
                      && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_X2)
                      && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_SPECIAL_MASKING_BEHAVIOUR_TOGGLE_BUTTON_RECTANGLE_Y2)) {
                        ++lower_interface_currently_selected_masking_behaviour_value %= 3;
                        
                        for (int o = 0; o < currently_selected_object_count; o++) {
                           LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected = GetTerrainObject(currently_selected_object_array[o]);
                        
                           object_selected->object_flags &= ~(LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR | LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS);
                                       
                           if (lower_interface_currently_selected_masking_behaviour_value == LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_MASKING_ENABLED) {
                              object_selected->object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR;
                           } else
                           if (lower_interface_currently_selected_masking_behaviour_value == LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_ONLY_DRAW_ON_BLANKS) {
                              object_selected->object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR;
                              object_selected->object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS;
                           }
                        }
                          
                        action_taken_this_frame = true;
                     }                                    
                     if (action_taken_this_frame) break; 
                  }
                                                            
                  int xcell, ycell;               
                                                                      
                  int page_cell;     

                  for (ycell = 0; ycell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;
                              SelectionClear();                                        
                              SelectionSpecialClear();
                           } else
                           if (page_cell == 1) {
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_SPEC_OBJ;   
                              SelectionClear();                                        
                              SelectionSpecialClear();
                           } else
                           if (page_cell == 2) {
                              if (currently_selected_object_count != 0) {
                                 editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;   
                                 
                                 LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected =
                                      GetTerrainObject(currently_selected_object_array[0]);
                                      
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                    for (int p = 0; p < 16; p++) {
                                       right_interface_currently_held_pal_map[p] =
                                           ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p]; 
                                    }
                                    right_interface_currently_selected_texture_type = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16;
                                    right_interface_currently_selected_texture      = ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->object_id;
                                    
                                    right_interface_currently_selected_texture_swap_axes_value = object_selected->object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES;
                                 } else
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
                                    for (int p = 0; p < 256; p++) {
                                       right_interface_currently_held_pal_map[p] =
                                           ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p]; 
                                    }
                                    right_interface_currently_selected_texture_type = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256;
                                    right_interface_currently_selected_texture      = ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->object_id;
                                 
                                    right_interface_currently_selected_texture_swap_axes_value = object_selected->object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES;
                                 } else
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                    right_interface_currently_held_pal_map[0] =
                                        ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object_selected))->colour; 
                                 
                                    right_interface_currently_selected_texture_swap_axes_value = 0;
                                 }
                              }  
                           } else
                           if (page_cell == 3) {   
                              bool valid_texture_to_apply = false; 
                  
                              if (!(right_interface_currently_selected_texture & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) {
                                 if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                    if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_16s) {
                                       valid_texture_to_apply = true;
                                    }
                                 } else {
                                    if (right_interface_currently_selected_texture < loaded_active_texture_archive->no_texture_256s) {
                                       valid_texture_to_apply = true;
                                    }
                                 }
                              } else {
                                 int right_interface_currently_selected_texture_id = right_interface_currently_selected_texture & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                                 
                                 if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                    if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_16s) {
                                       valid_texture_to_apply = true;
                                    }
                                 } else {
                                    if (right_interface_currently_selected_texture_id < level_working_on->no_appended_texture_256s) {
                                       valid_texture_to_apply = true;
                                    }
                                 }         
                              }
                  
                              if (valid_texture_to_apply) {
                                 if (currently_selected_object_count != 0) {
                                    editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;   
   
                                    for (int o = 0; o < currently_selected_object_count; o++) {
                                       int currently_selected_index = currently_selected_object_array[o]; 
                                     
                                       LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected = GetTerrainObject(currently_selected_index);
                                                                                                                                      
                                       if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16;
                                             
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                             
                                             for (int p = 0; p < 16; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }
                                             
                                             object_selected->object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                                          } else
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {  
                                             ShiftLevelData((u8 *)GetTerrainObject(currently_selected_index + 1, true), SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_256_TO_LEVEL_TERRAIN_OBJECT_16, level_working_on->lemmings_level_file_size); 
                                          
                                             //allegro_message("selected object lives at %08X", (u8*)object_selected-main_memory_chunk);                                                                    
                                          
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16;
                                          
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                          
                                             for (int p = 0; p < 16; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }                                     
                                                        
                                             object_selected->object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                                          } else
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                             ShiftLevelData((u8 *)GetTerrainObject(currently_selected_index + 1, true), SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_1_TO_LEVEL_TERRAIN_OBJECT_16, level_working_on->lemmings_level_file_size); 
                                          
                                             //allegro_message("selected object lives at %08X", (u8*)object_selected-main_memory_chunk);                                                                    
                                          
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16;
                                          
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->tox = 0 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->toy = 0 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->tsx = 1 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->tsy = 1 << 8;
   
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                                                                 
                                             for (int p = 0; p < 16; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }
                                          }
                                       } else
                                       if (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                             ShiftLevelData((u8 *)GetTerrainObject(currently_selected_index + 1, true), SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_16_TO_LEVEL_TERRAIN_OBJECT_256, level_working_on->lemmings_level_file_size);                                        
                                          
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256;
                                          
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                             
                                             for (int p = 0; p < 256; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }      
                                             
                                             object_selected->object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                                          } else
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {  
                                             
                                             //allegro_message("selected object lives at %08X", (u8*)object_selected-main_memory_chunk);                                                                    
                                             
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256;
                                             
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                             
                                             for (int p = 0; p < 256; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }         
                                             
                                             object_selected->object_flags = 0 | right_interface_currently_selected_texture_swap_axes_value;
                                          } else
                                          if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                             ShiftLevelData((u8 *)GetTerrainObject(currently_selected_index + 1, true), SHIFT_DISTANCE_LEVEL_TERRAIN_OBJECT_1_TO_LEVEL_TERRAIN_OBJECT_16, level_working_on->lemmings_level_file_size); 
                                             
                                             //allegro_message("selected object lives at %08X", (u8*)object_selected-main_memory_chunk);                                                                    
                                             
                                             object_selected->object_type  = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256;
                                             
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->tox = 0 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->toy = 0 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->tsx = 1 << 8;
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->tsy = 1 << 8;
      
                                             ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->object_id = right_interface_currently_selected_texture;
                                             
                                             for (int p = 0; p < 256; p++) {
                                                ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p] = 
                                                    right_interface_currently_held_pal_map[p]; 
                                             }
                                          }
                                       }
                                    }
                                 }  
                              }
                           } else                      
                           if (page_cell == 4) {
                              
                              SelectionClear();                                        
                              SelectionSpecialClear();
                              
                              editor_currently_selected_tool = EDITOR_TOOL_NEW_TEXTURE_ON_CLICK;
                           } else                  
                           if (page_cell == 5) {
                              
                              SelectionClear();                                        
                              SelectionSpecialClear();
                              
                              editor_currently_selected_tool = EDITOR_TOOL_NEW_SOLID_TEXTURE_ON_CLICK;
                           } else          
                           if (page_cell == 6) { 
                              if (currently_selected_object_count != 0) {
                                 if (currently_selected_object_array[currently_selected_object_count - 1] != 0) {
                                    for (int o = currently_selected_object_count - 1; o >= 0; o--) {
                                       SwapLevelObjects(currently_selected_object_array[o], currently_selected_object_array[o] - 1); 
                                       --currently_selected_object_array[o];
                                    }
                                 }
                              } else
                              if (currently_selected_special_object_count != 0) {
                                 if (currently_selected_special_object_array[0] != level_working_on->runtime_stats.no_entrances - 1) {
                                    for (int o = 0; o < currently_selected_special_object_count; o++) {
                                       if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                                        && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) { 
                                          SwapLevelSpecialObjects(currently_selected_special_object_array[o], currently_selected_special_object_array[o] + 1); 
                                          ++currently_selected_special_object_array[o];
                                       } 
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) { 
                                       level_working_on->runtime_stats.trap_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_TRAPS_START] = TRAP_Z_BACKGROUND;
                                    } 
                                 }
                                                                    
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) { 
                                       level_working_on->runtime_stats.hazard_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_HAZARDS_START] = HAZARD_Z_BACKGROUND;
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) { 
                                       level_working_on->runtime_stats.uninteractive_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START] = UNINTERACTIVE_Z_BACKGROUND;
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) { 
                                       level_working_on->runtime_stats.water_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_WATERS_START] = WATER_Z_BACKGROUND;
                                    } 
                                 }                                 
                              }
                           } else      
                           if (page_cell == 7) { 
                              if (currently_selected_object_count != 0) {
                                 if (currently_selected_object_array[0] != level_working_on->no_terrain_objects - 1) {
                                    for (int o = 0; o < currently_selected_object_count; o++) {
                                       SwapLevelObjects(currently_selected_object_array[o], currently_selected_object_array[o] + 1); 
                                       ++currently_selected_object_array[o];
                                    }
                                 }
                              } else
                              
                              if (currently_selected_special_object_count != 0) {
                                 if (currently_selected_special_object_array[currently_selected_special_object_count - 1] != SELECTED_SPECIAL_OBJECTS_ENTRANCES_START) {
                                    for (int o = currently_selected_special_object_count - 1; o >= 0; o--) {
                                       if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                                        && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) { 
                                          SwapLevelSpecialObjects(currently_selected_special_object_array[o], currently_selected_special_object_array[o] - 1); 
                                          --currently_selected_special_object_array[o];
                                       } 
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) { 
                                       level_working_on->runtime_stats.trap_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_TRAPS_START] = TRAP_Z_FOREGROUND;
                                    } 
                                 }
                                                                    
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) { 
                                       level_working_on->runtime_stats.hazard_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_HAZARDS_START] = HAZARD_Z_FOREGROUND;
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) { 
                                       level_working_on->runtime_stats.uninteractive_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START] = UNINTERACTIVE_Z_FOREGROUND;
                                    }
                                 }
                                 
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) { 
                                       level_working_on->runtime_stats.water_z[currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_WATERS_START] = WATER_Z_FOREGROUND;
                                    } 
                                 }                                
                              }
                           } else          
                           if (page_cell == 8) {
                              if (currently_selected_object_count != 0) {
                                 for (int o = 0; o < currently_selected_object_count; o++) {
                                    int currently_selected_index = currently_selected_object_array[o]; 
                                  
                                    LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *object_selected = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_index);
                                                               
                                    if (object_selected->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                       object_selected->tox = object_selected->tox + ((object_selected->x2 - object_selected->x1) * object_selected->tsx);
                                       object_selected->tsx = 0-object_selected->tsx;
                                    }
                                 }         
                                 
                                 LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *object_selected_zero = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]);
                                    
                                 lower_interface_currently_selected_texture_adjustment_tox_value = object_selected_zero->tox;
                                 lower_interface_currently_selected_texture_adjustment_tsx_value = object_selected_zero->tsx;
                              } else
                              if (currently_selected_special_object_count != 0) {
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {  
                                       int entrance = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START; 
                                       level_working_on->runtime_stats.entrance_d[entrance] = ((level_working_on->runtime_stats.entrance_d[entrance] == 1)
                                                                                                   ? 0 : 1); 
                                    }
                                 }
                              }
                           } else
                           if (page_cell == 9) {
                              
                              SelectionClear();                                        
                              SelectionSpecialClear();
                              
                              editor_currently_selected_tool = EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK;
                           } else                 
                           if (page_cell == 10) {
                              if (currently_selected_object_count != 0) {
                                 editor_currently_selected_tool = EDITOR_TOOL_PLACE_DUPE_TEXTURE_ON_CLICK;   
                                 
                                 SelectionSpecialClear();
                              }  
                           } else
                           if (page_cell == 11) {
                              if (currently_selected_object_count != 0) {
                                 for (int o = 0; o < currently_selected_object_count; o++) {
                                    DeleteTerrainObject(currently_selected_object_array[o]);                             
                                 }                                 
                                 SelectionClear();
                              }
                              if (currently_selected_special_object_count != 0) {
                                 for (int o = 0; o < currently_selected_special_object_count; o++) {
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START) + 1); e < MAX_NO_ENTRANCES; e++) {
                                          level_working_on->runtime_stats.entrance_x[e - 1] = 
                                                 level_working_on->runtime_stats.entrance_x[e];
                                          level_working_on->runtime_stats.entrance_y[e - 1] = 
                                                 level_working_on->runtime_stats.entrance_y[e];
                                          level_working_on->runtime_stats.entrance_d[e - 1] = 
                                                 level_working_on->runtime_stats.entrance_d[e];
                                       }
                                                                        
                                       --(level_working_on->runtime_stats.no_entrances);
                                    } else
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_EXITS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_EXITS_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_EXITS_START) + 1); e < MAX_NO_EXITS; e++) {   
                                          level_working_on->runtime_stats.exit_x[e - 1] = 
                                                 level_working_on->runtime_stats.exit_x[e];
                                          level_working_on->runtime_stats.exit_y[e - 1] = 
                                                 level_working_on->runtime_stats.exit_y[e];
                                       }
                                                     
                                       --(level_working_on->runtime_stats.no_exits);
                                    } else
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_TRAPS_START) + 1); e < MAX_NO_TRAPS; e++) {
                                          level_working_on->runtime_stats.trap_x[e - 1] =
                                                 level_working_on->runtime_stats.trap_x[e];
                                          level_working_on->runtime_stats.trap_y[e - 1] = 
                                                 level_working_on->runtime_stats.trap_y[e];
                                          level_working_on->runtime_stats.trap_z[e - 1] = 
                                                 level_working_on->runtime_stats.trap_z[e];
                                          level_working_on->runtime_stats.trap_genus[e - 1] = 
                                                 level_working_on->runtime_stats.trap_genus[e];
                                       }
                                 
                                       --(level_working_on->runtime_stats.no_traps);
                                    } else
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_HAZARDS_START) + 1); e < MAX_NO_HAZARDS; e++) {
                                          level_working_on->runtime_stats.hazard_x[e - 1] =
                                                 level_working_on->runtime_stats.hazard_x[e];
                                          level_working_on->runtime_stats.hazard_y[e - 1] = 
                                                 level_working_on->runtime_stats.hazard_y[e];
                                          level_working_on->runtime_stats.hazard_z[e - 1] = 
                                                 level_working_on->runtime_stats.hazard_z[e];
                                          level_working_on->runtime_stats.hazard_genus[e - 1] = 
                                                 level_working_on->runtime_stats.hazard_genus[e];
                                       }
                                 
                                       --(level_working_on->runtime_stats.no_hazards);
                                    } else
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START) + 1); e < MAX_NO_UNINTERACTIVES; e++) {
                                          level_working_on->runtime_stats.uninteractive_x[e - 1] =
                                                 level_working_on->runtime_stats.uninteractive_x[e];
                                          level_working_on->runtime_stats.uninteractive_y[e - 1] = 
                                                 level_working_on->runtime_stats.uninteractive_y[e];
                                          level_working_on->runtime_stats.uninteractive_z[e - 1] = 
                                                 level_working_on->runtime_stats.uninteractive_z[e];
                                          level_working_on->runtime_stats.uninteractive_genus[e - 1] = 
                                                 level_working_on->runtime_stats.uninteractive_genus[e];
                                       }
                                 
                                       --(level_working_on->runtime_stats.no_uninteractives);
                                    }
                                     
                                    if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
                                     && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {
                                       for (int e = ((currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_WATERS_START) + 1); e < MAX_NO_WATERS; e++) {
                                          level_working_on->runtime_stats.water_x1[e - 1] = 
                                                 level_working_on->runtime_stats.water_x1[e];
                                          level_working_on->runtime_stats.water_x2[e - 1] = 
                                                 level_working_on->runtime_stats.water_x2[e];
                                          level_working_on->runtime_stats.water_y[e - 1] = 
                                                 level_working_on->runtime_stats.water_y[e];
                                       }
                                 
                                       --(level_working_on->runtime_stats.no_waters);
                                    } 
                                 }                              
                              }                                                
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;
                              
                              SelectionClear();                                        
                              SelectionSpecialClear();
                           }
                                           
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;    
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;                  
               } else
               if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_AREA_SETUP) {
                  if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X1)
                   && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y1)
                   && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_X2)
                   && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURES_AREAS_TOGGLE_BUTTON_RECTANGLE_Y2)) {
                     lower_interface_current_option_set = LOWER_INTERFACE_OPTION_SET_DEFAULT; 
                     
                     editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;         
                                                                          
                     SelectionClear();                                        
                     SelectionSpecialClear();        
                     
                     action_taken_this_frame = true;
                  }                                    
                  if (action_taken_this_frame) break;   
                  
    
                  int xcell, ycell;               
                                                                      
                  int page_cell;     
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;
                              SelectionAreaClear();        
                           } else             
                           if (page_cell == 2) {
                              if (currently_selected_area_object_count != 0) {
                                 for (int o = 0; o < currently_selected_area_object_count; o++) {
                                    if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START)
                                     && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {  
                                       int area = currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START; 
                                       level_working_on->runtime_stats.one_way_area_d[area] = ((level_working_on->runtime_stats.one_way_area_d[area] == 1)
                                                                                                   ? 0 : 1); 
                                    }
                                 }
                              }
                           } else  
                           if (page_cell == 3) {
                              editor_currently_selected_tool = EDITOR_TOOL_CAMERA_FOCUS_POSITION_ON_CLICK;
                              SelectionAreaClear();        
                           } else                   
                           if (page_cell == 4) {                                   
                              SelectionAreaClear();                                        
                              editor_currently_selected_tool = EDITOR_TOOL_NEW_STEEL_AREA_ON_CLICK;
                           } else                  
                           if (page_cell == 5) {
                              SelectionAreaClear();           
                              editor_currently_selected_tool = EDITOR_TOOL_NEW_ONE_WAY_AREA_ON_CLICK;
                           } else      
                           if (page_cell == 6) {
                              SelectionAreaClear();         
                              editor_currently_selected_tool = EDITOR_TOOL_SCALE_AREA_ON_CLICK;
                           } else                 
                           if (page_cell == 11) {
                              if (currently_selected_area_object_count != 0) {
                                 for (int o = 0; o < currently_selected_area_object_count; o++) {
                                    if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START)
                                     && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                                       for (int e = ((currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START) + 1); e < MAX_NO_STEEL_AREAS; e++) {   
                                          level_working_on->runtime_stats.steel_area_x1[e - 1] = 
                                                 level_working_on->runtime_stats.steel_area_x1[e];
                                          level_working_on->runtime_stats.steel_area_y1[e - 1] = 
                                                 level_working_on->runtime_stats.steel_area_y1[e];
                                          level_working_on->runtime_stats.steel_area_x2[e - 1] = 
                                                 level_working_on->runtime_stats.steel_area_x2[e];
                                          level_working_on->runtime_stats.steel_area_y2[e - 1] = 
                                                 level_working_on->runtime_stats.steel_area_y2[e];
                                       }                                                      
                                       --(level_working_on->runtime_stats.no_steel_areas);
                                    } else
                                    if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START)
                                     && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                                       for (int e = ((currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) + 1); e < MAX_NO_ONE_WAY_AREAS; e++) {   
                                          level_working_on->runtime_stats.one_way_area_x1[e - 1] = 
                                                 level_working_on->runtime_stats.one_way_area_x1[e];
                                          level_working_on->runtime_stats.one_way_area_y1[e - 1] = 
                                                 level_working_on->runtime_stats.one_way_area_y1[e];
                                          level_working_on->runtime_stats.one_way_area_x2[e - 1] = 
                                                 level_working_on->runtime_stats.one_way_area_x2[e];
                                          level_working_on->runtime_stats.one_way_area_y2[e - 1] = 
                                                 level_working_on->runtime_stats.one_way_area_y2[e];
                                          level_working_on->runtime_stats.one_way_area_d[e - 1] = 
                                                 level_working_on->runtime_stats.one_way_area_d[e];
                                       }                                                      
                                       --(level_working_on->runtime_stats.no_one_way_areas);
                                    }
                                 }                              
                              }                                                
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;
                              
                              SelectionAreaClear();        
                           }
                                           
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;    
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;                     
               }                         
            } else     
            if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION  )) {
               // Check for the tab changing triangles.
               if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y1)
                && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_CAPTION_RECTANGLE_Y2)) {
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_LEFT  )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_LEFT_TRIANGLE_X_RIGHT )) {
                     right_interface_current_tab = RangeWrap(right_interface_current_tab - 1, 4); 
                                                                                                                                         
                     action_taken_this_frame = true;
                  } else
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_LEFT )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TAB_SELECTOR_RIGHT_TRIANGLE_X_RIGHT)) {    
                     right_interface_current_tab = RangeWrap(right_interface_current_tab + 1, 4); 
                                                                                                                                       
                     action_taken_this_frame = true;
                  }
               } 
               if (action_taken_this_frame) break; 
               
               if ((right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION) 
                || (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER   ) 
                || (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR   )) { 
                  if ((mouse_x >= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION                                                                        ))
                   && (mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION                                                                        ))
                   && (mouse_x <= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_X_SIZE + 1))
                   && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELL_Y_SIZE + 1))) {
                     right_interface_currently_selected_texture_window_zoom <<= 1;
                     if (right_interface_currently_selected_texture_window_zoom == 16) {
                        right_interface_currently_selected_texture_window_zoom = 1;
                     }
                      
                     action_taken_this_frame = true;
                  } else
                  if ((mouse_x >= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION                                                                                ))
                   && (mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION                                                                                ))
                   && (mouse_x <= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_X_SIZE))
                   && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_POSITION + SCREEN_LAYOUT_RIGHT_INTERFACE_CURRENTLY_SELECTED_TEXTURE_SWAP_AXES_BOX_Y_SIZE))) {
                     if (right_interface_currently_selected_texture_swap_axes_value == 0) {
                        right_interface_currently_selected_texture_swap_axes_value = LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES;
                     } else {
                        right_interface_currently_selected_texture_swap_axes_value = 0;
                     }  
                  }
               }                         
               if (action_taken_this_frame) break; 
                
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION) { 
                  // Checking for the page changing triangles 
                  if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1)
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2)) {
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_LEFT  )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_LEFT_TRIANGLE_X_RIGHT )) {
                        if (right_interface_texture_selection_database_size == 0) {
                           right_interface_texture_selection_current_page = 0;
                        } else {
                           right_interface_texture_selection_current_page = RangeWrap(right_interface_texture_selection_current_page - 1,
                                                                                    ((right_interface_texture_selection_database_size - 1) / (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                                                                            * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y)) + 1); 
                        }                  
                                                                                                                       
                        action_taken_this_frame = true;
                     } else
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_RIGHT_TRIANGLE_X_RIGHT)) {
                        if (right_interface_texture_selection_database_size == 0) {
                           right_interface_texture_selection_current_page = 0;
                        } else {
                           right_interface_texture_selection_current_page = RangeWrap(right_interface_texture_selection_current_page + 1,
                                                                                    ((right_interface_texture_selection_database_size - 1) / (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                                                                            * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y)) + 1); 
                          
                        }                                                                                                               
                        action_taken_this_frame = true;
                     }
                  } 
                  if (action_taken_this_frame) break;
                                                        
                  // Checking for the set changing rectangle 
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X1)
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_X2)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y1)
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_SELECTION_TITLE_RECTANGLE_Y2)) {
                     right_interface_texture_selection_current_set = RangeWrap(right_interface_texture_selection_current_set + 1, 4);
                     RecalculateRightInterfaceTextureSelectionDatabaseSize();                                                                                                                    
                     action_taken_this_frame = true;
                  } 
                  if (action_taken_this_frame) break;
               
                  // Checking for the texture clicking.
                  bool texture_click_successful = false;
                                                                      
                  unsigned int this_page_cell;                         
                  unsigned int logical_real_cell;
                  int xcell, ycell;
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X; xcell++) {
                        this_page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X);
                        logical_real_cell = this_page_cell + right_interface_texture_selection_current_page * (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                                             * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y);      
                        if (logical_real_cell < right_interface_texture_selection_database_size) {
                                                                                                                
                           if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(xcell)  )
                            && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_RIGHT(xcell) )
                            && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_BOTTOM(ycell))) {
                              right_interface_currently_selected_texture      = logical_real_cell | ((right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT)
                                                                                                    ? LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT
                                                                                                    : 0); 
                              right_interface_currently_selected_texture_type = right_interface_texture_selection_current_set &~ RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT;
                              texture_click_successful = true; 
                              action_taken_this_frame = true;
                           }
                           if (texture_click_successful) break;
                        }                        
                        if (texture_click_successful) break;
                     }                   
                     if (texture_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;
               } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER) {               
                  // Checking for the palette mapper top palette clicking.
                  bool palette_click_successful = false;
                  int xcell, ycell;                              
                  
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION      )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION + 177)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION + 177)) {
                     for (ycell = 0; ycell < ((!(right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256))
                                               ? 1
                                               : SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y); ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; xcell++) { 
                           if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(xcell)  )
                            && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_RIGHT(xcell) )
                            && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_BOTTOM(ycell))) {
                              right_interface_currently_held_pal_map[xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X] = right_interface_currently_selected_palette_mapper_palette_entry;
                              palette_click_successful = true; 
                              action_taken_this_frame = true;
                           }
                           if (palette_click_successful) break;
                        }                        
                        if (palette_click_successful) break;
                     }          
                     if (action_taken_this_frame) break;
                  } else
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_X_POSITION      )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_X_POSITION + 177)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_Y_POSITION      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_BOTTOM_START_Y_POSITION + 177)) {
                     for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y; ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; xcell++) { 
                           if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_Y_BOTTOM(ycell))) {
                              if (MouseIsDown(MOUSE_RMB)) {
                                 for (int e = 0; e < 16; e++) {
                                    right_interface_currently_held_pal_map[e] = e + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; 
                                 }                                    
                                 palette_click_successful = true; 
                                 action_taken_this_frame = true;
                              } else
                              if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_LEFT(xcell) )
                               && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_BOTTOM_X_RIGHT(xcell))) {
                                 right_interface_currently_selected_palette_mapper_palette_entry = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X;
                                 palette_click_successful = true; 
                                 action_taken_this_frame = true;
                              }
                           }
                           if (palette_click_successful) break;
                        }                        
                        if (palette_click_successful) break;
                     }          
                     if (action_taken_this_frame) break;
                  }                                                                   
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1 674
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2 785
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_CAPTION_X (2 + ((SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1 + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2) / 2))
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y1 290
#define SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y2 305
                  
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X1)
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_X2)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y1)
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_RESET_BUTTON_RECTANGLE_Y2)) {
                     for (int p = 0; p < 256; p++) {
                        right_interface_currently_held_pal_map[p] = p; 
                     }
                     action_taken_this_frame = true;
                  }        
                  if (action_taken_this_frame) break;
               } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR) {                     
                  // Checking for the palette editor top palette clicking.
                  bool palette_click_successful = false;
                  int xcell, ycell; 
                  
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION      )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION + 177)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION + 177)) {
                     for (ycell = 0; ycell < (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT); ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; xcell++) { 
                           if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(xcell)  )
                            && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(xcell) )
                            && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(ycell))) {
                              int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; 
                              right_interface_currently_selected_palette_editor_left_colour = c;        
                              
                              if (c != 0) {
                                 Split15BitColour(level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour],
                                                  &right_interface_currently_selected_palette_editor_colour_r_value,
                                                  &right_interface_currently_selected_palette_editor_colour_g_value,
                                                  &right_interface_currently_selected_palette_editor_colour_b_value);
                              }  
                                                
                              palette_click_successful = true; 
                              action_taken_this_frame = true;
                           }
                           if (palette_click_successful) break;
                        }                        
                        if (palette_click_successful) break;
                     }          
                     if (action_taken_this_frame) break;
                  }

                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_LEFT     )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_X_FAR_RIGHT)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_TOP      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_ONE_WAY_PANEL_BOX_Y_BOTTOM   )) {
                     level_working_on->one_way_colour = right_interface_currently_selected_palette_editor_left_colour;    
                     
                     action_taken_this_frame = true;
                     break;
                  }

                  // Checking for the editor tool clicking.
                  bool tool_click_successful = false;
                                                                      
                  int page_cell;     
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_TOOLS_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              if ((right_interface_currently_selected_palette_editor_left_colour != 0)
                               && (right_interface_currently_selected_palette_editor_right_colour != 0)) {
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour] =
                                   level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour ];
                              }
                           } else
                           if (page_cell == 1) {           
                              if ((right_interface_currently_selected_palette_editor_left_colour != 0)
                               && (right_interface_currently_selected_palette_editor_right_colour != 0)) {
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour ] =
                                   level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour];
                              }
                           } else           
                           if (page_cell == 2) {            
                              if ((right_interface_currently_selected_palette_editor_left_colour != 0)
                               && (right_interface_currently_selected_palette_editor_right_colour != 0)) {
                                 int temp = level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour];
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour ] =
                                   level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour];
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour ] = temp;
                              }
                           } else                   
                           if (page_cell == 3) {         
                              if ((right_interface_currently_selected_palette_editor_left_colour != 0)
                               && (right_interface_currently_selected_palette_editor_right_colour != 0)) {
                                 int temp = level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour];
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour ] =
                                   level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour];
                                 level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_right_colour ] = temp;
                              }
                                                         
                              bool colour_a_originally[256];
                              for (int o = 0; o < level_working_on->no_terrain_objects; o++) {
                                 for (int c = 0; c < 256; c++) colour_a_originally[c] = false;
                                  
                                 LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object = GetTerrainObject(o); 
                                 if (object->object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                    const int no_colours = ((object->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256)
                                                              ? 256
                                                              : 16);
                                    for (int c = 0; c < no_colours; c++) {
                                       if ((((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->pal_map[c]) ==
                                                  right_interface_currently_selected_palette_editor_left_colour) {
                                          colour_a_originally[c] = true;
                                       }
                                    }
                                    for (int c = 0; c < no_colours; c++) {
                                       if ((((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->pal_map[c]) ==
                                                  right_interface_currently_selected_palette_editor_right_colour) {
                                          (((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->pal_map[c]) = right_interface_currently_selected_palette_editor_left_colour;                                                                       
                                       }  
                                       if (colour_a_originally[c]) {
                                          (((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->pal_map[c]) = right_interface_currently_selected_palette_editor_right_colour;                                                                       
                                       }  
                                    }     
                                 } else {
                                    if ((((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->colour) == 
                                                right_interface_currently_selected_palette_editor_right_colour) {
                                       (((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->colour) = right_interface_currently_selected_palette_editor_left_colour;                                                                         
                                    } else
                                    if ((((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->colour) == 
                                                right_interface_currently_selected_palette_editor_left_colour) {
                                       (((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->colour) = right_interface_currently_selected_palette_editor_right_colour;                                                                         
                                    } 
                                 } 
                              }
                           } else                   
                           if (page_cell == 4) { 
                              if ((right_interface_currently_selected_palette_editor_left_colour != 0)
                               && (right_interface_currently_selected_palette_editor_right_colour != 0)) {
                                 int colour_a = right_interface_currently_selected_palette_editor_left_colour;
                                 int colour_b = right_interface_currently_selected_palette_editor_right_colour; 
                           
                                 if (colour_a > colour_b) {
                                    colour_a ^= colour_b ^= colour_a ^= colour_b;
                                 }   
                                 int length = (colour_b - colour_a) + 1;
                                 
                                 int s_c = level_working_on->runtime_stats.level_palette[colour_a];
                                 int e_c = level_working_on->runtime_stats.level_palette[colour_b];
                              
                                 unsigned int s_c_r, s_c_g, s_c_b;
                                 unsigned int e_c_r, e_c_g, e_c_b;
                              
                                 Split15BitColour(s_c, &s_c_r, &s_c_g, &s_c_b);
                                 Split15BitColour(e_c, &e_c_r, &e_c_g, &e_c_b);
                              
                                 int delta_r = e_c_r - s_c_r;  
                                 int delta_g = e_c_g - s_c_g;
                                 int delta_b = e_c_b - s_c_b;
                              
                                 float dr = (((float)(delta_r)) / ((float)(length - 1)));
                                 float dg = (((float)(delta_g)) / ((float)(length - 1)));
                                 float db = (((float)(delta_b)) / ((float)(length - 1)));
                               
                                 //allegro_message("Between %d and %d:\n"
                                 //                "%d and %d giving red channel difference,\n"
                                 //                "%d is %f with length %d.",
                                 //                s_c,
                                 //                e_c,
                                 //                s_c_r,
                                 //                e_c_g,
                                 //                delta_r,
                                 //                dr,
                                 //                length);
                               
                                 float rr = s_c_r, rg = s_c_g, rb = s_c_b;
                              
                                 for (int c = colour_a + 1; c < colour_b; c++) {
                                    rr += dr;
                                    rg += dg;
                                    rb += db;
                                 
                                    //allegro_message("This is colour %d (iteration %d)\n"
                                    //                "dr is %f\n"
                                    //                "rr is %f\n",
                                    //                c,
                                    //                c - (colour_a + 1),
                                    //                dr,
                                    //                rr);
                                    
                                    level_working_on->runtime_stats.level_palette[c] = RGB15A(FloatRound(rr),
                                                                                FloatRound(rg),
                                                                                FloatRound(rb));
                                 }
                              }                        
                              tool_click_successful = true; 
                              action_taken_this_frame = true;  
                           } else
                           if (page_cell == 8) {
                              int a_line = right_interface_currently_selected_palette_editor_left_colour  / 16;
                              int b_line = right_interface_currently_selected_palette_editor_right_colour / 16;
                              
                              for (int c = 0; c < 16; c++) {
                                 int dst = b_line * 16 + c;
                                 int src = a_line * 16 + c;
                                 
                                 if (dst == 0) continue; 
                                 
                                 level_working_on->runtime_stats.level_palette[dst] = level_working_on->runtime_stats.level_palette[src];
                              }
                           } else
                           if (page_cell == 9) {
                              int a_line = right_interface_currently_selected_palette_editor_left_colour  / 16;
                              int b_line = right_interface_currently_selected_palette_editor_right_colour / 16;
                              
                              for (int c = 0; c < 16; c++) {
                                 int dst = a_line * 16 + c;
                                 int src = b_line * 16 + c;
                                 
                                 if (dst == 0) continue; 
                                 
                                 level_working_on->runtime_stats.level_palette[dst] = level_working_on->runtime_stats.level_palette[src];
                              }
                           }
                        }
                        if (tool_click_successful) break;    
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;    
               } else if (right_interface_current_tab == RIGHT_INTERFACE_TAB_SPECIAL_OBJECTS) {  
                  // Check for the tab changing triangles.
                  if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y1)
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_TITLE_RECTANGLE_Y2)) {
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_LEFT_TRIANGLE_X_RIGHT)) {
                        // We're moving the current special object backwards one...
                        // For entrance and exit (for which there is only one genus), we need to move the object back. 
                        // And set the genus to the max value for the object we're switching to
                        // For objects with multiple genuses, we need to check if we're on genus zero
                        // and turn the object back if that's the case (and then reset the genus if appropriate)
                        // if it's not appropriate, then decrement the genus.
                        right_interface_special_objects_current_genus--;
                        
                        if (right_interface_special_objects_current_genus == -1) {
                           right_interface_special_objects_current_object = RangeWrap(right_interface_special_objects_current_object - 1, 5); // This 5 controls the number of tabs in the special object selector
                           right_interface_special_objects_current_genus  = no_genuses_for_special_object[right_interface_special_objects_current_object] - 1;
                        }                       
                        
                        ResetSpecialObjectPalettePointerBasedOnGenus();     
                        
                        Split15BitColour(COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour),
                                         &right_interface_currently_selected_special_objects_left_colour_r_value,
                                         &right_interface_currently_selected_special_objects_left_colour_g_value,
                                         &right_interface_currently_selected_special_objects_left_colour_b_value);
                        action_taken_this_frame = true;
                     } else
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_RIGHT_TRIANGLE_X_RIGHT)) {    
                        right_interface_special_objects_current_genus++;
                        
                        if (right_interface_special_objects_current_genus == no_genuses_for_special_object[right_interface_special_objects_current_object]) {
                           right_interface_special_objects_current_object = RangeWrap(right_interface_special_objects_current_object + 1, 5); // This 5 controls the number of tabs in the special object selector
                           right_interface_special_objects_current_genus  = 0;
                        }                          
                        
                        ResetSpecialObjectPalettePointerBasedOnGenus();                                                                                                                 
                        
                        Split15BitColour(COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour),
                                         &right_interface_currently_selected_special_objects_left_colour_r_value,
                                         &right_interface_currently_selected_special_objects_left_colour_g_value,
                                         &right_interface_currently_selected_special_objects_left_colour_b_value);                                                                                                               
                        action_taken_this_frame = true;
                     }
                  } 
                  if (action_taken_this_frame) break; 
                  
                  // Check for the object junctioning changing triangles.
                  if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP   ) // Assume that both triangles have the same y coordinates
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM)) {
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT)) {
                        u32 *n; // This is the value we're mucking about with. 
                        
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                           n = &(level_working_on->runtime_stats.entrance_genus_junction);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                           n = &(level_working_on->runtime_stats.exit_genus_junction);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                           n = &(level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                           n = &(level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                           n = &(level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]);
                        } 
                        
                        int max_available_standard_type_no_of_object, max_available_custom_type_no_of_object;
                     
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                           max_available_standard_type_no_of_object = no_standard_entrances;
                           max_available_custom_type_no_of_object   = no_custom_entrances;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                           max_available_standard_type_no_of_object = no_standard_exits;
                           max_available_custom_type_no_of_object   = no_custom_exits;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                           max_available_standard_type_no_of_object = no_standard_traps;
                           max_available_custom_type_no_of_object   = no_custom_traps;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                           max_available_standard_type_no_of_object = no_standard_hazards;
                           max_available_custom_type_no_of_object   = no_custom_hazards;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                           max_available_standard_type_no_of_object = no_standard_uninteractives;
                           max_available_custom_type_no_of_object   = no_custom_uninteractives;
                        }           
                        
                        if (!((*n) & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {
                           // If we're trying to backwards from standard zero, try to hit custom max, else hit standard max.
                           if ((*n) == 0) {
                              if (max_available_custom_type_no_of_object != 0) {
                                 (*n) = (max_available_custom_type_no_of_object - 1) | LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
                              } else {
                                 (*n) = max_available_standard_type_no_of_object - 1;
                              }
                           } else {
                              (*n)--;    
                           }
                        } else {
                           // If we're trying to backwards from custom zero, hit standard max, else just decrement.    
                           if (((*n) & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT) == 0) {
                              (*n) = max_available_standard_type_no_of_object - 1;
                           } else {
                              (*n)--;    
                           }                               
                        }   
                        
                        action_taken_this_frame = true;
                     } else
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT)) {    
                        u32 *n; // This is the value we're mucking about with. 
                        
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                           n = &(level_working_on->runtime_stats.entrance_genus_junction);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                           n = &(level_working_on->runtime_stats.exit_genus_junction);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                           n = &(level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                           n = &(level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]);
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                           n = &(level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]);
                        } 
                        
                        int max_available_standard_type_no_of_object, max_available_custom_type_no_of_object;
                     
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                           max_available_standard_type_no_of_object = no_standard_entrances;
                           max_available_custom_type_no_of_object   = no_custom_entrances;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                           max_available_standard_type_no_of_object = no_standard_exits;
                           max_available_custom_type_no_of_object   = no_custom_exits;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                           max_available_standard_type_no_of_object = no_standard_traps;
                           max_available_custom_type_no_of_object   = no_custom_traps;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                           max_available_standard_type_no_of_object = no_standard_hazards;
                           max_available_custom_type_no_of_object   = no_custom_hazards;
                        } else
                        if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                           max_available_standard_type_no_of_object = no_standard_uninteractives;
                           max_available_custom_type_no_of_object   = no_custom_uninteractives;
                        }           
                        
                        if (!((*n) & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {
                           // If we're trying to forwards from standard max, try to hit custom zero, else hit standard zero.
                           if ((*n) == (max_available_standard_type_no_of_object - 1)) {
                              if (max_available_custom_type_no_of_object != 0) {
                                 (*n) = 0 | LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
                              } else {
                                 (*n) = 0;
                              }
                           } else {
                              (*n)++;    
                           }
                        } else {
                           // If we're trying to forwards from custom max, hit standard zero, else just increment.    
                           if (((*n) & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT) == (max_available_custom_type_no_of_object - 1)) {
                              (*n) = 0;
                           } else {
                              (*n)++; 
                           }                               
                        }   
                                                                                                                               
                        action_taken_this_frame = true;
                     }
                  } 
                  if (action_taken_this_frame) break;                     
                  
                  // Check for the water junctioning changing triangles.
                  if ((mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_TOP   ) // Assume that both triangles have the same y coordinates
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_Y_BOTTOM)) {
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_LEFT_TRIANGLE_X_RIGHT)) {                        
                        if (!(level_working_on->runtime_stats.water_genus_junction & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {
                           // If we're trying to backwards from standard zero, try to hit custom max, else hit standard max.
                           if (level_working_on->runtime_stats.water_genus_junction == 0) {
                              if (no_custom_waters != 0) {
                                 level_working_on->runtime_stats.water_genus_junction = (no_custom_waters - 1) | LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
                              } else {
                                 level_working_on->runtime_stats.water_genus_junction = no_standard_waters - 1;
                              }
                           } else {
                              level_working_on->runtime_stats.water_genus_junction--;    
                           }
                        } else {
                           // If we're trying to backwards from custom zero, hit standard max, else just decrement.    
                           if ((level_working_on->runtime_stats.water_genus_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT) == 0) {
                              level_working_on->runtime_stats.water_genus_junction = no_standard_waters - 1;     
                           } else {
                              level_working_on->runtime_stats.water_genus_junction--;    
                           }                               
                        }   
                        
                        action_taken_this_frame = true;
                     } else
                     if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_LEFT )
                      && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_JUNCTION_SELECTOR_RIGHT_TRIANGLE_X_RIGHT)) {    
                        if (!(level_working_on->runtime_stats.water_genus_junction & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {
                           // If we're trying to forwards from standard max, try to hit custom zero, else hit standard zero.
                           if (level_working_on->runtime_stats.water_genus_junction == (no_standard_waters - 1)) {
                              if (no_custom_waters != 0) {
                                 level_working_on->runtime_stats.water_genus_junction = 0 | LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT;
                              } else {
                                 level_working_on->runtime_stats.water_genus_junction = 0;
                              }
                           } else {
                              level_working_on->runtime_stats.water_genus_junction++;    
                           }
                        } else {
                           // If we're trying to forwards from custom max, hit standard zero, else just increment.    
                           if ((level_working_on->runtime_stats.water_genus_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT) == (no_custom_waters - 1)) {
                              level_working_on->runtime_stats.water_genus_junction = 0;
                           } else {
                              level_working_on->runtime_stats.water_genus_junction++; 
                           }                               
                        }   
                                                                                                                               
                        action_taken_this_frame = true;
                     }
                  } 
                  if (action_taken_this_frame) break;  
                         
                  // Palette stuff.        
                  bool palette_click_successful = false;                                 
                                 
                  int xcell, ycell;               
                                 
                  for (ycell = 0; ycell < 1; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X; xcell++) { 
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(ycell))) {
                           int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X; 
                           right_interface_currently_selected_special_objects_left_colour = c;   
                           
                           if (c != 0) {                                                                        
                              Split15BitColour(COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour),
                                               &right_interface_currently_selected_special_objects_left_colour_r_value,
                                               &right_interface_currently_selected_special_objects_left_colour_g_value,
                                               &right_interface_currently_selected_special_objects_left_colour_b_value);
                           }
                           palette_click_successful = true; 
                           action_taken_this_frame = true;
                        }
                        if (palette_click_successful) break;
                     }                        
                     if (palette_click_successful) break;
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_X; xcell++) { 
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(ycell))) {
                           int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X + 16;
                           right_interface_currently_selected_special_objects_left_colour = c; 
                           
                           if (c != 16) {                                                              
                              Split15BitColour(level_working_on->runtime_stats.water_palette[right_interface_currently_selected_special_objects_left_colour & 15],
                                               &right_interface_currently_selected_special_objects_left_colour_r_value,
                                               &right_interface_currently_selected_special_objects_left_colour_g_value,
                                               &right_interface_currently_selected_special_objects_left_colour_b_value);
                           }
                           palette_click_successful = true; 
                           action_taken_this_frame = true;                              
                        }
                        if (palette_click_successful) break;
                     }                        
                     if (palette_click_successful) break;
                  }          
                  if (action_taken_this_frame) break;
                  
                  
                  // Checking for the editor tool clicking.
                  bool tool_click_successful = false;
                                                                      
                  int page_cell;     
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TOOLS_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                                 if (level_working_on->runtime_stats.no_entrances != MAX_NO_ENTRANCES) {
                                    SelectionClear();                                        
                                    SelectionSpecialClear();
                              
                                    editor_currently_selected_tool = EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK;
                                 }
                              } else   
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                                 if (level_working_on->runtime_stats.no_exits != MAX_NO_EXITS) {         
                                    SelectionClear();                                        
                                    SelectionSpecialClear();
                                    
                                    editor_currently_selected_tool = EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK;
                                 }
                              } else   
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                                 if (level_working_on->runtime_stats.no_traps != MAX_NO_TRAPS) { 
                                    SelectionClear();                                        
                                    SelectionSpecialClear();
                                    
                                    editor_currently_selected_tool = EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK;
                                 }
                              } else   
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                                 if (level_working_on->runtime_stats.no_hazards != MAX_NO_HAZARDS) { 
                                    SelectionClear();                                        
                                    SelectionSpecialClear();
                                    
                                    editor_currently_selected_tool = EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK;
                                 }
                              } else   
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                                 if (level_working_on->runtime_stats.no_uninteractives != MAX_NO_UNINTERACTIVES) { 
                                    SelectionClear();                                        
                                    SelectionSpecialClear();
                                    
                                    editor_currently_selected_tool = EDITOR_TOOL_PLACE_SPEC_OBJ_ON_CLICK;
                                 }
                              }   
                           } else
                           if (page_cell == 1) {
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                                 level_working_on->runtime_stats.no_entrances = 0;  
                                 SelectionSpecialClear();
                              } else
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                                 level_working_on->runtime_stats.no_exits = 0;     
                                 SelectionSpecialClear();    
                              } else
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                                 level_working_on->runtime_stats.no_traps = 0;  
                                 SelectionSpecialClear();    
                              } else
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                                 level_working_on->runtime_stats.no_hazards = 0;  
                                 SelectionSpecialClear();    
                              } else
                              if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                                 level_working_on->runtime_stats.no_uninteractives = 0;  
                                 SelectionSpecialClear();    
                              }
                           }        
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;
                     }                        
                     if (tool_click_successful) break;
                  }                          
                  if (tool_click_successful) break;
                  for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_OBJECT_TOOLS_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              if (level_working_on->runtime_stats.no_waters != MAX_NO_WATERS) {
                                 SelectionClear();                                        
                                 SelectionSpecialClear();
                              
                                 editor_currently_selected_tool = EDITOR_TOOL_PLACE_WATER_ON_CLICK;
                              }
                           } else
                           if (page_cell == 1) {
                              level_working_on->runtime_stats.no_waters = 0;
                           }        
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;
                     }                        
                     if (tool_click_successful) break;
                  }                          
                  if (tool_click_successful) break;
                     
                  for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_TOOLS_BUTTON_CELL_Y_BOTTOM(ycell))) {                               
                           if (page_cell == 0) {
                              if ((!((right_interface_currently_selected_special_objects_left_colour == 0)
                                  || (right_interface_currently_selected_special_objects_left_colour == 16)))
                               && (!((right_interface_currently_selected_special_objects_right_colour == 0)
                                  || (right_interface_currently_selected_special_objects_right_colour == 16)))) {
                                 COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_right_colour) =
                                   COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour);
                              }
                           } else
                           if (page_cell == 1) {   
                              if ((!((right_interface_currently_selected_special_objects_left_colour == 0)
                                  || (right_interface_currently_selected_special_objects_left_colour == 16)))
                               && (!((right_interface_currently_selected_special_objects_right_colour == 0)
                                  || (right_interface_currently_selected_special_objects_right_colour == 16)))) {
                                 COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour) =
                                   COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_right_colour);
                              }
                           } else           
                           if (page_cell == 2) {   
                              int temp = COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour);
                              COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour) =
                                COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_right_colour);
                              COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_right_colour) = temp;
                           } else                   
                           if (page_cell == 3) {     
                              int colour_a = right_interface_currently_selected_special_objects_left_colour;
                              int colour_b = right_interface_currently_selected_special_objects_right_colour; 
                        
                              if (colour_a > colour_b) {
                                 colour_a ^= colour_b ^= colour_a ^= colour_b;
                              }   
                              int length = (colour_b - colour_a) + 1;
                           
                              int s_c = COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(colour_a);
                              int e_c = COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(colour_b);
                           
                              unsigned int s_c_r, s_c_g, s_c_b;
                              unsigned int e_c_r, e_c_g, e_c_b;
                           
                              Split15BitColour(s_c, &s_c_r, &s_c_g, &s_c_b);
                              Split15BitColour(e_c, &e_c_r, &e_c_g, &e_c_b);
                           
                              int delta_r = e_c_r - s_c_r;  
                              int delta_g = e_c_g - s_c_g;
                              int delta_b = e_c_b - s_c_b;
                           
                              float dr = (((float)(delta_r)) / ((float)(length - 1)));
                              float dg = (((float)(delta_g)) / ((float)(length - 1)));
                              float db = (((float)(delta_b)) / ((float)(length - 1)));
                            
                              //allegro_message("Between %d and %d:\n"
                              //                "%d and %d giving red channel difference,\n"
                              //                "%d is %f with length %d.",
                              //                s_c,
                              //                e_c,
                              //                s_c_r,
                              //                e_c_g,
                              //                delta_r,
                              //                dr,
                              //                length);
                            
                              float rr = s_c_r, rg = s_c_g, rb = s_c_b;
                           
                              for (int c = colour_a + 1; c < colour_b; c++) {
                                 rr += dr;
                                 rg += dg;
                                 rb += db;
                              
                                 //allegro_message("This is colour %d (iteration %d)\n"
                                 //                "dr is %f\n"
                                 //                "rr is %f\n",
                                 //                c,
                                 //                c - (colour_a + 1),
                                 //                dr,
                                 //                rr);
                              
                                 COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(c) = RGB15A(FloatRound(rr),
                                                                                 FloatRound(rg),
                                                                                 FloatRound(rb));
                              }    
                           } else                   
                           if (page_cell == 4) { 
                              if (right_interface_currently_selected_special_objects_left_colour < 16) {
                                 bool current_object_type_is_custom;    
                                                               
                                 int no_current_type_of_object;           
                                 int max_available_type_no_of_object;  
                                 
                                 LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_for_special_object;
                               
                                 if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_ENTRANCE) {
                                    // Get the raw number for the entrance genus junction 
                                    int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                                                       & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                     
                                    // Return the correct graphical object for the entrance based on the genus junction value.   
                                    active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                                ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                                                :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);
                                                        
                                 } else
                                 if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_EXIT) {
                                    // Get the raw number for the exit genus junction 
                                    int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                                                       & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                     
                                    // Return the correct graphical object for the exit based on the genus junction value.   
                                    active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                                ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                                                :   custom_exit_graphical_objects.at(exit_genus_junction_id);
                                 } else
                                 if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_TRAP) {
                                    // Get the raw number for the trap genus junction 
                                    int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]
                                                                       & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                     
                                    // Return the correct graphical object for the trap based on the genus junction value.   
                                    active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.trap_genus_junctions[right_interface_special_objects_current_genus]
                                                                                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                                ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                                                :   custom_trap_graphical_objects.at(trap_genus_junction_id);
                                 } else
                                 if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_HAZARD) {
                                    // Get the raw number for the hazard genus junction 
                                    int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]
                                                                       & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                     
                                    // Return the correct graphical object for the hazard based on the genus junction value.   
                                    active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[right_interface_special_objects_current_genus]
                                                                                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                                ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                                                :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);
                                 } else
                                 if (right_interface_special_objects_current_object == RIGHT_INTERFACE_SPECIAL_OBJECTS_OBJECT_UNINTERACTIVE) {
                                    // Get the raw number for the uninteractive genus junction 
                                    int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]
                                                                       & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                     
                                    // Return the correct graphical object for the uninteractive based on the genus junction value.   
                                    active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[right_interface_special_objects_current_genus]
                                                                                                              & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                                ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                                                :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);
                                 }  
                                            
                                 for (int palette_entry = 1; palette_entry < 16; palette_entry++) {
                                    COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(palette_entry) = 
                                       active_graphical_object_for_special_object->ideal_palette[palette_entry]; 
                                 }
                              } else {                                                              
                                 // Get the raw number for the water genus 
                                 int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                                                    & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                  
                                 // Return the correct graphical object for the water based on the genus junction value.   
                                 LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_for_special_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                                           & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                             ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                                                             :   custom_water_graphical_objects.at(water_genus_junction_id);
                                                     
                              
                                 
                                 for (int palette_entry = 1; palette_entry < 16; palette_entry++) {
                                    level_working_on->runtime_stats.water_palette[palette_entry] = 
                                        active_graphical_object_for_special_object->ideal_palette[palette_entry]; 
                                 }
                              }
                           }            
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;
               }
            }        
         } else 
         if (MouseRelease(MOUSE_LMB)) {
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_TEXTURE) {
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;
               
               TruncateLevelObjectsToLevel();
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_SPEC_OBJ) {
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_SPEC_OBJ;
            } else                                
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_TEXTURE) { 
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;
               
               TruncateLevelObjectsToLevel();
               
               SelectionClear();
            } else                          
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_WATER) { 
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_SPEC_OBJ;
                              
               SelectionSpecialClear();
            } else                     
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_TEXTURE_LMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK;
               
               TruncateLevelObjectsToLevel();
               
               SelectionClear();
            } else           
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_WATER_LMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK;
               
               TruncateLevelObjectsToLevel();
               
               SelectionClear();
            } else           
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_AREA) {
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;
               
               TruncateLevelAreaObjectsToLevel();
            } else      
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_AREA_LMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_AREA_ON_CLICK;
               
               TruncateLevelAreaObjectsToLevel();
               
               SelectionAreaClear();   
            } else             
            if (editor_currently_selected_tool == EDITOR_TOOL_CAMERA_FOCUS_POSITIONING_LMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_CAMERA_FOCUS_POSITION_ON_CLICK;
            } else                 
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_AREA) { 
               editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;

               TruncateLevelAreaObjectsToLevel();
               
               SelectionAreaClear();
            } else                          
            0;                    
         } else   
         if (MouseDown(MOUSE_RMB)) {          
            if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_END_Y_POSITION  )) { 
               // Checking for the editor tool clicking.  
               bool tool_click_successful = false;
               
               if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_DEFAULT) { 
                  int xcell, ycell;               
                                                                      
                  int page_cell;     
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(ycell))) {   
                           if (page_cell == 0) {
                              SelectionClear();
                              SelectionSpecialClear();
                              SelectionAreaClear();
                              
                              for (int o = 0; o < level_working_on->no_terrain_objects; o++) {
                                 SelectionAddItemNumber(o); 
                              }                                    
                                 
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;   
                           } else   
                           if (page_cell == 1) {
                              SelectionClear();     
                              SelectionSpecialClear();
                              SelectionAreaClear();
                              
                              for (int o = 0; o < level_working_on->runtime_stats.no_entrances; o++) {
                                 SelectionSpecialAddItemNumber(o + SELECTED_SPECIAL_OBJECTS_ENTRANCES_START); 
                              }      
                              for (int o = 0; o < level_working_on->runtime_stats.no_exits; o++) {
                                 SelectionSpecialAddItemNumber(o + SELECTED_SPECIAL_OBJECTS_EXITS_START); 
                              }   
                              for (int o = 0; o < level_working_on->runtime_stats.no_traps; o++) {
                                 SelectionSpecialAddItemNumber(o + SELECTED_SPECIAL_OBJECTS_TRAPS_START);
                              }   
                              for (int o = 0; o < level_working_on->runtime_stats.no_hazards; o++) {
                                 SelectionSpecialAddItemNumber(o + SELECTED_SPECIAL_OBJECTS_HAZARDS_START);
                              }   
                              for (int o = 0; o < level_working_on->runtime_stats.no_uninteractives; o++) {
                                 SelectionSpecialAddItemNumber(o + SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START);
                              }                                    
                                 
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_SPEC_OBJ;   
                           } else   
                           if (page_cell == 2) {
                              if (currently_selected_object_count != 0) {
                                 editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;   
                                 
                                 LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected =
                                      GetTerrainObject(currently_selected_object_array[0]);
                                      
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                    for (int p = 0; p < 16; p++) {
                                       right_interface_currently_held_pal_map[p] =
                                           ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p]; 
                                    }
                                 } else
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
                                    for (int p = 0; p < 256; p++) {
                                       right_interface_currently_held_pal_map[p] =
                                           ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p]; 
                                    }
                                 } else
                                 if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                    right_interface_currently_held_pal_map[0] =
                                        ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object_selected))->colour; 
                                 }
                              }  
                           } else
                           if (page_cell == 3) {   
                              if (currently_selected_object_count != 0) {
                                 editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;   
                              
                                 for (int o = 0; o < currently_selected_object_count; o++) {
                                    int currently_selected_index = currently_selected_object_array[o]; 
                                  
                                    LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object_selected = GetTerrainObject(currently_selected_index);
                                                                                                                                   
                                    if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                       for (int p = 0; p < 16; p++) {
                                          ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object_selected))->pal_map[p] = 
                                              right_interface_currently_held_pal_map[p]; 
                                       }
                                    } else
                                    if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {  
                                       for (int p = 0; p < 256; p++) {
                                          ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object_selected))->pal_map[p] = 
                                             right_interface_currently_held_pal_map[p]; 
                                       }
                                    } else
                                    if (object_selected->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                       ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object_selected))->colour = 
                                          right_interface_currently_held_pal_map[0]; 
                                    }
                                 }
                              }  
                           } else                  
                           if (page_cell == 8) {
                              if (currently_selected_object_count != 0) {
                                 for (int o = 0; o < currently_selected_object_count; o++) {
                                    int currently_selected_index = currently_selected_object_array[o]; 
                                  
                                    LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *object_selected = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_index);
                                                               
                                    if (object_selected->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                       object_selected->toy = object_selected->toy + ((object_selected->y2 - object_selected->y1) * object_selected->tsy);
                                       object_selected->tsy = 0-object_selected->tsy;
                                    }
                                 }         
                                 
                                 LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *object_selected_zero = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]);
                                    
                                 lower_interface_currently_selected_texture_adjustment_toy_value = object_selected_zero->toy;
                                 lower_interface_currently_selected_texture_adjustment_tsy_value = object_selected_zero->tsy;
                              }
                           }
                           
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;    
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;                  
               } else
               if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_AREA_SETUP) { 
                  int xcell, ycell;               
                                                                      
                  int page_cell;     
                  
                  for (ycell = 0; ycell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_Y_COUNT; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT; xcell++) {
                        page_cell = xcell + (ycell * SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_GRID_CELL_X_COUNT);
                        if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_TOOL_BUTTON_CELL_Y_BOTTOM(ycell))) {   
                           if (page_cell == 0) {
                              SelectionClear();
                              SelectionSpecialClear();
                              SelectionAreaClear();
                              
                              for (int o = 0; o < level_working_on->runtime_stats.no_steel_areas; o++) {
                                 SelectionAreaAddItemNumber(o + SELECTED_AREA_OBJECTS_STEEL_AREAS_START); 
                              }      
                              for (int o = 0; o < level_working_on->runtime_stats.no_one_way_areas; o++) {
                                 SelectionAreaAddItemNumber(o + SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START); 
                              }                                  
                                 
                              editor_currently_selected_tool = EDITOR_TOOL_SELECT_AREA;   
                           }
                           
                           tool_click_successful = true; 
                           action_taken_this_frame = true;  
                        }
                        if (tool_click_successful) break;    
                     }                        
                     if (tool_click_successful) break;
                  }                   
                  if (action_taken_this_frame) break;                  
               }               
            } else
            if ((mouse_x >= SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LEVEL_PANE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_LEVEL_PANE_END_Y_POSITION  )) {  
               bool click_successful = false;                 
                  
               int level_x, level_y;                                                             
               ConvertScreenCoordsToLevelCoords(mouse_x, mouse_y, level_x, level_y);
               
               if (editor_currently_selected_tool == EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK) {                            
                  int texture_under_cursor = TextureTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", texture_under_cursor);
                                 
                  if (texture_under_cursor != -1) {
                     editor_currently_selected_tool = EDITOR_TOOL_SCALING_TEXTURE_RMB;
                           
                     SelectionAddItemNumber(texture_under_cursor);
                     
                     LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(texture_under_cursor); 
                              
                     currently_selected_object_handles_x1[0] = generic_object_info->x2 - (mouse_x >> 1) - camera_x_inset; 
                     currently_selected_object_handles_y1[0] = generic_object_info->y2 - (mouse_y >> 1) - camera_y_inset; 
                  } else {
                     int object_under_cursor = SpecialObjectTopAtPoint(level_x, level_y); 
                     
                     if ((object_under_cursor >= SELECTED_SPECIAL_OBJECTS_WATERS_START) 
                      && (object_under_cursor <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {
                        editor_currently_selected_tool = EDITOR_TOOL_SCALING_WATER_RMB;
                        
                        SelectionSpecialAddItemNumber(object_under_cursor);  
                        
                        int water = object_under_cursor - SELECTED_SPECIAL_OBJECTS_WATERS_START;
                  
                        currently_selected_special_object_handles_x[0] = level_working_on->runtime_stats.water_x2[water] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_special_object_handles_y[0] = level_working_on->runtime_stats.water_y[water]  - (mouse_y >> 1) - camera_y_inset;                                                                            
                      }    
                  }
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
               
               if (editor_currently_selected_tool == EDITOR_TOOL_SCALE_AREA_ON_CLICK) {                            
                  int area_under_cursor = AreaTopAtPoint(level_x, level_y);     
                  //allegro_message("Ya click on %d!", area_under_cursor);
                                 
                  if (area_under_cursor != -1) {
                     editor_currently_selected_tool = EDITOR_TOOL_SCALING_AREA_RMB;
                           
                     SelectionAreaAddItemNumber(area_under_cursor);
                     
                     if ((area_under_cursor >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                      && (area_under_cursor <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                        int steel_area = area_under_cursor - SELECTED_AREA_OBJECTS_STEEL_AREAS_START; 
                                
                        currently_selected_object_handles_x1[0] = level_working_on->runtime_stats.steel_area_x2[steel_area] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_object_handles_y1[0] = level_working_on->runtime_stats.steel_area_y2[steel_area] - (mouse_y >> 1) - camera_y_inset; 
                     } else 
                     if ((area_under_cursor >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                      && (area_under_cursor <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                        int one_way_area = area_under_cursor - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START; 
                                
                        currently_selected_object_handles_x1[0] = level_working_on->runtime_stats.one_way_area_x2[one_way_area] - (mouse_x >> 1) - camera_x_inset; 
                        currently_selected_object_handles_y1[0] = level_working_on->runtime_stats.one_way_area_y2[one_way_area] - (mouse_y >> 1) - camera_y_inset; 
                     } 
                  }
                  
                  action_taken_this_frame = true;   
               }                                
               if (action_taken_this_frame) break;
            } else            
            if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION  )) {     
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION) {  
                  if (right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT) {
                     // Checking for the texture clicking.
                     bool texture_click_successful = false;
                                                                      
                     unsigned int this_page_cell;                         
                     unsigned int logical_real_cell;
                     int xcell, ycell;
                  
                     for (ycell = 0; ycell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y; ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X; xcell++) {
                           this_page_cell = xcell + (ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X);
                           logical_real_cell = this_page_cell + right_interface_texture_selection_current_page * (SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_X
                                                                                                             * SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_PREVIEW_TEXTURE_CELLS_Y);      
                           if (logical_real_cell < right_interface_texture_selection_database_size) {
                                                                                                                 
                              if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_LEFT(xcell)  )
                               && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_X_RIGHT(xcell) )
                               && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_TOP(ycell)   )
                               && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_TEXTURE_CELL_Y_BOTTOM(ycell))) {
                                 //right_interface_currently_selected_texture      = logical_real_cell | ((right_interface_texture_selection_current_set & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT)
                                 //                                                                      ? LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT
                                 //                                                                      : 0); 
                                 //right_interface_currently_selected_texture_type = right_interface_texture_selection_current_set &~ RIGHT_INTERFACE_TEXTURE_SELECTION_SET_CUSTOM_BIT;

                                 // Use logical real cell, the actual index of the texture we've located to take action!
                                 
                                 int replace_delete_result = alert3(NULL,
                                                                    "Do what with this custom texture?",
                                                                    NULL,
                                                                    "Replace", "Delete", "Cancel",
                                                                    0, 0, 0);
                                                                    
                                 if (replace_delete_result == 1) {
                                    MenuLoadImageToObjectSequence(logical_real_cell, (!(right_interface_texture_selection_current_set
                                                                                       & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_256_COLOUR_BIT))
                                                                                          ? LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16
                                                                                          : LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256,
                                                                                     true, false);
                                 } else
                                 if (replace_delete_result == 2) {
                                    int delete_result = alert(NULL,
                                                              "Really delete this custom texture?",
                                                              NULL,
                                                              "Delete it!", "Cancel!",
                                                              0, 0);
                                                               
                                    if (delete_result == 1) {
                                       int data_length;
                                       
                                       if (!(right_interface_texture_selection_current_set
                                           & RIGHT_INTERFACE_TEXTURE_SELECTION_SET_256_COLOUR_BIT)) {
                                          LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *generic_database_entry = GetAppendedTexture16(logical_real_cell);
                                          LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *next_database_entry    = GetAppendedTexture16(logical_real_cell + 1);
                                                                                    
                                          data_length = ((u8 *)(generic_database_entry)) - ((u8 *)(next_database_entry));
                                          ShiftLevelData((u8 *)next_database_entry, data_length, level_working_on->lemmings_level_file_size);
                                                                                    
                                          --level_working_on->no_appended_texture_16s;
                                          
                                          RecalculateRightInterfaceTextureSelectionDatabaseSize();
                                          RefreshAppendedTexture256sBegin();
                                          RefreshTerrainObjectsBegin();                                                  
                                          
                                          // Now it's time for me to reset any objects using this texture to default,
                                          // and then decrement any other custom image texture using objects.
                                          
                                          LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info;
                                          int object_id_no_custom;
                                          
                                          for (int o = 0; o < level_working_on->no_terrain_objects; o++) {
                                             generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(o);
                                             
                                             if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
                                                continue;
                                             }
                                             
                                             if (generic_object_info->object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT) {
                                                object_id_no_custom = generic_object_info->object_id & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                                                if (object_id_no_custom == logical_real_cell) {
                                                   generic_object_info->object_id = 0;
                                                } else
                                                if (object_id_no_custom > logical_real_cell) {
                                                   generic_object_info->object_id = (object_id_no_custom - 1) | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                                                }
                                             }
                                          }
                                          
                                          if ((right_interface_currently_selected_texture      == (logical_real_cell | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) 
                                           && (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16                )) {  
                                             right_interface_currently_selected_texture = 0;  
                                          } else
                                          if ((right_interface_currently_selected_texture      >  (logical_real_cell | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) 
                                           && (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16                )) {  
                                             --right_interface_currently_selected_texture; 
                                          }                                       
                                       } else {
                                          LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *generic_database_entry = GetAppendedTexture256(logical_real_cell);
                                          LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *next_database_entry    = GetAppendedTexture256(logical_real_cell + 1);
                                          
                                          data_length = ((u8 *)(generic_database_entry)) - ((u8 *)(next_database_entry));
                                          
                                          ShiftLevelData((u8 *)next_database_entry, data_length, level_working_on->lemmings_level_file_size);
                                                                                    
                                          --level_working_on->no_appended_texture_256s;
                                          
                                          RecalculateRightInterfaceTextureSelectionDatabaseSize();
                                          RefreshAppendedTexture256sBegin();
                                          RefreshTerrainObjectsBegin();
                                                                                    
                                          LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *generic_object_info;
                                          int object_id_no_custom;
                                          
                                          for (int o = 0; o < level_working_on->no_terrain_objects; o++) {
                                             generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)GetTerrainObject(o);
                                             
                                             if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
                                                continue;
                                             }
                                             
                                             if (generic_object_info->object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT) {
                                                object_id_no_custom = generic_object_info->object_id & ~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                                                if (object_id_no_custom == logical_real_cell) {
                                                   generic_object_info->object_id = 0;
                                                } else
                                                if (object_id_no_custom > logical_real_cell) {
                                                   generic_object_info->object_id = (object_id_no_custom - 1) | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT;
                                                }
                                             }
                                          }
                                                     
                                          if ((right_interface_currently_selected_texture      == (logical_real_cell | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) 
                                           && (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256                )) {  
                                             right_interface_currently_selected_texture = 0;
                                          } else
                                          if ((right_interface_currently_selected_texture      >  (logical_real_cell | LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT)) 
                                           && (right_interface_currently_selected_texture_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256                )) {  
                                             --right_interface_currently_selected_texture;
                                          } 
                                       } 
                                    }
                                 }                             

                                 texture_click_successful = true; 
                                 action_taken_this_frame = true;
                              }
                              if (texture_click_successful) break;
                           }                        
                           if (texture_click_successful) break;
                        }                   
                        if (texture_click_successful) break;
                     }                   
                     if (action_taken_this_frame) break;
                  }
               } else
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER) {
                  // Checking for the palette mapper top palette clicking.
                  bool palette_click_successful = false;
                  int xcell, ycell; 
                  
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION      )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_X_POSITION + 177)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_MAPPED_PALETTE_GRID_TOP_START_Y_POSITION + 177)) {
                     for (ycell = 0; ycell < (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_Y); ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X; xcell++) { 
                           if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_LEFT(xcell)  )
                            && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_X_RIGHT(xcell) )
                            && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_TOP_Y_BOTTOM(ycell))) {
                              right_interface_currently_selected_palette_mapper_palette_entry = right_interface_currently_held_pal_map[xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_MAPPER_PALETTE_ENTRY_CELLS_X];
                              palette_click_successful = true; 
                              action_taken_this_frame = true;
                           }
                           if (palette_click_successful) break;
                        }                        
                        if (palette_click_successful) break;
                     }          
                     if (action_taken_this_frame) break;
                  }                  
               } else
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR) {                     
                  // Checking for the palette editor top palette clicking.
                  bool palette_click_successful = false;
                  int xcell, ycell; 
                  
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION      )
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_X_POSITION + 177)
                   && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION      )
                   && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_START_Y_POSITION + 177)) {
                     for (ycell = 0; ycell < (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_Y_COUNT); ycell++) {
                        for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; xcell++) { 
                           if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_LEFT(xcell)  )
                            && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_X_RIGHT(xcell) )
                            && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_TOP(ycell)   )
                            && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_ENTRY_Y_BOTTOM(ycell))) {               
                              int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_PALETTE_GRID_CELL_X_COUNT; 
                              right_interface_currently_selected_palette_editor_right_colour = c;    
                              
                              palette_click_successful = true; 
                              action_taken_this_frame = true;
                           }
                           if (palette_click_successful) break;
                        }
                        if (palette_click_successful) break; 
                     }          
                     if (action_taken_this_frame) break;
                  }
               } else                        
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_SPECIAL_OBJECTS) {      
                  bool palette_click_successful = false;
                  int xcell, ycell;
                     
                  for (ycell = 0; ycell < 1; ycell++) {
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X; xcell++) { 
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_Y_BOTTOM(ycell))) {
                           int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X;
                           right_interface_currently_selected_special_objects_right_colour = c; 
                              
                           if (c != 0) {                                                              
                              Split15BitColour(COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_right_colour),
                                               &right_interface_currently_selected_special_objects_left_colour_r_value,
                                               &right_interface_currently_selected_special_objects_left_colour_g_value,
                                               &right_interface_currently_selected_special_objects_left_colour_b_value);
                           }
                           palette_click_successful = true; 
                           action_taken_this_frame = true;                                  
                        }
                        if (palette_click_successful) break;
                     }                        
                     if (palette_click_successful) break;       
                     for (xcell = 0; xcell < SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_CELLS_X; xcell++) { 
                        if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_LEFT(xcell)  )
                         && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_X_RIGHT(xcell) )
                         && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_TOP(ycell)   )
                         && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_WATER_PALETTE_STRIP_Y_BOTTOM(ycell))) {
                           int c = xcell + ycell * SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_PALETTE_STRIP_CELLS_X + 16;
                           right_interface_currently_selected_special_objects_right_colour = c; 

                           if (c != 16) {                              
                              Split15BitColour(level_working_on->runtime_stats.water_palette[right_interface_currently_selected_special_objects_right_colour & 15],
                                               &right_interface_currently_selected_special_objects_left_colour_r_value,
                                               &right_interface_currently_selected_special_objects_left_colour_g_value,
                                               &right_interface_currently_selected_special_objects_left_colour_b_value);
                           }   
                           palette_click_successful = true; 
                           action_taken_this_frame = true;
                        }
                        if (palette_click_successful) break;
                     }                        
                     if (palette_click_successful) break;
                  }          
                  if (action_taken_this_frame) break;        
               }   
            }
         } else          
         if (MouseRelease(MOUSE_RMB)) {
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_TEXTURE_RMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK;
               
               TruncateLevelObjectsToLevel();
               
               SelectionClear();
            } else                 
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_WATER_RMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_TEXTURE_ON_CLICK;
               
               TruncateLevelObjectsToLevel();
               
               SelectionClear();
            } else            
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_AREA_RMB) { 
               editor_currently_selected_tool = EDITOR_TOOL_SCALE_AREA_ON_CLICK;
               
               TruncateLevelObjectsToLevel();
               
               SelectionAreaClear();
            } else                 
            0;                    
         } else   
         if (MouseIsDown(MOUSE_LMB)) {
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_TEXTURE) {
               for (int o = 0; o < currently_selected_object_count; o++) {
                  LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]);
                  
                  generic_object_info->x1 = currently_selected_object_handles_x1[o] + (mouse_x >> 1) + camera_x_inset;                 
                  generic_object_info->x2 = currently_selected_object_handles_x2[o] + (mouse_x >> 1) + camera_x_inset;                  
                  generic_object_info->y1 = currently_selected_object_handles_y1[o] + (mouse_y >> 1) + camera_y_inset;                 
                  generic_object_info->y2 = currently_selected_object_handles_y2[o] + (mouse_y >> 1) + camera_y_inset;                 
               }
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_SPEC_OBJ) {
               int item;
               
               for (int o = 0; o < currently_selected_special_object_count; o++) {
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_ENTRANCES_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_ENTRANCES_END  )) {
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_ENTRANCES_START;                                        
                     level_working_on->runtime_stats.entrance_x[item] = currently_selected_special_object_handles_x[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.entrance_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  } else
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_EXITS_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_EXITS_END  )) {      
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_EXITS_START;                                        
                     level_working_on->runtime_stats.exit_x[item] = currently_selected_special_object_handles_x[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.exit_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  } else
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_TRAPS_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_TRAPS_END  )) {  
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_TRAPS_START;                                        
                     level_working_on->runtime_stats.trap_x[item] = currently_selected_special_object_handles_x[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.trap_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  } else
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_HAZARDS_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_HAZARDS_END  )) {  
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_HAZARDS_START;                                        
                     level_working_on->runtime_stats.hazard_x[item] = currently_selected_special_object_handles_x[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.hazard_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  } else
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_END  )) {  
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_UNINTERACTIVES_START;                                        
                     level_working_on->runtime_stats.uninteractive_x[item] = currently_selected_special_object_handles_x[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.uninteractive_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  }     
                  if ((currently_selected_special_object_array[o] >= SELECTED_SPECIAL_OBJECTS_WATERS_START)
                   && (currently_selected_special_object_array[o] <= SELECTED_SPECIAL_OBJECTS_WATERS_END  )) {  
                     item = currently_selected_special_object_array[o] - SELECTED_SPECIAL_OBJECTS_WATERS_START;                                        
                     level_working_on->runtime_stats.water_x1[item] = currently_selected_special_object_handles_x[o]  + (mouse_x >> 1) + camera_x_inset; 
                     level_working_on->runtime_stats.water_x2[item] = currently_selected_special_object_handles_x2[o] + (mouse_x >> 1) + camera_x_inset;                              
                     level_working_on->runtime_stats.water_y[item] = currently_selected_special_object_handles_y[o] + (mouse_y >> 1) + camera_y_inset; 
                  }     
               }
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_TEXTURE) {  
               LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]);
                                 
               generic_object->x2 = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
               generic_object->y2 = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
               
               if (generic_object->x2 < generic_object->x1) generic_object->x1 = generic_object->x2;
               if (generic_object->y2 < generic_object->y1) generic_object->y1 = generic_object->y2;  
               
               action_taken_this_frame = true;    
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_WATER) { 
               int water = level_working_on->runtime_stats.no_waters - 1;
               
               level_working_on->runtime_stats.water_x2[water] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
               level_working_on->runtime_stats.water_y[water]  = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
                            
               if (level_working_on->runtime_stats.water_x2[water]
                 < level_working_on->runtime_stats.water_x1[water])
                      level_working_on->runtime_stats.water_x1[water]
                    = level_working_on->runtime_stats.water_x2[water];
               
               action_taken_this_frame = true;    
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_TEXTURE_LMB) {  
               LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]);
                                 
               generic_object->x1 = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
               generic_object->y1 = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
               if (generic_object->x2 < generic_object->x1) generic_object->x1 = generic_object->x2;
               if (generic_object->y2 < generic_object->y1) generic_object->y1 = generic_object->y2;  
               
               action_taken_this_frame = true;    
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_WATER_LMB) {                                   
               int water = currently_selected_special_object_array[0] - SELECTED_SPECIAL_OBJECTS_WATERS_START;
               
               level_working_on->runtime_stats.water_x1[water] = currently_selected_special_object_handles_x[0] + (mouse_x >> 1) + camera_x_inset;
               level_working_on->runtime_stats.water_y[water]  = currently_selected_special_object_handles_y[0] + (mouse_y >> 1) + camera_y_inset;
                            
               if (level_working_on->runtime_stats.water_x2[water]
                 < level_working_on->runtime_stats.water_x1[water])
                      level_working_on->runtime_stats.water_x1[water]
                    = level_working_on->runtime_stats.water_x2[water]; 
               
               action_taken_this_frame = true;    
            } else                                                                
            if (editor_currently_selected_tool == EDITOR_TOOL_MOVING_AREA) {
               for (int o = 0; o < currently_selected_area_object_count; o++) {
                  if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                   && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                     int steel_area = currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START; 
                     level_working_on->runtime_stats.steel_area_x1[steel_area] = currently_selected_object_handles_x1[o] + (mouse_x >> 1) + camera_x_inset;                 
                     level_working_on->runtime_stats.steel_area_x2[steel_area] = currently_selected_object_handles_x2[o] + (mouse_x >> 1) + camera_x_inset;                  
                     level_working_on->runtime_stats.steel_area_y1[steel_area] = currently_selected_object_handles_y1[o] + (mouse_y >> 1) + camera_y_inset;                 
                     level_working_on->runtime_stats.steel_area_y2[steel_area] = currently_selected_object_handles_y2[o] + (mouse_y >> 1) + camera_y_inset;                 
                  } else
                  if ((currently_selected_area_object_array[o] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                   && (currently_selected_area_object_array[o] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                     int one_way_area = currently_selected_area_object_array[o] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START; 
                     level_working_on->runtime_stats.one_way_area_x1[one_way_area] = currently_selected_object_handles_x1[o] + (mouse_x >> 1) + camera_x_inset;                 
                     level_working_on->runtime_stats.one_way_area_x2[one_way_area] = currently_selected_object_handles_x2[o] + (mouse_x >> 1) + camera_x_inset;                  
                     level_working_on->runtime_stats.one_way_area_y1[one_way_area] = currently_selected_object_handles_y1[o] + (mouse_y >> 1) + camera_y_inset;                 
                     level_working_on->runtime_stats.one_way_area_y2[one_way_area] = currently_selected_object_handles_y2[o] + (mouse_y >> 1) + camera_y_inset;                 
                  } 
               }
            } else   
            if (editor_currently_selected_tool == EDITOR_TOOL_PLACING_AREA) { 
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) {
                  int steel_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START;
                  level_working_on->runtime_stats.steel_area_x2[steel_area] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.steel_area_y2[steel_area] = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
               
                  if (level_working_on->runtime_stats.steel_area_x2[steel_area]
                      < level_working_on->runtime_stats.steel_area_x1[steel_area])
                     level_working_on->runtime_stats.steel_area_x1[steel_area] =
                        level_working_on->runtime_stats.steel_area_x2[steel_area];
                  if (level_working_on->runtime_stats.steel_area_y2[steel_area]
                      < level_working_on->runtime_stats.steel_area_y1[steel_area])
                     level_working_on->runtime_stats.steel_area_y1[steel_area] =
                        level_working_on->runtime_stats.steel_area_y2[steel_area];  
               } else    
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START)
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {         
                  int one_way_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START;
                  level_working_on->runtime_stats.one_way_area_x2[one_way_area] = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_START_X_POSITION) >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.one_way_area_y2[one_way_area] = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_START_Y_POSITION) >> 1) + camera_y_inset;
                
                  if (level_working_on->runtime_stats.one_way_area_x2[one_way_area]
                      < level_working_on->runtime_stats.one_way_area_x1[one_way_area])
                     level_working_on->runtime_stats.one_way_area_x1[one_way_area] =
                        level_working_on->runtime_stats.one_way_area_x2[one_way_area];
                  if (level_working_on->runtime_stats.one_way_area_y2[one_way_area]
                      < level_working_on->runtime_stats.one_way_area_y1[one_way_area])
                     level_working_on->runtime_stats.one_way_area_y1[one_way_area] =
                        level_working_on->runtime_stats.one_way_area_y2[one_way_area];  
               }     
               
               action_taken_this_frame = true;    
            } else                                    
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_AREA_LMB) { 
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) { 
                  int steel_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START;
                   
                  level_working_on->runtime_stats.steel_area_x1[steel_area] = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.steel_area_y1[steel_area] = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
                   if (level_working_on->runtime_stats.steel_area_x2[steel_area]
                       < level_working_on->runtime_stats.steel_area_x1[steel_area])
                      level_working_on->runtime_stats.steel_area_x1[steel_area] =
                         level_working_on->runtime_stats.steel_area_x2[steel_area];
                   if (level_working_on->runtime_stats.steel_area_y2[steel_area]
                       < level_working_on->runtime_stats.steel_area_y1[steel_area])
                      level_working_on->runtime_stats.steel_area_y1[steel_area] =
                         level_working_on->runtime_stats.steel_area_y2[steel_area];  
               } else
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                  int one_way_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START;
                  
                  level_working_on->runtime_stats.one_way_area_x1[one_way_area] = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.one_way_area_y1[one_way_area] = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
                   if (level_working_on->runtime_stats.one_way_area_x2[one_way_area]
                       < level_working_on->runtime_stats.one_way_area_x1[one_way_area])
                      level_working_on->runtime_stats.one_way_area_x1[one_way_area] =
                         level_working_on->runtime_stats.one_way_area_x2[one_way_area];
                   if (level_working_on->runtime_stats.one_way_area_y2[one_way_area]
                       < level_working_on->runtime_stats.one_way_area_y1[one_way_area])
                      level_working_on->runtime_stats.one_way_area_y1[one_way_area] =
                         level_working_on->runtime_stats.one_way_area_y2[one_way_area];  
               }
               
               action_taken_this_frame = true;    
            } else                      
            if (editor_currently_selected_tool == EDITOR_TOOL_CAMERA_FOCUS_POSITIONING_LMB) { 
               level_working_on->runtime_stats.camera_x = ((mouse_x - SCREEN_LAYOUT_LEVEL_PANE_X_POSITION) >> 1) + camera_x_inset;
               level_working_on->runtime_stats.camera_y = ((mouse_y - SCREEN_LAYOUT_LEVEL_PANE_Y_POSITION) >> 1) + camera_y_inset;

               action_taken_this_frame = true;    
            } else
            if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_END_Y_POSITION  )) {   
               if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION)
                && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION)
                && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_END     )
                && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_END     )) {  
                  int mouse_inset_x = mouse_x - SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION; 
                  int mouse_inset_y = mouse_y - SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION;
                  
                  int tool = mouse_inset_x / SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH;
                  
                  ++lemming_tool_bonk_timer %= LEMMING_TOOL_BONK_LENGTH;
                  
                  if (lemming_tool_bonk_timer == 0) {
                     if (tool == 0) {
                        if (level_working_on->runtime_stats.release_rate != RELEASE_RATE_MAXIMUM) {
                           ++level_working_on->runtime_stats.release_rate;
                        }
                     } else {
                        if (level_working_on->runtime_stats.tool_complement[tool - 1] != 100) {
                           ++level_working_on->runtime_stats.tool_complement[tool - 1];
                        }
                     }
                  }       
                  action_taken_this_frame = true;
               } 
               
               if (action_taken_this_frame) break;
               
               if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_DEFAULT) {
                  if (currently_selected_object_count != 0) {
                     if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START)
                      && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_END  )) {
                        int offset_from_left = mouse_x - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_START;
                        int x_zone_t = ((offset_from_left * SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_TOP)    / SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_LENGTH) - (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_TOP   /2);
                        int x_zone_b = ((offset_from_left * SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_BOTTOM) / SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_X_LENGTH) - (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_NO_ZONES_BOTTOM/2);

                        if (MouseIsDown(MOUSE_RMB)) {
                           if (x_zone_t > 0) {
                              x_zone_t =  ClosestPowerOfTwo( x_zone_t);
                           } else {
                              x_zone_t = -ClosestPowerOfTwo(-x_zone_t);
                           }
                           if (x_zone_b > 0) {
                              x_zone_b =  ClosestPowerOfTwo( x_zone_b);
                           } else {
                              x_zone_b = -ClosestPowerOfTwo(-x_zone_b);
                           }
                        }

                        if ((mouse_y >= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y
                                         - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOX_Y
                                         + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))) {
                           lower_interface_currently_selected_texture_adjustment_tox_value = x_zone_t;
                           for (int o = 0; o < currently_selected_object_count; o++) {
                              LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                              if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                 generic_object_info->tox = lower_interface_currently_selected_texture_adjustment_tox_value;
                              }
                           }              
                        } else 
                        if ((mouse_y >= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y
                                         - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TOY_Y
                                         + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))) {
                           lower_interface_currently_selected_texture_adjustment_toy_value = x_zone_t;
                           for (int o = 0; o < currently_selected_object_count; o++) {
                              LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                              if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                 generic_object_info->toy = lower_interface_currently_selected_texture_adjustment_toy_value;
                              }
                           }              
                        } else 
                        if ((mouse_y >= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y
                                         - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSX_Y
                                         + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))) {
                           lower_interface_currently_selected_texture_adjustment_tsx_value = x_zone_b;
                           for (int o = 0; o < currently_selected_object_count; o++) {
                              LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                              if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                 generic_object_info->tsx = lower_interface_currently_selected_texture_adjustment_tsx_value;
                              }
                           }              
                        } else 
                        if ((mouse_y >= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y
                                         - SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_TSY_Y
                                         + SCREEN_LAYOUT_LOWER_INTERFACE_TEXTURE_ADJUSTMENT_PROPERTY_SLIDER_Y_RANGE))) {
                           lower_interface_currently_selected_texture_adjustment_tsy_value = x_zone_b;
                           for (int o = 0; o < currently_selected_object_count; o++) {
                              LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object_info = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[o]); 
                              if (generic_object_info->object_header.object_type != LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
                                 generic_object_info->tsy = lower_interface_currently_selected_texture_adjustment_tsy_value;
                              }
                           }              
                        }
                     }
                     if (action_taken_this_frame) break;
                  }
               }
            } else
            if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_RIGHT_INTERFACE_START_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_X_POSITION  )
             && (mouse_y <= SCREEN_LAYOUT_RIGHT_INTERFACE_END_Y_POSITION  )) {           
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_TEXTURE_SELECTION) { 
               } else
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_MAPPER) {
               } else
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_PALETTE_EDITOR) {          
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START)
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_END  )) {
                     if (right_interface_currently_selected_palette_editor_left_colour != 0) { 
                        int offset_from_left = mouse_x - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_START;
                        int x_zone = (offset_from_left * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_NO_ZONES) / SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_X_LENGTH;
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_R_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_palette_editor_colour_r_value = x_zone;
                           level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour] = RGB15A(right_interface_currently_selected_palette_editor_colour_r_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_g_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_b_value);                
                        } else  
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_G_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_palette_editor_colour_g_value = x_zone;
                           level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour] = RGB15A(right_interface_currently_selected_palette_editor_colour_r_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_g_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_b_value);                
                        } else  
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))
                        && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_B_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_palette_editor_colour_b_value = x_zone;
                           level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour] = RGB15A(right_interface_currently_selected_palette_editor_colour_r_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_g_value,
                                                                                                                                   right_interface_currently_selected_palette_editor_colour_b_value);                
                        }
                     }          
                  }
                  if (action_taken_this_frame) break;
               } else
               if (right_interface_current_tab == RIGHT_INTERFACE_TAB_SPECIAL_OBJECTS) {          
                  if ((mouse_x >= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START)
                   && (mouse_x <= SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_END  )) {
                     if ((right_interface_currently_selected_special_objects_left_colour != 0)
                      && (right_interface_currently_selected_special_objects_left_colour != 16)) { 
                        int offset_from_left = mouse_x - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_START;
                        int x_zone = (offset_from_left * SCREEN_LAYOUT_RIGHT_INTERFACE_PALETTE_EDITOR_CHANNEL_SLIDER_NO_ZONES) / SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_X_LENGTH;
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_R_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_special_objects_left_colour_r_value = x_zone;
                           COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour)
                              = RGB15A(right_interface_currently_selected_special_objects_left_colour_r_value,
                                       right_interface_currently_selected_special_objects_left_colour_g_value,
                                       right_interface_currently_selected_special_objects_left_colour_b_value);                
                        } else  
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_G_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_special_objects_left_colour_g_value = x_zone;
                           COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour)
                              = RGB15A(right_interface_currently_selected_special_objects_left_colour_r_value,
                                       right_interface_currently_selected_special_objects_left_colour_g_value,
                                       right_interface_currently_selected_special_objects_left_colour_b_value);                
                        } else  
                        if ((mouse_y >= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y
                                         - SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))
                         && (mouse_y <= (SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_B_Y
                                         + SCREEN_LAYOUT_RIGHT_INTERFACE_SPECIAL_OBJECTS_CHANNEL_SLIDER_Y_RANGE))) {
                           right_interface_currently_selected_special_objects_left_colour_b_value = x_zone;
                           COLOUR_REFERRED_TO_BY_SPECIAL_INDEX(right_interface_currently_selected_special_objects_left_colour)
                              = RGB15A(right_interface_currently_selected_special_objects_left_colour_r_value,
                                       right_interface_currently_selected_special_objects_left_colour_g_value,
                                       right_interface_currently_selected_special_objects_left_colour_b_value);                
                        }          
                     }
                  }
                  if (action_taken_this_frame) break;
               }
            } 
            if (action_taken_this_frame) break; 
         } else   
         if (MouseIsDown(MOUSE_RMB)) {
            if ((mouse_x >= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION)
             && (mouse_y >= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION)
             && (mouse_x <= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_END     )
             && (mouse_y <= SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_END     )) {  
               int mouse_inset_x = mouse_x - SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_X_POSITION; 
               int mouse_inset_y = mouse_y - SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_INTERFACE_Y_POSITION;
                  
               int tool = mouse_inset_x / SCREEN_LAYOUT_LOWER_INTERFACE_LEMMING_TOOL_WIDTH;
                  
               ++lemming_tool_bonk_timer %= LEMMING_TOOL_BONK_LENGTH;
                 
               if (lemming_tool_bonk_timer == 0) {
                  if (tool == 0) {
                     if (level_working_on->runtime_stats.release_rate != RELEASE_RATE_MINIMUM) {
                        --level_working_on->runtime_stats.release_rate;
                     }
                  } else {
                     if (level_working_on->runtime_stats.tool_complement[tool - 1] != 0) {
                        --level_working_on->runtime_stats.tool_complement[tool - 1];
                     }
                  }
               } 
               action_taken_this_frame = true;
            } 
                            
            if (action_taken_this_frame) break; 
            
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_TEXTURE_RMB) {  
               LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *generic_object = (LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)GetTerrainObject(currently_selected_object_array[0]);
                                 
               generic_object->x2 = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
               generic_object->y2 = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
               if (generic_object->x2 < generic_object->x1) generic_object->x2 = generic_object->x1;
               if (generic_object->y2 < generic_object->y1) generic_object->y2 = generic_object->y1;      
               
               action_taken_this_frame = true;
            } else            
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_WATER_RMB) {                                   
               int water = currently_selected_special_object_array[0] - SELECTED_SPECIAL_OBJECTS_WATERS_START;
               
               level_working_on->runtime_stats.water_x2[water] = currently_selected_special_object_handles_x[0] + (mouse_x >> 1) + camera_x_inset;
               level_working_on->runtime_stats.water_y[water]  = currently_selected_special_object_handles_y[0] + (mouse_y >> 1) + camera_y_inset;
                            
               if (level_working_on->runtime_stats.water_x2[water]
                 < level_working_on->runtime_stats.water_x1[water])
                      level_working_on->runtime_stats.water_x1[water]
                    = level_working_on->runtime_stats.water_x2[water]; 
               
               action_taken_this_frame = true;    
            } else
            if (editor_currently_selected_tool == EDITOR_TOOL_SCALING_AREA_RMB) { 
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_STEEL_AREAS_START) 
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_STEEL_AREAS_END  )) { 
                  int steel_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_STEEL_AREAS_START;
                   
                  level_working_on->runtime_stats.steel_area_x2[steel_area] = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.steel_area_y2[steel_area] = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
                   if (level_working_on->runtime_stats.steel_area_x2[steel_area]
                       < level_working_on->runtime_stats.steel_area_x1[steel_area])
                      level_working_on->runtime_stats.steel_area_x1[steel_area] =
                         level_working_on->runtime_stats.steel_area_x2[steel_area];
                   if (level_working_on->runtime_stats.steel_area_y2[steel_area]
                       < level_working_on->runtime_stats.steel_area_y1[steel_area])
                      level_working_on->runtime_stats.steel_area_y1[steel_area] =
                         level_working_on->runtime_stats.steel_area_y2[steel_area];  
               } else
               if ((currently_selected_area_object_array[0] >= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) 
                && (currently_selected_area_object_array[0] <= SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_END  )) {
                  int one_way_area = currently_selected_area_object_array[0] - SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START;
                  
                  level_working_on->runtime_stats.one_way_area_x2[one_way_area] = currently_selected_object_handles_x1[0] + (mouse_x >> 1) + camera_x_inset;
                  level_working_on->runtime_stats.one_way_area_y2[one_way_area] = currently_selected_object_handles_y1[0] + (mouse_y >> 1) + camera_y_inset;
               
                   if (level_working_on->runtime_stats.one_way_area_x2[one_way_area]
                       < level_working_on->runtime_stats.one_way_area_x1[one_way_area])
                      level_working_on->runtime_stats.one_way_area_x1[one_way_area] =
                         level_working_on->runtime_stats.one_way_area_x2[one_way_area];
                   if (level_working_on->runtime_stats.one_way_area_y2[one_way_area]
                       < level_working_on->runtime_stats.one_way_area_y1[one_way_area])
                      level_working_on->runtime_stats.one_way_area_y1[one_way_area] =
                         level_working_on->runtime_stats.one_way_area_y2[one_way_area];  
               } 
               
               action_taken_this_frame = true;    
            }
            
            if (action_taken_this_frame) break;
            
         }
      } while (0); // This is where all of the breaks lead to!!
                     
      // Let's change the level here. 
                             
      // Rendering         
      RenderLevel(level_data, level_working_on, loaded_active_texture_archive);       
                                                          
      for (int y = 0; y < DSX_BACKBUFFER_Y_SIZE; y++) {
         for (int x = 0; x < DSX_BACKBUFFER_X_SIZE; x++) {
            DSX_backbuff[x][y] = 0;
         }  
      }                      
                    
      for (int e = 0; e < level_working_on->runtime_stats.no_traps; e++) {
         if (level_working_on->runtime_stats.trap_z[e] != TRAP_Z_BACKGROUND) continue;
          
         // Get the raw number for the trap genus junction 
         int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the trap based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_objects.at(trap_genus_junction_id);    
           
         // Return the correct graphical object sprite for the trap based on the genus junction value.   
         DSX_SPRITE *active_trap_graphical_object_sprite = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_object_sprites.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_object_sprites.at(trap_genus_junction_id);    
           
         DSX_DrawTrap(active_trap_graphical_object,
                      active_trap_graphical_object_sprite,
                      active_trap_graphical_object->representing_frame,    
                      false,
                      level_working_on->runtime_stats.trap_x[e],
                      level_working_on->runtime_stats.trap_y[e],
                      level_working_on->runtime_stats.trap_genus_palettes[level_working_on->runtime_stats.trap_genus[e]]);
      }    
           
      for (int e = 0; e < level_working_on->runtime_stats.no_hazards; e++) {     
         if (level_working_on->runtime_stats.hazard_z[e] != HAZARD_Z_BACKGROUND) continue;
         
         // Get the raw number for the hazard genus junction 
         int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the hazard based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);    
           
         // Return the correct graphical object sprite for the hazard based on the genus junction value.   
         DSX_SPRITE *active_hazard_graphical_object_sprite = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_object_sprites.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_object_sprites.at(hazard_genus_junction_id);    
           
         DSX_DrawHazard(active_hazard_graphical_object,
                      active_hazard_graphical_object_sprite,
                      active_hazard_graphical_object->representing_frame,    
                      false,
                      level_working_on->runtime_stats.hazard_x[e],
                      level_working_on->runtime_stats.hazard_y[e],
                      level_working_on->runtime_stats.hazard_genus_palettes[level_working_on->runtime_stats.hazard_genus[e]]);
      }         
      
      for (int e = 0; e < level_working_on->runtime_stats.no_uninteractives; e++) {    
         if (level_working_on->runtime_stats.uninteractive_z[e] != UNINTERACTIVE_Z_BACKGROUND) continue;
         
         // Get the raw number for the uninteractive genus junction 
         int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the uninteractive based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);    
           
         // Return the correct graphical object sprite for the uninteractive based on the genus junction value.   
         DSX_SPRITE *active_uninteractive_graphical_object_sprite = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id);    
           
         DSX_DrawUninteractive(active_uninteractive_graphical_object,
                        active_uninteractive_graphical_object_sprite,
                        active_uninteractive_graphical_object->representing_frame,      
                        false,
                        level_working_on->runtime_stats.uninteractive_x[e],
                        level_working_on->runtime_stats.uninteractive_y[e],
                        level_working_on->runtime_stats.uninteractive_genus_palettes[level_working_on->runtime_stats.uninteractive_genus[e]]);
      }           

      // Get the raw number for the water genus junction 
      int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the water based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_water_graphical_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                                  :   custom_water_graphical_objects.at(water_genus_junction_id);
   
      // Return the correct graphical object sprite for the water based on the genus junction value.   
      DSX_SPRITE *active_water_graphical_object_sprite = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_water_graphical_object_sprites.at(water_genus_junction_id)
                                                                                  :   custom_water_graphical_object_sprites.at(water_genus_junction_id);
      
      for (int w = 0; w < level_working_on->runtime_stats.no_waters; w++) { 
         if (level_working_on->runtime_stats.water_z[w] == WATER_Z_BACKGROUND) {
            DSX_DrawWaterArea(active_water_graphical_object,
                              active_water_graphical_object_sprite,
                              active_water_graphical_object->representing_frame,
                              level_working_on->runtime_stats.water_x1[w],
                              level_working_on->runtime_stats.water_x2[w],
                              level_working_on->runtime_stats.water_y[w]);
         }                                     
      }     
      
      // Get the raw number for the entrance genus junction 
      int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the entrance based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                  :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);    
        
      // Return the correct graphical object sprite for the entrance based on the genus junction value.   
      DSX_SPRITE *active_entrance_graphical_object_sprite = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_entrance_graphical_object_sprites.at(entrance_genus_junction_id)
                                                                                  :   custom_entrance_graphical_object_sprites.at(entrance_genus_junction_id);    
           
      for (int e = 0; e < level_working_on->runtime_stats.no_entrances; e++) {
         DSX_DrawEntrance(active_entrance_graphical_object,
                          active_entrance_graphical_object_sprite,
                          active_entrance_graphical_object->representing_frame,
                          false,
                          level_working_on->runtime_stats.entrance_x[e],
                          level_working_on->runtime_stats.entrance_y[e]);
      }                   
        
      // Get the raw number for the exit genus junction 
      int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                         & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
       
      // Return the correct graphical object for the exit based on the genus junction value.   
      LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                  :   custom_exit_graphical_objects.at(exit_genus_junction_id);    
        
      // Return the correct graphical object sprite for the exit based on the genus junction value.   
      DSX_SPRITE *active_exit_graphical_object_sprite = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                  ? standard_exit_graphical_object_sprites.at(exit_genus_junction_id)
                                                                                  :   custom_exit_graphical_object_sprites.at(exit_genus_junction_id);    
        
      for (int e = 0; e < level_working_on->runtime_stats.no_exits; e++) {
         DSX_DrawExit(active_exit_graphical_object,
                      active_exit_graphical_object_sprite,
                      active_exit_graphical_object->representing_frame,       
                          false,
                      level_working_on->runtime_stats.exit_x[e],
                      level_working_on->runtime_stats.exit_y[e]);
      }         
      
      DSX_DrawLevel();            
      
      for (int e = 0; e < level_working_on->runtime_stats.no_traps; e++) {
         if (level_working_on->runtime_stats.trap_z[e] != TRAP_Z_FOREGROUND) continue;
          
         // Get the raw number for the trap genus junction 
         int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the trap based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_objects.at(trap_genus_junction_id);    
           
         // Return the correct graphical object sprite for the trap based on the genus junction value.   
         DSX_SPRITE *active_trap_graphical_object_sprite = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_trap_graphical_object_sprites.at(trap_genus_junction_id)
                                                                                     :   custom_trap_graphical_object_sprites.at(trap_genus_junction_id);    
           
         DSX_DrawTrap(active_trap_graphical_object,
                      active_trap_graphical_object_sprite,
                      active_trap_graphical_object->representing_frame,    
                      false,
                      level_working_on->runtime_stats.trap_x[e],
                      level_working_on->runtime_stats.trap_y[e],
                      level_working_on->runtime_stats.trap_genus_palettes[level_working_on->runtime_stats.trap_genus[e]]);
      }    
           
      for (int e = 0; e < level_working_on->runtime_stats.no_hazards; e++) {     
         if (level_working_on->runtime_stats.hazard_z[e] != HAZARD_Z_FOREGROUND) continue;
         
         // Get the raw number for the hazard genus junction 
         int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the hazard based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);    
           
         // Return the correct graphical object sprite for the hazard based on the genus junction value.   
         DSX_SPRITE *active_hazard_graphical_object_sprite = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_hazard_graphical_object_sprites.at(hazard_genus_junction_id)
                                                                                     :   custom_hazard_graphical_object_sprites.at(hazard_genus_junction_id);    
           
         DSX_DrawHazard(active_hazard_graphical_object,
                      active_hazard_graphical_object_sprite,
                      active_hazard_graphical_object->representing_frame,    
                      false,
                      level_working_on->runtime_stats.hazard_x[e],
                      level_working_on->runtime_stats.hazard_y[e],
                      level_working_on->runtime_stats.hazard_genus_palettes[level_working_on->runtime_stats.hazard_genus[e]]);
      }         
      
      for (int e = 0; e < level_working_on->runtime_stats.no_uninteractives; e++) {    
         if (level_working_on->runtime_stats.uninteractive_z[e] != UNINTERACTIVE_Z_FOREGROUND) continue;
         
         // Get the raw number for the uninteractive genus junction 
         int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                            & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
          
         // Return the correct graphical object for the uninteractive based on the genus junction value.   
         LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);    
           
         // Return the correct graphical object sprite for the uninteractive based on the genus junction value.   
         DSX_SPRITE *active_uninteractive_graphical_object_sprite = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                   & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                     ? standard_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id)
                                                                                     :   custom_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id);    
           
         DSX_DrawUninteractive(active_uninteractive_graphical_object,
                        active_uninteractive_graphical_object_sprite,
                        active_uninteractive_graphical_object->representing_frame,      
                        false,
                        level_working_on->runtime_stats.uninteractive_x[e],
                        level_working_on->runtime_stats.uninteractive_y[e],
                        level_working_on->runtime_stats.uninteractive_genus_palettes[level_working_on->runtime_stats.uninteractive_genus[e]]);
      }                         

      for (int w = 0; w < level_working_on->runtime_stats.no_waters; w++) {
         if (level_working_on->runtime_stats.water_z[w] == WATER_Z_FOREGROUND) {
            DSX_DrawWaterArea(active_water_graphical_object,
                              active_water_graphical_object_sprite,
                              active_water_graphical_object->representing_frame,
                              level_working_on->runtime_stats.water_x1[w],
                              level_working_on->runtime_stats.water_x2[w],
                              level_working_on->runtime_stats.water_y[w]);
         }                
      }                                  
      
      DSX_MagicDSXScreenToLevelPane2X();

      ++currently_selected_object_blink_frame %= CURRENTLY_SELECTED_OBJECT_BLINK_RATE;

      if (currently_selected_object_blink_frame >= CURRENTLY_SELECTED_OBJECT_BLINK_RATE/2) {
         for (int i = 0; i < currently_selected_object_count; i++) {
            DrawSelectedBoxOnItem(currently_selected_object_array[i]);                 
         }
         for (int i = 0; i < currently_selected_special_object_count; i++) {      
            DrawSelectedBoxOnSpecialItem(currently_selected_special_object_array[i]);                 
         }
      }
      
      if (lower_interface_current_option_set == LOWER_INTERFACE_OPTION_SET_AREA_SETUP) {
         for (int a = 0; a < level_working_on->runtime_stats.no_steel_areas; a++) {
            DrawSteelArea(a, SelectionAreaIsItemSelected(a + SELECTED_AREA_OBJECTS_STEEL_AREAS_START) && (currently_selected_object_blink_frame >= CURRENTLY_SELECTED_OBJECT_BLINK_RATE/2), false);
         }
         for (int a = 0; a < level_working_on->runtime_stats.no_one_way_areas; a++) {
            DrawOneWayArea(a, SelectionAreaIsItemSelected(a + SELECTED_AREA_OBJECTS_ONE_WAY_AREAS_START) && (currently_selected_object_blink_frame >= CURRENTLY_SELECTED_OBJECT_BLINK_RATE/2), false);
         }             

         DrawCameraFocus();
      }                 
                     
      ++transparent_colour_blink_frame %= TRANSPARENT_COLOUR_BLINK_RATE;
      
      
      if (transparent_colour_blink_frame == 0) {             
         level_working_on->runtime_stats.level_palette[0]     = RGB15A(31, 0,31);
         level_working_on->runtime_stats.entrance_palette[0]  = RGB15A(31, 0,31);
         level_working_on->runtime_stats.exit_palette[0]      = RGB15A(31, 0,31);
         
         for (int e = 0; e < NO_TRAP_GENUSES; e++) {
            level_working_on->runtime_stats.trap_genus_palettes[e][0]          = RGB15A(31, 0,31);
         }
         for (int e = 0; e < NO_HAZARD_GENUSES; e++) {
            level_working_on->runtime_stats.hazard_genus_palettes[e][0]        = RGB15A(31, 0,31);
         }
         for (int e = 0; e < NO_UNINTERACTIVE_GENUSES; e++) {
            level_working_on->runtime_stats.uninteractive_genus_palettes[e][0] = RGB15A(31, 0,31);
         }
         
         level_working_on->runtime_stats.water_palette[0]     = RGB15A(31, 0,31);
      } else
      if (transparent_colour_blink_frame == TRANSPARENT_COLOUR_BLINK_RATE/2) { 
         level_working_on->runtime_stats.level_palette[0]     = RGB15A(26, 0,26);
         level_working_on->runtime_stats.entrance_palette[0]  = RGB15A(26, 0,26);
         level_working_on->runtime_stats.exit_palette[0]      = RGB15A(26, 0,26);   
         
         for (int e = 0; e < NO_TRAP_GENUSES; e++) {
            level_working_on->runtime_stats.trap_genus_palettes[e][0]          = RGB15A(26, 0,26);
         }                            
         for (int e = 0; e < NO_HAZARD_GENUSES; e++) {
            level_working_on->runtime_stats.hazard_genus_palettes[e][0]        = RGB15A(26, 0,26);
         }
         for (int e = 0; e < NO_UNINTERACTIVE_GENUSES; e++) {
            level_working_on->runtime_stats.uninteractive_genus_palettes[e][0] = RGB15A(26, 0,26);
         }
         
         level_working_on->runtime_stats.water_palette[0]     = RGB15A(26, 0,26);
      }    
      
      
      BlitLevelPaneToBackbuffer();     
      
      BlitUpperInterfaceToBackbuffer();
      BlitLowerInterfaceToBackbuffer();
                                           
      BlitRightInterfaceToBackbuffer();
      
      if (key[KEY_Q]) {
         for (int o = 0; o<level_working_on->no_terrain_objects; o++) {
            LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *object = GetTerrainObject(o);
            
            if (object->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {
               allegro_message("Object %d: 16 colours.\n\n"
                                "X1: %d, Y1: %d\n" 
                                "X2: %d, Y2: %d\n" 
                                "tox: %d, toy: %d\n" 
                                "tsx: %d, tsy: %d\n" 
                                "object ID: %u!"
                                "%s%s%s",
                                                  o, ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->x1,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->y1,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->x2,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->y2,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->tox,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->toy,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->tsx,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->tsy,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_id &~ LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT
                                                         ? "\nCustom texture!": "",
                                                    (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE)
                                                         ? "\nSubtractive!" : "",
                                                    (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR)
                                                         ? (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS)
                                                             ? "\nMasking behaviour!" : "\nOnly draws on blanks!"
                                                         : "");
            } else 
            if (object->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
               allegro_message("Object %d: 256 colours.\n\n"
                                "X1: %d, Y1: %d\n" 
                                "X2: %d, Y2: %d\n" 
                                "tox: %d, toy: %d\n" 
                                "tsx: %d, tsy: %d\n" 
                                "object ID: %u!"     
                                "%s%s%s",
                                                  o, ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->x1,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->y1,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->x2,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->y2,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->tox,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->toy,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->tsx,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->tsy,
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->object_id &~ LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT, 
                                                     ((LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)(object))->object_id & LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT
                                                         ? "\nCustom texture!": "",
                                                    (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE)
                                                         ? "\nSubtractive!" : "",
                                                    (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR)
                                                         ? (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS)
                                                             ? "\nMasking behaviour!" : "\nOnly draws on blanks!"
                                                         : "");
            } else 
            if (object->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
               allegro_message("Object %d: 1 colour.\n\n"
                                "X1: %d, Y1: %d\n" 
                                "X2: %d, Y2: %d\n" 
                                "colour: %d!"       
                                "%s%s",
                                               o, ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->x1,
                                                  ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->y1,
                                                  ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->x2,
                                                  ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->y2,
                                                  ((LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)(object))->colour,
                                                 (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE)
                                                      ? "\nSubtractive!" : "",
                                                 (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR)
                                                      ? (((LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *)(object))->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS)
                                                          ? "\nMasking behaviour!" : "\nOnly draws on blanks!"
                                                      : "");
            } else 0;
         } 
      }
      
      if (key[KEY_W]) {
         char output_string[32768];                    
   
         sprintf(output_string, "Items: %d | Object list:", currently_selected_object_count);
   
         for (int a = 0; a < currently_selected_object_count; a++) {
            sprintf(output_string, "%s %d,", output_string, currently_selected_object_array[a]);   
         }
   
         allegro_message(output_string);   
      }

      nowclock = clock();
      //textprintf_right(bitmap_backbuffer, font, 795, 1, WHITE, "%d", nowclock - oldclock);
      textprintf_right(bitmap_backbuffer, font, 795, 1, WHITE, "%.2f", 1.0f/((float)(nowclock - oldclock)/(float)CLOCKS_PER_SEC));
      oldclock = nowclock;
      
      //textprintf(bitmap_backbuffer, font, 300, 300, WHITE, "%s%s%s%s%s",
      //                                                     RMBConjunctionIsDown(x) ? "O" : " ",
      //                                                     RMBConjunctionDown(x) ? "O" : " ",
      //                                                     RMBConjunctionHeld(x) ? "O" : " ",
      //                                                     RMBConjunctionRelease(x) ? "O" : " ",
      //                                                     RMBConjunctionIdle(x) ? "O" : " ");
                                           
      draw_sprite(bitmap_backbuffer, bitmap_cursor, mouse_x, mouse_y);
      
      pageflip();   
   }
   
   return 0;
}       

////////////////////////////////////////////////////////////////
                                   
void MenuNewLevelSequence() {
   int confirmation_value = alert("This action will destroy the",
                                  "level currently held in memory!",
                                  "Proceed?",
                                  "Yes", "Cancel",
                                  'y', 'n');  
                                  
   if (confirmation_value == 1) {
      char texture_archive_selected[2048];
      
      if (!MenuSelectTextureArchiveSequence(texture_archive_selected, true)) {
         // Ask the user to select a texture archive from the list.
         // If the user cancels, do nothing.
         return;
      } else {  
         InitialiseLevel(texture_archive_selected);
         InitialiseGUI();  
         return;                                 
      }
   } else
   if (confirmation_value == 2) {
      return;
   }
}
                    
void MenuLoadLevelSequence() {
   int confirmation_value = alert("This action will destroy the",
                                  "level currently held in memory!",
                                  "Proceed?",
                                  "Yes", "Cancel",
                                  'y', 'n');  
                                               
   if (confirmation_value == 1) { 
      bool retry_file_selection = true;         
      do {
         char filename[8192];
         replace_filename(filename, global_argv[0], "", 8191);
      
         int file_selector_result = file_select_ex("Select a Lemmings DS level (*.lds) to load:",
                                                                                        filename,
                                                                                           "lds",
                                                                                            8191,
                                                                      GLOBAL_FILE_SELECTOR_WIDTH,
                                                                     GLOBAL_FILE_SELECTOR_HEIGHT);
                                                                                          
         if (file_selector_result == 0) {  
            retry_file_selection = false;
         } else {                   
            if (strcmp(get_extension(filename), "lds") != 0) {
               int retry_bad_selection_result = alert("Invalid file.",
                                                      NULL,           
                                                      "Retry?",
                                                      "Yes", "No",
                                                      'y', 'n');   
                                                      
               if (retry_bad_selection_result == 1) {
               } else
               if (retry_bad_selection_result == 2) {
                  retry_file_selection = false;
               }                                             
            } else {         
               // We've picked a file. It exists. We think.
               bool retry_file_loading_sequence = true;
               do {
                  long input_filesize = file_size(filename);
                  
                  if ((input_filesize == 0)
                   || (input_filesize < sizeof(LEMMINGS_LEVEL_LDS_FILE_V7))) {
                     int bad_loading_selection_result = alert("File doesn't exist, or is too small.",
                                                              NULL,           
                                                              "Retry?",
                                                              "Yes", "No",
                                                              'y', 'n'); 
                                                              
                     if (bad_loading_selection_result == 1) { 
                        continue;
                     } else
                     if (bad_loading_selection_result == 2) {
                        retry_file_loading_sequence = false;
                        continue;
                     }    
                  }         
                  
                  u8 *main_memory_chunk_temporary = new u8[MAIN_MEMORY_CHUNK_SIZE];
                  LEMMINGS_LEVEL_LDS_FILE_V7 *level_working_on_temporary = (LEMMINGS_LEVEL_LDS_FILE_V7 *)main_memory_chunk_temporary;

                  // If it works, then let's use the return keyword.
                  FILE *input_file = fopen(filename, "rb"); 

                     // We've got a FILE handle for the input file, let's go to town!
                     fread(level_working_on_temporary, input_filesize, 1, input_file); 
                  
                  fclose(input_file);
                  
                  enum INVALID_LOAD_REASON {
                     INVALID_LOAD_REASON_VALID = 0,
                     INVALID_LOAD_REASON_BAD_CHUNK_SIZE,
                     INVALID_LOAD_REASON_BAD_VERSION_NUMBER,
                     INVALID_LOAD_REASON_CANT_LOAD_ARCHIVE,
                     INVALID_LOAD_REASON_ARCHIVE_SIZE_MISMATCH,
                     INVALID_LOAD_REASON_ARCHIVE_BAD_VERSION_NUMBER,
                     INVALID_LOAD_REASON_COUNT, 
                  };
                  
                  const char *invalid_load_reason_string[INVALID_LOAD_REASON_COUNT] = 
                      {"This shouldn't happen. Yikes!",
                       "Bad chunk size.",
                       "Version mismatch!",
                       "Couldn't load texture archive",
                       "Archive size mismatch",
                       "Texture archive version mismatch!"};
                  
                  bool valid_load = true;
                  INVALID_LOAD_REASON invalid_load_reason = INVALID_LOAD_REASON_VALID;
                                                                                         
                  char texture_archive_load_location[1024];
                  long texture_archive_input_filesize;
                                                 
                  if (input_filesize != level_working_on_temporary->lemmings_level_file_size) {
                     valid_load = false;
                     invalid_load_reason = INVALID_LOAD_REASON_BAD_CHUNK_SIZE;
                  } else
                  if (level_working_on_temporary->version_number != LEMMINGS_LEVEL_VERSION){
                     valid_load = false;
                     invalid_load_reason = INVALID_LOAD_REASON_BAD_VERSION_NUMBER;
                  }                             
                                    
                  LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_active_texture_archive_temporary;
                         
                  if (valid_load) {
                     sprintf(texture_archive_load_location, "custom_texture_archives/%s.LTA", level_working_on_temporary->texture_archive_using);
                                                                   
                     texture_archive_input_filesize = file_size(texture_archive_load_location);
                     
                     if ((texture_archive_input_filesize == 0)
                      || (texture_archive_input_filesize < sizeof(LEMMINGS_TEXTURE_ARCHIVE_HEADER))) {
                        sprintf(texture_archive_load_location, "standard_texture_archives/%s.LTA", level_working_on_temporary->texture_archive_using);
                                                                      
                        texture_archive_input_filesize = file_size(texture_archive_load_location);
                        
                        if ((texture_archive_input_filesize == 0)
                         || (texture_archive_input_filesize < sizeof(LEMMINGS_TEXTURE_ARCHIVE_HEADER))) {
                           valid_load = false;
                           invalid_load_reason = INVALID_LOAD_REASON_CANT_LOAD_ARCHIVE;
                        } 
                     } 
                  }
                  
                  if (valid_load) {   
                     loaded_active_texture_archive_temporary = (LEMMINGS_TEXTURE_ARCHIVE_HEADER *)new u8[texture_archive_input_filesize];    
                     FILE *texture_archive = fopen(texture_archive_load_location, "rb");
                                                    
                        fread(loaded_active_texture_archive_temporary, texture_archive_input_filesize, 1, texture_archive);   
                        
                     fclose(texture_archive);  
                     
                     if (texture_archive_input_filesize != loaded_active_texture_archive_temporary->texture_archive_file_size) {
                        valid_load = false;
                        invalid_load_reason = INVALID_LOAD_REASON_ARCHIVE_SIZE_MISMATCH;
                     }
                  }

                  if (valid_load) {                                          
                     memcpy(main_memory_chunk, main_memory_chunk_temporary, input_filesize);
                     delete[] main_memory_chunk_temporary;   
                     delete[] loaded_active_texture_archive;  
                                  
                     loaded_active_texture_archive = loaded_active_texture_archive_temporary;   
                                      
                     InitialiseGUI();        
                  
                     retry_file_loading_sequence = false;
                     retry_file_selection = false;   
                     return;
                  } else {                                                     
                     delete[] main_memory_chunk_temporary;       
                     delete[] loaded_active_texture_archive_temporary;   
                     int bad_loading_selection_result = alert("File loading failed!",
                                     invalid_load_reason_string[invalid_load_reason],          
                                                                            "Retry?",
                                                                         "Yes", "No",
                                                                            'y', 'n'); 
                                                              
                     if (bad_loading_selection_result == 1) {
                     } else
                     if (bad_loading_selection_result == 2) {
                        retry_file_loading_sequence = false;
                     }    
                  }                  
               } while (retry_file_loading_sequence);                                          
            }
         }
      } while (retry_file_selection); 
   } else
   if (confirmation_value == 2) {
   }                   
   
   // Use the return keyword from inside the loop if the load went as planned.
   // Otherwise it'll run out here and produce this error.
    
   alert(NULL,           
         "Load Cancelled",
         NULL,
         "OK", NULL,
         0, 0);   
   return;
}
       
void MenuSaveLevelSequence() {
      bool retry_file_selection = true;         
      do {
         char filename[8192];        
         replace_filename(filename, global_argv[0], "", 8191);
      
         int file_selector_result = file_select_ex("Choose a filename for your Lemmings DS level (*.lds):",
                                                                                                  filename,
                                                                                                     "lds",
                                                                                                      8191,
                                                                                GLOBAL_FILE_SELECTOR_WIDTH,
                                                                               GLOBAL_FILE_SELECTOR_HEIGHT);
                                                                                          
         if (file_selector_result == 0) {  
            retry_file_selection = false;
         } else {                   
            if (strcmp(get_extension(filename), "lds") != 0) {
               int retry_bad_selection_result = alert("Invalid file.",
                                                      NULL,           
                                                      "Retry?",
                                                      "Yes", "No",
                                                      'y', 'n');   
                                                      
               if (retry_bad_selection_result == 1) {
               } else
               if (retry_bad_selection_result == 2) {
                  retry_file_selection = false;
               }                                             
            } else {         
               // We've picked a file. It doesn't exist. We think.
               bool retry_file_saving_sequence = true;
               do {                  
                  if (exists(filename)) {
                     int bad_saving_selection_result = alert(NULL,           
                                                             "File already exists, overwrite?",
                                                             NULL,           
                                                             "Yes", "No",
                                                             'y', 'n'); 
                                                              
                     if (bad_saving_selection_result == 1) {
                     } else
                     if (bad_saving_selection_result == 2) {
                        retry_file_saving_sequence = false;
                        continue;
                     }    
                  }          
                  
                  // If it works, then let's use the return keyword.
                  FILE *input_file = fopen(filename, "wb");          
                  
                     // Before we save the file, let's make an accurate level preview:
                     RenderLevelPreviewIntoPreviewArea(level_data, level_working_on->preview_data);

                     // We've got a FILE handle for the input file, let's go to town!
                     fwrite(main_memory_chunk, level_working_on->lemmings_level_file_size, 1, input_file); 
                  
                  fclose(input_file);
                  
                  retry_file_saving_sequence = false;
                  return;                  
               } while (retry_file_saving_sequence);                                          
            }
         }
      } while (retry_file_selection); 
   
   // Use the return keyword from inside the loop if the load went as planned.
   // Otherwise it'll run out here and produce this error.
    
   alert(NULL,           
         "Save Cancelled",
         NULL,
         "OK", NULL,
         0, 0);   
   return;
}              
       
int MenuQuitEditorSequence() {
   int confirmation_value = alert("If you quit you will lose the",
                                  "level currently held in memory!",
                                  "Proceed?",
                                  "Yes",
                                  "Cancel", 'y', 'n');  
                                  
   if (confirmation_value == 1) {
      return 1;
   } else
   if (confirmation_value == 2) {
      return 2;
   }
}

#define IMPORTING_16_COLOUR_OBJECT_PICK_PALETTE_CANCEL (-1)
int Importing16ColourObject_PickPalette(const u16 *incoming_palette_555);

void MenuLoadImageToObjectSequence(int destination_slot, int type, bool image_copy, bool palette_copy) {
   int confirmation_value;
   
   if (destination_slot != -1) { 
      confirmation_value = alert("This action will replace the",
                                 "image currently held in memory!",
                                 "Proceed?",
                                 "Yes", "Cancel",
                                 'y', 'n');  
   } else {
      confirmation_value = 1;
   }
                                               
   if (confirmation_value == 1) { 
      bool retry_file_selection = true;         
      do {
         char filename[8192];        
         replace_filename(filename, global_argv[0], "", 8191);
      
         int file_selector_result = file_select_ex((type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                                                      ? "Select a 16 colour bitmap (*.bmp) to load:"
                                                      : "Select a 256 colour bitmap (*.bmp) to load:",
                                                                             filename,
                                                                                "bmp",
                                                                                 8191,
                                                           GLOBAL_FILE_SELECTOR_WIDTH,
                                                          GLOBAL_FILE_SELECTOR_HEIGHT);
                                                                                          
         if (file_selector_result == 0) {  
            retry_file_selection = false;
         } else {                   
            if (strcmp(get_extension(filename), "bmp") != 0) {
               int retry_bad_selection_result = alert("Invalid file.",
                                                      NULL,           
                                                      "Retry?",
                                                      "Yes", "No",
                                                      'y', 'n');   
                                                      
               if (retry_bad_selection_result == 1) {
               } else
               if (retry_bad_selection_result == 2) {
                  retry_file_selection = false;
               }                                             
            } else {         
               // We've picked a file. It exists. We think.
               bool retry_file_loading_sequence = true;
               do {
                  long input_filesize = file_size(filename);
                  
                  if (input_filesize == 0) {
                     int bad_loading_selection_result = alert("File doesn't exist!",
                                                              NULL,           
                                                              "Retry?",
                                                              "Yes", "No",
                                                              'y', 'n'); 
                                                              
                     if (bad_loading_selection_result == 1) {  
                        continue;
                     } else                                   
                     if (bad_loading_selection_result == 2) {
                        retry_file_loading_sequence = false;
                        continue;
                     }    
                  }       
                                    
                  bool valid_load = true;
                  
                  MEMORY_BMP bmp_file;
                  MemoryBMP_Initialise(&bmp_file);
                  
                  valid_load = MemoryBMP_LoadBMP(&bmp_file, filename);
                  
                  if (valid_load) {
                     valid_load = false;
                     if ((type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16)
                      && (MemoryBMP_GetNoColours(&bmp_file) == 16           )) {
                        valid_load = true;
                     } else
                     if ((type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256)
                      && (MemoryBMP_GetNoColours(&bmp_file) == 256           )) {
                        valid_load = true;
                     }
                  }
                                    
                  // ------------------------------------------
                  
                  if (valid_load) {                                      
                     //allegro_message("Incoming length: %d bytes.", bmp_file.memory_length);
                     
                     int width, height;
                                                                             
                     //allegro_message("Incoming size: %d bytes.", bmp_file.bmp_memory_begin->fileheader.bfSize);
                     
                     width  = MemoryBMP_GetWidth(&bmp_file);
                     height = MemoryBMP_GetHeight(&bmp_file);        
                                
                     //allegro_message("Incoming width + height: %d and %d.", width, height);
               
                     int new_object_id;
                     
                     if (type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) { 
                        if (image_copy) {    
                           new_object_id = (destination_slot == -1)
                                             ? level_working_on->no_appended_texture_16s
                                             : destination_slot;            
                                          
                           if (destination_slot != -1) {       
                              // We're shifting the texture ahead of this one back
                              // to destroy the existing picture.                    
                              LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *existing_texture_to_be_destroyed = GetAppendedTexture16(new_object_id); 
                              u8 *location_of_next_picture_to_shift_back = ((u8*)(existing_texture_to_be_destroyed)) + (existing_texture_to_be_destroyed->my_mem_size);
                               
                              // shift the level data down to destroy the old info.
                              ShiftLevelData(location_of_next_picture_to_shift_back, 0 - (existing_texture_to_be_destroyed->my_mem_size),
                                                                                       level_working_on->lemmings_level_file_size); 
                           }      
                        
                           // Calculate the amount of memory to shift up by.
                           // The size of the image in nibbles, taken to the next byte,
                           // halved to get bytes, then padded to the next word.
                           unsigned int image_memory_size = sizeof(LEMMINGS_LEVEL_APPENDED_TEXTURE_16) + to_next((to_next((width * height), 2)/2),4);
                                             
                           LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *existing_texture_to_be_moved = GetAppendedTexture16(new_object_id, true);              
                                                                      
                           // Shift the level data up to make room for the texture data.
                           ShiftLevelData((u8 *)existing_texture_to_be_moved, image_memory_size,
                                                                                   level_working_on->lemmings_level_file_size);   
                                             
                           LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *new_texture_generated_by_moving = existing_texture_to_be_moved;  
                         
                           new_texture_generated_by_moving->xs = width;
                           new_texture_generated_by_moving->ys = height;
                                                 
                           for (int byte = 0; byte < (((width * height) + 1) / 2); byte++) {
                              new_texture_generated_by_moving->data[byte] =  MemoryBMP_GetPixel(&bmp_file, (0 + (byte*2)) % width,
                                                                                                           (0 + (byte*2)) / width)
                                                                          | (MemoryBMP_GetPixel(&bmp_file, (1 + (byte*2)) % width,
                                                                                                           (1 + (byte*2)) / width)) << 4;
                           }
                           
                           new_texture_generated_by_moving->my_mem_size = image_memory_size;
                         
                           if (destination_slot == -1) {                                                        
                               ++level_working_on->no_appended_texture_16s;
                           }       
                        }   
                        
                        if (palette_copy) {      
                           u16 incoming_palette[16];
                                     
                           int colour;
                           RGBQUAD *current_tuple;
                           
                           for (int e = 0; e < MemoryBMP_GetNoPaletteTuples(&bmp_file); e++) {
                              current_tuple = MemoryBMP_GetPaletteEntryTuple(&bmp_file, e);
                               
                              Create15BitColour(&colour, current_tuple->rgbRed,
                                                         current_tuple->rgbGreen,
                                                         current_tuple->rgbBlue);  
                               
                              incoming_palette[e] = colour; 
                           }  
                              
                           int palette_line = Importing16ColourObject_PickPalette(incoming_palette);
                           
                           if (palette_line == IMPORTING_16_COLOUR_OBJECT_PICK_PALETTE_CANCEL) {
                              alert(NULL, "Texture imported successfully, palette import cancelled.",
                                          NULL,
                                          "OK", NULL,
                                          0, 0);
                           } else {
                              for (int e = 0; e < MemoryBMP_GetNoPaletteTuples(&bmp_file); e++) {
                                 level_working_on->runtime_stats.level_palette[palette_line * 16 + e] = incoming_palette[e]; 
                              }  
                           }
                        }
                        
                        RefreshAppendedTexture256sBegin();
                        RefreshTerrainObjectsBegin();
                        RecalculateRightInterfaceTextureSelectionDatabaseSize();
                     } else
                     if (type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {
                        if (image_copy) {
                           new_object_id = (destination_slot == -1)
                                             ? level_working_on->no_appended_texture_256s
                                             : destination_slot;
                                          
                           if (destination_slot != -1) {         
                              // We're shifting the texture ahead of this one back
                              // to destroy the existing picture.                    
                              LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *existing_texture_to_be_destroyed = GetAppendedTexture256(new_object_id); 
                              u8 *location_of_next_picture_to_shift_back = ((u8*)(existing_texture_to_be_destroyed)) + (existing_texture_to_be_destroyed->my_mem_size);
                               
                              // shift the level data down to destroy the old info.
                              ShiftLevelData(location_of_next_picture_to_shift_back, 0 - (existing_texture_to_be_destroyed->my_mem_size),
                                                                                       level_working_on->lemmings_level_file_size); 
                           }            
                        
                           // Calculate the amount of memory to shift up by.
                           // The size of the image in bytes, then padded to the next word.
                           unsigned int image_memory_size = sizeof(LEMMINGS_LEVEL_APPENDED_TEXTURE_256) + to_next(width * height, 4);
                                             
                           LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *existing_texture_to_be_moved = GetAppendedTexture256(new_object_id, true);              
                                                                      
                           // Shift the level data up to make room for the texture data.
                           ShiftLevelData((u8 *)existing_texture_to_be_moved, image_memory_size,
                                                                                   level_working_on->lemmings_level_file_size);   
                                             
                           LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *new_texture_generated_by_moving = existing_texture_to_be_moved;  
                         
                           new_texture_generated_by_moving->xs = width;
                           new_texture_generated_by_moving->ys = height;
                                                 
                           for (int byte = 0; byte < (width * height); byte++) {
                              new_texture_generated_by_moving->data[byte] =  MemoryBMP_GetPixel(&bmp_file, byte % width,
                                                                                                           byte / width);
                           }
                           
                           new_texture_generated_by_moving->my_mem_size = image_memory_size;
                         
                           if (destination_slot == -1) {                                                        
                               ++level_working_on->no_appended_texture_256s;
                           }                         
                        }   
                        
                        if (palette_copy) {
                           int colour;
                           RGBQUAD *current_tuple;
                           
                           for (int e = 0; e < MemoryBMP_GetNoPaletteTuples(&bmp_file); e++) {
                              current_tuple = MemoryBMP_GetPaletteEntryTuple(&bmp_file, e);
                               
                              Create15BitColour(&colour, current_tuple->rgbRed,
                                                         current_tuple->rgbGreen,
                                                         current_tuple->rgbBlue);  
                               
                              level_working_on->runtime_stats.level_palette[e] = colour; 
                           }
                        }
                        
                        RefreshAppendedTexture256sBegin();
                        RefreshTerrainObjectsBegin();
                        RecalculateRightInterfaceTextureSelectionDatabaseSize();
                     }
                                                                                                    
                     //allegro_message("Shift complete!");                        
                     
                     MemoryBMP_UnloadBMP(&bmp_file);   
                                                               
                     //allegro_message("Unload complete!");
                     return;
                  } else {                                
                     MemoryBMP_UnloadBMP(&bmp_file);
                      
                     int bad_loading_selection_result = alert("File loading failed!",
                                                                                NULL,          
                                                                            "Retry?",
                                                                         "Yes", "No",
                                                                            'y', 'n'); 
                                                              
                     if (bad_loading_selection_result == 1) {
                     } else
                     if (bad_loading_selection_result == 2) {
                        retry_file_loading_sequence = false;
                     }    
                  }                  
               } while (retry_file_loading_sequence);                                          
            }
         }
      } while (retry_file_selection); 
   } else
   if (confirmation_value == 2) {
   }                   
   
   // Use the return keyword from inside the loop if the load went as planned.
   // Otherwise it'll run out here and produce this error.
    
   alert(NULL,           
         "Load Cancelled",
         NULL,
         "OK", NULL,
         0, 0);   
   return;
}             

// #define IMPORTING_16_COLOUR_OBJECT_PICK_PALETTE_CANCEL -1

int d_radio_palette_proc(int msg, DIALOG *d, int c) {
   if (msg == MSG_DRAW) { 
      int colour_lines = BLACK;
      if (d->flags & D_SELECTED) {
         colour_lines = WHITE;
      }
      
      int x1 = d->x;
      int y1 = d->y;
      int x2 = d->x + d->w - 1;
      int y2 = d->y + d->h - 1;
      
      hline(screen, x1, y1, x2, colour_lines);
      hline(screen, x1, y2, x2, colour_lines);  
      
      for (int line = 0; line < 17; line++) {
         vline(screen, x1 + line * 11, y1, y2, colour_lines);  
      } 
      
      u16 *palette = ((u16 *)(d->dp));    
      
      for (int colour = 0; colour < 16; colour++) {
         rectfill15a(screen, x1 + (colour    ) * 11 + 1,
                             y1                     + 1,
                             x1 + (colour + 1) * 11 - 1,
                             y2                     - 1, palette[colour]);  
      } 
      
      return D_O_K;
   } else {
      return d_radio_proc(msg, d, c); 
   }
}

int Importing16ColourObject_PickPalette(const u16 *incoming_palette_555) {
   int palette = 0;
   
   DIALOG dialog_import_16_colour_palette[] = {
      /* (proc)                    (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                                               (dp2) (dp3) */
      { d_shadow_box_proc,         176, 144, 456, 336, 0,   0,   0,    0,      0,   0,   NULL,                                              NULL, NULL },
      { d_ctext_proc,              396, 160, 136, 8,   0,   0,   0,    0,      0,   0,   (void*)"Choose palette line to import to:",        NULL, NULL },
      { d_ctext_proc,              396, 184, 136, 8,   0,   0,   0,    0,      0,   0,   (void*)"Incoming palette:",                        NULL, NULL },
      { d_radio_palette_proc,      308, 200, 177, 12,  0,   0,   0,    0,      0,   0,   (void*)incoming_palette_555,                       NULL, NULL },
      { d_ctext_proc,              396, 232, 136, 8,   0,   0,   0,    0,      0,   0,   (void*)"Palette grid:",                            NULL, NULL },
#define IMPORT_16_COLOUR_PALETTE_DIALOG_RADIO_ITEMS_BEGIN 5      
      { d_radio_palette_proc,      308, 248, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x00])), NULL, NULL },
      { d_radio_palette_proc,      308, 259, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x10])), NULL, NULL },
      { d_radio_palette_proc,      308, 270, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x20])), NULL, NULL },
      { d_radio_palette_proc,      308, 281, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x30])), NULL, NULL },
      { d_radio_palette_proc,      308, 292, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x40])), NULL, NULL },
      { d_radio_palette_proc,      308, 303, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x50])), NULL, NULL },
      { d_radio_palette_proc,      308, 314, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x60])), NULL, NULL },
      { d_radio_palette_proc,      308, 325, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x70])), NULL, NULL },
      { d_radio_palette_proc,      308, 336, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x80])), NULL, NULL },
      { d_radio_palette_proc,      308, 347, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0x90])), NULL, NULL },
      { d_radio_palette_proc,      308, 358, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xA0])), NULL, NULL },
      { d_radio_palette_proc,      308, 369, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xB0])), NULL, NULL },
      { d_radio_palette_proc,      308, 380, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xC0])), NULL, NULL },
      { d_radio_palette_proc,      308, 391, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xD0])), NULL, NULL },
      { d_radio_palette_proc,      308, 402, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xE0])), NULL, NULL },
      { d_radio_palette_proc,      308, 413, 177, 11,  0,   0,   0,    0,      1,   1,   (void*)(&(level_working_on->runtime_stats.level_palette[0xF0])), NULL, NULL },
      { d_button_proc,             192, 440, 208, 24,  0,   0,   0,    D_EXIT, 0,   0,   (void*)"Import to this line",                      NULL, NULL },
      { d_button_proc,             408, 440, 208, 24,  0,   0,   0,    D_EXIT, 0,   0,   (void*)"Don't import the palette",                 NULL, NULL },
      { NULL,                      0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                                              NULL, NULL }
   };                                                                       
    
   dialog_import_16_colour_palette[3].flags = D_SELECTED;

   for (int item = 0; dialog_import_16_colour_palette[item].proc; item++) {
      dialog_import_16_colour_palette[item].fg = WHITE;
      dialog_import_16_colour_palette[item].bg = DGREY;       
   }
       
   dialog_import_16_colour_palette[IMPORT_16_COLOUR_PALETTE_DIALOG_RADIO_ITEMS_BEGIN].flags = D_SELECTED;

   int dialog_quit_trigger = popup_dialog(dialog_import_16_colour_palette, -1);
   
   if (dialog_quit_trigger == 21) {      
      for (int radio = 0; radio < 16; radio++) {
         if (dialog_import_16_colour_palette[radio + IMPORT_16_COLOUR_PALETTE_DIALOG_RADIO_ITEMS_BEGIN].flags & D_SELECTED) {
            return radio; 
         } 
      }            
   } else
   if (dialog_quit_trigger == 22) {
      // Cancelling should take no action on the level_working_on memory!
      return IMPORTING_16_COLOUR_OBJECT_PICK_PALETTE_CANCEL;
   }                                             
}              

typedef struct tagALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT {
   int low;
   int high;
} ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT;                                                 
// Set the d2 of a d_edit_number_proc to point to this, and it will endeavour to be always valid.

// This edit proc is a special proc which wraps a d_edit_proc, providing it with an automatic
// number clamping system.
int d_edit_number_proc(int msg, DIALOG *d, int c) {
   if (msg == MSG_CHAR) {
      char ascii = c & 0xff;
      char scan  = c >> 8;
      if (!((ascii >= '0')
         && (ascii <= '9'))
      && (scan != KEY_BACKSPACE)) {
         return D_USED_CHAR;
      }

      if (scan != KEY_BACKSPACE) {          // number keys add the number on
         if (strlen((char *)d->dp) != d->d1) {
            ((char *)d->dp)[strlen((char *)d->dp)] = ascii;
            d->d2++;
         }
      } else {                   
         if (d->d2 != 0) {               // backspace does two things
            for (int c = d->d2; c < strlen((char *)d->dp); c++) {
               ((char *)d->dp)[c - 1] = ((char *)d->dp)[c]; // move all the existing characters down
            }                           
            for (int c = strlen((char *)d->dp) - 1; c < d->d1; c++) {
               ((char *)d->dp)[c] = 0;                   // fill the rest with null
            }
            d->d2--;
         }
      }

      int value = -1;

      sscanf((char *)d->dp, "%d", &value);

      //sprintf((char *)dialog_test_p[12].dp, "scan result: %d:%s", value, (char *)d->dp);
      //SEND_MESSAGE(&dialog_test_p[12], MSG_DRAW, 0);

      SEND_MESSAGE(d, MSG_DRAW, 0);                    

      if (strlen((char *)d->dp) != 0) {
         ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT *value_bounds = (ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT *)d->dp2;

         if (value < value_bounds->low)  value = value_bounds->low;
         if (value > value_bounds->high) value = value_bounds->high;

         sprintf((char *)d->dp, "%d", value);

         SEND_MESSAGE(d, MSG_DRAW, 0);
      }

      d->d2 = strlen((char *)d->dp);

      return D_USED_CHAR;
   } else
   if (msg == MSG_LOSTFOCUS) {
      int result = d_edit_proc(msg, d, c);

      int value = -1;

      sscanf((char *)d->dp, "%d", &value);

      //sprintf((char *)dialog_test_p[12].dp, "scan result: %d:%s", value, (char *)d->dp);
      //SEND_MESSAGE(&dialog_test_p[12], MSG_DRAW, 0);

      SEND_MESSAGE(d, MSG_CLICK, 0);

      ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT *value_bounds = (ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT *)d->dp2;

      if (value < value_bounds->low)  value = value_bounds->low;
      if (value > value_bounds->high) value = value_bounds->high;

      sprintf((char *)d->dp, "%d", value);

      SEND_MESSAGE(d, MSG_DRAW, 0);

      d->d2 = strlen((char *)d->dp);

      return result;
   } else {
      return d_edit_proc(msg, d, c);
   }
}

//#define MAX_LEMMINGS 100
#define MAX_LEMMINGS 999

void MenuLevelInfoSequence() {
     
   // These store the temporary information used in the dialog. 
   char level_name[31];

   char no_lemmings_string[4];
   char lemmings_to_be_saved_string[4];
   char time_in_minutes_string[2];             
       
   char rating_string[16];    
   
   char description[64];
                                                                        
   strncpy((char *) level_name,   (const char *)level_working_on->runtime_stats.level_name,         31);
   strncpy((char *)rating_string, (const char *)level_working_on->runtime_stats.rating_description, 16);
   sprintf(         no_lemmings_string, "%d", level_working_on->runtime_stats.lemmings);
   sprintf(lemmings_to_be_saved_string, "%d", level_working_on->runtime_stats.to_be_saved);
   sprintf(     time_in_minutes_string, "%d", level_working_on->runtime_stats.time_in_minutes); 
   strncpy((char *)description,   (const char *)level_working_on->runtime_stats.description,        64);

   DIALOG *dialog_level_info_p; // Pointer to the dialog.

   // These are the managers for the dialogs number entries.
   ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT no_lemmings_edit_number_struct          = {  0, MAX_LEMMINGS};
   ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT lemmings_to_be_saved_edit_number_struct = {  1, MAX_LEMMINGS};
   ALLEGRO_DIALOG_D2_EDIT_NUMBER_STRUCT time_in_minutes_edit_number_struct      = {  0,   9};

   DIALOG dialog_level_info[] = {
      /* (proc)            (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                                                  (dp2) (dp3) */
      { d_shadow_box_proc, 192, 160, 440, 320,  0,   0,   0,    0,      0,   0,   NULL,                                                   NULL, NULL },
      { d_text_proc,       344, 176, 136,  8,   0,   0,   0,    0,      0,   0,   (void*)"Level Information",                             NULL, NULL },
      { d_text_proc,       216, 216, 360,  8,   0,   0,   0,    0,      0,   0,   (void*)"Level Name:  [                               ]",NULL, NULL },
      { d_text_proc,       248, 248, 152,  8,   0,   0,   0,    0,      0,   0,   (void*)"Number of Lemmings:",                           NULL, NULL },
      { d_text_proc,       216, 272, 184,  8,   0,   0,   0,    0,      0,   0,   (void*)"To Be Saved (Lemmings):",                       NULL, NULL },
      { d_text_proc,       360, 296, 184,  8,   0,   0,   0,    0,      0,   0,   (void*)"Time:    Minutes",                              NULL, NULL },
      { d_text_proc,       344, 320, 56,   8,   0,   0,   0,    0,      0,   0,   (void*)"Rating:",                                       NULL, NULL },
      { d_edit_proc,       328, 216, 248,  8,   0,   0,   0,    0,      30,  0,   (void*)level_name,                                      NULL, NULL },
      { d_edit_proc,       408, 320, 200,  8,   0,   0,   0,    0,      15,  0,   (void*)rating_string,                                      NULL, NULL },
      { d_text_proc,       328, 200, 176,  8,   0,   0,   0,    0,      0,   0,   (void*)"LEVEL XX ...                 30",               NULL, NULL },
      { d_edit_number_proc,408, 248, 48,   8,   0,   0,   0,    0,      3,   0,   no_lemmings_string,                                    (&no_lemmings_edit_number_struct),          NULL },
      { d_edit_number_proc,408, 272, 48,   8,   0,   0,   0,    0,      3,   0,   lemmings_to_be_saved_string,                           (&lemmings_to_be_saved_edit_number_struct), NULL },
      { d_edit_number_proc,408, 296, 24,   8,   0,   0,   0,    0,      1,   0,   time_in_minutes_string,                                (&time_in_minutes_edit_number_struct),      NULL },
      { d_text_proc,       216, 408, 360,  8,   0,   0,   0,    0,      0,   0,   (void*)"Description: [                               ]",NULL, NULL },
      { d_edit_proc,       328, 408, 248,  8,   0,   0,   0,    0,      63,  0,   (void*)description,                                     NULL, NULL },
      { d_button_proc,     240, 440, 128, 24,   0,   0,   0,    D_EXIT, 0,   0,   (void*)"OK",      /* object 15 */                       NULL, NULL },
      { d_button_proc,     448, 440, 128, 24,   0,   0,   0,    D_EXIT, 0,   0,   (void*)"Cancel",  /* object 16 */                       NULL, NULL },
      { NULL,              0,   0,   0,    0,   0,   0,   0,    0,      0,   0,   NULL,                                                   NULL, NULL }
   };

   for (int item = 0; dialog_level_info[item].proc; item++) {
      dialog_level_info[item].fg = WHITE;
      dialog_level_info[item].bg = DGREY;       
   }
   
   int dialog_quit_trigger = popup_dialog(dialog_level_info, -1);
   
   if (dialog_quit_trigger == 15) {
      // This should commit the changes you made into the level_working_on struct.
      strncpy((char *)level_working_on->runtime_stats.level_name,         (const char *)level_name,    31);                         
      strncpy((char *)level_working_on->runtime_stats.rating_description, (const char *)rating_string, 16);                        
      strncpy((char *)level_working_on->runtime_stats.description,        (const char *)description,   64);      
      
      int lemmings_temp;
      int to_be_saved_temp;
      int time_in_minutes_temp;             
      
      sscanf(         no_lemmings_string, "%d", &lemmings_temp);    
      sscanf(lemmings_to_be_saved_string, "%d", &to_be_saved_temp);
      sscanf(     time_in_minutes_string, "%d", &time_in_minutes_temp);
      
      level_working_on->runtime_stats.lemmings        = lemmings_temp;
      level_working_on->runtime_stats.to_be_saved     = to_be_saved_temp;
      level_working_on->runtime_stats.time_in_minutes = time_in_minutes_temp;                 
   } else
   if (dialog_quit_trigger == 16) {
      // Cancelling should take no action on the level_working_on memory!
   }

} 

void MenuSaveImageSequence() {
      bool retry_file_selection = true;         
      do {
         char filename[8192];        
         replace_filename(filename, global_argv[0], "", 8191);
      
         int file_selector_result = file_select_ex("Choose a filename for your screenshot (*.bmp):",
                                                                                                  filename,
                                                                                                     "bmp",
                                                                                                      8191,
                                                                                GLOBAL_FILE_SELECTOR_WIDTH,
                                                                               GLOBAL_FILE_SELECTOR_HEIGHT);
                                                                                          
         if (file_selector_result == 0) {  
            retry_file_selection = false;
         } else {                   
            if (strcmp(get_extension(filename), "bmp") != 0) {
               int retry_bad_selection_result = alert("Invalid file.",
                                                      NULL,           
                                                      "Retry?",
                                                      "Yes", "No",
                                                      'y', 'n');   
                                                      
               if (retry_bad_selection_result == 1) {
               } else
               if (retry_bad_selection_result == 2) {
                  retry_file_selection = false;
               }                                             
            } else {         
               // We've picked a file. It doesn't exist. We think.
               bool retry_file_saving_sequence = true;
               do {                  
                  if (exists(filename)) {
                     int bad_saving_selection_result = alert(NULL,           
                                                             "File already exists, overwrite?",
                                                             NULL,           
                                                             "Yes", "No",
                                                             'y', 'n'); 
                                                              
                     if (bad_saving_selection_result == 1) {
                     } else
                     if (bad_saving_selection_result == 2) {
                        retry_file_saving_sequence = false;
                        continue;
                     }    
                  }        
                  
                  // Let's go for it!
                  BITMAP *level_screenshot = create_bitmap_ex(24, LEVEL_X_SIZE, LEVEL_Y_SIZE);
                  
                  BITMAP *single_screenshot;
                  
                  int original_camera_x_inset = camera_x_inset;
                  
                  for (camera_x_inset = 0; (camera_x_inset) < LEVEL_X_SIZE; camera_x_inset += LEVEL_DISPLAY_X_SIZE) {
                     // This iteration would allow the sub bitmap to be created
                     // over the edge of the level_screenshot bitmap
                     // reset the camera x inset to the inside of the
                     // level for a final iteration.
                     // (This should fulfill the for condition!)
                     if ((camera_x_inset + LEVEL_DISPLAY_X_SIZE) > LEVEL_X_SIZE) {
                        camera_x_inset = LEVEL_X_SIZE - LEVEL_DISPLAY_X_SIZE;
                     }                          
                                                         
                     // ** THIS IS ALL COPIED FROM THE LEVEL RENDERING CODE IN THE MAIN LOOP!!!!! ** //        
                     // ** THIS IS ALL COPIED FROM THE LEVEL RENDERING CODE IN THE MAIN LOOP!!!!! ** //                                                   
                     for (int y = 0; y < DSX_BACKBUFFER_Y_SIZE; y++) {
                        for (int x = 0; x < DSX_BACKBUFFER_X_SIZE; x++) {
                           DSX_backbuff[x][y] = 0;//(u16)(rand()*31.0f*31.0f*31.0f);
                        }  
                     }                                       
                             
                     // Get the raw number for the water genus junction 
                     int water_genus_junction_id = (level_working_on->runtime_stats.water_genus_junction
                                                        & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                      
                     // Return the correct graphical object for the water based on the genus junction value.   
                     LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_water_graphical_object = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_water_graphical_objects.at(water_genus_junction_id)
                                                                                                 :   custom_water_graphical_objects.at(water_genus_junction_id);
                  
                     // Return the correct graphical object sprite for the water based on the genus junction value.   
                     DSX_SPRITE *active_water_graphical_object_sprite = (!(level_working_on->runtime_stats.water_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_water_graphical_object_sprites.at(water_genus_junction_id)
                                                                                                 :   custom_water_graphical_object_sprites.at(water_genus_junction_id);
               
                     for (int w = 0; w < level_working_on->runtime_stats.no_waters; w++) {
                        if (level_working_on->runtime_stats.water_z[w] == WATER_Z_BACKGROUND) {
                           DSX_DrawWaterArea(active_water_graphical_object,
                                             active_water_graphical_object_sprite,
                                             active_water_graphical_object->representing_frame,
                                             level_working_on->runtime_stats.water_x1[w],
                                             level_working_on->runtime_stats.water_x2[w],
                                             level_working_on->runtime_stats.water_y[w]);
                        }                                     
                     }                    
               
                     // Get the raw number for the entrance genus junction 
                     int entrance_genus_junction_id = (level_working_on->runtime_stats.entrance_genus_junction
                                                        & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                      
                     // Return the correct graphical object for the entrance based on the genus junction value.   
                     LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_entrance_graphical_object = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_entrance_graphical_objects.at(entrance_genus_junction_id)
                                                                                                 :   custom_entrance_graphical_objects.at(entrance_genus_junction_id);    
                       
                     // Return the correct graphical object sprite for the entrance based on the genus junction value.   
                     DSX_SPRITE *active_entrance_graphical_object_sprite = (!(level_working_on->runtime_stats.entrance_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_entrance_graphical_object_sprites.at(entrance_genus_junction_id)
                                                                                                 :   custom_entrance_graphical_object_sprites.at(entrance_genus_junction_id);    
                          
                     for (int e = 0; e < level_working_on->runtime_stats.no_entrances; e++) {
                        DSX_DrawEntrance(active_entrance_graphical_object,
                                         active_entrance_graphical_object_sprite,
                                         active_entrance_graphical_object->representing_frame,  
                                         false,
                                         level_working_on->runtime_stats.entrance_x[e],
                                         level_working_on->runtime_stats.entrance_y[e]);
                     }                   
               
                     // Get the raw number for the exit genus junction 
                     int exit_genus_junction_id = (level_working_on->runtime_stats.exit_genus_junction
                                                        & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                      
                     // Return the correct graphical object for the exit based on the genus junction value.   
                     LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_exit_graphical_object = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_exit_graphical_objects.at(exit_genus_junction_id)
                                                                                                 :   custom_exit_graphical_objects.at(exit_genus_junction_id);    
                       
                     // Return the correct graphical object sprite for the exit based on the genus junction value.   
                     DSX_SPRITE *active_exit_graphical_object_sprite = (!(level_working_on->runtime_stats.exit_genus_junction
                                                                                               & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                 ? standard_exit_graphical_object_sprites.at(exit_genus_junction_id)
                                                                                                 :   custom_exit_graphical_object_sprites.at(exit_genus_junction_id);    
                       
                     for (int e = 0; e < level_working_on->runtime_stats.no_exits; e++) {
                        DSX_DrawExit(active_exit_graphical_object,
                                     active_exit_graphical_object_sprite,
                                     active_exit_graphical_object->representing_frame,   
                                     false,
                                     level_working_on->runtime_stats.exit_x[e],
                                     level_working_on->runtime_stats.exit_y[e]);
                     }                   
                                          
                                   
                     for (int e = 0; e < level_working_on->runtime_stats.no_traps; e++) {
                        // Get the raw number for the trap genus junction 
                        int trap_genus_junction_id = (level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                           & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                         
                        // Return the correct graphical object for the trap based on the genus junction value.   
                        LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_trap_graphical_object = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_trap_graphical_objects.at(trap_genus_junction_id)
                                                                                                    :   custom_trap_graphical_objects.at(trap_genus_junction_id);    
                          
                        // Return the correct graphical object sprite for the trap based on the genus junction value.   
                        DSX_SPRITE *active_trap_graphical_object_sprite = (!(level_working_on->runtime_stats.trap_genus_junctions[level_working_on->runtime_stats.trap_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_trap_graphical_object_sprites.at(trap_genus_junction_id)
                                                                                                    :   custom_trap_graphical_object_sprites.at(trap_genus_junction_id);    
                          
                        DSX_DrawTrap(active_trap_graphical_object,
                                     active_trap_graphical_object_sprite,
                                     active_trap_graphical_object->representing_frame,   
                                     false,
                                     level_working_on->runtime_stats.trap_x[e],
                                     level_working_on->runtime_stats.trap_y[e],
                                     level_working_on->runtime_stats.trap_genus_palettes[level_working_on->runtime_stats.trap_genus[e]]);
                     } 
                         
                     for (int e = 0; e < level_working_on->runtime_stats.no_hazards; e++) {
                        // Get the raw number for the hazard genus junction 
                        int hazard_genus_junction_id = (level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                           & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                         
                        // Return the correct graphical object for the hazard based on the genus junction value.   
                        LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_hazard_graphical_object = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_hazard_graphical_objects.at(hazard_genus_junction_id)
                                                                                                    :   custom_hazard_graphical_objects.at(hazard_genus_junction_id);    
                          
                        // Return the correct graphical object sprite for the hazard based on the genus junction value.   
                        DSX_SPRITE *active_hazard_graphical_object_sprite = (!(level_working_on->runtime_stats.hazard_genus_junctions[level_working_on->runtime_stats.hazard_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_hazard_graphical_object_sprites.at(hazard_genus_junction_id)
                                                                                                    :   custom_hazard_graphical_object_sprites.at(hazard_genus_junction_id);    
                          
                        DSX_DrawHazard(active_hazard_graphical_object,
                                     active_hazard_graphical_object_sprite,
                                     active_hazard_graphical_object->representing_frame,    
                                     false,
                                     level_working_on->runtime_stats.hazard_x[e],
                                     level_working_on->runtime_stats.hazard_y[e],
                                     level_working_on->runtime_stats.hazard_genus_palettes[level_working_on->runtime_stats.hazard_genus[e]]);
                     }        
                     
                     for (int e = 0; e < level_working_on->runtime_stats.no_uninteractives; e++) {
                        // Get the raw number for the uninteractive genus junction 
                        int uninteractive_genus_junction_id = (level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                           & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                         
                        // Return the correct graphical object for the uninteractive based on the genus junction value.   
                        LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_uninteractive_graphical_object = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_uninteractive_graphical_objects.at(uninteractive_genus_junction_id)
                                                                                                    :   custom_uninteractive_graphical_objects.at(uninteractive_genus_junction_id);    
                          
                        // Return the correct graphical object sprite for the uninteractive based on the genus junction value.   
                        DSX_SPRITE *active_uninteractive_graphical_object_sprite = (!(level_working_on->runtime_stats.uninteractive_genus_junctions[level_working_on->runtime_stats.uninteractive_genus[e]]
                                                                                                  & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT))
                                                                                                    ? standard_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id)
                                                                                                    :   custom_uninteractive_graphical_object_sprites.at(uninteractive_genus_junction_id);    
                          
                        DSX_DrawUninteractive(active_uninteractive_graphical_object,
                                       active_uninteractive_graphical_object_sprite,
                                       active_uninteractive_graphical_object->representing_frame, 
                                       false,
                                       level_working_on->runtime_stats.uninteractive_x[e],
                                       level_working_on->runtime_stats.uninteractive_y[e],
                                       level_working_on->runtime_stats.uninteractive_genus_palettes[level_working_on->runtime_stats.uninteractive_genus[e]]);
                     }           
                     
                     DSX_DrawLevel();                    
               
                     for (int w = 0; w < level_working_on->runtime_stats.no_waters; w++) {
                        if (level_working_on->runtime_stats.water_z[w] == WATER_Z_FOREGROUND) {
                           DSX_DrawWaterArea(active_water_graphical_object,
                                             active_water_graphical_object_sprite,
                                             active_water_graphical_object->representing_frame,
                                             level_working_on->runtime_stats.water_x1[w],
                                             level_working_on->runtime_stats.water_x2[w],
                                             level_working_on->runtime_stats.water_y[w]);
                        }                
                     }                            
                     // ** THIS IS ALL COPIED FROM THE LEVEL RENDERING CODE IN THE MAIN LOOP!!!!! ** //        
                     // ** THIS IS ALL COPIED FROM THE LEVEL RENDERING CODE IN THE MAIN LOOP!!!!! ** //        
                     
                     single_screenshot = create_sub_bitmap(level_screenshot, camera_x_inset, 0, LEVEL_DISPLAY_X_SIZE, LEVEL_DISPLAY_Y_SIZE);
 
                     DSX_MagicDSXScreenToLevelPane1X(single_screenshot);
                     
                     //allegro_message("Drawing from %d, %d - %d, %d. %08X", camera_x_inset, 0, camera_x_inset + LEVEL_DISPLAY_X_SIZE - 1, LEVEL_DISPLAY_Y_SIZE - 1, (int)single_screenshot);
                     
                     destroy_bitmap(single_screenshot);   
                  }
                  
                  save_bmp(filename, level_screenshot, NULL);
                  
                  destroy_bitmap(level_screenshot);
                  
                  camera_x_inset = original_camera_x_inset;
                  
                  retry_file_saving_sequence = false;
                  return;                  
               } while (retry_file_saving_sequence);                                          
            }
         }
      } while (retry_file_selection); 
   
   // Use the return keyword from inside the loop if the load went as planned.
   // Otherwise it'll run out here and produce this error.
    
   alert(NULL,           
         "Save Cancelled",
         NULL,
         "OK", NULL,
         0, 0);   
   return;
}                                

typedef struct tagALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT {
   const char *string[3];
} ALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT;   

#define SECRET_STRING_LENGTH 20

#define SECRET_STRING_CODE(a00, a01, a02, a03, a04, a05, a06, a07, a08, a09, \
                           a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) \
                     a00 * a00 , a01 * a01 , a02 * a02 , a03 * a03 , a04 * a04 ,   \
                     a05 * a05 , a06 * a06 , a07 * a07 , a08 * a08 , a09 * a09 ,   \
                     a10 * a10 , a11 * a11 , a12 * a12 , a13 * a13 , a14 * a14 ,   \
                     a15 * a15 , a16 * a16 , a17 * a17 , a18 * a18 , a19 * a19                      

const u16 secret_string_source_1[SECRET_STRING_LENGTH] =
//                       1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0
   {SECRET_STRING_CODE('T','h','i','s',' ','v','e','r','s','i','o','n',' ','t','o',':',  0,  0,  0,  0)};
const u16 secret_string_source_2[SECRET_STRING_LENGTH] =
//                       1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0
   {SECRET_STRING_CODE('A','l','l',  0,'o','f',  0,'y','a','!',  0,  0,  0,  0,  0,  0,  0,  0,  0,  0)};
const u16 secret_string_source_3[SECRET_STRING_LENGTH] =
//                       1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0
   {SECRET_STRING_CODE(  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0)};

         
int d_hidden_button_alert_proc(int msg, DIALOG *d, int c) {  
   if (msg == MSG_CLICK) {
      alert(((ALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT *)(d->dp2))->string[0],
            ((ALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT *)(d->dp2))->string[1],
            ((ALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT *)(d->dp2))->string[2],
                      "Roger",
                         NULL,
                            0,
                            0);
   }
   return D_O_K;
}           
           
void MenuAboutDialogSequence() {
   char secret_string_1[SECRET_STRING_LENGTH];
   char secret_string_2[SECRET_STRING_LENGTH];
   char secret_string_3[SECRET_STRING_LENGTH];
   
   for (int c = 0; c < SECRET_STRING_LENGTH; c++) {
      secret_string_1[c] = IntSqrt(secret_string_source_1[c]);
      secret_string_2[c] = IntSqrt(secret_string_source_2[c]);
      secret_string_3[c] = IntSqrt(secret_string_source_3[c]);
   }                                          
   
   ALLEGRO_DIALOG_DP2_HIDDEN_BUTTON_ALERT_STRUCT secret_dialog = {secret_string_1, secret_string_2, secret_string_3};
     
   DIALOG about_screen_dialog[] = {
      /* (proc)                         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                                                     (dp2) (dp3) */
      { d_shadow_box_proc,              200, 192, 400, 224, 0,   0,   0,    0,      0,   0,   NULL,                                                    NULL, NULL },
      { d_ctext_proc,                   400, 208, 32,  8,   0,   0,   0,    0,      0,   0,   (void *)"About the Lemmings DS Builder",                 NULL, NULL }, 
      
      { d_ctext_proc,                   400, 240, 32,  8,   0,   0,   0,    0,      0,   0,   (void *)"This version built 10:05pm on 14th Jun. 2007",  NULL, NULL }, 
      { d_ctext_proc,                   400, 256, 32,  8,   0,   0,   0,    0,      0,   0,   (void *)"by Mathew Carr",                                NULL, NULL }, 
   
      { d_hidden_button_alert_proc,     0,   0,   40,  40,  0,   0,   0,    0,      0,   0,   NULL,                                                    (void *)&secret_dialog, NULL},
      { d_button_proc,                  352, 384, 96,  16,  0,   0,   0,    D_EXIT, 0,   0,   (void *)"Yay!",                                          NULL, NULL },
      { NULL,                           0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                                                    NULL, NULL }
   };  
                      
   for (int item = 0; about_screen_dialog[item].proc; item++) {
      about_screen_dialog[item].fg = WHITE;
      about_screen_dialog[item].bg = DGREY;       
   }                          
   
   popup_dialog(about_screen_dialog, 4);     
}

struct TEXTURE_ARCHIVE_REFERENCE {
   char *real_name;      // The real name of the texture archive.
   char *displayed_name; // The displayed name of the texture archive
   
   bool custom; // Aflag showing whether or not this texture archive is custom.    
};

// This manages all of the strings used in the texture archive selection
// listbox.
std::vector<TEXTURE_ARCHIVE_REFERENCE> texture_archive_selection_listbox_strings;

char *TextureArchiveSelectionListbox(int index, int *list_size) {
   // If index is zero or positive, the function should return
   // a pointer to the string which is to be displayed at position
   // index in the list. If index is negative, it should return
   // NULL and list_size should be set to the number of items
   // in the list.
   
   if (index < 0) {
      *list_size = texture_archive_selection_listbox_strings.size();
      
      return NULL;
   } else {
      return texture_archive_selection_listbox_strings[index].displayed_name;    
   }     
}

bool MenuSelectTextureArchiveSequence(char *selected_archive_string_buffer, bool cancel_permitted) {
   // This structure holds the result of a file search using allegro's functions.
   al_ffblk allegro_search_results;  
    
   int search_return;
   
   search_return = al_findfirst("standard_texture_archives/*.LTA", &allegro_search_results, FA_RDONLY | FA_HIDDEN | FA_ARCH);    
   
   while (search_return == 0) {
      TEXTURE_ARCHIVE_REFERENCE new_texture_archive_reference;   
         
      new_texture_archive_reference.real_name = new char[512];
      memset(new_texture_archive_reference.real_name, 0, 512);
      // Copy the filename without the extension to the newly instantiated string.
      strncpy(new_texture_archive_reference.real_name, allegro_search_results.name, strlen(allegro_search_results.name) - 4);
         
      new_texture_archive_reference.displayed_name = new char[512];
      memset(new_texture_archive_reference.displayed_name, 0, 512);
      strcpy(new_texture_archive_reference.displayed_name, new_texture_archive_reference.real_name);
      strcat(new_texture_archive_reference.displayed_name, " <Standard>");
         
      texture_archive_selection_listbox_strings.push_back(new_texture_archive_reference);   
      
      search_return = al_findnext(&allegro_search_results);     
   }
   
   al_findclose(&allegro_search_results);
   
   search_return = al_findfirst("custom_texture_archives/*.LTA", &allegro_search_results, FA_RDONLY | FA_HIDDEN | FA_ARCH);    
   
   while (search_return == 0) {
      TEXTURE_ARCHIVE_REFERENCE new_texture_archive_reference;   
         
      new_texture_archive_reference.real_name = new char[512];
      memset(new_texture_archive_reference.real_name, 0, 512);
      // Copy the filename without the extension to the newly instantiated string.
      strncpy(new_texture_archive_reference.real_name, allegro_search_results.name, strlen(allegro_search_results.name) - 4);
         
      bool replacement_occured = false;   
         
      // Try to find a matching real name in the vector already.
      for (int i = 0; i < texture_archive_selection_listbox_strings.size(); i++) {
         if (stricmp(new_texture_archive_reference.real_name, texture_archive_selection_listbox_strings[i].real_name) == 0) {
            texture_archive_selection_listbox_strings[i].custom = true;     
            
            memset(texture_archive_selection_listbox_strings[i].displayed_name, 0, 512);
            strcpy(texture_archive_selection_listbox_strings[i].displayed_name, new_texture_archive_reference.real_name);
            strcat(texture_archive_selection_listbox_strings[i].displayed_name, " <Custom>");
            
            replacement_occured = true;
         }
      }   
         
      if (!replacement_occured) {         
         new_texture_archive_reference.displayed_name = new char[512];
         memset(new_texture_archive_reference.displayed_name, 0, 512);
         strcpy(new_texture_archive_reference.displayed_name, new_texture_archive_reference.real_name);
         strcat(new_texture_archive_reference.displayed_name, " <Custom>");          
          
         texture_archive_selection_listbox_strings.push_back(new_texture_archive_reference);  
      }
          
      
      search_return = al_findnext(&allegro_search_results);     
   }
   
   al_findclose(&allegro_search_results);
                 
   DIALOG texture_archive_selection_screen_dialog[] = {
      /* (proc)            (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                                      (dp2) (dp3) */
      { d_shadow_box_proc, 216, 144, 368, 304, 0,   0,   0,    0,      0,   0,   NULL,                                     NULL, NULL },
      { d_ctext_proc,      400, 160, 136, 8,   0,   0,   0,    0,      0,   0,   (void*)"Select Texture Archive to Link:", NULL, NULL },
      { d_text_list_proc,  256, 184, 280, 208, 0,   0,   0,    0,      0,   0,   (void*)TextureArchiveSelectionListbox,    NULL, NULL },
      { d_button_proc,     232, 408, 160, 24,  0,   0,   0,    D_EXIT, 0,   0,   (void*)"Link and Continue",               NULL, NULL },
      { d_button_proc,     408, 408, 160, 24,  0,   0,   0,    D_EXIT, 0,   0,   (void*)"Do not Link; Abort",              NULL, NULL },
      { NULL,              0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                                     NULL, NULL }
   };
                      
   for (int item = 0; texture_archive_selection_screen_dialog[item].proc; item++) {
      texture_archive_selection_screen_dialog[item].fg = WHITE;
      texture_archive_selection_screen_dialog[item].bg = DGREY;       
   }                          
   
   if (!cancel_permitted) {
      texture_archive_selection_screen_dialog[4].flags |= D_DISABLED;
   }
   
   int button_exit = popup_dialog(texture_archive_selection_screen_dialog, 4); 
                
   bool result = false;
   
   if (button_exit == 3) {
      strcpy(selected_archive_string_buffer, texture_archive_selection_listbox_strings[texture_archive_selection_screen_dialog[2].d1].real_name); 
      result = true;    
   } else {
      result = false;    
   }
                          
   for (int s = 0; s < texture_archive_selection_listbox_strings.size(); s++) {
      delete[] texture_archive_selection_listbox_strings[s].displayed_name; 
      delete[] texture_archive_selection_listbox_strings[s].real_name; 
   }
   texture_archive_selection_listbox_strings.clear();
   
   return result;
}
 
////////////////////////////////////////////////////////////////

void InitialiseGUI() {
   gui_fg_color = WHITE;
   gui_bg_color = DGREY;  
     
   editor_currently_selected_tool = EDITOR_TOOL_SELECT_TEXTURE;
     
   right_interface_current_tab                     = RIGHT_INTERFACE_TAB_TEXTURE_SELECTION;                       
   right_interface_texture_selection_current_set   = RIGHT_INTERFACE_TEXTURE_SELECTION_SET_STANDARD_16;
   right_interface_texture_selection_current_page  = 0;                           
   right_interface_currently_selected_texture      = 0;
   right_interface_currently_selected_texture_type = 0;    
   RecalculateRightInterfaceTextureSelectionDatabaseSize();
   
   for (int p = 0; p < 256; p++) {
      right_interface_currently_held_pal_map[p] = p;
   }                              
   
   right_interface_currently_selected_texture_swap_axes_value = 0;
                       
   currently_selected_object_blink_frame = 0;
   transparent_colour_blink_frame = 0;
   
   right_interface_currently_selected_palette_mapper_palette_entry = 0;  

   right_interface_currently_selected_texture_window_zoom = 1;

   right_interface_currently_selected_palette_editor_left_colour  = 1;
   right_interface_currently_selected_palette_editor_right_colour = 2;
                                                                            
   Split15BitColour(level_working_on->runtime_stats.level_palette[right_interface_currently_selected_palette_editor_left_colour],
                    &right_interface_currently_selected_palette_editor_colour_r_value,
                    &right_interface_currently_selected_palette_editor_colour_g_value,
                    &right_interface_currently_selected_palette_editor_colour_b_value);       
                    
   right_interface_special_objects_current_object = 0;
   right_interface_special_objects_current_genus  = 0;      
     
   ResetSpecialObjectPalettePointerBasedOnGenus();  
             
   right_interface_currently_selected_special_objects_left_colour  = 1;
   right_interface_currently_selected_special_objects_right_colour = 2;
   
   Split15BitColour(level_working_on->runtime_stats.entrance_palette[right_interface_currently_selected_special_objects_left_colour],
                    &right_interface_currently_selected_special_objects_left_colour_r_value,
                    &right_interface_currently_selected_special_objects_left_colour_g_value,
                    &right_interface_currently_selected_special_objects_left_colour_b_value); 
                    
   SelectionClear();
   SelectionSpecialClear();
   SelectionAreaClear();
   
   lower_interface_current_option_set = LOWER_INTERFACE_OPTION_SET_DEFAULT;

   lower_interface_currently_selected_texture_adjustment_tox_value = 0;
   lower_interface_currently_selected_texture_adjustment_toy_value = 0;
   lower_interface_currently_selected_texture_adjustment_tsx_value = 64;
   lower_interface_currently_selected_texture_adjustment_tsy_value = 64;

   lower_interface_currently_selected_subtractive_value    = 0;
   lower_interface_currently_selected_masking_behaviour_value = LOWER_INTERFACE_CURRENTLY_SELECTED_MASKING_BEHAVIOUR_VALUE_NO_MASKING;
                                 
   RefreshAppendedTexture256sBegin();
   RefreshTerrainObjectsBegin();
}
                                                          
void InitialiseLevel(const char *texture_archive_without_extension) {            
   memset(level_working_on, 0, sizeof(LEMMINGS_LEVEL_LDS_FILE_V7));               
   
   level_working_on->lemmings_level_file_size = sizeof(LEMMINGS_LEVEL_LDS_FILE_V7);    
   level_working_on->version_number           = LEMMINGS_LEVEL_VERSION;
                                                   
   strcpy(level_working_on->validation_string, correct_validation_string);

   strcpy((char*)level_working_on->runtime_stats.level_name,  "Enter Level Name Here");
   strcpy((char*)level_working_on->runtime_stats.description, "Enter Level Description Here");
                                              
   level_working_on->runtime_stats.lemmings        = 20;
   level_working_on->runtime_stats.to_be_saved     = 10;
   level_working_on->runtime_stats.release_rate    = 50;
   level_working_on->runtime_stats.time_in_minutes = 5;         
   strcpy(level_working_on->runtime_stats.rating_description, "Custom");

   level_working_on->runtime_stats.tool_complement[TOOL_CLIMBER]  = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_FLOATER]  = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_EXPLODER] = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_BLOCKER]  = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_BUILDER]  = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_BASHER]   = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_MINER]    = 20;
   level_working_on->runtime_stats.tool_complement[TOOL_DIGGER]   = 20;

   level_working_on->runtime_stats.no_entrances      = 0;
   level_working_on->runtime_stats.no_exits          = 0;  
   level_working_on->runtime_stats.no_traps          = 0;   
   level_working_on->runtime_stats.no_hazards        = 0;   
   level_working_on->runtime_stats.no_uninteractives = 0;    
   level_working_on->runtime_stats.no_waters         = 0;      
   level_working_on->runtime_stats.no_steel_areas    = 0; 
   level_working_on->runtime_stats.no_one_way_areas  = 0;    
   
   level_working_on->runtime_stats.camera_x         = 128;
   level_working_on->runtime_stats.camera_y         = 168/2;            
   
   CameraFocusTo(level_working_on->runtime_stats.camera_x, level_working_on->runtime_stats.camera_y);
     
   level_working_on->one_way_colour = 255; 
   
   char texture_archive_load_location[1024];
   
   sprintf(texture_archive_load_location, "custom_texture_archives/%s.LTA", texture_archive_without_extension);
   
   // First check custom location:
   if (!file_exists(texture_archive_load_location)) {
      sprintf(texture_archive_load_location, "standard_texture_archives/%s.LTA", texture_archive_without_extension);
   
      if (!file_exists(texture_archive_load_location)) {
         allegro_message("Texture archive loading error.");
         exit(1); 
      }
   }
   
   FILE *texture_archive = fopen(texture_archive_load_location, "rb");
   
      if (texture_archive == NULL) {
         allegro_message("Texture archive loading error.");
         exit(1); 
      } else {
         u32 loaded_file_size;
         
         fread((void *)&loaded_file_size, 4, 1, texture_archive);
         
         if (loaded_active_texture_archive != NULL) delete[] loaded_active_texture_archive;
             
         loaded_active_texture_archive = (LEMMINGS_TEXTURE_ARCHIVE_HEADER *)new u8[loaded_file_size];    
         
         //allegro_message("load in size = %d", loaded_file_size);
          
         rewind(texture_archive);
         
         fread(loaded_active_texture_archive, loaded_file_size, 1, texture_archive);   
      }
      
   fclose(texture_archive);  
   
   strcpy(level_working_on->texture_archive_using, texture_archive_without_extension);                                                      
   
   for (int palette_entry = 0; palette_entry < 256; palette_entry++) {
      level_working_on->runtime_stats.level_palette[palette_entry] = loaded_active_texture_archive->ideal_palette[palette_entry]; 
   }                             
   
   for (int palette_entry = 0; palette_entry < 16; palette_entry++) {
      level_working_on->runtime_stats.entrance_palette[     palette_entry] = standard_entrance_graphical_objects     .at(0)->ideal_palette[palette_entry]; 
      level_working_on->runtime_stats.exit_palette[         palette_entry] = standard_exit_graphical_objects         .at(0)->ideal_palette[palette_entry]; 
      
      for (int e = 0; e < NO_TRAP_GENUSES; e++) {
         level_working_on->runtime_stats.trap_genus_palettes[e][palette_entry] = standard_trap_graphical_objects.at(0)->ideal_palette[palette_entry]; 
      }
      for (int e = 0; e < NO_HAZARD_GENUSES; e++) {
         level_working_on->runtime_stats.hazard_genus_palettes[e][palette_entry] = standard_hazard_graphical_objects.at(0)->ideal_palette[palette_entry]; 
      }
      for (int e = 0; e < NO_UNINTERACTIVE_GENUSES; e++) {
         level_working_on->runtime_stats.uninteractive_genus_palettes[e][palette_entry] = standard_uninteractive_graphical_objects.at(0)->ideal_palette[palette_entry]; 
      }
      
      level_working_on->runtime_stats.water_palette[palette_entry] = standard_water_graphical_objects.at(0)->ideal_palette[palette_entry]; 
   }
   
   level_working_on->no_terrain_objects = 0;

   RefreshAppendedTexture256sBegin();   
   RefreshTerrainObjectsBegin();         
   
   RenderLevel(level_data, level_working_on, loaded_active_texture_archive);
}
                       
static bool exitrequest = false;

void die(void) {
   exitrequest = true;
}                     
                       
int main (int argc, char *argv[]) {
   global_argc = argc; 
   global_argv = argv;
   
   allegro_init();
                              
   install_keyboard();        
   install_mouse();        
   install_timer();                                 
   set_color_depth(24);                                
               
   if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, WINDOW_X_SIZE, WINDOW_Y_SIZE, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
      return 1;
   }                                               
                                
   set_window_close_hook(die);     
    
   set_window_title("Lemmings DS Builder 2007-06-04");  
   
   MakeColours();                       
   
   text_mode(-1);            
   
   bitmap_backbuffer = create_bitmap(WINDOW_X_SIZE, WINDOW_Y_SIZE);  
   clear_bitmap(bitmap_backbuffer);                 
                                      
   bitmap_pane_level = create_bitmap(BITMAP_PANE_LEVEL_X_SIZE, BITMAP_PANE_LEVEL_Y_SIZE);  
   clear_bitmap(bitmap_pane_level); 
   
   LoadLevelEditorResources();   
   
   SetUpRightInterfaceTexturePreviewCells();     
   
   main_memory_chunk = new u8[MAIN_MEMORY_CHUNK_SIZE];   
   level_working_on = (LEMMINGS_LEVEL_LDS_FILE_V7 *)main_memory_chunk;
                     
   char texture_archive_selected[2048];
   
   if (!MenuSelectTextureArchiveSequence(texture_archive_selected, false)) exit(1);
                                                          
   DSX_GreedyGatherAllAvailableGraphicalObjects();
                                                         
   InitialiseLevel(texture_archive_selected);            
   InitialiseGUI();        
   
      LevelEditorLoop();   /////////////////////////////////////////////////////////////
   
   DSX_DestroyGraphicalObjectsAndSprites();
   
   delete[] main_memory_chunk;    
   
   delete[] loaded_active_texture_archive;
   
   UnloadLevelEditorResources();    
   
   destroy_bitmap(bitmap_pane_level);
   destroy_bitmap(bitmap_backbuffer);
      
   allegro_exit();
   return 0;
} END_OF_MAIN();

