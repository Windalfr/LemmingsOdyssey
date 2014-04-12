//
// Lemmings DS
//
// (c) September 2006
//
// ds_sound_trigger_lemmings_ds.c
//   Defines and malarky for lemmings ds specifically.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#include "ds_sound_trigger_system.h"
#include "ds_sound_trigger_lemmings_ds.h"
#include "../build/lem_sfx_lets_go_raw.h"
#include "../build/lem_sfx_trapdoor_open_raw.h"
#include "../build/lem_sfx_tool_select_raw.h"
#include "../build/lem_sfx_lemming_select_raw.h"
#include "../build/lem_sfx_oh_no_raw.h"
#include "../build/lem_sfx_lemming_save_raw.h"
#include "../build/lem_sfx_lemming_explode_raw.h"
#include "../build/lem_sfx_release_rate_change_raw.h"
#include "../build/lem_sfx_release_rate_change_hi_raw.h"
#include "../build/lem_sfx_lemming_squish_raw.h"
#include "../build/lem_sfx_steel_hit_raw.h"
#include "../build/lem_sfx_lemming_scream_raw.h"
#include "../build/lem_sfx_builder_chink_raw.h"
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

// Just in case you're wondering, 17358 >> 14 is a semitone.

const int ds_sound_trigger_lemmings_ds_tool_select_relative_frequency[NO_TOOLS] = {
           /*  0 */                            DS_SOUND_USE_NATURAL_FREQUENCY,
           /*  1 */                           (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /*  2 */                          ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /*  3 */                         (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /*  4 */                        ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /*  5 */                       (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /*  6 */                      ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /*  7 */                     (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
};

// The release rate frequencies are repeated every three values.
// After 72 values, change to the different sample rate 'bink'.

const int ds_sound_trigger_lemmings_ds_release_rate_change_relative_frequency[72] = {
           /*  0 */                            DS_SOUND_USE_NATURAL_FREQUENCY,
           /*  1 */                            DS_SOUND_USE_NATURAL_FREQUENCY,
           /*  2 */                            DS_SOUND_USE_NATURAL_FREQUENCY,
           /*  3 */                           (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /*  4 */                           (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /*  5 */                           (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /*  6 */                          ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /*  7 */                          ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /*  8 */                          ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /*  9 */                         (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 10 */                         (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 11 */                         (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 12 */                        ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 13 */                        ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 14 */                        ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 15 */                       (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 16 */                       (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 17 */                       (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 18 */                      ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 19 */                      ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 20 */                      ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 21 */                     (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 22 */                     (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 23 */                     (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 24 */                    ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 25 */                    ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 26 */                    ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 27 */                   (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 28 */                   (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 29 */                   (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 30 */                  ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 31 */                  ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 32 */                  ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 33 */                 (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 34 */                 (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 35 */                 (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 36 */          2 *               DS_SOUND_USE_NATURAL_FREQUENCY,
           /* 37 */          2 *               DS_SOUND_USE_NATURAL_FREQUENCY,
           /* 38 */          2 *               DS_SOUND_USE_NATURAL_FREQUENCY,
           /* 39 */          2 *              (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /* 40 */          2 *              (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /* 41 */          2 *              (DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14),
           /* 42 */          2 *             ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /* 43 */          2 *             ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /* 44 */          2 *             ((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14),
           /* 45 */          2 *            (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 46 */          2 *            (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 47 */          2 *            (((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 48 */          2 *           ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 49 */          2 *           ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 50 */          2 *           ((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 51 */          2 *          (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 52 */          2 *          (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 53 */          2 *          (((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 54 */          2 *         ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 55 */          2 *         ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 56 */          2 *         ((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 57 */          2 *        (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 58 */          2 *        (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 59 */          2 *        (((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 60 */          2 *       ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 61 */          2 *       ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 62 */          2 *       ((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 63 */          2 *      (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 64 */          2 *      (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 65 */          2 *      (((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 66 */          2 *     ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 67 */          2 *     ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 68 */          2 *     ((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 69 */          2 *    (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 70 */          2 *    (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
           /* 71 */          2 *    (((((((((((DS_SOUND_USE_NATURAL_FREQUENCY * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14) * 17358 >> 14),
};

// This struct tells the ds sound system how to play each sound.
const DS_SOUND_TRIGGER_REFERENCE_STRUCT ds_sound_trigger_lemmings_ds_sfx_reference[] = {
  /* LEMMINGS_DS_SOUND_TYPE_NULL,                   */
                                          {(const void *)NULL,
                                                            0,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LETS_GO,                */
                           {(const void *)lem_sfx_lets_go_raw,
                                                        37586 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN,          */     
                           {(const void *)lem_sfx_trapdoor_open_raw,
                                                        27270 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT,            */        
                           {(const void *)lem_sfx_tool_select_raw,
                                                        23790 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,            */
                           {(const void *)lem_sfx_lemming_select_raw,
                                                         7748 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_OH_NO,                  */
                           {(const void *)lem_sfx_oh_no_raw,
                                                        31600 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_SAVE,           */         
                           {(const void *)lem_sfx_lemming_save_raw,
                                                        20598 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE,        */      
                           {(const void *)lem_sfx_lemming_explode_raw,
                                                         5940 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE,    */
                           {(const void *)lem_sfx_release_rate_change_raw,
                                                        23072 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE_HI,    */
                           {(const void *)lem_sfx_release_rate_change_hi_raw,
                                                        11536 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_SQUISH,         */
                           {(const void *)lem_sfx_lemming_squish_raw,
                                                        19248 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,              */   
                           {(const void *)lem_sfx_steel_hit_raw,
                                                         7688 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL, */   
                           {(const void *)lem_sfx_lemming_scream_raw,
                                                        11724 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_BUILDER_CHINK,          */   
                           {(const void *)lem_sfx_builder_chink_raw,
                                                         1846 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER,      */   
                           {(const void *)lem_sfx_lemming_scream_raw,
                                                        11724 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_DROWN,          */  
                                          {(const void *)NULL,
                                                            0,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,      */   
                           {(const void *)lem_sfx_lemming_scream_raw,
                                                        11724 >> 2,
                                                            1,
                                                        22050},
  /* LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER,   */
                           {(const void *)lem_sfx_lemming_scream_raw,
                                                        11724 >> 2,
                                                            1,
                                                        22050},
};
             
u32 music_preference = MUSIC_PREFERENCE_MUSIC_AND_SOUND;

// This function applies a standard request for each lemmings ds sound.
// Irrelevant values (frequency for 'oh no', etc) are ignored.
void DSSoundTriggerLemmingsDS_RequestStandardSound(int sound_type,
                                                   int relative_frequency,
                                                   int sound_location,
                                                   int camera_x_inset) {

   if (!(music_preference & MUSIC_PREFERENCE_SOUND)) return;

   switch (sound_type) {
      case (LEMMINGS_DS_SOUND_TYPE_LETS_GO) : {
         DSSoundTrigger_AddRequestCentre(LEMMINGS_DS_SOUND_TYPE_LETS_GO,
                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                                    127);
      } break;           
      case (LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT) : {                
         // The tool number is passed in as 'relative_frequency'.
         DSSoundTrigger_AddRequestCentre(LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT,
                        ds_sound_trigger_lemmings_ds_tool_select_relative_frequency[relative_frequency],
                                                                        127);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT) : {
         DSSoundTrigger_AddRequestCentre(LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,
                                                DS_SOUND_USE_NATURAL_FREQUENCY,
                                                                           127);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_OH_NO) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_OH_NO,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_SAVE) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_SAVE,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE) : {
         // The release rate is passed in as 'relative_frequency'.
         if (relative_frequency < 72) {
            DSSoundTrigger_AddRequestCentre(LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE,
                                            ds_sound_trigger_lemmings_ds_release_rate_change_relative_frequency[relative_frequency],
                                            127);
         } else {
            DSSoundTrigger_AddRequestCentre(LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE_HI,
                                            ds_sound_trigger_lemmings_ds_release_rate_change_relative_frequency[relative_frequency - 36],
                                            127);
         }
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_SQUISH) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_SQUISH,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_STEEL_HIT) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;                                             
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_BUILDER_CHINK) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_BUILDER_CHINK,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;                               
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_DROWN) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_DROWN,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
      case (LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER) : {
         DSSoundTrigger_AddRequestBasedOnLocationWCamera(LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER,
                                                         DS_SOUND_USE_NATURAL_FREQUENCY,
                                                         127,
                                                         sound_location,
                                                         camera_x_inset);
      } break;
   }
}
