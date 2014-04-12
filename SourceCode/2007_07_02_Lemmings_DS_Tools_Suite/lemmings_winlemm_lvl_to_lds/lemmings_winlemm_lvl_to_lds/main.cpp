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

#define DONT_NEED_DSX

#include "lemmings_level.h"
#include "lemmings_graphical_object.h"

#include "windows_lemmings_level.h"
#include "windows_lemmings_terra_object.h"  

// This program uses LodePNG by Lode Vandevenne
// See LodePNG.h for further information.
#include "lodepng.h"

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

       
// These hold the numbers of standard and custom exits respectively.
int no_standard_exits = 0;

// These are vectors of pointers to the graphical objects containing the exits.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_exit_graphical_objects;

// These hold the numbers of standard and custom entrances respectively.
int no_standard_entrances = 0;

// These are vectors of pointers to the graphical objects containing the entrances.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_entrance_graphical_objects;

// These hold the numbers of standard and custom traps respectively.
int no_standard_traps = 0;

// These are vectors of pointers to the graphical objects containing the traps.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_trap_graphical_objects;

// These hold the numbers of standard and custom hazards respectively.
int no_standard_hazards = 0;

// These are vectors of pointers to the graphical objects containing the hazards.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_hazard_graphical_objects;
// These hold the numbers of standard and custom uninteractives respectively.
int no_standard_uninteractives = 0;

// These are vectors of pointers to the graphical objects containing the uninteractives.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_uninteractive_graphical_objects;

// These hold the numbers of standard and custom waters respectively.
int no_standard_waters = 0;

// These are vectors of pointers to the graphical objects containing the waters.
std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> standard_water_graphical_objects;

// This function generalises the loading of graphical objects of a specific type from a specific
// location into a specific vector.
void GreedyGatherObjectCategory(std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> *destination_go_vector,     // Where will the pointer to this new memory go?
                                int *quantity_int,                                                          // Where's an int to keep track of this stuff?
                                const char *source_directory,                                               // Where's the directory?
                                const char *source_graphical_object_type);                                  // Whats the filename start?

// This function will gather all of the graphical objects it can from the standard and custom directories.
void GreedyGatherAllAvailableGraphicalObjects() {
   const char *standard_loading_location = "graphical_objects/"; 
   
   const char *object_category_strings[] = {"exit",
                                            "entrance",
                                            "trap",
                                            "hazard",
                                            "uninteractive",
                                            "water"};
     
   GreedyGatherObjectCategory(&standard_exit_graphical_objects,
                                  &no_standard_exits,
                                  standard_loading_location,
                                  object_category_strings[0]);  
     
   GreedyGatherObjectCategory(&standard_entrance_graphical_objects,
                                  &no_standard_entrances,
                                  standard_loading_location,
                                  object_category_strings[1]);  
     
   GreedyGatherObjectCategory(&standard_trap_graphical_objects,
                                  &no_standard_traps,
                                  standard_loading_location,
                                  object_category_strings[2]);  
     
   GreedyGatherObjectCategory(&standard_hazard_graphical_objects,
                                  &no_standard_hazards,
                                  standard_loading_location,
                                  object_category_strings[3]);  
     
   GreedyGatherObjectCategory(&standard_uninteractive_graphical_objects,
                                  &no_standard_uninteractives,
                                  standard_loading_location,
                                  object_category_strings[4]);  
     
   GreedyGatherObjectCategory(&standard_water_graphical_objects,
                                  &no_standard_waters,
                                  standard_loading_location,
                                  object_category_strings[5]);    
}                                      

void GreedyGatherObjectCategory(std::vector<LEMMINGS_GRAPHICAL_OBJECT_HEADER *> *destination_go_vector,     // Where will the pointer to this new memory go?
                                int *quantity_int,                                                          // Where's an int to keep track of this stuff?
                                const char *source_directory,                                               // Where's the directory?
                                const char *source_graphical_object_type) {                                 // Whats the filename start?
   // Reset counter                                 
   *quantity_int = 0;                                 
                                    
   // Grab all you can                               
   do {         
      // This holds the filename of the file we're going to load
      char incoming_graphical_object_filename[16384];
      sprintf(incoming_graphical_object_filename, "%s%s_%d.lgo", source_directory, source_graphical_object_type, *quantity_int);
                                                    
      if (!file_exists(incoming_graphical_object_filename)) {
         //allegro_message("found %d %ss in %s.", *quantity_int, source_graphical_object_type, source_directory);
         return;
      }
      
      unsigned int graphical_object_filesize;
      
      // Open the file
      FILE *graphical_object_file = fopen(incoming_graphical_object_filename, "rb");
                            
      // Grab the expected filesize                                                
      fread(&graphical_object_filesize, 4, 1, graphical_object_file);
      
      rewind(graphical_object_file);                                

      // Allocate memory for a memory instance of the file
      u8 *graphical_object_memory_instance = new u8[graphical_object_filesize];
                                
      // Load the file into memory
      fread(graphical_object_memory_instance, graphical_object_filesize, 1, graphical_object_file); 
      
      // Close the file.
      fclose(graphical_object_file);
      
      // Clonk this pointer onto the graphical object pointer vector.
      destination_go_vector->push_back((LEMMINGS_GRAPHICAL_OBJECT_HEADER *)graphical_object_memory_instance);
      
      (*quantity_int)++;
   } while (1);   
}

void DestroyGraphicalObjects() {
   for (int o = 0; o < no_standard_exits; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_exit_graphical_objects.at(o);
   }  
                             
   for (int o = 0; o < no_standard_entrances; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_entrance_graphical_objects.at(o);
   }  
   
   for (int o = 0; o < no_standard_traps; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_trap_graphical_objects.at(o);
   }  
   
   for (int o = 0; o < no_standard_hazards; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_hazard_graphical_objects.at(o);
   }  
   
   for (int o = 0; o < no_standard_uninteractives; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_uninteractive_graphical_objects.at(o);
   }  
   
   for (int o = 0; o < no_standard_waters; o++) {
      // Delete the new'd u8 array that contains the graphical object
      delete[] standard_water_graphical_objects.at(o);
   }  
}

void CopyPaletteFromGraphicalObjectIntoPaletteGiven(const LEMMINGS_GRAPHICAL_OBJECT_HEADER *graphical_object, u16 *palette) {
   for (int pe = 0; pe < 16; pe++) {
      palette[pe] = graphical_object->ideal_palette[pe]; 
   }  
}


// Identify arguments and determine ideal number of arguments to expect.
enum MAIN_ARGUMENT {
   MAIN_ARGUMENT_APPLICATION_LOCATION = 0,   
   MAIN_ARGUMENT_SOURCE_LEVEL_TYPE    = 1,
   MAIN_ARGUMENT_SOURCE_LEVEL_RATING  = 2,
   MAIN_ARGUMENT_SOURCE_LEVEL         = 3,
   MAIN_ARGUMENT_WINLEMM_HYPER_FLAG   = 4,
};

