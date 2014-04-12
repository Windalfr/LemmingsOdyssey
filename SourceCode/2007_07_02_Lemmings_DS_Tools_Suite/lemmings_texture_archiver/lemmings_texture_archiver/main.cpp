//
// Lemmings DS - lemmings_texture_archiver
//
// (c) March 2007
//
// main.cpp
//   lemmings_texture_archiver application converts a directory of
//   consecutively named 16 or 256 colour paletted png files and
//   outputs a 'lemmings_texture_archive' .LTA file containing all
//   of the textures ready to be accessed as a 'USING' by Lemmings
//   DS v4.
//
// By Mathew Carr. 
// mattcarr@gmail.com
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <string>

#include "lemmings_texture_archive.h"

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

// Identify arguments and determine ideal number of arguments to expect.
enum MAIN_ARGUMENT {
   MAIN_ARGUMENT_APPLICATION_LOCATION,
   MAIN_ARGUMENT_SOURCE_DIRECTORY,
   MAIN_ARGUMENT_OUTPUT_FILE_NAME,  
   MAIN_ARGUMENT_FORCE_FLAG,  
};

const int IDEAL_NO_ARGS_1 = MAIN_ARGUMENT_FORCE_FLAG;
const int IDEAL_NO_ARGS_2 = MAIN_ARGUMENT_FORCE_FLAG + 1;

