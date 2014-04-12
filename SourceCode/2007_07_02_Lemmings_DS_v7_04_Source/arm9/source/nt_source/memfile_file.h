//
// MEMFILE memory FILE simulator.
//
// (c) September 2006
//
// memfile_file.h
//   Engine for reading from memory as if it were a FILE stream.
//
// By Mathew Carr.
// mattcarr@gmail.com
//
// You can have this thing... knock yourself out.
// Find a use: holler, wouldya? :)
//

#ifndef __MEMFILE_FILE_H__
#define __MEMFILE_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct tagMEMFILE_FILE {
   const void *memory_start;
   const void *memory_cursor;
} MEMFILE_FILE;

MEMFILE_FILE *MEMFILE_fopen(const void *source_memory);
        void  MEMFILE_fseek(MEMFILE_FILE *file, size_t pos, int position);
         int  MEMFILE_ftell(MEMFILE_FILE *file);
         int  MEMFILE_fread(void *destination, int item_size, int no_items, MEMFILE_FILE *file);
        void  MEMFILE_fclose(MEMFILE_FILE *file);
        
#ifdef __cplusplus
}
#endif

#endif // __MEMFILE_FILE_H__

