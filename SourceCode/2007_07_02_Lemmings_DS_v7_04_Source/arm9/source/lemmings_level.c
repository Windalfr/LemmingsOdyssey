// Lemmings level stuff
//  by Mathew Carr
                                                  
#include "utility.h"           
#include "string.h"
#include "lemmings_level.h"   
#include "lemmings_level_appended_texture.h"   

// This string must match.
const char *correct_validation_string = "meow!v5";
// Specify it like this, to be megasure.  

void RenderLevelOneWayAreaIndicator(u8 level_data[][LEVEL_Y_SIZE], int x1, int y1, int x2, int y2, int d, int c);
                                            
void RenderLevel(u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_LDS_FILE_V7 *level_file, const LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_archive_texture_archive) {
   // Erase everything.
   int x, y;
   
   for (int y = 0; y < LEVEL_Y_SIZE; y++) {
      for (int x = 0; x < LEVEL_X_SIZE; x++) {
         level_data[x][y] = 0;
      }
   }
       
   // Make a set of image sources from the texture archive texture 16s.
   LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE identified_texture_archive_16_image_sources[loaded_archive_texture_archive->no_texture_16s];
   
   // We want to move through all of the texture 16s in the archive
   for (int texture_archive_texture_16 = 0; texture_archive_texture_16 < loaded_archive_texture_archive->no_texture_16s; texture_archive_texture_16++) {
      LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *located_texture_archive_texture_16 = GetTextureArchiveTexture16(loaded_archive_texture_archive, texture_archive_texture_16);
      
      // Copy the parameters from the texture archive to the image source database.
      identified_texture_archive_16_image_sources[texture_archive_texture_16].xs   = located_texture_archive_texture_16->xs;
      identified_texture_archive_16_image_sources[texture_archive_texture_16].ys   = located_texture_archive_texture_16->ys;
      identified_texture_archive_16_image_sources[texture_archive_texture_16].data = located_texture_archive_texture_16->data; 
   }
   
    // Make a set of image sources from the texture archive texture 256s.
   LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE identified_texture_archive_256_image_sources[loaded_archive_texture_archive->no_texture_256s];
    
   // We want to move through all of the texture 256s in the archive
   for (int texture_archive_texture_256 = 0; texture_archive_texture_256 < loaded_archive_texture_archive->no_texture_256s; texture_archive_texture_256++) {
      LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *located_texture_archive_texture_256 = GetTextureArchiveTexture256(loaded_archive_texture_archive, texture_archive_texture_256);
      
      // Copy the parameters from the texture archive to the image source database.
      identified_texture_archive_256_image_sources[texture_archive_texture_256].xs   = located_texture_archive_texture_256->xs;
      identified_texture_archive_256_image_sources[texture_archive_texture_256].ys   = located_texture_archive_texture_256->ys;
      identified_texture_archive_256_image_sources[texture_archive_texture_256].data = located_texture_archive_texture_256->data;
   
      //allegro_message("Identifying texture archive 256 no. %d.\nx: %d\ny: %d", texture_archive_texture_256, identified_texture_archive_256_image_sources[texture_archive_texture_256].xs, identified_texture_archive_256_image_sources[texture_archive_texture_256].ys);
   }

   // Use this pointer to traverse the data that appears at the end of a level chunk.
   u8 *level_file_moving_ptr = ((u8*)level_file) + (sizeof(LEMMINGS_LEVEL_LDS_FILE_V7));

   // Create a temporary image source database referring to the appended textures:
   LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE identified_appended_texture_16_image_sources[level_file->no_appended_texture_16s];
               
   // Loop through all of the appended texture 16s after the lemmings level chunk.
   for (int appended_texture_16 = 0; appended_texture_16 < level_file->no_appended_texture_16s; appended_texture_16++) {
      LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *located_appended_texture_16 = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_16 *)level_file_moving_ptr);
       
      // Copy the parameters from the appended texture to the image source database.
      identified_appended_texture_16_image_sources[appended_texture_16].xs   = located_appended_texture_16->xs;
      identified_appended_texture_16_image_sources[appended_texture_16].ys   = located_appended_texture_16->ys;
      identified_appended_texture_16_image_sources[appended_texture_16].data = located_appended_texture_16->data;

      // Skip past this texture.
      level_file_moving_ptr += located_appended_texture_16->my_mem_size;
   }

   // Create a temporary image source database referring to the appended textures:
   LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE identified_appended_texture_256_image_sources[level_file->no_appended_texture_256s];
               
   // Loop through all of the appended texture 256s after the lemmings level chunk.
   for (int appended_texture_256 = 0; appended_texture_256 < level_file->no_appended_texture_256s; appended_texture_256++) {
      LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *located_appended_texture_256 = ((LEMMINGS_LEVEL_APPENDED_TEXTURE_256 *)level_file_moving_ptr);
       
      // Copy the parameters from the appended texture to the image source database.
      identified_appended_texture_256_image_sources[appended_texture_256].xs   = located_appended_texture_256->xs;
      identified_appended_texture_256_image_sources[appended_texture_256].ys   = located_appended_texture_256->ys;
      identified_appended_texture_256_image_sources[appended_texture_256].data = located_appended_texture_256->data;

      level_file_moving_ptr += located_appended_texture_256->my_mem_size;
   }
   
   // Now the pointer resides at the location of the terrain objects.

   for (int level_object = 0; level_object < level_file->no_terrain_objects; level_object++) {
      const LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *located_terrain_object_header = ((LEMMINGS_LEVEL_TERRAIN_OBJECT_HEADER *)level_file_moving_ptr); 
      
      if (located_terrain_object_header->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_1) {
         const LEMMINGS_LEVEL_TERRAIN_OBJECT_1  *located_terrain_object = (const LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *)level_file_moving_ptr;                                  
            
         RenderLevelObject1ToLevel( level_data, located_terrain_object);
                                        
         level_file_moving_ptr += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_1);
      } else {
         if (located_terrain_object_header->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16) {    
            const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *located_terrain_object = (const LEMMINGS_LEVEL_TERRAIN_OBJECT_16  *)level_file_moving_ptr;                                  
            
            int custom_object = (located_terrain_object->object_id &   LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT ) != 0;
            int object_id     = (located_terrain_object->object_id & (~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT));   
                       
            RenderLevelObject16ToLevel(level_data,
                                       located_terrain_object,
                                       (custom_object
                                        ? (&identified_appended_texture_16_image_sources[object_id])
                                        : (&identified_texture_archive_16_image_sources[object_id])));
                                        
            level_file_moving_ptr += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16);
         } else
         if (located_terrain_object_header->object_type == LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_256) {    
            const LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *located_terrain_object = (const LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *)level_file_moving_ptr;                                  
            
            int custom_object = (located_terrain_object->object_id &   LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT ) != 0;
            int object_id     = (located_terrain_object->object_id & (~LEMMINGS_LEVEL_TERRAIN_OBJECT_CUSTOM_SET_BIT));   
                       
            RenderLevelObject256ToLevel(level_data,
                                       located_terrain_object,
                                       (custom_object
                                        ? (&identified_appended_texture_256_image_sources[object_id])
                                        : (&identified_texture_archive_256_image_sources[object_id])));
                                        
            level_file_moving_ptr += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_256);
         }    
      }
   }
   
   // Now we need to render the one way areas on top of the level.
   if (level_file->one_way_colour != 0) {
      for (int one_way_area = 0; one_way_area < level_file->runtime_stats.no_one_way_areas; one_way_area++) {
         RenderLevelOneWayAreaIndicator(level_data,
                                        level_file->runtime_stats.one_way_area_x1[one_way_area],
                                        level_file->runtime_stats.one_way_area_y1[one_way_area],
                                        level_file->runtime_stats.one_way_area_x2[one_way_area],
                                        level_file->runtime_stats.one_way_area_y2[one_way_area],
                                        level_file->runtime_stats.one_way_area_d[ one_way_area],
                                        level_file->one_way_colour); 
      }
   }
} 

