//
// Key-Value
//
// (c) April 2007
//
// key_value.c
//   An associate array of strings on strings.
//
// By Mathew Carr.
// mattcarr@gmail.com
//
        
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
        
#include "key_value.h"

int noop_printf(const char *s, ...) { 
   return 0;
}
#define printf noop_printf

#define CR 0x0D
#define LF 0x0A

// I like Windows' newlines. I think they're much cooler than Linux' ones
// However, the DS works Linuxy, therefore, I must be able to strip errant newline
// characters from the end of imported strings.
void StripWindowsNewlines(char *s) {
   int len = 0;

   // This could be turned into a single line.
   // But I won't do that, because that would be WEIRD.
   while (s[len] != 0) len++;

   // Don't touch strings of less than two characters.
   if (len < 2) return;       
   
   printf("Stripping string '%s', len = %d.\n", s, len);

   if ((s[len - 2] == CR)
    && (s[len - 1] == LF)) {
      s[len - 2] = LF;
      s[len - 1] = 0;
   }

   return;
}

// This gets rid of a SINGLE newline character.
void StripTrailingNewlines(char *s) {
   int len = 0;

   // This could be turned into a single line.
   // But I won't do that, because that would be WEIRD.
   while (s[len] != 0) len++;

   // Don't touch strings of less than one character.
   if (len < 1) return;                
   
   printf("Stripping string '%s', len = %d.\n", s, len);

   if ((s[len - 1] == LF)) {
      s[len - 1] = 0x00;
   }

   return;
}
// Frees memory associated with a KEY_VALUE_PAIR
// It doesn't touch 'next_node'.
void KeyValueP_Dissolve(KEY_VALUE_PAIR *pair) {
   printf("\"%s\"->\"%s\" is being dissolved!\n", pair->key, pair->value);
   if (pair->key   != NULL) free((void *)pair->key  );
   if (pair->value != NULL) free((void *)pair->value);
   printf("Dissolution complete.\n");
}

// Searches through the KEY_VALUE_PAIRs associated with a CONTROLLER
// and returns KEY_VALUE_CONTROLLER_KEY_FOUND if the key exists and
// KEY_VALUE_CONTROLLER_KEY_NOT_FOUND if it doesn't.
int KeyValueC_KeyExists(KEY_VALUE_CONTROLLER *controller, const char *target_key) {
   KEY_VALUE_PAIR *key_value_pair = controller->header;

   while (key_value_pair != NULL) {
      printf("I'm checking for duplicate key!\n");

      // Check to see if the item under the cursor has the key.
      if (strcmp(key_value_pair->key, target_key) == 0) {
         printf("Key Found.\n");
         return KEY_VALUE_CONTROLLER_KEY_FOUND;
      }

      // Hit the next node.
      key_value_pair = (KEY_VALUE_PAIR *)key_value_pair->next_node;
   }

   printf("Key Not Found.\n");
   return KEY_VALUE_CONTROLLER_KEY_NOT_FOUND;
}

// Searches through the KEY_VALUE_PAIRs associated with a CONTROLLER
// and returns the value for that key if the key exists and
// NULL if it doesn't.
const char *KeyValueC_KeyLookup(KEY_VALUE_CONTROLLER *controller, const char *target_key) {
   KEY_VALUE_PAIR *key_value_pair = controller->header;

   while (key_value_pair != NULL) {
      // Check to see if the item under the cursor has the key.
      if (strcmp(key_value_pair->key, target_key) == 0) {
         return key_value_pair->value;
      }

      // Hit the next node.
      key_value_pair = (KEY_VALUE_PAIR *)key_value_pair->next_node;
   }

   return NULL;
}

// Searches through the KEY_VALUE_PAIRs associated with a CONTROLLER
// and returns the relevant key pair if the key exists and
// NULL if it doesn't.
KEY_VALUE_PAIR *KeyValueC_KeyPairRetrieve(KEY_VALUE_CONTROLLER *controller, const char *target_key) {
   KEY_VALUE_PAIR *key_value_pair = controller->header;

   while (key_value_pair != NULL) {
      // Check to see if the item under the cursor has the key.
      if (strcmp(key_value_pair->key, target_key) == 0) {
         return key_value_pair;
      }

      // Hit the next node.
      key_value_pair = (KEY_VALUE_PAIR *)key_value_pair->next_node;
   }

   return NULL;
}

