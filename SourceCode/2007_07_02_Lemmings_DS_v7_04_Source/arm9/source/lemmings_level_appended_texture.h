//
// Lemmings DS Shared File
//
// (c) April 2006, May 2006, June 2006, July 2006, August 2006
//
// lemmings_level_object_database.h
//   Structures and headers for Lemmings DS Image Source/Texture management,
//   and rendering.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//

#ifndef LEMMINGS_LEVEL_OBJECT_DATABASE_H
#define LEMMINGS_LEVEL_OBJECT_DATABASE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9          
#include <nds.h>
#else
#include "ds_types.h"
#endif

typedef struct tagLEMMINGS_LEVEL_APPENDED_TEXTURE_16 {
   // This holds the size of this appended object 
   // in bytes. It's always a multiple of 4.
   u32 my_mem_size;     
        
   s32 xs, ys; // Size and dimensions of the data.
   
   u8  data[]; // This is xs * ys NIBBLES (range 0x0 - 0xF) of data.
               // To the next word.
} LEMMINGS_LEVEL_APPENDED_TEXTURE_16;

typedef struct tagLEMMINGS_LEVEL_APPENDED_TEXTURE_256 { 
   // This holds the size of this appended object 
   // in bytes. It's always a multiple of 4.       
   u32 my_mem_size;     
   
   s32 xs, ys; // Size and dimensions of the data.
   
   u8  data[]; // This is xs * ys BYTES (range 0x00 - 0xFF) of data. 
               // To the next word.
} LEMMINGS_LEVEL_APPENDED_TEXTURE_256;
                              
#ifdef __cplusplus
}
#endif

#endif // LEMMINGS_LEVEL_OBJECT_DATABASE_H
