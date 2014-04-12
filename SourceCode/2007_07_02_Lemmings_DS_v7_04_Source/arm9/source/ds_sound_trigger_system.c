//
// Lemmings DS
//
// (c) August 2006
//
// ds_sound_trigger_system.c
//   Functions for manipulating the DS Sound Trigger System sound request
//   pile.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//

#include "ds_sound_trigger_system.h"

int ds_sound_trigger_number_of_requests = 0;

int ds_sound_next_channel_to_use = 15;

DS_SOUND_TRIGGER_REQUEST_STRUCT ds_sound_trigger_request_pile[NO_MAX_DS_SOUND_TRIGGER_REQUESTS];

void DSSoundTrigger_ClearPile() {
   ds_sound_trigger_number_of_requests = 0;
}

// add a request to the back of the array, INTERNAL ONLY
void DSSoundTrigger_AddRequestInternal(int sound_type, int pan, int relative_frequency, int volume) {
   if (ds_sound_trigger_number_of_requests == NO_MAX_DS_SOUND_TRIGGER_REQUESTS-1) return;

   ds_sound_trigger_request_pile[ds_sound_trigger_number_of_requests].sound_type         = sound_type;  
   ds_sound_trigger_request_pile[ds_sound_trigger_number_of_requests].pan                = pan;
   ds_sound_trigger_request_pile[ds_sound_trigger_number_of_requests].relative_frequency = relative_frequency;
   ds_sound_trigger_request_pile[ds_sound_trigger_number_of_requests].volume             = volume;
   
   ds_sound_trigger_number_of_requests++;
}

// add a request to the back of the array, check for dupes, INTERNAL ONLY
void DSSoundTrigger_AddRequestNoDupeInternal(int sound_type, int pan, int relative_frequency, int volume) {
   if (ds_sound_trigger_number_of_requests == NO_MAX_DS_SOUND_TRIGGER_REQUESTS-1) return;

   for (u32 r = 0; r < ds_sound_trigger_number_of_requests; r++) {
      if ((ds_sound_trigger_request_pile[r].sound_type         == sound_type        )
       && (ds_sound_trigger_request_pile[r].pan                == pan               )
       && (ds_sound_trigger_request_pile[r].relative_frequency == relative_frequency)
       && (ds_sound_trigger_request_pile[r].volume             == volume            )) {
         return;
      } else {
      }
   }       
   DSSoundTrigger_AddRequestInternal(sound_type,
                                            pan,
                             relative_frequency,
                                         volume);
}

// Make a request for sound based on the sound's location in world space.
// This function allows you to specify the location of the camera, so your
// code doesn't have to do it itself.
void DSSoundTrigger_AddRequestBasedOnLocationWCamera(int sound_type,
                                                     int relative_frequency,
                                                     int volume,
                                                     int sound_location,   // The location in world space of the sound effect
                                                     int camera_x_inset) { // The location in world space displayed at the far left of the screen.
   if (ds_sound_trigger_number_of_requests == NO_MAX_DS_SOUND_TRIGGER_REQUESTS-1) return;
   
   int location_in_camera = sound_location - camera_x_inset;
   int final_pan_value;

   // This makes sure the sound effect has one of the specific allowed panning locations
   // (this avoids dupe sounds that are right next to each other)
   if (location_in_camera < 32) {
      final_pan_value = 0;
   } else
   if (location_in_camera < 96) {
      final_pan_value = 32;
   } else
   if (location_in_camera < 160) {
      final_pan_value = 64;
   } else
   if (location_in_camera < 244) {
      final_pan_value = 95;
   } else {
      final_pan_value = 127;
   }            

   DSSoundTrigger_AddRequestNoDupeInternal(sound_type,
                                      final_pan_value,
                                   relative_frequency,
                                               volume);
}

// Make a request for sound based on the sound's location in world space.
// This function doesn't allow you to specify the location of the camera
void DSSoundTrigger_AddRequestBasedOnLocation(int sound_type,
                                              int relative_frequency,
                                              int volume,
                                              int sound_location) {
   DSSoundTrigger_AddRequestBasedOnLocationWCamera(sound_type,
                                                   relative_frequency,
                                                   volume,
                                                   sound_location,
                                                   0);
}
                                        
// Make a request for sound based, centred.
void DSSoundTrigger_AddRequestCentre(int sound_type,
                                     int relative_frequency,
                                     int volume) {
   DSSoundTrigger_AddRequestBasedOnLocationWCamera(sound_type,
                                                    relative_frequency,
                                                  volume,
                                                  127,
                                                   0);
}

// This will execute all requests in the queue.
void DSSoundTrigger_ExecuteRequestQueue() {
   DS_SOUND_TRIGGER_REQUEST_STRUCT   *ds_sound_top_request;
   DS_SOUND_TRIGGER_REFERENCE_STRUCT *ds_sound_reference_item;

   for (; ds_sound_trigger_number_of_requests > 0;) {
      ds_sound_top_request    = DSSoundTrigger_GetRequest();
      ds_sound_reference_item = &(ds_sound_trigger_reference_struct_array[ds_sound_top_request->sound_type]);

      int final_frequency = (ds_sound_reference_item->natural_frequency
                           * ds_sound_top_request->  relative_frequency) >> 14;
      // now we play the sound.
      DSSoundTrigger_ExternalCall_CommandPlayOneShotSample(
                               ds_sound_next_channel_to_use,
                                            final_frequency,
                       ds_sound_reference_item->sample_data,
                       ds_sound_reference_item->sample_length,
                       ds_sound_top_request->volume,
                       ds_sound_reference_item->sample_format,       
                       ds_sound_top_request->pan,
                                                      false);

      --ds_sound_next_channel_to_use;

      if (ds_sound_next_channel_to_use < DS_SOUND_NEARMOST_CHANNEL_USED) {
         ds_sound_next_channel_to_use = 15;
      }

      DSSoundTrigger_DitchThatRequest();
   }
}

// return a pointer to the top request in the array
DS_SOUND_TRIGGER_REQUEST_STRUCT *DSSoundTrigger_GetRequest() {
   return &ds_sound_trigger_request_pile[0]; // I'm sure this should be zero.
}

// ditch the top request in the requests array.
void DSSoundTrigger_DitchThatRequest() {
   for (int r = 0; r < ds_sound_trigger_number_of_requests; r++) {
      ds_sound_trigger_request_pile[r] = ds_sound_trigger_request_pile[r+1];
   }
   ds_sound_trigger_number_of_requests--;
}

DS_SOUND_TRIGGER_REFERENCE_STRUCT *ds_sound_trigger_reference_struct_array = NULL;

void (*DSSoundTrigger_ExternalCall_CommandPlayOneShotSample)(int, int, const void*, int, int, int, int, bool) = NULL;

void DSSoundTriggerSetup(const DS_SOUND_TRIGGER_REFERENCE_STRUCT *reference_struct_array,
                                void (*playback_command)(int, int, const void*, int, int, int, int, bool)) {
   ds_sound_trigger_reference_struct_array = reference_struct_array;
   DSSoundTrigger_ExternalCall_CommandPlayOneShotSample = playback_command;
}

