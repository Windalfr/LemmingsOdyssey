//
// MEMFILE memory FILE simulator.
//
// (c) September 2006
//
// memfile_file.c
//   Engine for reading from memory as if it were a FILE stream.
//
// By Mathew Carr.
// mattcarr@gmail.com
//
// You can have this thing... knock yourself out.
// Find a use: holler, wouldya? :)
//

#include "memfile_file.h"

MEMFILE_FILE *MEMFILE_fopen(const void *source_memory) {
   MEMFILE_FILE *new_file = (MEMFILE_FILE *)malloc(sizeof(MEMFILE_FILE));
   
   new_file->memory_start  = source_memory;
   new_file->memory_cursor = source_memory;

   return new_file;
}

void MEMFILE_fseek(MEMFILE_FILE *file, size_t pos, int position) {
   switch (position) {
      case (SEEK_SET) : {
         file->memory_cursor = file->memory_start  + pos;
      } break;
      case (SEEK_CUR) : {
         file->memory_cursor = file->memory_cursor + pos;
      } break;
      case (SEEK_END) : {
         // Ummm... pass.
      } break;
   }
}

int MEMFILE_ftell(MEMFILE_FILE *file) {
   return file->memory_cursor - file->memory_start;
}

static void tepplescpy(const void *in, void *out, unsigned int length) {
   unsigned char *source = (unsigned char *)in;
   unsigned char *dest   = (unsigned char *)out;
   for (; length > 0; length--) {
      *dest++ = *source++;
   }
}

int MEMFILE_fread(void *destination, int item_size, int no_items, MEMFILE_FILE *file) {
   int bytes = item_size * no_items;
   
   tepplescpy(file->memory_cursor, destination, bytes);
   
   file->memory_cursor += bytes;

   return bytes;
}

void MEMFILE_fclose(MEMFILE_FILE *file) {
   free(file);
}