int KeyValueC_AddPair(KEY_VALUE_CONTROLLER *controller, const char *key, const char *value) {
   printf("Trying to add '%s'->'%s', attempting duplicate key test.\n", key, value);

   // Fail if this is a duplicate key.
   if (KeyValueC_KeyExists(controller, key) == KEY_VALUE_CONTROLLER_KEY_FOUND) {
      return KEY_VALUE_CONTROLLER_ADD_PAIR_FAILURE;
   }

   // This points to the pointer we should manipulate to add the key.
   KEY_VALUE_PAIR **pair_ptr_to_manipulate;

   // If the controller doesn't reference any nodes, then the controllers
   // pointer is the one that should be manipulated.
   if (controller->header == NULL) {
      pair_ptr_to_manipulate = &controller->header;

      printf("I have identified that the controller pointer is NULL.\n");
   } else {
      printf("I'm attempting to hunt for a null pointer in your list.\n");

      // Start by looking at the node that the controller points to.
      KEY_VALUE_PAIR *target = controller->header;

      // Don't stop until you've found a node whose next node is NULL.
      while (target->next_node != NULL) {
         // Retrieve the next target.
         target = (KEY_VALUE_PAIR *)target->next_node;
      }

      // The pointer we should manipulate is the next node pointer
      // that turned out to be null.
      pair_ptr_to_manipulate = (KEY_VALUE_PAIR **)(&(target->next_node));
   }

   // Allocate memory for the new key value pair.
   KEY_VALUE_PAIR *new_pair = (KEY_VALUE_PAIR *)malloc(sizeof(KEY_VALUE_PAIR));

   // Allocate memory for the new key.
   new_pair->key = (const char *)malloc(strlen(key) + 1);
   // Blank it
   memset(new_pair->key, 0, strlen(key) + 1);
   // Copy from the key parameter to the new memory.
   strcpy((char *)new_pair->key, key);

   // Allocate memory for the new value.
   new_pair->value = (const char *)malloc(strlen(value) + 1);      
   // Blank it
   memset(new_pair->value, 0, strlen(value) + 1);
   // Copy from the value parameter to the new memory.
   strcpy((char *)new_pair->value, value);

   // There is no next node.
   new_pair->next_node = NULL;

   // Copy the new pair pointer to the determined destination.
   *pair_ptr_to_manipulate = new_pair;

   return KEY_VALUE_CONTROLLER_ADD_PAIR_SUCCESS;
};

// This function adds the specified pair into the controller.
// If the key exists, it replaces the value for that key with the specified value.
void KeyValueC_AddPairOrReplaceValue(KEY_VALUE_CONTROLLER *controller, const char *key, const char *value) {
   printf("Trying to add '%s'->'%s', attempting duplicate key test.\n", key, value);

   // Retrieve pair and replace if this is a duplicate key.
   if (KeyValueC_KeyExists(controller, key) == KEY_VALUE_CONTROLLER_KEY_FOUND) {
      KEY_VALUE_PAIR *retrieved_pair = KeyValueC_KeyPairRetrieve(controller, key);
      
      // Free the old value.
      free((void *)retrieved_pair->value);
      
      // Allocate memory for the new value.
      retrieved_pair->value = (const char *)malloc(strlen(value) + 1);    
      // Blank it
      memset(retrieved_pair->value, 0, strlen(value) + 1);
      // Copy from the value parameter to the new memory.
      strcpy((char *)retrieved_pair->value, value);
   } else {
      // Just an ordinary pair add.
      KeyValueC_AddPair(controller, key, value);
   }
};

