//
// Key-Value
//
// (c) April 2007
//
// key_value.h
//   Structures and headers for an associate array of strings on strings.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#ifndef __KEY_VALUE_H__
#define __KEY_VALUE_H__

#ifdef __cplusplus
extern "C" {
#endif
        
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void StripWindowsNewlines(char *s);
void StripTrailingNewlines(char *s);

// This structure marries a 'key' to a 'value'.
typedef struct tagKEY_VALUE_PAIR {
   const char *key;   // A string 'key'
   const char *value; // A string 'value' for this key.
   
   void *next_node;   // The next node.
                      // NULL if invalid.
   // These things are always malloced.
} KEY_VALUE_PAIR;

// This structure masterminds KEY_VALUE_PAIRs
typedef struct tagKEY_VALUE_CONTROLLER {
   KEY_VALUE_PAIR *header; // Header to singly linked list of key value pairs.
                           // NULL if invalid.     
} KEY_VALUE_CONTROLLER;
                                             
#define KEY_VALUE_CONTROLLER_KEY_FOUND     1
#define KEY_VALUE_CONTROLLER_KEY_NOT_FOUND 0

#define KEY_VALUE_CONTROLLER_ADD_PAIR_SUCCESS 1
#define KEY_VALUE_CONTROLLER_ADD_PAIR_FAILURE 0   

#define KEY_VALUE_CONTROLLER_POPULATION_SUCCESS 1
#define KEY_VALUE_CONTROLLER_POPULATION_FAILURE 0

            int KeyValueC_KeyExists(            KEY_VALUE_CONTROLLER *controller, const char *target_key);
    const char *KeyValueC_KeyLookup(            KEY_VALUE_CONTROLLER *controller, const char *target_key);
KEY_VALUE_PAIR *KeyValueC_KeyPairRetrieve(      KEY_VALUE_CONTROLLER *controller, const char *target_key);
            int KeyValueC_AddPair(              KEY_VALUE_CONTROLLER *controller, const char *key, const char *value);
           void KeyValueC_AddPairOrReplaceValue(KEY_VALUE_CONTROLLER *controller, const char *key, const char *value);
            int KeyValueC_PopulateFromFile(     KEY_VALUE_CONTROLLER *controller, FILE *source_file);
           void KeyValueC_WriteAllToFile(       KEY_VALUE_CONTROLLER *controller, FILE *target_file);
           void KeyValueC_WritePairToFile(      KEY_VALUE_CONTROLLER *controller, FILE *target_file, const char *specified_key);
           void KeyValueC_Destroy(              KEY_VALUE_CONTROLLER *controller);

KEY_VALUE_CONTROLLER *KeyValueC_Create();
             
#ifdef __cplusplus
}
#endif

#endif // __KEY_VALUE_H__
