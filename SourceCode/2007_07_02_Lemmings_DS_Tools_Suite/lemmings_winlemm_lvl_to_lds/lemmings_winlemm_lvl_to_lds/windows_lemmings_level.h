//
// Lemmings DS - Windows Lemmings level definition.
//
// (c) December 2006
//
// windows_lemmings_level.h
//   Structures for reading and managing original Lemmings windows levels.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#ifndef __WINDOWS_LEMMINGS_LEVEL_H__
#define __WINDOWS_LEMMINGS_LEVEL_H__
               
#include "utility.h"
#include "types.h"            
            
// Because Windows lemmings levels use a different endian to the DS,
// it would get MESSY AS HECK to try to retrieve the elements using
// a straight interpret or pointer cast, so I'm going to use struct
// methods instead.

// This is why the structs contain u8's called *_bytes: they're
// just markers for the layout. :)    

#define WINDOWS_TO_LDS_SHIFT_Y (    8)
// All of the objects in a Windows level must be shifted down
// by this amount as LDS levels are actually TALLER than their
// Windows counterparts.          

   /* ------------------------------------------------------------ */

// This holds the layout of the prelude containing all of the descriptive level information.
typedef struct tagWINDOWS_LEMMINGS_DESCRIPTION_STRUCT {
   u8 release_rate_bytes[2];   
   u8 no_lems_bytes[2];     
   u8 to_be_saved_bytes[2];     
   u8 time_in_minutes_bytes[2];     
   u8 no_skills_bytes[16];     
   u8 start_screen_xpos_bytes[2];     
   u8 graphics_set_bytes[2];    
   
   u8 extended_graphics_set_bytes[2];
   u8 extra_special_something_bytes[2];   
} __attribute__ ((packed)) WINDOWS_LEMMINGS_DESCRIPTION_STRUCT;

// This struct holds the same information as the struct above, but
// in a more easily retrievable format.
typedef struct tagSANE_WINDOWS_LEMMINGS_DESCRIPTION_STRUCT {
   int release_rate;
   int number_of_lemmings;
   int to_be_saved;
   int time_in_minutes;
   int no_skills[8];
   int screen_start_x_pos; // This has been translated!
   int graphics_set;     
} SANE_WINDOWS_LEMMINGS_DESCRIPTION_STRUCT;     

   /* ------------------------------------------------------------ */
                     
// This holds the layout of each interactive object in the level; they're 8 bytes long.
typedef struct tagWINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT {
   u8 xpos_bytes[2];
   u8 ypos_bytes[2];
   u8 object_id_bytes[2];
   u8 modifier_bytes[2];  
} __attribute__ ((packed)) WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT;     

#define WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_TYPE_EXIT     0 
#define WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_TYPE_ENTRANCE 1 

// This struct holds the same information as the struct above, but
// in a more easily retrievable format.
typedef struct tagSANE_WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT {
   int xpos; // This has been translated!
   int ypos; 
   
   int object_type;
   
   bool behind_all_others;
} SANE_WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT;         

   /* ------------------------------------------------------------ */
                     
// This holds the layout of each terrain object in the level; they're 4 bytes long.
typedef struct tagWINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT {
   unsigned xpos_msb_bits   :  4;                          
   unsigned modifier_nibble :  4; 
   unsigned xpos_lsb_byte   :  8; 
                                        
   unsigned ypos_ms_byte    :  7; 
                                        
   unsigned ypos_sign_bit   :  1;    
                                
   unsigned terrain_id_bits :  7; 
             
   unsigned ypos_ls_bit     :  1;    
} __attribute__ ((packed)) WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT;         

// This struct holds the same information as the struct above, but
// in a more easily retrievable format.
typedef struct tagSANE_WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT {
   int xpos; // This has been translated!
   int ypos; 
   
   int terrain_type;
   
   bool zero_overwrite;
   bool upside_down;
   bool behind_all_others;
} SANE_WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT; 

   /* ------------------------------------------------------------ */

// This holds the layout of each terrain object in the level; they're 4 bytes long.
typedef struct tagWINDOWS_LEMMINGS_STEEL_AREA_STRUCT {
   unsigned xpos_ms_byte   : 8;    
                                
   unsigned ypos_bits      : 7;    
             
   unsigned xpos_ls_bit    : 1; 
                 
   unsigned height_nibble  : 4;                            
   unsigned width_nibble   : 4;        
   u8 empty_byte[1];                             
} __attribute__ ((packed)) WINDOWS_LEMMINGS_STEEL_AREA_STRUCT;       

// This struct holds the same information as the struct above, but
// in a more easily retrievable format.
typedef struct tagSANE_WINDOWS_LEMMINGS_STEEL_AREA_STRUCT {
   int xpos; // This has been translated!
   int ypos; 
   
   int width;  // This has been moderated.
   int height;
} SANE_WINDOWS_LEMMINGS_STEEL_AREA_STRUCT; 

   /* ------------------------------------------------------------ */

// This holds the layout of a single Windows lemmings level.
typedef struct tagWINDOWS_LEMMINGS_LEVEL_FILE_STRUCT {
   // This holds generic information about a level; it's 32 bytes long.    
   WINDOWS_LEMMINGS_DESCRIPTION_STRUCT description; 
   
   #define NO_WINDOWS_LEMMINGS_INTERACTIVE_OBJECTS 32             
   // This holds information about each interactive object in the level; they're 8 bytes long.                                        
   WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT object[NO_WINDOWS_LEMMINGS_INTERACTIVE_OBJECTS];   
   
   #define NO_WINDOWS_LEMMINGS_TERRAIN_OBJECTS 400             
   // This holds information about each terrain object in the level; they're 4 bytes long.                                        
   WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT terrain[NO_WINDOWS_LEMMINGS_TERRAIN_OBJECTS];   
   
   #define NO_WINDOWS_LEMMINGS_STEEL_AREAS 32             
   // This holds information about each steel area in the level; they're 4 bytes long.                                        
   WINDOWS_LEMMINGS_STEEL_AREA_STRUCT steel_area[NO_WINDOWS_LEMMINGS_STEEL_AREAS];    
   
   // This is the level name.
   char level_name[32];           
   
   // These methods help analyse the data contained in a WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT
   void GetSaneDescriptionStruct(      SANE_WINDOWS_LEMMINGS_DESCRIPTION_STRUCT        *destination) const;     
   bool GetSaneInteractiveObjectStruct(SANE_WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT *destination, u32 object_no) const;       
   bool GetSaneTerrainObjectStruct(    SANE_WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT     *destination, u32 object_no) const;       
   bool GetSaneSteelAreaStruct(        SANE_WINDOWS_LEMMINGS_STEEL_AREA_STRUCT         *destination, u32 object_no) const;                                                                   
} WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT;

#endif // __WINDOWS_LEMMINGS_LEVEL_H__