// Entry point.
int main(int argc, char *argv[]) {                                        
      printf("----------------------------------------------\n"
             "----------------------------------------------\n"
             " lemmings_amigalemm_lvl_to_lds by Mathew Carr \n"
             "----------------------------------------------\n"
             "-------------[ http://www.mrdictionary.net ]--\n"
             "----------------------------------------------\n"
             "\n");                                   
   
   for (int e = 0; e<argc;e++) printf("Arg %d: '%s'\n", e, argv[e]);
   
   // Error if you don't give a single filename level to convert.
   // If you supply three arguments, third must be level name switch.
   if (argc < 4) {                                                          
      printf("Bad arguments. (Too few)\n");
      return 0;
   }       
   
   if (argc == 5) {
      if (strcmp(argv[MAIN_ARGUMENT_WINLEMM_HYPER_FLAG], "N") != 0) {                             
         printf("Bad arguments. (Flag ending)\n");
         
         for (int e = 0; e<argc;e++) printf("'%s'\n", argv[e]);
         return 0;
      }
   }                             
   
   static bool need_to_load_graphical_objects = true;
       
   if (need_to_load_graphical_objects) {
      GreedyGatherAllAvailableGraphicalObjects();
      need_to_load_graphical_objects = false;
   }
      
   // If the used specifies the Magic parameter as the source name.
   if (strcmp(argv[MAIN_ARGUMENT_SOURCE_LEVEL], "Magic") == 0) {
      // We must convert all Original and Oh No! More Lemmings levels!
      
      // This holds the filename of the next level to convert.
      char filename[32];
      
      // This array emulates argv.
      char *pointers[5] = {argv[MAIN_ARGUMENT_APPLICATION_LOCATION],
                           argv[MAIN_ARGUMENT_SOURCE_LEVEL_TYPE], filename, argv[MAIN_ARGUMENT_WINLEMM_HYPER_FLAG]};
      
      // Output filename string as LVL%d.LVL]
      // for all levels from 0 to 30 for 0 to 3
      for (int l = 0; l < 30; l++) {
         for (int n = 0; n < 4; n++) {
            sprintf(filename, "Level%03d.%01d.lvl", l, n);
            main(argc, pointers);
         }
      }
      
      // We've converted all the levels.
      printf("Phew!\n");
      
      // Destroy the loaded graphcial objects upon program termination
      DestroyGraphicalObjects();
      
      // The end.
      exit(0);
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
   unsigned char *loaded_file = new unsigned char[input_file_size];    
   input_file = fopen(argv[MAIN_ARGUMENT_SOURCE_LEVEL], "rb");  
      fread(loaded_file, input_file_size, 1, input_file);
   fclose(input_file); 
   
   // Don't touch that file any more.
   input_file = NULL;                         
   
   // Interpret the loaded file array as a Windows lemmings level.
   const WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT *loaded_lemmings_level = (const WINDOWS_LEMMINGS_LEVEL_FILE_STRUCT *)loaded_file;
   
   /* ------------------------------------------------------------ */  
   
   // This struct holds a sane version of the information loaded into loaded_file (and therefore loaded_lemmings_level)
   SANE_WINDOWS_LEMMINGS_DESCRIPTION_STRUCT sane_incoming_description;
   
   // Populate sane_incoming_description with a sane description of loaded_lemmings_level
   loaded_lemmings_level->GetSaneDescriptionStruct(&sane_incoming_description);
                                                                                 
   /* ------------------------------------------------------------ */       
                                        
   // Begin to define the output level in this struct.      
   LEMMINGS_LEVEL_LDS_FILE_V7 new_level_file;
   memset(&new_level_file, 0, sizeof(LEMMINGS_LEVEL_LDS_FILE_V7));               
      
   // Until extra stuff is added onto the end of the struct,
   // the chunk size is the size of the LEMMINGS_LEVEL_LDS_FILE_* by itself.
   new_level_file.lemmings_level_file_size = sizeof(LEMMINGS_LEVEL_LDS_FILE_V7);   
     
   // Set the level version number.
   new_level_file.version_number = LEMMINGS_LEVEL_VERSION;     
   
   // Copy the currect validation string
   strcpy((char *)new_level_file.validation_string, correct_validation_string);
                                                                                                                       
   // Where does the memory containing the loadeds level name string start?
   const char *level_name_stored_string_start_ptr = loaded_lemmings_level->level_name;  
                                                                                            
   // Use this variable to find the first character of the level name that is not a space.
   const char *level_name_start_ptr = level_name_stored_string_start_ptr;  
   while (*level_name_start_ptr == 0x20) level_name_start_ptr++; 
    
   // Use this variable to find the first pair of space characters.
   const char *level_name_end_ptr = level_name_start_ptr;   
   while ((!((*level_name_end_ptr == 0x20) && (*(level_name_end_ptr + 1) == 0x20)))
       && ((level_name_end_ptr - level_name_stored_string_start_ptr) < 31         )) level_name_end_ptr++; 
           // This expression holds the offset of the end pointer from the start of the string
           // We don't want to peek at level name memory beyond character 31.
   
   // Calculate the length of the level name.
   int level_name_length = (level_name_end_ptr - level_name_start_ptr) + 1;
                        
   // Copy the level name from the located memory.                     
   strncpy(new_level_file.runtime_stats.level_name, level_name_start_ptr, min(30, level_name_length));
   // Always make sure that the copied string is null terminated.
   new_level_file.runtime_stats.level_name[min(30, level_name_length - 1)] = 0x00;
   
   /* ------------------------ */
   
   // Find the location of the last backslash in argv[1].   
   char *character_past_slash = argv[MAIN_ARGUMENT_SOURCE_LEVEL];
   char *search_last_slash_argv1 = argv[MAIN_ARGUMENT_SOURCE_LEVEL];
   while (search_last_slash_argv1 = strstr(search_last_slash_argv1 + 1, "/")) { // find the last slash.
      character_past_slash = search_last_slash_argv1 + 1;   
   }                             
   
   /* ------------------------ */
   
   // Create description string.
   sprintf((char *)new_level_file.runtime_stats.description, "Converted from '%s'", character_past_slash);  
   // Potentially unsafe sprintf!
   
   new_level_file.runtime_stats.lemmings        = sane_incoming_description.number_of_lemmings;
   new_level_file.runtime_stats.to_be_saved     = sane_incoming_description.to_be_saved;
   new_level_file.runtime_stats.release_rate    = sane_incoming_description.release_rate;
   new_level_file.runtime_stats.time_in_minutes = sane_incoming_description.time_in_minutes;
   
   strncpy(new_level_file.runtime_stats.rating_description, argv[MAIN_ARGUMENT_SOURCE_LEVEL_RATING], 15);
   
   // You'll have to set this manually in the editor if you're making a
   // proper Lemmings rated level set.
                 
   // Copy the skills data.                 
   for (unsigned int skill = 0; skill < 8; skill++) {
      new_level_file.runtime_stats.tool_complement[skill] = sane_incoming_description.no_skills[skill];  
   }                                                  
   
   // Position camera (assume the start position is referring to the left edge of the screen.
   new_level_file.runtime_stats.camera_x = (sane_incoming_description.screen_start_x_pos + 16) + 128;
   // 128 because windows wants to place the left edge of the camera, and lemmings ds wants to place the center of the camera
   
   // The camera is always centred vertically.
   new_level_file.runtime_stats.camera_y = 168 / 2;
   
   //TODO set colour 240 based on the current graphial set
   
   // Use set instead of the big kahuna thing to refer to the current graphics set.
   int &set = sane_incoming_description.graphics_set;     
   
   // Oh no levels need their set adjusted.
   if (strcmp(argv[MAIN_ARGUMENT_SOURCE_LEVEL_TYPE], "ONML") == 0) {
      set += 5;
   }
   
   // Set the one way colour for this level.
   new_level_file.one_way_colour = 240;
   
   u16 standard_one_way_colours[9] = {RGB15A(30, 30,  0),
                                      RGB15A( 8,  8, 28),
                                      RGB15A( 8,  8, 28),
                                      RGB15A(30, 26, 26),
                                      RGB15A(30,  4,  4),
                                      RGB15A(30, 26, 26),
                                      RGB15A(30, 26, 26),
                                      RGB15A(30, 26, 26),
                                      RGB15A(30, 26, 26),};
   
   new_level_file.runtime_stats.level_palette[240] = standard_one_way_colours[set];
                                                  
   printf("Graphics set: %2d\n", set);
   
   // Copy the appropriate 'using' string to the new struct:
   sprintf(new_level_file.texture_archive_using, "winlemm_%s", windows_lemmings_terra_object_theme_names[set]);
                     
   printf("Set to use default texture archive: '%s.LTA'\n", new_level_file.texture_archive_using);                  
                      
   /* ------------------------------------------------------------ */
   
   // We need to set up the correct junctioning for this particular graphics set
   
   new_level_file.runtime_stats.entrance_genus_junction = set;      
   new_level_file.runtime_stats.exit_genus_junction     = set;   
   
   // Set nine doesn't have a respective water!
   if (set < 9) {
      new_level_file.runtime_stats.water_genus_junction = set;
   }
   
   // Set up the appropriate junctions for the different graphics sets:   
   switch (set) {
      case (0) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 0;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 1;
         new_level_file.runtime_stats.trap_genus_junctions[2] = 2;
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 2;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
      } break;    
      case (1) : {
         new_level_file.runtime_stats.hazard_genus_junctions[0] = 0;
         new_level_file.runtime_stats.hazard_genus_junctions[1] = 1;
         new_level_file.runtime_stats.hazard_genus_junctions[2] = 2;     
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 3;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
      } break;    
      case (2) : {
         new_level_file.runtime_stats.trap_genus_junctions[0]   = 3;
         new_level_file.runtime_stats.hazard_genus_junctions[0] = 3;      
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 4;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
      } break;    
      case (3) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 4;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 5;
         new_level_file.runtime_stats.trap_genus_junctions[2] = 6;     
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 5;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
      } break;      
      case (4) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 7;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 8;
         new_level_file.runtime_stats.trap_genus_junctions[2] = 9;         
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 1;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 6;
      } break;      
      case (5) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 10;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 11;      
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 1;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 7;
      } break;       
      case (6) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 12;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 13;
         new_level_file.runtime_stats.trap_genus_junctions[2] = 14;        
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 8;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 9;
         new_level_file.runtime_stats.uninteractive_genus_junctions[3] = 1;
      } break;       
      case (7) : {
         new_level_file.runtime_stats.trap_genus_junctions[0]   = 15;
         new_level_file.runtime_stats.hazard_genus_junctions[0] = 4;   
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 10;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
      } break;   
      case (8) : {
         new_level_file.runtime_stats.trap_genus_junctions[0] = 16;
         new_level_file.runtime_stats.trap_genus_junctions[1] = 17;     
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 0;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 11;
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 1;
         new_level_file.runtime_stats.uninteractive_genus_junctions[3] = 12;
      } break;          
      case (9) : {
         new_level_file.runtime_stats.uninteractive_genus_junctions[0] = 13;
         new_level_file.runtime_stats.uninteractive_genus_junctions[1] = 14;     
         new_level_file.runtime_stats.uninteractive_genus_junctions[2] = 15;
         new_level_file.runtime_stats.uninteractive_genus_junctions[3] = 16;
         new_level_file.runtime_stats.uninteractive_genus_junctions[4] = 17;
         new_level_file.runtime_stats.uninteractive_genus_junctions[5] = 18;
         new_level_file.runtime_stats.uninteractive_genus_junctions[6] = 19;
         new_level_file.runtime_stats.uninteractive_genus_junctions[7] = 20;
      } break;          
   }                                                                      
                      
   /* ------------------------------------------------------------ */
   
   // Use the default palettes from the graphical objects junctioned:
      
   CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_exit_graphical_objects[new_level_file.runtime_stats.exit_genus_junction],
                                                  new_level_file.runtime_stats.exit_palette);
   CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_entrance_graphical_objects[new_level_file.runtime_stats.entrance_genus_junction],
                                                  new_level_file.runtime_stats.entrance_palette);
                                                  
   for (int trap_genus = 0; trap_genus < NO_TRAP_GENUSES; trap_genus++) {     
      CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_trap_graphical_objects[new_level_file.runtime_stats.trap_genus_junctions[trap_genus]],
                                                     new_level_file.runtime_stats.trap_genus_palettes[trap_genus]);
   }                                              
   for (int hazard_genus = 0; hazard_genus < NO_HAZARD_GENUSES; hazard_genus++) {     
      CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_hazard_graphical_objects[new_level_file.runtime_stats.hazard_genus_junctions[hazard_genus]],
                                                     new_level_file.runtime_stats.hazard_genus_palettes[hazard_genus]);
   }                                              
   for (int uninteractive_genus = 0; uninteractive_genus < NO_UNINTERACTIVE_GENUSES; uninteractive_genus++) {     
      CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_uninteractive_graphical_objects[new_level_file.runtime_stats.uninteractive_genus_junctions[uninteractive_genus]],
                                                     new_level_file.runtime_stats.uninteractive_genus_palettes[uninteractive_genus]);
   }                                             
                                                  
   CopyPaletteFromGraphicalObjectIntoPaletteGiven(standard_water_graphical_objects[new_level_file.runtime_stats.water_genus_junction],
                                                  new_level_file.runtime_stats.water_palette);
                      
                      
   /* ------------------------------------------------------------ */     
                                                                      
   const int T =  0; // trap
   const int H =  1; // hazard
   const int U =  2; // uninteractive
   const int X = -1; // something else       
   
   // This array links a set - object number pair to a 'genus' in the level
                                                                       /* 0  1  2  3  4  5  6  7  8  9 10 11  */ 
   const int genus_from_set_and_object_number[10][12] = /* 0 SOIL   */  {{X, X, 0, X, X, X, 0, 1, 1, 2, 2, X, },
                                                        /* 1 FIRE   */   {X, X, 0, X, X, X, 1, 0, 1, 2, 2, X, },
                                                        /* 2 MARBLE */   {X, X, 0, X, X, X, 1, 2, 0, 0, X, X, },
                                                        /* 3 PILLAR */   {X, X, 0, X, X, X, 1, 2, 0, 1, 2, X, },
                                                        /* 4 ICE    */   {X, X, 0, 1, X, X, X, 0, 2, 1, 2, X, },
                                                        /* 5 BRICK  */   {X, X, 0, X, X, X, 0, 1, 1, 2, X, X, },
                                                        /* 6 ROCK   */   {X, X, 0, X, X, X, 0, 1, 1, 2, 2, 3, },
                                                        /* 7 SNOW   */   {X, X, 0, X, X, X, 1, 2, 0, 0, X, X, },
                                                        /* 8 BUBBLE */   {X, X, 0, X, X, X, 1, 2, 0, 1, 3, X, },
                                                        /* 9 SNOW   */   {X, X, 0, 1, 2, 3, 4, 5, 6, 7, X, X, }};                       
   
                                                                      /* 0  1  2  3  4  5  6  7  8  9 10 11  */ 
   const int type_from_set_and_object_number [10][12] = /* 0 SOIL   */  {{X, X, U, X, X, X, T, U, T, U, T, X, },
                                                        /* 1 FIRE   */   {X, X, U, X, X, X, U, H, H, U, H, X, },
                                                        /* 2 MARBLE */   {X, X, U, X, X, X, U, U, T, H, X, X, },
                                                        /* 3 PILLAR */   {X, X, U, X, X, X, U, U, T, T, T, X, },
                                                        /* 4 ICE    */   {X, X, U, U, X, X, X, T, U, T, T, X, },
                                                        /* 5 BRICK  */   {X, X, U, X, X, X, T, T, U, U, X, X, },
                                                        /* 6 ROCK   */   {X, X, U, X, X, X, T, U, T, U, T, U, },
                                                        /* 7 SNOW   */   {X, X, U, X, X, X, U, U, T, H, X, X, },
                                                        /* 8 BUBBLE */   {X, X, U, X, X, X, U, U, T, T, U, X, },
                                                        /* 9 SNOW   */   {X, X, U, U, U, U, U, U, U, U, X, X, }};         
         
   int current_entrance      = 0;                      
   int current_exit          = 0; 
   int current_trap          = 0; 
   int current_hazard        = 0;     
   int current_uninteractive = 0;                       
   int current_water_area    = 0;                         
   int current_one_way_area  = 0;                    
   int current_steel_area    = 0;
   
   // Now iterate through all of the Windows lemmings 'Objects'
   for (int object_no = 0; object_no < 32; object_no++) {
      SANE_WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_STRUCT sane_object;      
                                                                 
      // Get a sane version of this object.
      // If the object retrieved was FILLER, then ignore this item.
      if (!loaded_lemmings_level->GetSaneInteractiveObjectStruct(&sane_object, object_no)) continue;
      
      printf("Interactive %2d = Type %2d: ", object_no, sane_object.object_type);
      
      if (sane_object.object_type == WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_TYPE_EXIT) {         
         
         new_level_file.runtime_stats.exit_x[current_exit] = sane_object.xpos + standard_exit_graphical_objects[set]->handle_x;   
         new_level_file.runtime_stats.exit_y[current_exit] = sane_object.ypos + standard_exit_graphical_objects[set]->handle_y;   
         
         printf("Exit     %2d: X=%4d, Y=%4d.\n",current_exit, sane_object.xpos, sane_object.ypos);     
         current_exit++;   
      } else
      if (sane_object.object_type == WINDOWS_LEMMINGS_INTERACTIVE_OBJECT_TYPE_ENTRANCE) {  
             
         new_level_file.runtime_stats.entrance_x[current_entrance] = sane_object.xpos + standard_entrance_graphical_objects[set]->handle_x;   
         new_level_file.runtime_stats.entrance_y[current_entrance] = sane_object.ypos + standard_entrance_graphical_objects[set]->handle_y;   
         new_level_file.runtime_stats.entrance_d[current_entrance] = 1; // Do they always walk to the right?
                           
         printf("Entrance %2d: X=%4d, Y=%4d.\n",current_entrance, sane_object.xpos, sane_object.ypos);  
           
         current_entrance++;   
      } else
      if (((set == 0) && (sane_object.object_type == 5)) // These all refer to water,
       || ((set == 1) && (sane_object.object_type == 5))
       || ((set == 2) && (sane_object.object_type == 5))
       || ((set == 3) && (sane_object.object_type == 5))
       || ((set == 4) && (sane_object.object_type == 6))
       || ((set == 5) && (sane_object.object_type == 5))
       || ((set == 6) && (sane_object.object_type == 5))
       || ((set == 7) && (sane_object.object_type == 5))
       || ((set == 8) && (sane_object.object_type == 5))) {          
         // Lemmings DS can only handle so many water areas, so continue if limit reached.
         if (current_water_area == MAX_NO_WATERS) continue;
         
         new_level_file.runtime_stats.water_x1[current_water_area] = sane_object.xpos;
         new_level_file.runtime_stats.water_x2[current_water_area] = sane_object.xpos + 63;
         
         new_level_file.runtime_stats.water_y[current_water_area] = sane_object.ypos + standard_water_graphical_objects[set]->handle_y;   
         
         new_level_file.runtime_stats.water_z[current_water_area] = sane_object.behind_all_others ? WATER_Z_BACKGROUND : WATER_Z_FOREGROUND;
                                 
         printf("Water    %2d: X=%4d, Y=%4d.\n",current_water_area, sane_object.xpos, sane_object.ypos);  
                                 
         current_water_area++;   
      } else
      if (((set == 0) && (sane_object.object_type == 3)) // These all refer to one way blocks, pointing left.
       || ((set == 1) && (sane_object.object_type == 3))
       || ((set == 2) && (sane_object.object_type == 3))
       || ((set == 3) && (sane_object.object_type == 3))
       || ((set == 4) && (sane_object.object_type == 4))
       || ((set == 5) && (sane_object.object_type == 3))
       || ((set == 6) && (sane_object.object_type == 3))
       || ((set == 7) && (sane_object.object_type == 3))
       || ((set == 8) && (sane_object.object_type == 3))) {          
         // Lemmings DS can only handle MAX_NO_ONE_WAY_AREAS one way areas, so continue after the MAX_NO_ONE_WAY_AREASth one is converted.
         if (current_one_way_area == MAX_NO_ONE_WAY_AREAS) continue;
         
         new_level_file.runtime_stats.one_way_area_x1[current_one_way_area] = sane_object.xpos;
         new_level_file.runtime_stats.one_way_area_x2[current_one_way_area] = sane_object.xpos + 31;
         new_level_file.runtime_stats.one_way_area_y1[current_one_way_area] = sane_object.ypos;
         new_level_file.runtime_stats.one_way_area_y2[current_one_way_area] = sane_object.ypos + 31;
         
         new_level_file.runtime_stats.one_way_area_d[current_one_way_area] = 0;
                           
         printf("Oneway < %2d: X=%4d, Y=%4d.\n",current_one_way_area, sane_object.xpos, sane_object.ypos);  
           
         current_one_way_area++;   
      } else
      if (((set == 0) && (sane_object.object_type == 4)) // These all refer to one way blocks, pointing right.
       || ((set == 1) && (sane_object.object_type == 4))
       || ((set == 2) && (sane_object.object_type == 4))
       || ((set == 3) && (sane_object.object_type == 4))
       || ((set == 4) && (sane_object.object_type == 5))
       || ((set == 5) && (sane_object.object_type == 4))
       || ((set == 6) && (sane_object.object_type == 4))
       || ((set == 7) && (sane_object.object_type == 4))
       || ((set == 8) && (sane_object.object_type == 4))) {          
         // Lemmings DS can only handle MAX_NO_ONE_WAY_AREAS one way areas, so continue after the MAX_NO_ONE_WAY_AREASth one is converted.
         if (current_one_way_area == MAX_NO_ONE_WAY_AREAS) continue;
         
         new_level_file.runtime_stats.one_way_area_x1[current_one_way_area] = sane_object.xpos;
         new_level_file.runtime_stats.one_way_area_x2[current_one_way_area] = sane_object.xpos + 31;
         new_level_file.runtime_stats.one_way_area_y1[current_one_way_area] = sane_object.ypos;
         new_level_file.runtime_stats.one_way_area_y2[current_one_way_area] = sane_object.ypos + 31;
         
         new_level_file.runtime_stats.one_way_area_d[current_one_way_area] = 1;
                           
         printf("Oneway > %2d: X=%4d, Y=%4d.\n",current_one_way_area, sane_object.xpos, sane_object.ypos);  
           
         current_one_way_area++;   
      } else {
         switch (type_from_set_and_object_number[set][sane_object.object_type]) {
            case (T) : { // The current set and object number return a TRAP         
               // Lemmings DS can only handle so many traps:
               if (current_trap == MAX_NO_TRAPS) continue;
      
               int actual_trap_object_number = new_level_file.runtime_stats.trap_genus_junctions[genus_from_set_and_object_number[set][sane_object.object_type]];
      
               new_level_file.runtime_stats.trap_x[current_trap] = sane_object.xpos + standard_trap_graphical_objects[actual_trap_object_number]->handle_x;   
               new_level_file.runtime_stats.trap_y[current_trap] = sane_object.ypos + standard_trap_graphical_objects[actual_trap_object_number]->handle_y; 
               
               new_level_file.runtime_stats.trap_z[current_trap] = sane_object.behind_all_others ? TRAP_Z_BACKGROUND : TRAP_Z_FOREGROUND;
                                 
               // This particular trap is trap zero.
               new_level_file.runtime_stats.trap_genus[current_trap] = genus_from_set_and_object_number[set][sane_object.object_type];
                       
               printf("Trap     %2d: X=%4d, Y=%4d, A.type=%2d, (GEN=%2d).\n", current_trap, sane_object.xpos, sane_object.ypos, actual_trap_object_number, new_level_file.runtime_stats.trap_genus[current_trap]);  
                 
               current_trap++;   
            } break;             
            case (H) : { // The current set and object number return a HAZARD         
               // Lemmings DS can only handle so many hazards:
               if (current_hazard == MAX_NO_HAZARDS) continue;
      
               int actual_hazard_object_number = new_level_file.runtime_stats.hazard_genus_junctions[genus_from_set_and_object_number[set][sane_object.object_type]];
      
               new_level_file.runtime_stats.hazard_x[current_hazard] = sane_object.xpos + standard_hazard_graphical_objects[actual_hazard_object_number]->handle_x; 
               new_level_file.runtime_stats.hazard_y[current_hazard] = sane_object.ypos + standard_hazard_graphical_objects[actual_hazard_object_number]->handle_y; 
               
               new_level_file.runtime_stats.hazard_z[current_hazard] = sane_object.behind_all_others ? HAZARD_Z_BACKGROUND : HAZARD_Z_FOREGROUND;
                                 
               // This particular hazard is hazard zero.
               new_level_file.runtime_stats.hazard_genus[current_hazard] = genus_from_set_and_object_number[set][sane_object.object_type];
                                 
               printf("Hazard   %2d: X=%4d, Y=%4d, A.type=%2d, (GEN=%2d).\n", current_hazard, sane_object.xpos, sane_object.ypos, actual_hazard_object_number, new_level_file.runtime_stats.hazard_genus[current_hazard]);  
                 
               current_hazard++;   
            } break;             
            case (U) : { // The current set and object number return a UNINTERACTIVE         
               // Lemmings DS can only handle so many uninteractives:
               if (current_uninteractive == MAX_NO_UNINTERACTIVES) continue;
      
               int actual_uninteractive_object_number = new_level_file.runtime_stats.uninteractive_genus_junctions[genus_from_set_and_object_number[set][sane_object.object_type]];
      
               new_level_file.runtime_stats.uninteractive_x[current_uninteractive] = sane_object.xpos + standard_uninteractive_graphical_objects[actual_uninteractive_object_number]->handle_x; 
               new_level_file.runtime_stats.uninteractive_y[current_uninteractive] = sane_object.ypos + standard_uninteractive_graphical_objects[actual_uninteractive_object_number]->handle_y; 
               
               new_level_file.runtime_stats.uninteractive_z[current_uninteractive] = sane_object.behind_all_others ? UNINTERACTIVE_Z_BACKGROUND : UNINTERACTIVE_Z_FOREGROUND;
                                 
               // This particular uninteractive is uninteractive zero.
               new_level_file.runtime_stats.uninteractive_genus[current_uninteractive] = genus_from_set_and_object_number[set][sane_object.object_type];
                                 
               printf("Unint.   %2d: X=%4d, Y=%4d, A.type=%2d, (GEN=%2d).\n", current_uninteractive, sane_object.xpos, sane_object.ypos, actual_uninteractive_object_number, new_level_file.runtime_stats.uninteractive_genus[current_uninteractive]);  
                 
               current_uninteractive++;   
            } break;    
         }    
      } 
   }
         
   /* ------------------------------------------------------------ */        
   
   // Now iterate through all of the Windows lemmings 'Steel areas'
   for (int object_no = 0; object_no < 32; object_no++) {
      // Get a sane version of this steel area.
      SANE_WINDOWS_LEMMINGS_STEEL_AREA_STRUCT sane_steel_area;      
      if (!loaded_lemmings_level->GetSaneSteelAreaStruct(&sane_steel_area, object_no)) break;
      // If the steel area retrieved was FILLER, then break this loop.
      
      new_level_file.runtime_stats.steel_area_x1[current_steel_area] = sane_steel_area.xpos;
      new_level_file.runtime_stats.steel_area_y1[current_steel_area] = sane_steel_area.ypos;
      
      new_level_file.runtime_stats.steel_area_x2[current_steel_area] = sane_steel_area.xpos + sane_steel_area.width  - 1;
      new_level_file.runtime_stats.steel_area_y2[current_steel_area] = sane_steel_area.ypos + sane_steel_area.height - 1;
                                                                         
      /** This is added because some original lemmings levels are WEIRD. **/                                                                     
      new_level_file.runtime_stats.steel_area_y1[current_steel_area] += 2;
      
      printf("Steel %2d: X1=%4d, Y1=%4d, X2=%4d, Y2=%4d.\n",current_steel_area, new_level_file.runtime_stats.steel_area_x1[current_steel_area],
                                                                                new_level_file.runtime_stats.steel_area_y1[current_steel_area],
                                                                                new_level_file.runtime_stats.steel_area_x2[current_steel_area],
                                                                                new_level_file.runtime_stats.steel_area_y2[current_steel_area]);  
   
      current_steel_area++;   
      
      // Lemmings DS can only handle 32 steel areas, so break after the 32nd one is converted.
      if (current_steel_area == MAX_NO_STEEL_AREAS) break;
   }                                                        
   
   // Use the tallied totals to populate the level file struct.
   new_level_file.runtime_stats.no_entrances      = current_entrance;   
   new_level_file.runtime_stats.no_exits          = current_exit;        
   new_level_file.runtime_stats.no_traps          = current_trap;   
   new_level_file.runtime_stats.no_hazards        = current_hazard;   
   new_level_file.runtime_stats.no_uninteractives = current_uninteractive;   
   new_level_file.runtime_stats.no_waters         = current_water_area;         
                      
   // Populate the file struct using the tallied total.
   new_level_file.runtime_stats.no_steel_areas    = current_steel_area;     
   new_level_file.runtime_stats.no_one_way_areas  = current_one_way_area;  
   
   /* ------------------------------------------------------------ */ 
   
   // Now we have to decode and store the terra objects for the given style.
   
   // First, get the number of terra objects for the given graphics set.
   int no_terra_objects = windows_lemmings_terra_object_count_per_style[set];
   
   printf("There are %d terra objects in the %s style.\n", no_terra_objects, windows_lemmings_terra_object_theme_names[set]);
            
   // This struct holds the size of a loaded texture from a png.                  
   typedef struct tagLEMMINGS_LEVEL_OBJECT_DATABASE_ENTRY {
      s32 xs, ys; // Size and dimensions of the data.
   } LEMMINGS_LEVEL_OBJECT_DATABASE_ENTRY;

   // This array holds information about all of the loaded terra objects.
   LEMMINGS_LEVEL_OBJECT_DATABASE_ENTRY terra_objects[no_terra_objects];
   // It's used when placing the objects onto the screen
   // and also when writing the data to the file after the loading is complete.
      
   // This decoder will decode the incoming PNG images.
   LodePNG::Decoder decoder;
   
   for (int terra_no = 0; terra_no < no_terra_objects; terra_no++) {
      // This stores the PNG file in memory while it is being worked on.
      std::vector<unsigned char> buffer;   
      
      // This stores the PNG decompressed image in memory while it is being worked on.
      std::vector<unsigned char> image;   
      
      // This holds the filename of the file we're going to load as the [terra_no]th terra_object image.
      char incoming_terra_object_image_filename[16384];
      sprintf(incoming_terra_object_image_filename, "windows_lemmings_terrain_images/%s/%d.png",
                                                    windows_lemmings_terra_object_theme_names[set],
                                                    terra_no); 
                                                       
      //printf("%s\n", incoming_terra_object_image_filename);
      
      // Hope it converts into a std::string using this constructor.
      std::string string_incoming_terra_object_image_filename = std::string(incoming_terra_object_image_filename);
      
      //std::cout << string_incoming_terra_object_image_filename << std::endl;
      
      // Load in the terra object PNG.
      LodePNG::loadFile(buffer, string_incoming_terra_object_image_filename);
      
      // Decode the image, retaining the image pixel index values.
      decoder.decodeGeneric(image, buffer);
      
      terra_objects[terra_no].xs  = decoder.getWidth();
      terra_objects[terra_no].ys = decoder.getHeight();
            
      //printf("%d", decoder.getError());exit(0);
      
      printf("%s %d is %d by %d.\n", windows_lemmings_terra_object_theme_names[set],
                                     terra_no,
                                     terra_objects[terra_no].xs,
                                     terra_objects[terra_no].ys);
      
      // Extract the palette from the first terra object:
      if (terra_no == 0) {
         LodePNG::Decoder::Info first_image_info_struct = decoder.getInfo();
         
         for (int palette_entry = 0; palette_entry < 16; palette_entry++) {
            int i_r, i_g, i_b; // These are the incoming palette colours.
            
            int nds_r, nds_g, nds_b; // These are the incoming colours reduced to 15-bit.
            
            i_r = first_image_info_struct.palette[0 + palette_entry * 4];
            i_g = first_image_info_struct.palette[1 + palette_entry * 4];
            i_b = first_image_info_struct.palette[2 + palette_entry * 4]; 
            
            // Reduce the colours to 15 bit.
            nds_r = (int)((31.0f * i_r) / 255.0f);
            nds_g = (int)((31.0f * i_g) / 255.0f);
            nds_b = (int)((31.0f * i_b) / 255.0f);
               
#define RGB15(r,g,b) (((b)<<10)|((g)<<5)|(r))

            u16 nds_colour = RGB15A(nds_r, nds_g, nds_b);
                        
            // Fill in the palette of the level using the colours from the first image in the terra object source library.
            new_level_file.runtime_stats.level_palette[palette_entry] = nds_colour;
                                         
            //printf("PE %02d: r=%03d, g=%03d, b=%03d -> R=%02d, G=%02d, B=%02d.\n", palette_entry, i_r, i_g, i_b, nds_r, nds_g, nds_b);        
         }

// These are taken from Lemmings DS, and need to be specified here.
#define SPECIAL_COLOUR_BUILDER_BRICK   253
#define SPECIAL_COLOUR_MAP_LEMMING     254
#define SPECIAL_COLOUR_MAP_BORDER      255
         new_level_file.runtime_stats.level_palette[SPECIAL_COLOUR_BUILDER_BRICK] = RGB15A(31, 31,  6);
         new_level_file.runtime_stats.level_palette[SPECIAL_COLOUR_MAP_LEMMING]   = RGB15A( 5, 31,  5);
         new_level_file.runtime_stats.level_palette[SPECIAL_COLOUR_MAP_BORDER]    = RGB15A(31, 31, 31);
      }
      
      // At this point, I would like to thank Lode Vandevenne
      // without whom this application would not have been possible.
      // Kudos to you, LodePNG has performed admirably once again.                                                  
   }
   
   /* ------------------------------------------------------------ */ 
   
   // Now we have to manage all of the terrain pieces placed throughout the level.
   int no_placed_terrain_pieces = 0;
   
   // This vector will hold all of the LEMMINGS_LEVEL_TERRAIN_OBJECT_16 which make up the level.
   std::vector<LEMMINGS_LEVEL_TERRAIN_OBJECT_16> level_terrain_object_16s;
   
   for (int current_terrain_object_read = 0; current_terrain_object_read < 400; current_terrain_object_read++) {
      // Get a sane version of this terrain object.
      SANE_WINDOWS_LEMMINGS_TERRAIN_OBJECT_STRUCT sane_terrain_object;      
      if (!loaded_lemmings_level->GetSaneTerrainObjectStruct(&sane_terrain_object, current_terrain_object_read)) {
         printf("I think that's filler.\n");
         
         // If the terrain object retrieved was FILLER, then skip this loop.
         continue;
      };
      
      // If the continue wasn't used, this is a valid terrain placement, so let's increment the placed terrain counter.
      no_placed_terrain_pieces++;
              // Terrain Placement
      printf("TPL %03d: x = %4d, y = %4d, id = %2d.\n", current_terrain_object_read, sane_terrain_object.xpos, sane_terrain_object.ypos, sane_terrain_object.terrain_type);
      
      // This is the new object generated from the information contained in the Windows lemmings terrain object.
      LEMMINGS_LEVEL_TERRAIN_OBJECT_16 derived_lemmings_level_object;
      
      // The object type is always 16 colours.
      derived_lemmings_level_object.object_header.object_type = LEMMINGS_LEVEL_TERRAIN_OBJECT_TYPE_LEVEL_OBJECT_16;
      
      // Now we have to determine what flags should be applied to this object.
      derived_lemmings_level_object.object_header.object_flags = 0;
      
      // If this object is allowed to write over other objects with transparency, set the subtractive flag.
      if (sane_terrain_object.zero_overwrite)    derived_lemmings_level_object.object_header.object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SUBTRACTIVE;
      
      // If this object is only allowed to write blank areas, set the appropriate flags.
      if (sane_terrain_object.behind_all_others) derived_lemmings_level_object.object_header.object_flags |= LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_SPECIAL_MASKING_BEHAVIOUR
                                                                                                              | LEMMINGS_LEVEL_TERRAIN_OBJECT_FLAGS_FLAG_ONLY_DRAW_ON_BLANKS;
      
      // Set the position of the object:
      derived_lemmings_level_object.x1 = sane_terrain_object.xpos;   
      derived_lemmings_level_object.y1 = sane_terrain_object.ypos;   
      derived_lemmings_level_object.x2 = sane_terrain_object.xpos + terra_objects[sane_terrain_object.terrain_type].xs - 1;   
      derived_lemmings_level_object.y2 = sane_terrain_object.ypos + terra_objects[sane_terrain_object.terrain_type].ys - 1;   
      
      // printf("W: % 4d, H: % 4d.\n", terra_objects[sane_terrain_object.terrain_type].xs, terra_objects[sane_terrain_object.terrain_type].ys);
                                             
      // The texture scaling parameters for the x axis are always all defaults.
      derived_lemmings_level_object.tox = 0;
      derived_lemmings_level_object.tsx = 1 << 8;  
      
      // If the object is the right way up, give it normal TOY and TSY parameters, else flip it.
      if (!sane_terrain_object.upside_down) {
         derived_lemmings_level_object.toy = 0;
         derived_lemmings_level_object.tsy = 1 << 8;             
      } else {
         derived_lemmings_level_object.toy = (terra_objects[sane_terrain_object.terrain_type].ys - 1) * (1 << 8);
         derived_lemmings_level_object.tsy = -1 << 8;     
      }
      
      // The object id is the same as the terrain type.
      derived_lemmings_level_object.object_id = sane_terrain_object.terrain_type;
      
      for (int palette_mapping_entry = 0; palette_mapping_entry < 16; palette_mapping_entry++) {
         derived_lemmings_level_object.pal_map[palette_mapping_entry] = palette_mapping_entry; 
      }
      
      // Push the new object onto the LEMMINGS_LEVEL_TERRAIN_OBJECT_16 vector.
      level_terrain_object_16s.push_back(derived_lemmings_level_object);
   }
   
   // Use the number from the flor loop above to specify how many objects there are in the level.
   new_level_file.no_terrain_objects = no_placed_terrain_pieces;
   
   // Update the chunk size to reflect the added terrain placements
   new_level_file.lemmings_level_file_size += sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16) * new_level_file.no_terrain_objects;
   
   /* ------------------------------------------------------------ */ 

   int no_characters_before_slash_argv1 = ((int)(((char *)(character_past_slash)) - ((char *)(argv[MAIN_ARGUMENT_SOURCE_LEVEL]))));
   
   // Insert the version specifier into the output filename.
   //char *output_filename = new char[strlen(argv[1]) + 1 + 4];
   char *output_filename = new char[1024]; // Just reserve a whole wodge of memory here.
   // Copy the initial part of the string up to and including the slash.
   strncpy(output_filename                          , argv[MAIN_ARGUMENT_SOURCE_LEVEL], no_characters_before_slash_argv1);
   // Add in the version prefix.
   strcpy(output_filename+no_characters_before_slash_argv1, "v07_");           
   // Then paste the rest of the name. (this should be something like FILENAME.LDS)
   strcpy(output_filename+no_characters_before_slash_argv1+4, argv[MAIN_ARGUMENT_SOURCE_LEVEL]+no_characters_before_slash_argv1);
                                                                  
   char *extension_location = output_filename+no_characters_before_slash_argv1;
   char *search_extension_location = output_filename+no_characters_before_slash_argv1;
   while (search_extension_location = strstr(search_extension_location + 1, ".")) { // find the last '.'.
      extension_location = search_extension_location;   
   }                 
   
   // This will point to a string that will be inserted into the output filename
   // before the period.
   char *level_name_infix_string = new char[64];
   
   // Fill the level_name_infix_string full of null.
   memset((void *)level_name_infix_string, 0, 64);  
                                                      
   // Only do level name inclusion switch check if the argument is there.
   if (argc == 5) {                                                              
      // If the name switch is set, then fill level_name_infix_string with the level name.
      if (strcmp(argv[MAIN_ARGUMENT_WINLEMM_HYPER_FLAG], "N") == 0) {            
         level_name_infix_string[0] = '_';
               
         char *destination_char = &(level_name_infix_string[1]);  
         const char *source_char = (const char *)new_level_file.runtime_stats.level_name;      
                                                 
         // Copy the level name string across, but only cool characters.
         do {
            if (((*source_char) != '\\')
             && ((*source_char) != '/' )
             && ((*source_char) != ':' )
             && ((*source_char) != '*' )
             && ((*source_char) != '?' )
             && ((*source_char) != '"' )
             && ((*source_char) != '<' )
             && ((*source_char) != '>' )
             && ((*source_char) != '|' )) {
               *destination_char++ = (((*source_char) == ' ') ? '_' : (*source_char)); 
            }
         } while (*source_char++);
      }
   }
                                                               
   // Replace the end of the string with a level name + .lds extension.
   sprintf(extension_location, "%s.lds", level_name_infix_string);       
   
   delete[] level_name_infix_string;
     
   /* ------------------------------------------------------------ */ 
    
   printf("Level Name: '%s', [L = %d], Description: '%s'\n", new_level_file.runtime_stats.level_name, level_name_length, new_level_file.runtime_stats.description);    
   printf("Lems: %d, Save: %d, RR: %d, Time: %dm, Rating: %s\n", new_level_file.runtime_stats.lemmings,
                                                                 new_level_file.runtime_stats.to_be_saved,
                                                                 new_level_file.runtime_stats.release_rate,
                                                                 new_level_file.runtime_stats.time_in_minutes,
                                                                 new_level_file.runtime_stats.rating_description);
   printf("Cl: %2d, Fl: %2d, Ex: %2d, Bl: %2d, Bu: %2d, Ba: %2d, Mi: %2d, Di: %2d\n", new_level_file.runtime_stats.tool_complement[0],
                                                                                      new_level_file.runtime_stats.tool_complement[1],
                                                                                      new_level_file.runtime_stats.tool_complement[2],
                                                                                      new_level_file.runtime_stats.tool_complement[3],
                                                                                      new_level_file.runtime_stats.tool_complement[4],
                                                                                      new_level_file.runtime_stats.tool_complement[5],
                                                                                      new_level_file.runtime_stats.tool_complement[6],
                                                                                      new_level_file.runtime_stats.tool_complement[7]); 
    
   printf("Created %s : Chunk size = %d \n",output_filename, new_level_file.lemmings_level_file_size);
                                     
   FILE *output_file = fopen(output_filename, "wb");                       
      fwrite(&new_level_file, sizeof(LEMMINGS_LEVEL_LDS_FILE_V7), 1, output_file); 
         
      //for (int terrain_placement = new_level_file.no_terrain_objects-1; terrain_placement >= 0; terrain_placement--) { SILLY BACKWARDS MODE
      for (int terrain_placement = 0; terrain_placement < new_level_file.no_terrain_objects; terrain_placement++) {
         fwrite(&level_terrain_object_16s[terrain_placement], sizeof(LEMMINGS_LEVEL_TERRAIN_OBJECT_16), 1, output_file);
      }
   fclose(output_file);

   // Remove the loaded Windows lemmings file from memory.
   delete[] loaded_file;               

   /* ------------------------------------------------------------ */
   
   // Now the level should be rendered to create a preview!

   // Reopen the file that we just wrote!
   FILE *reloading_level_file = fopen(output_filename, "rb");

   // Load the filesize from the level
   u32 level_filesize;
   fread(&level_filesize, 4, 1, reloading_level_file);

   // Rewind the file
   rewind(reloading_level_file);

   // Allocate memory for the level file
   LEMMINGS_LEVEL_LDS_FILE_V7 *loaded_level_file = (LEMMINGS_LEVEL_LDS_FILE_V7 *) new u8[level_filesize];

   // Load the file into memory
   fread(loaded_level_file, level_filesize, 1, reloading_level_file);

   // Close the level file after level has been loaded.
   fclose(reloading_level_file);
   
   //----

   // Construct the texture archive full location string for the loaded level:
   char texture_archive_full_location[256];
   memset(texture_archive_full_location, 0, 256);
                                                                                 
   sprintf(texture_archive_full_location, "texture_archives/%s.LTA", loaded_level_file->texture_archive_using);

   // Load the texture archive.
   FILE *texture_archive_file = fopen(texture_archive_full_location, "rb");

   // Load the filesize from the texture archive
   u32 texture_archive_filesize;
   fread(&texture_archive_filesize, 4, 1, texture_archive_file);

   // Rewind the file
   rewind(texture_archive_file);

   // Allocate memory for the texture archive file
   LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_texture_archive_file = (LEMMINGS_TEXTURE_ARCHIVE_HEADER *) new u8[texture_archive_filesize];  
   
   fread(loaded_texture_archive_file, texture_archive_filesize, 1, texture_archive_file);

   fclose(texture_archive_file);
   
   printf("Rendering level using texture archive '%s' (size %d).\n", texture_archive_full_location, texture_archive_filesize);

   u8 temp_level_data[LEVEL_X_SIZE][LEVEL_Y_SIZE];

   // Render the level into the prototype
   RenderLevel(temp_level_data, loaded_level_file, loaded_texture_archive_file);       
   
   printf("Rendering level preview.\n");

   RenderLevelPreviewIntoPreviewArea(temp_level_data, loaded_level_file->preview_data);

   // Deallocate the texture archive from memory.         
   delete[] loaded_texture_archive_file; 
   
   // Now we have to write this newly edited level file back on top of the original one!

   // Reopen the file that we just read!!!
   FILE *rewriting_level_file = fopen(output_filename, "wb");

      // Write the file to disk
      fwrite(loaded_level_file, level_filesize, 1, rewriting_level_file);

   // Close the level file after level has been loaded.
   fclose(rewriting_level_file);
   
   //----
   
   // Deallocate the level file from memory.         
   delete[] loaded_level_file; 

   // Get rid of the new filename string.
   delete[] output_filename;

   /* ------------------------------------------------------------ */

   return 0;
}
