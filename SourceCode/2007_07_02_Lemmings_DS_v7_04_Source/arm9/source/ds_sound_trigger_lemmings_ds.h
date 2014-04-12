//
// Lemmings DS
//
// (c) September 2006
//
// ds_sound_trigger_lemmings_ds.h
//   Defines and malarky for lemmings ds specifically.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#ifndef __DS_SOUND_TRIGGER_LEMMINGS_DS_H__
#define __DS_SOUND_TRIGGER_LEMMINGS_DS_H__
  
#ifdef __cplusplus
extern "C" {
#endif                     

#include "ds_sound_trigger_system.h"
#include <nds.h>

#define MUSIC_PREFERENCE_SILENCE         0x00
#define MUSIC_PREFERENCE_MUSIC           0x01
#define MUSIC_PREFERENCE_SOUND           0x02
#define MUSIC_PREFERENCE_MUSIC_AND_SOUND (MUSIC_PREFERENCE_MUSIC | MUSIC_PREFERENCE_SOUND)
extern u32 music_preference;

#define NO_TOOLS 8
extern const int ds_sound_trigger_lemmings_ds_tool_select_relative_frequency[NO_TOOLS];

extern const int ds_sound_trigger_lemmings_ds_release_rate_change_relative_frequency[72];

enum {
   LEMMINGS_DS_SOUND_TYPE_NULL,
   LEMMINGS_DS_SOUND_TYPE_LETS_GO,
   LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN,
   LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,
   LEMMINGS_DS_SOUND_TYPE_OH_NO,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_SAVE,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE,
   LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE,
   LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE_HI,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_SQUISH,
   LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
   LEMMINGS_DS_SOUND_TYPE_BUILDER_CHINK,          
   LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_DROWN,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,
   LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER,
};

extern const DS_SOUND_TRIGGER_REFERENCE_STRUCT ds_sound_trigger_lemmings_ds_sfx_reference[];

// This function applies a standard request for each lemmings ds sound.
// Irrelevant values (frequency for 'oh no', etc) are ignored.
void DSSoundTriggerLemmingsDS_RequestStandardSound(int sound_type,
                                                   int relative_frequency,
                                                   int sound_location,
                                                   int camera_x_inset);

#ifdef __cplusplus
}
#endif
                   
#endif // __DS_SOUND_TRIGGER_LEMMINGS_DS_H__