void RenderLevelObject16ToLevel( u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_16 *level_object_16, const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *source_object) {
   s32 dst_x, dst_y; // destination coordinates onto the level.
   s32   ctx,   cty; // current (source) texture coordinates.
                 
   s32 bound_x = Min(level_object_16->x2, LEVEL_X_SIZE-1);
   s32 bound_y = Min(level_object_16->y2, LEVEL_Y_SIZE-1);

   s32 shifted_texture_size_x = ((source_object->xs) << 8);
   s32 shifted_texture_size_y = ((source_object->ys) << 8);
   
   u32 source;

   if (!(level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES)) {
      for (dst_y = level_object_16->y1, cty = level_object_16->toy; dst_y <= bound_y; dst_y++, cty += level_object_16->tsy) {
         if (dst_y < 0) continue;
         cty = RangeWrap(cty, shifted_texture_size_y);       
   
         for (dst_x = level_object_16->x1, ctx = level_object_16->tox; dst_x <= bound_x; dst_x++, ctx += level_object_16->tsx) {
            if (dst_x < 0) continue;
            ctx = RangeWrap(ctx, shifted_texture_size_x);     
            source = level_object_16->pal_map[GetPixelFromImageSource16(source_object, ctx >> 8, cty >> 8)];
   
            if (source != 0) {
               if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
                  source = 0; // Draw with transparency if the 'source' is nonzero and we're in subtractive.
               }   
               if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
                  if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
                     if (!level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  } else {
                     if (level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  }
               } else {
                  level_data[dst_x][dst_y] = source;
               } 
            }
         }
      }
   } else {
      for (dst_y = level_object_16->y1, ctx = level_object_16->tox; dst_y <= bound_y; dst_y++, ctx += level_object_16->tsx) {
         if (dst_y < 0) continue;
         ctx = RangeWrap(ctx, shifted_texture_size_x);       
   
         for (dst_x = level_object_16->x1, cty = level_object_16->toy; dst_x <= bound_x; dst_x++, cty += level_object_16->tsy) {
            if (dst_x < 0) continue;
            cty = RangeWrap(cty, shifted_texture_size_y);
            source = level_object_16->pal_map[GetPixelFromImageSource16(source_object, ctx >> 8, cty >> 8)];
   
            if (source != 0) {
               if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
                  source = 0; // Draw with transparency if the 'source' is nonzero and we're in subtractive.
               }   
               if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
                  if (level_object_16->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
                     if (!level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  } else {
                     if (level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  }
               } else {
                  level_data[dst_x][dst_y] = source;
               } 
            }
         }
      }
   }
}

