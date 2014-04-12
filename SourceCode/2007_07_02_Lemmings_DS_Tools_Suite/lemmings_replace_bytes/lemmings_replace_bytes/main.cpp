//
// Lemmings DS - lemmings_winlemm_lvl_to_lds
//
// (c) March 2007
//
// main.cpp
//   lemmings_winlemm_lvl_to_lds commandline application converts 
//   Windows Lemmings levels into Version 5+ Lemmings DS .LDS levels.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>       

#include <iostream>
#include <vector>
#include <string>

#include "types.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// Does the file exist?
bool file_exists(const char *filename) {
   FILE *input_file;

   input_file = fopen(filename, "rb");

   if (input_file == NULL) {
      return false;
   } else {
      fclose(input_file);
      return true;
   }
}
          
// Get the file size through ftell.
int file_size(const char *filename) {
   FILE *input_file;

   input_file = fopen(filename, "rb");

   if (input_file == NULL) {
      return 0;
   } else {                 
      fseek(input_file, 0, SEEK_END);  
      int length = ftell(input_file);
   
      fclose(input_file);
      return length;
   }    
}

// Returns the argument a incrased to the next d.
int to_next(int a, int d) {
   return ((a + (d-1)) / d) * d;
}

// Identify arguments and determine ideal number of arguments to expect.
enum MAIN_ARGUMENT {
   MAIN_ARGUMENT_APPLICATION_LOCATION, 
   MAIN_ARGUMENT_DESTINATION_LEVEL,       
   MAIN_ARGUMENT_SOURCE_LEVEL,  
   MAIN_ARGUMENT_NO_LEMMINGS,
   MAIN_ARGUMENT_TO_BE_SAVED,       
   MAIN_ARGUMENT_RELEASE_RATE,
   MAIN_ARGUMENT_TIME,               
   MAIN_ARGUMENT_NO_TOOL_1,
   MAIN_ARGUMENT_NO_TOOL_2,
   MAIN_ARGUMENT_NO_TOOL_3,
   MAIN_ARGUMENT_NO_TOOL_4,
   MAIN_ARGUMENT_NO_TOOL_5,
   MAIN_ARGUMENT_NO_TOOL_6,
   MAIN_ARGUMENT_NO_TOOL_7,
   MAIN_ARGUMENT_NO_TOOL_8,
   MAIN_ARGUMENT_NEW_LEVEL_NAME,
   NO_MAIN_ARGUMENTS,
};

struct SIMPLE_WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT {
   u8 blank_1;
   u8 release_rate;
   u8 blank_2;
   u8 no_lems;
   u8 blank_3;
   u8 to_be_saved;
   u8 blank_4;
   u8 time;
   u8 tool_section[8][2]; // [n][0] is blank, [n][1] is value
   u8 untouched_bit[2048 - 24 - 32];
   u8 level_name[32];
} __attribute__ ((packed));

// Entry point.
int main(int argc, char *argv[]) {      
   for (int e = 0; e<argc;e++) printf("Arg %d: '%s'\n", e, argv[e]);
   
   // Error if you don't give a single filename level to convert.
   // If you supply three arguments, third must be level name switch.
   if (argc < NO_MAIN_ARGUMENTS) {                                                          
      printf("Bad arguments. (Too few)\n");
      return 0;
   }       
                                       
   // Get the size of the input file.                                    
   int input_file_size = file_size(argv[MAIN_ARGUMENT_SOURCE_LEVEL]);
      
   // The file is invalid if the filesize is zero.
   if (input_file_size == 0) {                
      printf("Can't find input file '%s'.\n", argv[MAIN_ARGUMENT_SOURCE_LEVEL]);
      return 0;                         
   // The file is invalid if the filesize is not 2Kib.
   } else if (input_file_size != 2048) {        
      printf("Not a valid Lemmings level. (Filesize not 2048 bytes)\n");
      return 0;
   }             
      
   // Handle to manage input file.
   FILE *input_file;                      
        
   // Load the incoming level into loaded_file array.                                          
   SIMPLE_WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT construct_file = {0};    
   input_file = fopen(argv[MAIN_ARGUMENT_SOURCE_LEVEL], "rb");  
      fseek(input_file, 24, SEEK_SET);
      fread(&(construct_file.untouched_bit), input_file_size, 1, input_file);
   fclose(input_file);         
   
   int arguments[100];
   
   for (int a = 0; a < 12; a++) {
      sscanf(argv[a + 3], "%d", &arguments[a]); 
   }              
   
   construct_file.no_lems      = arguments[0];
   construct_file.to_be_saved  = arguments[1];  
   construct_file.release_rate = arguments[2];
   construct_file.time         = arguments[3];
   
   construct_file.tool_section[0][1] = arguments[ 4];
   construct_file.tool_section[1][1] = arguments[ 5];
   construct_file.tool_section[2][1] = arguments[ 6];
   construct_file.tool_section[3][1] = arguments[ 7];
   construct_file.tool_section[4][1] = arguments[ 8];
   construct_file.tool_section[5][1] = arguments[ 9];
   construct_file.tool_section[6][1] = arguments[10];
   construct_file.tool_section[7][1] = arguments[11];
   
   memset(construct_file.level_name, 0x20, 32);
   
   strncpy((char *)construct_file.level_name, argv[MAIN_ARGUMENT_NEW_LEVEL_NAME], strlen(argv[MAIN_ARGUMENT_NEW_LEVEL_NAME]));
                                               
   FILE *output_file = fopen(argv[MAIN_ARGUMENT_DESTINATION_LEVEL], "wb");                       
      fwrite(&construct_file, 2048, 1, output_file); 
   fclose(output_file);
   
   return 0;
}