// Entry point.
int main(int argc, char *argv[]) {                                        
      printf("------------------------------------------\n"
             "------------------------------------------\n"
             " lemmings_texture_archiver by Mathew Carr \n"
             "------------------------------------------\n"
             "---------[ http://www.mrdictionary.net ]--\n"
             "------------------------------------------\n"
             "\n");
      
   // Error if you don't give a single filename level to convert.
   // If you supply three arguments, third must be level name switch.
   if ((argc != IDEAL_NO_ARGS_1)
    && (argc != IDEAL_NO_ARGS_2)) {
      printf("Bad arguments.\n"
              "Expected lemmings_texture_archiver SOURCE-DIRECTORY ARCHIVE_OUTPUT_NAME.LTA [F].\n");
      return 1;
   }
   
   // If the output file exists, error.
   if (file_exists(argv[MAIN_ARGUMENT_OUTPUT_FILE_NAME])) {
      printf("Output file exists: Please delete it first.\n");
      return 1;                         
   }                          
   
   bool force_16_flag = false;
   
   // Set the flag if there's a last argument, and it's F
   if (argc == IDEAL_NO_ARGS_2) {
      if (strcmp(argv[MAIN_ARGUMENT_FORCE_FLAG], "F") == 0) {
         printf("Force flag set: 256 colour textures will be truncated.\n");
         
         force_16_flag = true;
      }
   }                         
      
   // Set up a vector for storing texture object pointers.
   // We're going to populate this as the images are loaded,
   // and then splurge them all out when the loading is
   // complete.
   std::vector<void *> texture_archive_texture_object_16_pointers;
   std::vector<void *> texture_archive_texture_object_256_pointers;
           // (LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *) 
           
   // Count the number of texture objects.
   int no_texture_objects = 0;
   
   // We don't have a header instance yet, so we need to prepare a temporay location
   // in which to store the extracted palette.
   u16 extracted_palette[256];
           
   // Loop until told to quit (condition: unloadable png marking end of set.)
   do {    
      // This decoder will decode the incoming PNG images.
      LodePNG::Decoder decoder;                         
              
      // This stores the PNG file in memory while it is being worked on.
      std::vector<unsigned char> buffer;   
      
      // This stores the PNG decompressed image in memory while it is being worked on.
      std::vector<unsigned char> image;                          
      
      // This holds the filename of the file we're going to load
      char incoming_texture_image_filename[16384];
      sprintf(incoming_texture_image_filename, "%s%d.png",
                                                    argv[MAIN_ARGUMENT_SOURCE_DIRECTORY],
                                                    no_texture_objects); 
                                                                                    
      printf("%s... ", incoming_texture_image_filename);    
      
      // Convert image filename into a std::string using this constructor.
      std::string string_incoming_texture_image_filename = std::string(incoming_texture_image_filename);
                     
      // Load in the terrain image PNG.
      LodePNG::loadFile(buffer, string_incoming_texture_image_filename);  
      
      // Decode the image, retaining the image pixel index values.
      decoder.decodeGeneric(image, buffer);
      
      // Check for error here. If there's an error, we're done.
      if (decoder.hasError()) {
         if (decoder.getError() == 48) {
            printf("\nHold it! LodePNG error 48.\nDoes '%s' exist?\n", incoming_texture_image_filename); 
         } else {
            printf("\nHold it! LodePNG error != 48.\nMalformed PNG: '%s'?\n", incoming_texture_image_filename); 
         }
         break;
      }
      
      // Get image information.
      size_t width  = decoder.getWidth();
      size_t height = decoder.getHeight();    
                                                          
      LodePNG::Decoder::Info image_info_struct = decoder.getInfo();
         
      // Get the palette size.
      unsigned long paletteSize = image_info_struct.paletteSize;
      
      printf("has %d colours.\n", paletteSize);
                                      
      // This eventuality is very, very bad.
      if ((paletteSize > 256) || (paletteSize == 0)) {                             
         printf("Detected truecolour, dying!\n");
         
         // Destroy ALL objects in the vector, and terminate.
         for (int t = 0; t < texture_archive_texture_object_16_pointers.size(); t++) {
            printf("Killing texture object 16 %d.\n", t);
            delete[] ((unsigned char *)(texture_archive_texture_object_16_pointers[t]));
         }
         for (int t = 0; t < texture_archive_texture_object_256_pointers.size(); t++) {
            printf("Killing texture object 256 %d.\n", t);
            delete[] ((unsigned char *)(texture_archive_texture_object_256_pointers[t]));
         }
         
         return 0;
      }
      
      int bits_to_read_per_pixel; 
      int bits_to_store_per_pixel;                   
      
      // What vector this image will be placed into is dependent upon its colour depth.
      std::vector<void *> *destination_vector;
      
      // Use 8 bits to store 256 colour images.
      if (paletteSize <= 256) {
         bits_to_read_per_pixel  = 8; 
         bits_to_store_per_pixel = 8; 
         destination_vector = &texture_archive_texture_object_256_pointers;
      }  
      
      // Use 4 bits to store 16 colour images.
      if (paletteSize <=  16) {
         bits_to_read_per_pixel  = 4;   
         bits_to_store_per_pixel = 4;     
         destination_vector = &texture_archive_texture_object_16_pointers;
      }
                                                    
      // Override the storing attributes if the flag is set.
      if (force_16_flag) {
         bits_to_store_per_pixel = 4;     
         destination_vector = &texture_archive_texture_object_16_pointers;
      }
                                   
      // Determine the amount of memory required to store a LEMMINGS_TEXTURE_ARCHIVE_TEXTURE
      // holding this image.       
      
      // This is measured in bits, taken to the next byte, then divided again to get the number of bytes.                                  
      int memory_size_required       = to_next((bits_to_store_per_pixel * width * height), 8) / 8;
      
      // Increase this to the next word.
      int memory_size_required_total = to_next(memory_size_required + sizeof(LEMMINGS_TEXTURE_ARCHIVE_TEXTURE), 4);
      printf("Reserved %6d bytes (%2d + %6d): %3d colour %4d by %4d texture.\n", memory_size_required_total, sizeof(LEMMINGS_TEXTURE_ARCHIVE_TEXTURE), memory_size_required, paletteSize, width, height);
                                                                          
      // Reserve memory for the next texture object.
      LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *new_texture_object =
           (LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(new unsigned char[memory_size_required_total]);   
           
      // Blank the texture object.
      memset((void *)new_texture_object, 0, memory_size_required_total);  
                                  
      // At this pointer to the vector of pointers.                                                               
      destination_vector->push_back((void *)(new_texture_object));                                           
                                 
      // Now all we have to do, is set the relevant fields in new_texture_object,
      // then move onto the next image.
      new_texture_object->texture_chunk_size = memory_size_required_total;
      
      new_texture_object->xs = width;  
      new_texture_object->ys = height; 
      
      sprintf(new_texture_object->name, "Conv % 3d", no_texture_objects);
      
      // Rob the bytes from the buffer:
      if (bits_to_read_per_pixel == 4) {   
         for (int output_byte = 0; output_byte < ((image.size() + 1)/2); output_byte++) {
            int lower_nibble, higher_nibble; 
            
            // Get the lower and higher nibbles
            lower_nibble = image[output_byte * 2];
            
            // Don't forget that reading off the edge of the image vector would be BAD
            // If 'the nibble number we are reading' is valid:
            if (((output_byte * 2) + 1) < image.size()) {
               higher_nibble = image[(output_byte * 2) + 1];
            } else {
               // It's unused, set it to zero..
               higher_nibble = 0;    
            }
             
            // Combine the next two source image bytes into a byte as two nibbles.
            new_texture_object->data[output_byte] = lower_nibble | (higher_nibble << 4);
         }  
      } else {   
         if (bits_to_store_per_pixel == 8) {
            for (int output_byte = 0; output_byte < image.size(); output_byte++) {
               // Copy the byte from the image vector into the data array.
               new_texture_object->data[output_byte] = image[output_byte];
            }  
         } else
         if (bits_to_store_per_pixel == 4) {
            for (int output_byte = 0; output_byte < ((image.size() + 1)/2); output_byte++) {
               int lower_nibble, higher_nibble; 
               
               // Get the lower and higher nibbles
               lower_nibble = 0xF & image[output_byte * 2];
               
               // Don't forget that reading off the edge of the image vector would be BAD
               // If 'the nibble number we are reading' is valid:
               if (((output_byte * 2) + 1) < image.size()) {
                  higher_nibble = 0xF & image[(output_byte * 2) + 1];
               } else {
                  // It's unused, set it to zero..
                  higher_nibble = 0;    
               }
                
               // Combine the next two source image bytes into a byte as two nibbles.
               new_texture_object->data[output_byte] = lower_nibble | (higher_nibble << 4);
            }  
         } 
      }
      
      // Extract the palette from the first terrain image:
      if (no_texture_objects == 0) {         
         for (int palette_entry = 0; palette_entry < paletteSize; palette_entry++) {
            int i_r, i_g, i_b; // These are the incoming palette colours.
            
            int nds_r, nds_g, nds_b; // These are the incoming colours reduced to 15-bit.
            
            i_r = image_info_struct.palette[0 + palette_entry * 4];
            i_g = image_info_struct.palette[1 + palette_entry * 4];
            i_b = image_info_struct.palette[2 + palette_entry * 4]; 
            
            // Reduce the colours to 15 bit.
            nds_r = (int)((31.9f * ((float)(i_r))) / 255.0f);
            nds_g = (int)((31.9f * ((float)(i_g))) / 255.0f);
            nds_b = (int)((31.9f * ((float)(i_b))) / 255.0f);      
            
            printf("Extracted colour %3d: R: %2d   G: %2d   B: %2d\n", palette_entry, nds_r, nds_g, nds_b);
               
// This macro will convert 5 bit colour channel values into a composite 15 bit NDS colour with enabled alpha.
#define RGB15A(r,g,b) ((1<<15) | ((b)<<10) | ((g)<<5) | (r))

            u16 nds_colour = RGB15A(nds_r, nds_g, nds_b);
                        
            // Fill in the palette of the level using the colours from the first terrain image.
            extracted_palette[palette_entry] = nds_colour;
         }

// These are taken from Lemmings DS, and need to be specified here.
#define SPECIAL_COLOUR_BUILDER_BRICK   253
#define SPECIAL_COLOUR_MAP_LEMMING     254
#define SPECIAL_COLOUR_MAP_BORDER      255
         extracted_palette[SPECIAL_COLOUR_BUILDER_BRICK] = RGB15A(31, 31,  6);
         extracted_palette[SPECIAL_COLOUR_MAP_LEMMING]   = RGB15A( 5, 31,  5);
         extracted_palette[SPECIAL_COLOUR_MAP_BORDER]    = RGB15A(31, 31, 31);
      }                   
      
      // Increase successfully loaded texture count.
      no_texture_objects++;
      
      // Matt, continue moving terra loading code into here, so we can process images.
      // if we can't load an image, terminate this do loop and then fill in the header information.      
   } while (true);
   
   // This is measured in bits, taken to the next byte, then divided again to get the number of bytes.                                  
   int memory_size_required_for_header_object_offset_array = no_texture_objects * sizeof(u32);
      
   // Increase this to the next word.
   int memory_size_required_for_header_total = to_next(memory_size_required_for_header_object_offset_array + sizeof(LEMMINGS_TEXTURE_ARCHIVE_HEADER), 4);
   printf("Reserved %4d bytes (%2d + %4d): Header must direct to %d objects.\n",
                                                                memory_size_required_for_header_total,
                                                                sizeof(LEMMINGS_TEXTURE_ARCHIVE_HEADER),
                                                                memory_size_required_for_header_object_offset_array,
                                                                no_texture_objects);
      
                                              
   // Store general information in the header.
   LEMMINGS_TEXTURE_ARCHIVE_HEADER *generated_header = (LEMMINGS_TEXTURE_ARCHIVE_HEADER *)new unsigned char[memory_size_required_for_header_total];
   
   // Blank the header.
   memset((void *)generated_header, 0, memory_size_required_for_header_total);  

   // Populate header.
   generated_header->no_texture_16s  = texture_archive_texture_object_16_pointers.size();
   generated_header->no_texture_256s = texture_archive_texture_object_256_pointers.size();
   #define LEMMINGS_LEVEL_VERSION 5
   generated_header->version_number  = LEMMINGS_LEVEL_VERSION;

   // Generate offsets and file size.
   int cumulative_offset = memory_size_required_for_header_total;           
   int texture_position  = 0;
   
   for (int t = 0; t < texture_archive_texture_object_16_pointers.size(); texture_position++, t++) {
      // Store the current cumulative offset as the offset to this texture.
      generated_header->texture_object_offsets[texture_position] = cumulative_offset; 
      
      printf(" 16 Texture %3d at %6d bytes. (My length is %6d)\n", t, cumulative_offset, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_16_pointers[t]))->texture_chunk_size);
            
      // Advance the offset by the length of this texture chunk. 
      cumulative_offset += ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_16_pointers[t]))->texture_chunk_size; 
   }        
   
   for (int t = 0; t < texture_archive_texture_object_256_pointers.size(); texture_position++, t++) {
      // Store the current cumulative offset as the offset to this texture.
      generated_header->texture_object_offsets[texture_position] = cumulative_offset; 
      
      printf("256 Texture %3d at %6d bytes. (My length is %6d)\n", t, cumulative_offset, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_256_pointers[t]))->texture_chunk_size);
            
      // Advance the offset by the length of this texture chunk. 
      cumulative_offset += ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_256_pointers[t]))->texture_chunk_size; 
   }                                     
     
   printf("\nFinal length: %6d bytes.\n", cumulative_offset);
   
   // Copy palette
   for (int palette_entry = 0; palette_entry < 256; palette_entry++) {
      generated_header->ideal_palette[palette_entry] = extracted_palette[palette_entry]; 
   }
   
   // The cumulative offset will contain the length of the final file!
   generated_header->texture_archive_file_size = cumulative_offset;
                                                                                       
   // Open output file.
   FILE *output_file = fopen(argv[MAIN_ARGUMENT_OUTPUT_FILE_NAME], "wb");
   
      // Write header, then textures.
      fwrite(generated_header, 1, memory_size_required_for_header_total, output_file);      
         
         for (int t = 0; t < texture_archive_texture_object_16_pointers.size(); t++) {
            // Write texture.
            fwrite(texture_archive_texture_object_16_pointers[t], 1, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_16_pointers[t]))->texture_chunk_size, output_file);               
         }               
         for (int t = 0; t < texture_archive_texture_object_256_pointers.size(); t++) {
            // Write texture.
            fwrite(texture_archive_texture_object_256_pointers[t], 1, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_256_pointers[t]))->texture_chunk_size, output_file);               
         }                                                       
   
   // Close output file
   fclose(output_file);    
   
   // Epilogue:                         
                                      
   // Construct a final string.
   char final_output_result_string[2048];
   
   sprintf(final_output_result_string, "Created %s (%d bytes):\n%d texture 16s, %d texture 256s. Total: %d.\n",
                                       argv[MAIN_ARGUMENT_OUTPUT_FILE_NAME],
                                       generated_header->texture_archive_file_size,
                                       generated_header->no_texture_16s,
                                       generated_header->no_texture_256s,
                                       generated_header->no_texture_16s + generated_header->no_texture_256s);  
   
   // Destroy ALL objects in the vectors, and terminate.
   for (int t = 0; t < texture_archive_texture_object_16_pointers.size(); t++) {
      printf("Killing texture object  16 %3d. (%5d x %5d)\n", t, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_16_pointers[t]))->xs,
                                                                 ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_16_pointers[t]))->ys);
      delete[] ((unsigned char *)(texture_archive_texture_object_16_pointers[t]));
   }            
   for (int t = 0; t < texture_archive_texture_object_256_pointers.size(); t++) {
      printf("Killing texture object 256 %3d. (%5d x %5d)\n", t, ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_256_pointers[t]))->xs,
                                                                 ((LEMMINGS_TEXTURE_ARCHIVE_TEXTURE *)(texture_archive_texture_object_256_pointers[t]))->ys);
      delete[] ((unsigned char *)(texture_archive_texture_object_256_pointers[t]));
   }            
   
   // Destroy the header array.
   printf("Killing header.\n");
   delete[] ((unsigned char *)(generated_header));
         
   printf(final_output_result_string);      
         
   return 0;
                     
      // At this point, I would like to thank Lode Vandevenne
      // without whom this application would not have been possible.
      // Kudos to you! LodePNG has performed admirably once again!      
}