void RenderLevelObject256ToLevel( u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_256 *level_object_256, const LEMMINGS_LEVEL_TERRAIN_OBJECT_IMAGE_SOURCE *source_object) {
   s32 dst_x, dst_y; // destination coordinates onto the level.
   s32   ctx,   cty; // current (source) texture coordinates.

   s32 bound_x = Min(level_object_256->x2, LEVEL_X_SIZE-1);
   s32 bound_y = Min(level_object_256->y2, LEVEL_Y_SIZE-1);

   s32 shifted_texture_size_x = ((source_object->xs) << 8);
   s32 shifted_texture_size_y = ((source_object->ys) << 8);
   
   u32 source;                  

   if (!(level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SWAP_AXES)) {
      for (dst_y = level_object_256->y1, cty = level_object_256->toy; dst_y <= bound_y; dst_y++, cty += level_object_256->tsy) {
         if (dst_y < 0) continue;
         cty = RangeWrap(cty, shifted_texture_size_y);       
   
         for (dst_x = level_object_256->x1, ctx = level_object_256->tox; dst_x <= bound_x; dst_x++, ctx += level_object_256->tsx) {
            if (dst_x < 0) continue;
            ctx = RangeWrap(ctx, shifted_texture_size_x);     
            source = level_object_256->pal_map[GetPixelFromImageSource256(source_object, ctx >> 8, cty >> 8)];
   
            if (source != 0) {
               if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
                  source = 0; // Draw with transparency if the 'source' is nonzero and we're in subtractive.
               }   
               if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
                  if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
                     if (!level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  } else {
                     if (level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  }
               } else {
                  level_data[dst_x][dst_y] = source;
               } 
            }
         }
      }
   } else {
      for (dst_y = level_object_256->y1, ctx = level_object_256->tox; dst_y <= bound_y; dst_y++, ctx += level_object_256->tsx) {
         if (dst_y < 0) continue;
         ctx = RangeWrap(ctx, shifted_texture_size_x);       
   
         for (dst_x = level_object_256->x1, cty = level_object_256->toy; dst_x <= bound_x; dst_x++, cty += level_object_256->tsy) {
            if (dst_x < 0) continue;
            cty = RangeWrap(cty, shifted_texture_size_y);
            source = level_object_256->pal_map[GetPixelFromImageSource256(source_object, ctx >> 8, cty >> 8)];
   
            if (source != 0) {
               if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
                  source = 0; // Draw with transparency if the 'source' is nonzero and we're in subtractive.
               }   
               if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
                  if (level_object_256->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
                     if (!level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  } else {
                     if (level_data[dst_x][dst_y]) {
                        level_data[dst_x][dst_y] = source;
                     }
                  }
               } else {
                  level_data[dst_x][dst_y] = source;
               } 
            }
         }
      }
   }
}

