//
// Lemmings DS Shared File
//
// (c) April 2006, May 2006, June 2006, July 2006, August 2006
//
// lemmings_level.h
//   Structures and headers for Lemmings DS Level management, and rendering.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//
// This is the header for V. 2 Lemmings DS Levels!
//

#ifndef __LEMMINGS_LEVEL_H__
#define __LEMMINGS_LEVEL_H__
                                       
// This controls the compatibility of every app compiled using this header
#define LEMMINGS_LEVEL_VERSION 7

#ifdef __cplusplus
extern "C" {
#endif

#include "utility.h"
#include "types.h"

#include "lemmings_texture_archive.h"

// These are constants! CONSTANTS!
#define LEVEL_X_SIZE  (1600)
#define LEVEL_Y_SIZE  (168)

#define LEVEL_PREVIEW_DATA_X_SIZE (240)
#define LEVEL_PREVIEW_DATA_Y_SIZE (32)

// These count the number of pixels expressed in a single pixel of the preview data.
#define LEVEL_PREVIEW_DATA_X_RATIO (LEVEL_X_SIZE / LEVEL_PREVIEW_DATA_X_SIZE)
#define LEVEL_PREVIEW_DATA_Y_RATIO (LEVEL_Y_SIZE / LEVEL_PREVIEW_DATA_Y_SIZE)

// Hopefully this will allow the game to lock onto levels before and after by searching for the
// meow!
extern const char *correct_validation_string;             
// Back when the levels were appended onto a ROM, this would have made some sense
// However, it's just another validation thing now.

// These control the maximum number of each unique object you can place.
#define MAX_NO_ENTRANCES          16  
#define MAX_NO_EXITS              16    

#define MAX_NO_TRAPS              64
#define MAX_NO_HAZARDS            64
#define MAX_NO_UNINTERACTIVES     64   
#define MAX_NO_WATERS             16

#define NO_TRAP_GENUSES           16 // How many distinct trap types can you have at once?
#define NO_HAZARD_GENUSES          8 // How many distinct hazard types can you have at once?
#define NO_UNINTERACTIVE_GENUSES  26 // How many distinct uninteractive types can you have at once?
     
#define MAX_NO_STEEL_AREAS        64        
#define MAX_NO_ONE_WAY_AREAS      64

// These values define what values of z make the objects appear in
// behind or in front of the level texture scenery.
#define TRAP_Z_FOREGROUND 1
#define TRAP_Z_BACKGROUND 0

#define HAZARD_Z_FOREGROUND 1
#define HAZARD_Z_BACKGROUND 0

#define UNINTERACTIVE_Z_FOREGROUND 1
#define UNINTERACTIVE_Z_BACKGROUND 0

#define WATER_Z_FOREGROUND 1
#define WATER_Z_BACKGROUND 0

// *_genus_junctions[] returns the JUNCTIONED FINAL LINK TO OBJECT (exit standard 0, custom 1, etc) VALUE for a given genus.
// *_genus returns the GENUS for a given entity, eg: trap 0 has genus 1, trap 2 has genus 0, trap 3 has genus 0
// Traps 2 and 3 are the same genus, and will act the same.

typedef struct tagLEMMINGS_LEVEL_RUNTIME_STATS_V7 {
   char  level_name[32]; // Thirty characters,  null terminated.
   char description[64]; // Sixty three characters,  null terminated.
   
   u32  lemmings;
   u32  to_be_saved;
   u32  release_rate;
   u32  time_in_minutes;
   char rating_description[16]; // Fifteen characters, null terminated

   s32  camera_x;
   s32  camera_y;

   u8   tool_complement[8];            

   u16  level_palette[256];

   /* ---------------- */    

// This bit is set in the genus junction id if it is junctioned to a custom object.
#define LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT (1<<31)

   u32  no_entrances;   
   u32  entrance_genus_junction;    
   u16  entrance_palette[16];     

   s32  entrance_x[MAX_NO_ENTRANCES];
   s32  entrance_y[MAX_NO_ENTRANCES];
   u32  entrance_d[MAX_NO_ENTRANCES];   
  

   /* ---------------- */
                                      
   u32  no_exits;      
   u32  exit_genus_junction;        
   u16  exit_palette[16];   
   
   s32  exit_x[MAX_NO_EXITS];
   s32  exit_y[MAX_NO_EXITS];                                 

   /* ---------------- */          
      
   u32  no_traps;           
   u32  trap_genus_junctions[NO_TRAP_GENUSES]; // What object is junctioned to each genus?
   u32  trap_genus[MAX_NO_TRAPS];              // What genus does each trap belong to?
   
   s32  trap_x[MAX_NO_TRAPS];
   s32  trap_y[MAX_NO_TRAPS];
   s32  trap_z[MAX_NO_TRAPS];
   
   u16  trap_genus_palettes[NO_TRAP_GENUSES][16];

   /* ---------------- */           
      
   u32  no_hazards;           
   u32  hazard_genus_junctions[NO_HAZARD_GENUSES]; // What object is junctioned to each genus?   
   u16  hazard_genus_palettes[NO_HAZARD_GENUSES][16];
   u32  hazard_genus[MAX_NO_HAZARDS];              // What genus does each hazard belong to?
   
   s32  hazard_x[MAX_NO_HAZARDS];
   s32  hazard_y[MAX_NO_HAZARDS];
   s32  hazard_z[MAX_NO_HAZARDS];
   

   /* ---------------- */       
      
   u32  no_uninteractives;           
   u32  uninteractive_genus_junctions[NO_UNINTERACTIVE_GENUSES]; // What object is junctioned to each genus?    
   u16  uninteractive_genus_palettes[NO_UNINTERACTIVE_GENUSES][16];
   u32  uninteractive_genus[MAX_NO_UNINTERACTIVES];              // What genus does each uninteractive belong to?
   
   s32  uninteractive_x[MAX_NO_UNINTERACTIVES];
   s32  uninteractive_y[MAX_NO_UNINTERACTIVES];   
   u32  uninteractive_z[MAX_NO_UNINTERACTIVES];
   

   /* ---------------- */         

   u32  no_waters;     
   u32  water_genus_junction;     
   u16  water_palette[16]; 

   s32  water_x1[MAX_NO_WATERS];
   s32  water_x2[MAX_NO_WATERS];
   s32  water_y[MAX_NO_WATERS];
   u32  water_z[MAX_NO_WATERS];
   
   /* ---------------- */      
   
   u32  no_steel_areas;

   s32  steel_area_x1[MAX_NO_STEEL_AREAS];
   s32  steel_area_y1[MAX_NO_STEEL_AREAS];
   s32  steel_area_x2[MAX_NO_STEEL_AREAS];
   s32  steel_area_y2[MAX_NO_STEEL_AREAS];    

   u32  no_one_way_areas;

   s32  one_way_area_x1[MAX_NO_ONE_WAY_AREAS];
   s32  one_way_area_y1[MAX_NO_ONE_WAY_AREAS];
   s32  one_way_area_x2[MAX_NO_ONE_WAY_AREAS];
   s32  one_way_area_y2[MAX_NO_ONE_WAY_AREAS];

   u32  one_way_area_d[MAX_NO_ONE_WAY_AREAS];  
   
   /* ---------------- */
} LEMMINGS_LEVEL_RUNTIME_STATS_V7;

typedef struct tagLEMMINGS_LEVEL_LDS_FILE_V7 {
   u32  lemmings_level_file_size;

   u32  version_number;

   char validation_string[8];
   
   // This is a 240 x 16 preview of the level using the level's colours.
   u8   preview_data[LEVEL_PREVIEW_DATA_X_SIZE * LEVEL_PREVIEW_DATA_Y_SIZE];

   // The following struct is stored in memory during level play
   // to supply repeatedly used information.
   LEMMINGS_LEVEL_RUNTIME_STATS_V7 runtime_stats;
   
   char texture_archive_using[32]; // Thirty one character null terminated string
                                   // holding the filename (sans extension) of
                                   // the texture archive to use to render this level.
                                   
   u32 one_way_colour;        // This is the colour of the one way arrows. Zero is transparent. 

   u32 no_appended_texture_16s;   // How many object  16s are appended to the end of this struct?
   u32 no_appended_texture_256s;  // How many object 256s are appended after the 16s?
    
   u32 no_terrain_objects;    // How many terrain placement definitions appear after the 256s?      
} LEMMINGS_LEVEL_LDS_FILE_V7;

typedef struct tagLEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER {
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16   0
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256  1
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1    2
   u32 object_type;

#define LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE               0x01
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR 0x02
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS       0x04 // Absence of this means only draw on drawn!
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES                 0x08

   u32 object_flags;
} LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER;

// This bit is set in the object id if this object uses one of the added custom database objects.
#define LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT (1<<31)

typedef struct tagLEMMINGS_LEVEL_TERRAIN_OBJECT_16 {
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER object_header;

   s32  x1,  y1; // Top left coords
   s32  x2,  y2; // Bottom right coords.
   s32 tox, toy; // Top left pixel texture coordinates.
   s32 tsx, tsy; // Texture scale coordinates... fixed point (s1).23.8 // They're in 256ths
                 // That's the DX, DY on the texture per real pixel.

   u32 object_id;
   u8  pal_map[16];
} LEMMINGS_LEVEL_TERRAIN_OBJECT_16;

typedef struct tagLEMMINGS_LEVEL_TERRAIN_OBJECT_256 { 
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER object_header;

   s32  x1,  y1; // Top left coords
   s32  x2,  y2; // Bottom right coords.
   s32 tox, toy; // Top left pixel texture coordinates.
   s32 tsx, tsy; // Texture scale coordinates... fixed point (s1).23.8 // They're in 256ths
                 // That's the DX, DY on the texture per real pixel.

   u32 object_id;
   u8  pal_map[256];
} LEMMINGS_LEVEL_TERRAIN_OBJECT_256;

typedef struct tagLEMMINGS_LEVEL_TERRAIN_OBJECT_1 {
   LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER object_header;

   s32  x1,  y1; // Top left coords
   s32  x2,  y2; // Bottom right coords.

   u32 colour;   // 8 bit value for colour of this solid area.
} LEMMINGS_LEVEL_TERRAIN_OBJECT_1;

// This struct holds data pertaining to a terrain object draw request.
typedef struct tagLEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE {
   s32 xs, ys; // Size and dimensions of the data.
   
   u8 *data; // Pointer to the data to draw.     
} LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE;

void RenderLevelObject16ToLevel( u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_16  *level_object_16 , const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *source_object);
void RenderLevelObject256ToLevel(u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *level_object_256, const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *source_object);
void RenderLevelObject1ToLevel(  u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_1   *level_object_1);
   
void RenderLevel(u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_LDS_FILE_V7 *level_chunk, const LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_archive_texture_archive);
                                                           
// This will take an already rendered level and skrink it to the size of a Lemmings DS preview window
void RenderLevelPreviewIntoPreviewArea(u8 level_data[][LEVEL_Y_SIZE], u8 preview_data[]);
                                           
// Grab a nibble from a rectangular loaded texture in memory.
static inline u32 GetPixelFromImageSource16(const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *o, s32 x, s32 y) {
   u32 pixel_to_get = (x + ((o->xs)*y));
   // Grab the byte that's half way (against the offset pixel_to_get) into the image
   
   // If this is an even pixel, just use the low nibble.
   // If it's an odd pixel, then grab the high nibble and shift it down.
   
   // Only ever return a nibble.
   return (0x0F & ((o->data[pixel_to_get >> 1]) >> (4 * (pixel_to_get & 1))));
}

// Grab a byte from a rectangular loaded texture in memory.
static inline u32 GetPixelFromImageSource256(const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *o, s32 x, s32 y) {
   u32 pixel_to_get = (x + ((o->xs)*y));

   return o->data[pixel_to_get];
}                                          
           
#ifdef __cplusplus
}
#endif

#endif // __LEMMINGS_LEVEL_H__
