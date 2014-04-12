//
// Lemmings DS Shared File
//
// (c) March 2007
//
// lemmings_texture_archive.h
//   Structures and headers for Lemmings DS v4 Image Source/Texture management.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#ifndef LEMMINGS_TEXTURE_ARCHIVE_H
#define LEMMINGS_TEXTURE_ARCHIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9          
#include <nds.h>
#else
#include "ds_types.h"
#endif

typedef struct tagLEMMINGS_TEXTURE_ARCHIVE_HEADER {
   u32 texture_archive_file_size; // This is the size of this texture archive file in bytes.   
                                  // Because these are always word padded, this is always a multiple of four.
                                  
   u32 version_number;
   
   u32 no_texture_16s;            // The number of 16 colour textures in the file.   
   u32 no_texture_256s;           // The number of 256 colour textures in the file.   
   
   u16 ideal_palette[256];        // The ideal palette for this texture archive.
                                  // (Because all textures in an archive use the same palette)
                                  // If you want to make a mega lemmings multi-style level,
                                  // YOU'RE GOING TO HAVE TO COMBINE THE STYLES MANUALLY AHAHAHAHAHA.         
                                  
   u32 texture_object_offsets[];  // These are the offsets from (&this_object) which the texture objects
                                  // reside.                           
                                  // First the 16 colour ones, then the 256 colour ones.     
} LEMMINGS_TEXTURE_ARCHIVE_HEADER;

// This holds information about a single lemmings texture archive texture.
typedef struct tagLEMMINGS_TEXTURE_ARCHIVE_TEXTURE {
   u32 texture_chunk_size; // This is the size of this LEMMINGS_TEXTURE_ARCHIVE_TEXTURE instance
                           // within the texture archive file. 
                           // Starting at ((u8 *)(texture))[sizeof(LEMMINGS_TEXTURE_ARCHIVE_TEXTURE)]
                           // Because these are always word padded, this is always a multiple of four.
                           
   s32 xs, ys;             // The size of the texture (always rectangular)
   
   u32 format_flags;       // Combination of the following
   
   // First bit is the colour depth.
#define LEMMINGS_TEXTURE_ARCHIVE_TEXTURE_FORMAT_FLAGS_16_COLOURS         0x0000
#define LEMMINGS_TEXTURE_ARCHIVE_TEXTURE_FORMAT_FLAGS_256_COLOURS        0x0001
#define LEMMINGS_TEXTURE_ARCHIVE_TEXTURE_FORMAT_FLAGS_COLOUR_DEPTH_BIT   0x0001

   char name[16];          // Name of the texture
                           // NOT USED WHEN LOADING TEXTURE FOR DS.
                           // Just for the level editor.
                           
   u8 data[];              // After this is texture data.
} LEMMINGS_TEXTURE_ARCHIVE_TEXTURE;
// These are always aligned word boundaries within a texture archive.
                
static inline LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *GetTextureArchiveTexture16(const LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_active_texture_archive, int n) {     
   return (LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(((u8*)(loaded_active_texture_archive)) + loaded_active_texture_archive->texture_object_offsets[n]);                               
}

static inline LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *GetTextureArchiveTexture256(const LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_active_texture_archive, int n) {     
   return (LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(((u8*)(loaded_active_texture_archive)) + loaded_active_texture_archive->texture_object_offsets[n + loaded_active_texture_archive->no_texture_16s]);                               
}   
                              
#ifdef __cplusplus
}
#endif

#endif // LEMMINGS_TEXTURE_ARCHIVE_H