void RenderLevelObject1ToLevel( u8 level_data[][LEVEL_Y_SIZE], const LEMMINGS_LEVEL_TERRAIN_OBJECT_1 *level_object_1) {
   s32 dst_x, dst_y; // destination coordinates onto the level.

   s32 start_x = Max(level_object_1->x1, 0);
   s32 start_y = Max(level_object_1->y1, 0);

   s32 bound_x = Min(level_object_1->x2, LEVEL_X_SIZE-1);
   s32 bound_y = Min(level_object_1->y2, LEVEL_Y_SIZE-1);
                                          
   if (level_object_1->colour == 0) return;
   
   u32 source = level_object_1->colour;
       
   if (level_object_1->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE) {
      source = 0; // Draw with transparency if the 'source' is nonzero and we're in subtractive.
   }   
   
   for (dst_y = start_y; dst_y <= bound_y; dst_y++) {
      for (dst_x = start_x; dst_x <= bound_x; dst_x++) {
         if (level_object_1->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR) {
            if (level_object_1->object_header.object_flags & LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS) {
               if (!level_data[dst_x][dst_y]) {
                  level_data[dst_x][dst_y] = source;
               }
            } else {
               if (level_data[dst_x][dst_y]) {
                  level_data[dst_x][dst_y] = source;
               }
            }
         } else {
            level_data[dst_x][dst_y] = source;
         }
      }
   }
}

void RenderLevelOneWayAreaIndicator(u8 level_data[][LEVEL_Y_SIZE], int x1, int y1, int x2, int y2, int d, int c) {
   s32 dst_x, dst_y; // destination coordinates onto the level.

   s32 start_x = Max(x1, 0);
   s32 start_y = Max(y1, 0);

   s32 bound_x = Min(x2, LEVEL_X_SIZE-1);
   s32 bound_y = Min(y2, LEVEL_Y_SIZE-1);
   
   u32 ls_x, ls_y; 
   
   u32 draw;

   for (dst_y = start_y; dst_y <= bound_y; dst_y++) {
      for (dst_x = start_x; dst_x <= bound_x; dst_x++) {          
         ls_x = ((u32)dst_x) & 15;
         
         if (d == 0) {
            ls_x = 15 - ls_x;
         }
         
         ls_y = ((u32)dst_y) & 15;
                                 
         draw = ((ls_x > 0) && (ls_y >= (ls_x + 8)) && (ls_y <= (16 - ls_x)))
             || ((ls_x > 8) && (ls_y >= (ls_x - 8)) && (ls_y <= (16 - ls_x))); 
   
         if (draw) {
            if (level_data[dst_x][dst_y]) {
               level_data[dst_x][dst_y] = c;
            }
         }
      }
   }     
}

// This will take an already rendered level and skrink it to the size and format of a Lemmings DS preview window
void RenderLevelPreviewIntoPreviewArea(u8 level_data[][LEVEL_Y_SIZE], u8 preview_data[]) {
   // Blank the sub screens miniature version of the map:
   memset(preview_data, 0, LEVEL_PREVIEW_DATA_X_SIZE * LEVEL_PREVIEW_DATA_Y_SIZE);

   s32 column;          // 'column' is the map sprite column currently being used to draw to.
   s32   mapx ,   mapy; // 'map*' controls the current pixel within the miniature map sprite strip.
   s32 levelx , levely; // These control the current pixel within the real level_data map.

   // This is all hardcoded for a 240 by 32 preview area.

   // For each sprite column of the map strip:
   for (column = 0; column < 30; column++) {
      // For each row down a specific map sprite column:
      for (mapy = 0; mapy < 32; mapy++) {
         // Calculate the real map y pixel to grab the pixel data from.
         levely = (mapy * 21) >> 2; // This formula maps the 32 pixels of the miniature map
                                    // to 168 pixels of the real map screen.

         // For each pixel across the specific map sprite row:
         for (mapx = 0; mapx < 8; mapx++) {
            // Calculate the real map x pixel to grab the pixel data from.
            levelx = ((mapx + (column*8)) * 13653) >> 11; // This formula maps the 240 pixels of the miniature map
                                                          // to 1599 pixels of the real map screen.

            // Grab the pixel from the large map and store it into the miniature map.
            preview_data[(mapx) + (mapy*8) + (column * (32*8))] = level_data[levelx][levely];
         }
      }
   }
}

