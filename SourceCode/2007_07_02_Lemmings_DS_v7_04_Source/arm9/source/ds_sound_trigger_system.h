//
// Lemmings DS
//
// (c) August 2006
//
// ds_sound_trigger_system.h
//   Functions for manipulating the DS Sound Trigger System sound request
//   pile.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//

#ifndef __DS_SOUND_TRIGGER_SYSTEM_H__
#define __DS_SOUND_TRIGGER_SYSTEM_H__
  
#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>
#include <stdlib.h>

#define DS_SOUND_NEARMOST_CHANNEL_USED 10

typedef struct tagDS_SOUND_TRIGGER_REQUEST_STRUCT {
   int sound_type;         // The type of sound to be played (the sample)
   int pan;                // Panning value from 0 - 127 (64 middle)
#define DS_SOUND_USE_NATURAL_FREQUENCY (1 << 14)
   int relative_frequency; // 2.14 Fixed point multiplier for the frequency of the sample.
   int volume;             // Vol: 0 - 127
} DS_SOUND_TRIGGER_REQUEST_STRUCT;

extern int ds_sound_trigger_number_of_requests;

#define NO_MAX_DS_SOUND_TRIGGER_REQUESTS 64

extern DS_SOUND_TRIGGER_REQUEST_STRUCT ds_sound_trigger_request_pile[NO_MAX_DS_SOUND_TRIGGER_REQUESTS];

void DSSoundTrigger_ClearPile();         
DS_SOUND_TRIGGER_REQUEST_STRUCT *DSSoundTrigger_GetRequest();
void DSSoundTrigger_DitchThatRequest();

void DSSoundTrigger_AddRequestBasedOnLocationWCamera(int sound_type,
                                                     int relative_frequency,
                                                     int volume,
                                                     int sound_location, // The location in world space of the sound effect
                                                     int camera_x_inset); // The location in world space displayed at the far left of the screen.

void DSSoundTrigger_AddRequestBasedOnLocation(int sound_type,
                                              int relative_frequency,
                                              int volume,
                                              int sound_location); // The location in world space of the sound effect

void DSSoundTrigger_AddRequestCentre(int sound_type,
                                     int relative_frequency,
                                     int volume);

void DSSoundTrigger_ExecuteRequestQueue();

typedef struct tagDS_SOUND_TRIGGER_REFERENCE_STRUCT {
   const void *sample_data; 
   const int sample_length;
   const int sample_format;
   const int natural_frequency;
} DS_SOUND_TRIGGER_REFERENCE_STRUCT;

// This pointer should point to an array of reference structs
// which in turn tell the program what samples to play for each enumerated trigger
// in the game-specific header file.
extern DS_SOUND_TRIGGER_REFERENCE_STRUCT *ds_sound_trigger_reference_struct_array;

// This fucntion pointer must point to a function with the correct prototype.
// It is used to call arm7 to play the specified sample.
//extern void (*DSSoundTrigger_ExternalCall_CommandPlayOneShotSample)(int channel, int frequency, const void* data, int length, int volume, int format, bool loop);
extern void (*DSSoundTrigger_ExternalCall_CommandPlayOneShotSample)(int, int, const void*, int, int, int, int, bool);

void DSSoundTriggerSetup(const DS_SOUND_TRIGGER_REFERENCE_STRUCT *reference_struct_array,
                                void (*playback_command)(int, int, const void*, int, int, int, int, bool));

#ifdef __cplusplus
}
#endif
                   
#endif // __DS_SOUND_TRIGGER_SYSTEM_H__

