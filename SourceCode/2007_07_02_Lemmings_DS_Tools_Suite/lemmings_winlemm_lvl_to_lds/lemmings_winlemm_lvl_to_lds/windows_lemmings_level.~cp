//
// Lemmings DS - Windows Lemmings level definition.
//
// (c) December 2006
//
// windows_lemmings_level.cpp
//   Methods for interpreting original Lemmings windows levels.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#include <stdlib.h>
#include <stdio.h>

#include "windows_lemmings_level.h"

// These methods help analyse the data contained in a WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT
void WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT::GetSaneDescriptionStruct(      SANE_WINDOWS_LEMMINGS_DESCRIPTION_STRUCT        *destination) const {
   // Abort if the incoming pointer is null. (Which it really shouldn't be)
   if (destination == NULL) return;
   
   // Interpret the fields in the description part of this WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT
   destination->release_rate       =      description.release_rate_bytes     [0] << 8 |      description.release_rate_bytes     [1];  
   destination->number_of_lemmings =      description.no_lems_bytes          [0] << 8 |      description.no_lems_bytes          [1];  
   destination->to_be_saved        =      description.to_be_saved_bytes      [0] << 8 |      description.to_be_saved_bytes      [1];  
   destination->time_in_minutes    =      description.time_in_minutes_bytes  [0] << 8 |      description.time_in_minutes_bytes  [1];  
   
   for (unsigned int skill = 0; skill < 8; skill++) {
      destination->no_skills[skill] = description.no_skills_bytes[1 + 2*skill];  
   }
      
   destination->screen_start_x_pos =      description.start_screen_xpos_bytes[0] << 8 |      description.start_screen_xpos_bytes[1];  
   destination->graphics_set       =      description.graphics_set_bytes     [0] << 8 |      description.graphics_set_bytes     [1];  
}

bool WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT::GetSaneInteractiveObjectStruct(SANE_WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT *destination, u32 object_no) const {
   // Abort if the incoming pointer is null. (Which it really shouldn't be)
   if (destination == NULL) return false;  
   
   // Find the object requested:
   const WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT &our_object = object[object_no];
   
   // Is this a filler object?
   if ((our_object.xpos_bytes[0]      == 0x00)
    && (our_object.xpos_bytes[1]      == 0x00)
    && (our_object.ypos_bytes[0]      == 0x00)
    && (our_object.ypos_bytes[1]      == 0x00)
    && (our_object.object_id_bytes[0] == 0x00)
    && (our_object.object_id_bytes[1] == 0x00)
    && (our_object.modifier_bytes[0]  == 0x00)
    && (our_object.modifier_bytes[1]  == 0x00)) return false; // Inform that this object is no good.
    
   // This stores the x position after we've sorted it out and translated it. 
   s16 xposition;
   
   ((u8 *)(&xposition))[0] = our_object.xpos_bytes[1]; 
   ((u8 *)(&xposition))[1] = our_object.xpos_bytes[0]; // Flip the bytes of the x position. 
   
   xposition -= 16; // 0x0010 = 0 according to the LEMMINGS .LVL FILE FORMAT BY rt
   
   destination->xpos = xposition; // Calculation complete.        
    
   // This stores the y position after we've sorted it out and translated it. 
   s16 yposition;
   
   ((u8 *)(&yposition))[0] = our_object.ypos_bytes[1]; 
   ((u8 *)(&yposition))[1] = our_object.ypos_bytes[0]; // Flip the bytes of the x position.     
   
   yposition += WINDOWS_TO_LDS_SHIFT_Y; // Move down because of the level size difference. 
   
   destination->ypos = yposition; // Calculation complete.       
   
   destination->object_type = our_object.object_id_bytes[0] << 8 | our_object.object_id_bytes[1];
   
   if (our_object.modifier_bytes[0] & 0x80) {
      destination->behind_all_others = true;
   } else {
      destination->behind_all_others = false;
   }
   
   return true;
}

