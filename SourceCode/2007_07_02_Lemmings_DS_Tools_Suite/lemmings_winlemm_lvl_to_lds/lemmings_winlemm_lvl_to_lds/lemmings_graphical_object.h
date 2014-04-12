//
// Lemmings DS Shared File
//
// (c) May 2007
//
// lemmings_graphical_object.c                           
//   Structures and headers for Lemmings DS 'Graphical Object' management.
//   Graphical objects are graphical data and object properties all mashed into one.
//   Like a 'triggerable trap' object has trap graphic data and trap properties
//   like size and trigger zone etc.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#ifndef LEMMINGS_GRAPHICAL_OBJECT_H
#define LEMMINGS_GRAPHICAL_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#ifdef ARM9          
#include <nds.h>     

#include "ds_sprite.h"     
#else
#include "ds_types.h"
                       
#ifndef DONT_NEED_DSX
#define DS_SPRITE DSX_SPRITE   

#include "dsx_sprite.h"       
#endif
#endif


typedef struct tagLEMMINGS_GRAPHICAL_OBJECT_HEADER {
   u32 graphical_object_file_size; // This is the size of this texture archive file in bytes.   
                                  // Because these are always word padded, this is always a multiple of four.
                                  
   u32 version_number;
   
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_EXIT             0 // Lemmings exit through these.
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_ENTRANCE         1 // Lemmings enter through these.
                                                          // These are always ten frames long.
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_TRAP             2 // These are triggered by approaching lemmings.
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_HAZARD           3 // These incinerate all lemmings within range.
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_UNINTERACTIVE    4 // Lemmings walk in front of these.
#define LEMMINGS_GRAPHICAL_OBJECT_TYPE_WATER            5 // Lemmings drown in these.
   
   u32 graphical_object_type;           

   u32 graphic_width;             // These are the width and height of the actual graphic being displayed.
   u32 graphic_height;
   
   u32 no_total_frames;
                                  
   u32 no_primary_frames;         // The number of primary animation frames for this graphical object.
   u32 no_secondary_frames;                           
   
   u32 representing_frame;        // What frame should be shown as the preview in the level editor?

   s32 handle_x; // Where is the handle located on this graphical object?
   s32 handle_y; // (The handle is the active coordinate within the object)
   
   s32 active_zone_x1; // This is the zone within which a trap can be triggered (relative to the handle)
   s32 active_zone_y1; // it is also the active area for hazards.
   s32 active_zone_x2; 
   s32 active_zone_y2;
   
#define LEMMINGS_GRAPHICAL_OBJECT_ACTIVE_FLAG_TRAP_HURTS_GROUNDED   0x00000001
#define LEMMINGS_GRAPHICAL_OBJECT_ACTIVE_FLAG_TRAP_HURTS_UNGROUNDED 0x00000002
   u32 active_flags; // These flags control the behaviour of traps and other stuff

   char graphical_object_name[16];  // Descriptive name for this graphical object.
   
   u16 ideal_palette[16];         // The ideal palette for this graphical object. 0 is transparent.  
                                                               
   u32 graphic_offsets[];         // These are the offsets from (&this_object) where the frames themselves exist
} LEMMINGS_GRAPHICAL_OBJECT_HEADER;

// This holds information about a single frame within a graphical object
typedef struct tagLEMMINGS_GRAPHICAL_OBJECT_GRAPHIC {
   u32 graphic_size; // This is the size of this LEMMINGS_GRAPHICAL_OBJECT_GRAPHIC instance
                     // within the graphical object file. 
                     // Starting at ((u8 *)(texture))[sizeof(tagLEMMINGS_GRAPHICAL_OBJECT_GRAPHIC)]
                     // Because these are always word padded, this is always a multiple of four.
                              
   u32 frame_length; // How long should this frame be displayed for before moving onto the next?
                           
   u8 data[];        // After this is image data.
} LEMMINGS_GRAPHICAL_OBJECT_GRAPHIC;
// These are always aligned word boundaries within a graphical object.
                
static inline LEMMINGS_GRAPHICAL_OBJECT_GRAPHIC *GetGraphicalObjectGraphic(const LEMMINGS_GRAPHICAL_OBJECT_HEADER *loaded_active_graphical_object, int n) {     
   return (LEMMINGS_GRAPHICAL_OBJECT_GRAPHIC *)(((u8*)(loaded_active_graphical_object)) + loaded_active_graphical_object->graphic_offsets[n]);                               
}

#ifndef DONT_NEED_DSX

// This function will malloc a memory area of sizeof(DS_SPRITE) * the number of frames in the graphical object.
// and return it.
// To free this array, just free the single pointer, okay?
static inline DS_SPRITE *GraphicalObject_ConstructDSSpriteArray(const LEMMINGS_GRAPHICAL_OBJECT_HEADER *loaded_active_graphical_object, u16 *palette_to_set) {
   DS_SPRITE *malloced_array = (DS_SPRITE *)malloc(sizeof(DS_SPRITE) * loaded_active_graphical_object->no_total_frames);
   
   for (unsigned int frame = 0; frame < loaded_active_graphical_object->no_total_frames; frame++) {
      malloced_array[frame].data    = GetGraphicalObjectGraphic(loaded_active_graphical_object, frame)->data; 
      
      malloced_array[frame].width   = loaded_active_graphical_object->graphic_width;
      malloced_array[frame].height  = loaded_active_graphical_object->graphic_height;
      
      malloced_array[frame].palette = palette_to_set;
   }
   
   return malloced_array;   
}

#endif
                              
#ifdef __cplusplus
}
#endif

#endif // LEMMINGS_GRAPHICAL_OBJECT_H