// This populates a KEY_VALUE_CONTROLLER using pairs from the specified FILE. (opened as "r")
int KeyValueC_PopulateFromFile(KEY_VALUE_CONTROLLER *controller, FILE *source_file) {
   // This holds whether we're reading a key or a value.
   int reading_key = 1;

   // This will store the key until it is ready to be set.
   char stored_key[1024];
   // Zero it.
   memset(stored_key, 0, 1024);

   // Keep reeling off strings!
   while (feof(source_file) == 0) {
      char read_string[1024];   
      // Zero it.
      memset(read_string, 0, 1024);

      // Reel string.
      fgets(read_string, 1024, source_file);

      // Skip hashes.
      if (read_string[0] == '#') {
         continue;
      }

      // Get just the string by itself
      StripWindowsNewlines(read_string);
      StripTrailingNewlines(read_string);

      // If we're currently reading the key, then store the key we've just retrieved.
      if (reading_key == 1) {
         // But only if the key is non-blank.
         if (strcmp(read_string, "") == 0) {
            printf("I've read a blank key.\n");
            continue;
         }

         strcpy(stored_key, read_string);
      } else {
         // Add this pair.
         KeyValueC_AddPair(controller, stored_key, read_string);
      }

      // Invert reading key.
      reading_key = (reading_key == 1) ? 0 : 1;
   }

   // If we never got a final value, add a null value.
   if (reading_key == 0) {
      // Add this pair.
      KeyValueC_AddPair(controller, stored_key, "");
   }

   return KEY_VALUE_CONTROLLER_POPULATION_SUCCESS;
}

// This writes the values contained within this KEY_VALUE_CONTROLLER to the specified FILE. (opened as "w")
void KeyValueC_WriteAllToFile(KEY_VALUE_CONTROLLER *controller, FILE *target_file) {
   KEY_VALUE_PAIR *current_target = controller->header;
   KEY_VALUE_PAIR *   next_target = controller->header;

   while (current_target != NULL) {
      // Retrieve the next target.
      next_target = (KEY_VALUE_PAIR *)current_target->next_node;

      // Write this pair to the file.
      fputs(current_target->key,   target_file);
      fputc(CR,   target_file);
      fputc(LF,   target_file);
      fputs(current_target->value, target_file);
      fputc(CR,   target_file);
      fputc(LF,   target_file);

      // Bring the next target into shot.
      current_target = next_target;
   }     
}       

// This writes the key and value pair (if available) for the specified key within this
// KEY_VALUE_CONTROLLER to the specified FILE. (opened as "w")
void KeyValueC_WritePairToFile(KEY_VALUE_CONTROLLER *controller, FILE *target_file, const char *specified_key) {
   const char *found_value = KeyValueC_KeyLookup(controller, specified_key);

   if (found_value != NULL) {
      fputs(specified_key, target_file);
      fputc(CR,            target_file);
      fputc(LF,            target_file);
      fputs(found_value,   target_file);
      fputc(CR,            target_file);
      fputc(LF,            target_file);
   }
}       

// Dissolves, then frees this KEY_VALUE_CONTROLLER.
void KeyValueC_Destroy(KEY_VALUE_CONTROLLER *controller) {
   KEY_VALUE_PAIR *current_target = controller->header;
   KEY_VALUE_PAIR *   next_target = controller->header;
   
   while (current_target != NULL) {
      // Retrieve the next target.   
      next_target = (KEY_VALUE_PAIR *)current_target->next_node;
                                           
      printf("Freeing a key.\n");
      
      // Kill this target by first dissolving the links it holds,
      // then by killing the pair object itself.
      KeyValueP_Dissolve(current_target);
      free(current_target);                
      printf("A pair has been freed.\n");
      
      // Bring the next target into shot.
      current_target = next_target;
   }  
                                     
   printf("Freeing the controller.\n");
   free(controller);
}

// This function creates a new controller using malloc.
KEY_VALUE_CONTROLLER *KeyValueC_Create() {
   KEY_VALUE_CONTROLLER *new_controller = (KEY_VALUE_CONTROLLER *)malloc(sizeof(KEY_VALUE_CONTROLLER));
   
   new_controller->header = NULL;
                     
   return new_controller;  
} 