bool WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT::GetSaneTerrainObjectStruct(    SANE_WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT     *destination, u32 object_no) const {
   // Abort if the incoming pointer is null. (Which it really shouldn't be)
   if (destination == NULL) return false;  
   
   // Find the object requested:
   const WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT &our_object = terrain[object_no];
   
   printf("[%02X %02X %02X %02X] ", ((u8 *)(&our_object))[0], ((u8 *)(&our_object))[1], ((u8 *)(&our_object))[2], ((u8 *)(&our_object))[3]);
                                      
   // Is this a filler object?
   if ((our_object.modifier_nibble  == 0x0F)
    && (our_object.xpos_msb_bits    == 0x0F)
    && (our_object.xpos_lsb_byte    == 0xFF)
    && (our_object.ypos_ms_byte     == 0x7F)
    && (our_object.terrain_id_bits  == 0x7F)
    && (our_object.ypos_ls_bit      == 0x01)) return false; // Inform that this object is no good.
             
   printf("%02X%02X is ", our_object.xpos_msb_bits, our_object.xpos_lsb_byte);
   
   u32 xposition;
   
   xposition = (our_object.xpos_msb_bits << 8) | our_object.xpos_lsb_byte;    
   
   printf("%4d - ", xposition);
                                                                                                   
   xposition -= 16; // As documented. (not a lemmings ds thing!!!)     
   
   destination->xpos = xposition; // Calculation complete.
   
   s16 yposition;
                        
   (((u8 *)(&yposition))[0]) = (our_object.ypos_ms_byte << 1) | our_object.ypos_ls_bit;
   
   // If the sign bit is set, invert all of the bits in the most significant byte of the yposition short.
   (((u8 *)(&yposition))[1]) = 0xff * our_object.ypos_sign_bit;
   
   yposition -= 4;
   
   yposition += WINDOWS_TO_LDS_SHIFT_Y; // Move down because of the level size difference. 
                        
   destination->ypos = yposition; // Calculation complete.   

   destination->terrain_type      = our_object.terrain_id_bits & 63;
   
   destination->zero_overwrite    = (our_object.modifier_nibble & 2) != 0;
   destination->upside_down       = (our_object.modifier_nibble & 4) != 0;
   destination->behind_all_others = (our_object.modifier_nibble & 8) != 0;
                     // Nibble
   printf(" %s%s%s - N = %01X ", destination->zero_overwrite?"Y":"N",
                                 destination->upside_down?"Y":"N",
                                 destination->behind_all_others?"Y":"N",
                                 our_object.modifier_nibble);        
   
   if (destination->zero_overwrite && destination->behind_all_others) destination->zero_overwrite = false;

   return true;   
}

bool WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT::GetSaneSteelAreaStruct(        SANE_WINDOWS_LEMMINGS_STEEL_AREA_STRUCT         *destination, u32 object_no) const {
   // Abort if the incoming pointer is null. (Which it really shouldn't be)
   if (destination == NULL) return false;  
   
   // Find the object requested:
   const WINDOWS_LEMMINGS_STEEL_AREA_STRUCT &our_object = steel_area[object_no];
   
   // Is this a filler object?
   if ((our_object.xpos_ms_byte       == 0x00)
    && (our_object.xpos_ls_bit        == 0x00)
    && (our_object.ypos_bits          == 0x00)
    && (our_object.width_nibble       == 0x00)
    && (our_object.height_nibble      == 0x00)) return false; // Inform that this object is no good.
    
   // This stores the x position after we've sorted it out and translated it. 
   u16 xposition;
   
   xposition = (our_object.xpos_ms_byte << 1) | our_object.xpos_ls_bit; 
   
   xposition *= 4;  
   
   xposition -= 16;  
   
   destination->xpos = xposition; // Calculation complete.        
    
   // This stores the y position after we've sorted it out and translated it. 
   u16 yposition = our_object.ypos_bits; 
   
   yposition *= 4;           
   
   yposition += WINDOWS_TO_LDS_SHIFT_Y; // Move down because of the level size difference. 
   
   destination->ypos = yposition; // Calculation complete.   
                                                     
   s16 width, height;    
   
   width  = (our_object.width_nibble  + 1) * 4;
   height = (our_object.height_nibble + 1) * 4;
   
   destination->width  = width;
   destination->height = height;
   
   return true; 
}


