//
// Lemmings DS Builder
//
// (c) May 2006, June 2006, July 2006, August 2006
//
// bmp_loader.cpp
//   Implementation: loading 'memory bmp', a BMP file loaded straight
//   into memory. These functions manipulate these files and
//   extract the bitmap data.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//

#include <new>

#include "bmp_loader.h"
                                                 
// This loads a BMP into memory, using the specified MEMORY_BMP to track it.
bool MemoryBMP_LoadBMP(MEMORY_BMP *memory_bmp, const char *filename) {     
   // unload existing bitmap in memory  
   MemoryBMP_UnloadBMP(memory_bmp);
     
   // reset existing pointer
   memory_bmp->bmp_memory_begin = NULL; 
                      
   // open the file for reading           
   FILE *input_file = fopen(filename, "rb");
   
   // if the file couldn't be opened, fail
   if (input_file == NULL) {
      return false;
   }   
   
   // head to the end of the file
   if (fseek(input_file, 0, SEEK_END) != 0) return false;
                              
   // work out what position the file is at (basically: where is the end of the file?)
   int file_length = ftell(input_file);
   
   if (file_length == -1L) {   
      fclose(input_file);
      return false;
   }
   
   // store the length of the file
   memory_bmp->memory_length = file_length;
   
   // back to the start of the file.
   rewind(input_file);
   
   try {
      memory_bmp->bmp_memory_begin = (MEMORY_BMP_FILE *)(new u8[memory_bmp->memory_length]); 
   } catch (std::bad_alloc &e) {
      memory_bmp->bmp_memory_begin = NULL;
      MemoryBMP_UnloadBMP(memory_bmp);
      
      return false;
   }
   
   // read the file into memory
   fread((void *)memory_bmp->bmp_memory_begin, memory_bmp->memory_length, 1, input_file);
   
   fclose(input_file);

   return true;
}
         
void MemoryBMP_UnloadBMP(MEMORY_BMP *memory_bmp) {
   if (memory_bmp->bmp_memory_begin != NULL) {
      delete[] ((u8 *)(memory_bmp->bmp_memory_begin));
      memory_bmp->bmp_memory_begin = NULL;
   }  
     
   memory_bmp->memory_length = 0; 
   
   //memory_bmp->image_width   = 0; 
   //memory_bmp->image_height  = 0;  
}
         
void MemoryBMP_Initialise(MEMORY_BMP *memory_bmp) {     
   memory_bmp->bmp_memory_begin = NULL;     
   MemoryBMP_UnloadBMP(memory_bmp);
}
