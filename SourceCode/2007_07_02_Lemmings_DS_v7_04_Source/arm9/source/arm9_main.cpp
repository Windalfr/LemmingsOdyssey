/*
   Lemmings DS by Mathew Carr and the Lemmings DS Project Team.
   mattcarr@gmail.com
   http://www.mrdictionary.net/

   Hit v5 Early May 2005 (v4 was a bit poo, skipped it)
   Hit v7 Late May 2005 because 7 sounds nicer than 6, dontcha think?

   Formerly:

   Mathew Carr's sprite testing application v0.000001

   I think the only bit of this that is still AXE is the joypad reading code,
   and that might be common to all of the devkitPro based demo code... ah well.

   As of 6th Feburary 2007, even the AXE joypad code was removed in favour of the
   new DevKitPro r20 style input. I think.

   Based on:
     Axe clone
     Version 0.1
     Damian Yerrick (tepples@spamcop.net)
     http://www.pineight.com/ds/

     Subject to copyright license in README.txt

     Based on framebuffer_demo1 ARM9 Code
       Chris Double (chris.double@double.co.nz)
       http://www.double.co.nz/nintendo_ds
*/

#include "nds.h"
#include <stdlib.h>
#include <stdio.h>
#include <fat.h>
#include <sys/dir.h>
#include "errno.h"

//#define DS_DLDI_DEBUG
#include "ds_dldi_debug.h"

#include "types.h"
#include "ds_memory.h"
#include "ds_backbuff.h"
#include "ds_sprite.h"
#include "utility.h"

#include "key_value.h"

#include "stristr.h"

#include "global_gamephrases.h"

#include "lemmings_level.h"

#include "lemmings_graphical_object.h"

#include "graphicsglobals.h"

#include "lemgfx.h"
#include "gfx_walker.h"
#include "gfx_digger.h"
#include "gfx_basher.h"
#include "gfx_miner.h"
#include "gfx_builder.h"
#include "gfx_buildshrug.h"
#include "gfx_faller.h"
#include "gfx_umbrellawhip.h"
#include "gfx_floater.h"
#include "gfx_climber.h"
#include "gfx_alleyoop.h"
#include "gfx_blocker.h"
#include "gfx_smoosh.h"
#include "gfx_earpull.h"
#include "gfx_leaving.h"
#include "gfx_explodenum.h"
#include "gfx_explode.h"
#include "gfx_drowning.h"
#include "gfx_burnt.h"

#include "gfx_interface.h"
#include "gfx_interface_numbers.h"
#include "gfx_interface_selected_tool.h"

#include "menugfx_background_texture.h"
#include "menugfx_lemmings_logo.h"
#include "menugfx_sign_lemmings.h"
#include "menugfx_lemming_eye_frames.h"
#include "menugfx_lemmings_font.h"
#include "menugfx_scroller_lemming.h"
#include "menugfx_scroller_backdrop.h"
#include "menutext_mainscroller.h"

#include "levelinfogfx_wooden_panel.h"

// --------------------------------------------------

//#define SHOW_TWO_PLAYER_LEMS_OPTION

// --------------------------------------------------
//     TobW's NitroTracker Player Includes
// --------------------------------------------------

#include "ds_sound_trigger_system.h"

#include "nt_source/song.h"
#include "nt_source/xm_transport.h"
#include "nt_source/instrument.h"
#include "nt_source/sample.h"
#include "../../generic/command.h"

#include "ds_sound_trigger_system.h"
#include "ds_sound_trigger_lemmings_ds.h"

// --------------------------------------------------

// This vblank service routine processes pending incoming commands
void irqVBlank(void) {         
   //// Process any pending incoming commands, such as the NitroTracker player
   //// notification commands. :)
   CommandProcessCommands();
}

#define Key(x)        ( ((joy & (x)) == (x)))
#define KeyDown(x)    ( ((joy & (x)) == (x)) && !((joyp & (x)) == (x)))
#define KeyHeld(x)    ( ((joy & (x)) == (x)) &&  ((joyp & (x)) == (x)))
#define KeyRelease(x) (!((joy & (x)) == (x)) &&  ((joyp & (x)) == (x)))
#define KeyIdle(x)    (!((joy & (x)) == (x)) && !((joyp & (x)) == (x)))

// These hold the current and previous states of the DS' keys.
volatile int joy = 0, joyp = 0;

// This updates joy and joyp so that the Key* defines work.
void ScanKeypad() {
   // Scan the keypad and hinge.
   scanKeys();

   // Move last cycles input to the previous input variable.
   joyp = joy;
   // Get new input for the keypad, extra keypad, pen and hinge.
   joy = keysHeld();
   
   // If the lid is closed, disregard all buttons except the lid.
   if (joy & KEY_LID) {
      joy  = KEY_LID;
      joyp = KEY_LID;
   }
}

// This holds data relating to the location of the pen on the touch screen, at the last poll.
touchPosition touchXY;

// This updates touchXY: (if available)
void ScanPenCoords() {
   if (keysHeld() & KEY_TOUCH) {
      touchXY = touchReadXY();
   }
}

void LemmingsDS_UnrecoverableError(const char *title_string, const char *error_string_1,
                                                             const char *error_string_2 = NULL,
                                                             const char *error_string_3 = NULL,
                                                             const char *error_string_4 = NULL,
                                                             const char *error_string_5 = NULL,
                                                             const char *error_string_6 = NULL,
                                                             const char *error_string_7 = NULL,
                                                             const char *error_string_8 = NULL,
                                                             const char *error_string_9 = NULL);

#define LEMMINGS_DS_UNRECOVERABLE_ERROR_UNSPECIFIED_ERROR                                              0
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_MISSING_CONFIG_FILE                                            1
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_CONFIG_FILE                                            2
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_DIRECTORIES                                            3
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_RETRIEVE_VITAL_STATISTICS                              4
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_CHAR_PTR_ARRAY               5
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_FILENAME_STRING              6
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_XM_SONG                    7
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_CATEGORY_LOCATION_STRING           8
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE_PTR_ARRAY         9
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE                  10 
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_NAME_STRING             11
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE_PTR_ARRAY            12
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE                      13
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOCATION_STRING             14
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_NAME_STRING                 15
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_GRAPHICAL_OBJECT_FILE     16
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_INGAME_STATUS_STRUCT              17
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOADING                     18
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_TEXTURE_ARCHIVE_LOADING           19
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_DATA_ARRAY                  20
#define LEMMINGS_DS_UNRECOVERABLE_ERROR_THERE_ARENT_ANY_LEVELS                                        21

void LemmingsDS_UnrecoverableError(unsigned int error = LEMMINGS_DS_UNRECOVERABLE_ERROR_UNSPECIFIED_ERROR, const char *parameter = NULL);
                                                                      
                                                             
void LemmingsDS_FadeInMessage(int string_1_colour    , const char *string_1,
                              int string_2_colour = 0, const char *string_2 = NULL,
                              int string_3_colour = 0, const char *string_3 = NULL,
                              int string_4_colour = 0, const char *string_4 = NULL,
                              int string_5_colour = 0, const char *string_5 = NULL,
                              int string_6_colour = 0, const char *string_6 = NULL,
                              int string_7_colour = 0, const char *string_7 = NULL,
                              int string_8_colour = 0, const char *string_8 = NULL,
                              int string_9_colour = 0, const char *string_9 = NULL);
                              
void LemmingsDS_FadeFromWhiteToBlack();

// ----------------------------------------------------

// Global copies of sprite attribute memory, for MAIN and SUB
SpriteEntry sprite_shadow_m[128];
SpriteEntry sprite_shadow_s[128];

// Disable n many sprites, starting from sprite s, in sprite shadow sprite_shadow
void initSprites(SpriteEntry *sprite_shadow, int n, int s) {
	for(int i = 0; i < n; i++, s++) {
	   sprite_shadow[s].attribute[0] = ATTR0_DISABLED;
	   sprite_shadow[s].attribute[1] = 0;
	   sprite_shadow[s].attribute[2] = 0;
	   sprite_shadow[s].attribute[3] = 0;
   }
}

// Copy a sprite shadow to object attribute memory
void updateOAM(void *destination_OAM, SpriteEntry *sprite_shadow) {
	DC_FlushAll();
   dmaCopy(sprite_shadow, destination_OAM, 128 * sizeof(SpriteEntry));
}

void BlankAllPalettes() {
   for (int e = 0; e < 256; e++) {
      BG_PALETTE[e]         = 0;
      BG_PALETTE_SUB[e]     = 0;
      SPRITE_PALETTE[e]     = 0;
      SPRITE_PALETTE_SUB[e] = 0;
   }
	DC_FlushAll();
}

void BlankAllPalettesToWhite() {
   for (int e = 0; e < 256; e++) {
      BG_PALETTE[e]         = RGB15A(31, 31, 31);
      BG_PALETTE_SUB[e]     = RGB15A(31, 31, 31);
      SPRITE_PALETTE[e]     = RGB15A(31, 31, 31);
      SPRITE_PALETTE_SUB[e] = RGB15A(31, 31, 31);
   }
	DC_FlushAll();
}

void BlankAllSubPalettes() {
   for (int e = 0; e < 256; e++) {
      BG_PALETTE_SUB[e]     = 0;
      SPRITE_PALETTE_SUB[e] = 0;
   }
	DC_FlushAll();
}

#define DS_COLOUR_BLUE_PART  (31 << 10)
#define DS_COLOUR_GREEN_PART (31 <<  5)
#define DS_COLOUR_RED_PART   (31 <<  0)

inline void Split15BitColour(int colour, unsigned int *r, unsigned int *g, unsigned int *b) {
   *r = (colour & DS_COLOUR_RED_PART)   >>  0;
   *g = (colour & DS_COLOUR_GREEN_PART) >>  5;
   *b = (colour & DS_COLOUR_BLUE_PART)  >> 10;
}

// This function returns a 15 bit colour that's X out of Y positions between
// two 15 bit colours, X and Y. Like this:
//
//        X
//  0 . . . . . . . . Y
//  A - - - - - - - - B
//
// Shush. You couldn't do any better.
inline u16 ColourThatsXoutofYbetweenAandB(u16 a, u16 b, u32 x, u32 y) {
   unsigned int ar, ag, ab,
                br, bg, bb,
                fr, fg, fb;

   Split15BitColour(a, &ar, &ag, &ab);
   Split15BitColour(b, &br, &bg, &bb);

   int r_delta = br - ar;
   int g_delta = bg - ag;
   int b_delta = bb - ab;

   int r_step = IntDivS(r_delta << 8, y);
   int g_step = IntDivS(g_delta << 8, y);
   int b_step = IntDivS(b_delta << 8, y);

   fr = ar + ((r_step * x) >> 8);
   fg = ag + ((g_step * x) >> 8);
   fb = ab + ((b_step * x) >> 8);

   return RGB15A(fr, fg, fb);
}

// Blanks VRAM pages A and B.
void BlankVRAMPagesAB() {
   memset(VRAM_MAIN, 0, 256*256*4);
	DC_FlushAll();
}

// Blanks VRAM pages C and D.
void BlankVRAMPagesCD() {
   memset(VRAM_SUB, 0, 256*256*4);
	DC_FlushAll();
}
// Blanks VRAM pages A, B, C and D.
void BlankVRAMPagesABCD() {
   memset(VRAM_MAIN, 0, 256*256*4*2);
	DC_FlushAll();
}

// This resets all of the background translation registers for the main screen.
void ResetBackgroundTranslationRegistersMain() {
       BG0_X0 = 0;
       BG0_Y0 = 0;
       BG1_X0 = 0;
       BG1_Y0 = 0;
       BG2_X0 = 0;
       BG2_Y0 = 0;
       BG3_X0 = 0;
       BG3_Y0 = 0;
	DC_FlushAll();
}

// This resets all of the background translation registers for the sub screen.
void ResetBackgroundTranslationRegistersSub() {
   SUB_BG0_X0 = 0;
   SUB_BG0_Y0 = 0;
   SUB_BG1_X0 = 0;
   SUB_BG1_Y0 = 0;
   SUB_BG2_X0 = 0;
   SUB_BG2_Y0 = 0;
   SUB_BG3_X0 = 0;
   SUB_BG3_Y0 = 0;
	DC_FlushAll();
}

// ----------------------------------------------------

// Checks whether or not the string s ends in the string ends_in
int strendsin(const char *s, const char *ends_in) {
   // This string holds the location of a found occurense of ends_in in s.
   const char *found_occurence = s;

   while ((found_occurence = stristr(s, ends_in)) != NULL) {
      // Check to see if the character after the found occurence
      // is a null character. If it is, then it's at the end of the string
      // Right?
      if (found_occurence[strlen(ends_in)] == 0) return 1;

      // If this function has not returned 1 yet, then we need to search again.
      s = found_occurence;
   }

   // No match.
   return 0;
}

// Returns the location of the last instance of 'needle' in 'haystack'.
const char *strfindlast(const char *needle, const char *haystack) {
   const char *last_good = NULL;
   const char *searching = haystack - 1;
   while ((searching = (strstr(searching + 1, needle)))) { // Find the last needle.
      last_good = searching;
   }
   
   return last_good;
}

// ----------------------------------------------------

// Control method definitions...

#define CONTROL_STYLE_MAJOR_TAPPER  0  // Tap lemmings to assign.
#define CONTROL_STYLE_MAJOR_HOLDER  1  // Hold and snap to assign.
#define CONTROL_STYLE_MAJOR_TACTICS 2  // Tap lem to select, tap tool to assign.
u32 control_style_major = CONTROL_STYLE_MAJOR_TAPPER;

#define CONTROL_STYLE_PAUSE_HOLD   0 // Hold shoulders for non-master pause.
#define CONTROL_STYLE_PAUSE_TOGGLE 1 // Toggle shoulder for master pause.
u32 control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

s32 control_cursor_extension_size; // This is the number of pixels (screen pixels, that is) in each direction that the cursor
                                   // extends beyond the actual point where the click occurred.

// ----------------------------------------------------

// Lemmings DS config file management stuff!
// -----------------------------------------

// This holds the location of the root of the Lemmings DS
// file hierarchy.
#define LEMMINGS_DS_ROOT_DIR_ARRAY_LENGTH 256
char lemmings_ds_root_dir[LEMMINGS_DS_ROOT_DIR_ARRAY_LENGTH];

// This key value controller holds the values of the
// parameters specified in the config file.
KEY_VALUE_CONTROLLER *lemmings_ds_config_file_info;

// Specify the location of the Lemmings DS config file.
const char lemmings_ds_config_file_location[] = "/lemmingsds_config.txt";

#define LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_OK               0
#define LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_FILE_NOT_FOUND   1
#define LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_FILE         2
#define LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_DIRECTORIES  3

// Define here the names of the parameters used.
const char config_parameter_lemmings_ds_root_dir[]             = "Lemmings DS Directory";
const char config_parameter_sound_music_preference[]           = "Sound/Music Preference";
const char config_parameter_control_style_preference[]         = "Control Style";
const char config_parameter_cursor_extension_size_preference[] = "Cursor Extension Size";

// These are valid values for the sound and music preference parameter.
const char config_parameter_sound_music_preference_both[]    = "Music and Sound";
const char config_parameter_sound_music_preference_music[]   = "Music";
const char config_parameter_sound_music_preference_sound[]   = "Sound";
const char config_parameter_sound_music_preference_silence[] = "Silence";

// These are valid values for the control style preference parameter.
const char config_parameter_control_style_preference_tapper_hold[]    = "Tapper Hold";
const char config_parameter_control_style_preference_tapper_toggle[]  = "Tapper Toggle";
const char config_parameter_control_style_preference_holder[]         = "Holder";
const char config_parameter_control_style_preference_tactics_hold[]   = "Tactics Hold";
const char config_parameter_control_style_preference_tactics_toggle[] = "Tactics Toggle";

// This function loads the config values from the specified FILE.
// Returns true on success and false on failure.
int LemmingsDSConfig_LoadConfigFromFile() {
   // We will now attempt to load the Lemmings DS config file from
   // 'lemmings_ds_config_file_location'
   DebugAppend("Attempting to load Lemmings DS configuration from:\r\n'");
   DebugAppend(lemmings_ds_config_file_location);
   DebugAppend("'\r\n");
   DebugWrite();

   FILE *lemmings_ds_config_file = fopen(lemmings_ds_config_file_location, "rb");

   // Check for file loading failure.
   if (lemmings_ds_config_file == NULL) {
      DebugAppend("Config file loading failed. (NULL returned from fopen)\r\n");
      DebugWrite();

      // Reset errno
      errno = 0;

      return LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_FILE_NOT_FOUND;
   }

   DebugAppend("Config file acquired, loading.\r\n");
   DebugWrite();

   // Blank lemmings_ds_root_dir
   memset(lemmings_ds_root_dir, 0, LEMMINGS_DS_ROOT_DIR_ARRAY_LENGTH);

   // Create the KEY_VALUE_CONTROLLER that will manage the data pairs.
   lemmings_ds_config_file_info = KeyValueC_Create();

   DebugAppend("Populating KEY_VALUE_CONTROLLER using file:\r\n");
   DebugWrite();

   // Populate the controller.
   if (KeyValueC_PopulateFromFile(lemmings_ds_config_file_info, lemmings_ds_config_file)
        == KEY_VALUE_CONTROLLER_POPULATION_FAILURE) {
      // If the config file isn't found, the whole game is
      // in an indeterminate state.
      // Don't bother to clean up.
      // Do nothing, just crash.
      DebugAppend("Failed to populate.\r\n");
      DebugWrite();

      fclose(lemmings_ds_config_file);

      // Looks like we've got a bad file.
      return LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_FILE;
   } else {
      // Alright, now we should be able to retrieve the values
      // we want to know.

      // First up, lets see if the user has specified a directory
      // for Lemmings DS to be loaded from.
      if (KeyValueC_KeyExists(lemmings_ds_config_file_info, config_parameter_lemmings_ds_root_dir)
            == KEY_VALUE_CONTROLLER_KEY_NOT_FOUND) {
         // I couldn't find a place in the file where the user said what directory
         // Lemmings lives in. I don't know where to load levels or anything from now!
         DebugAppend("Lemmings DS root directory parameter missing.\r\n");
         DebugWrite();

         fclose(lemmings_ds_config_file);

         // As Victor would say: "Damn, this is bad."
         return LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_FILE;
      } else {
         // So, the user has specified a place for us to load files from.
         // Let's copy and convert this into the correct location for use:
         const char *read_config_parameter_lemmings_ds_root_dir_parameter = KeyValueC_KeyLookup(lemmings_ds_config_file_info,
                                                                                                config_parameter_lemmings_ds_root_dir);

         // Debug out the read parameter.
         DebugAppend("Lemmings DS root directory read as '");
         DebugAppend(read_config_parameter_lemmings_ds_root_dir_parameter);
         DebugAppend("' :\r\n");
         DebugWrite();

         // So, it seems we've read a directory.

         // Copy it into the string, append the trailing forwardslash.
         // If we read a blank key, we want to hold "/" in the root directory string.
         strcpy(lemmings_ds_root_dir, read_config_parameter_lemmings_ds_root_dir_parameter);

         strcat(lemmings_ds_root_dir, "/");

         // Set this flag to false if the directory loading failed.
         bool successful_directory_loading = true;

#define NO_LEMMINGS_DS_EXPECTED_DIRECTORIES_ON_CARD 10

// There IS a funky string literal concatenation syntax that can be used here
// to construct redundant directory tree strings, but it straddles multiple lines
// and IS WEIRD. And it's unnecessary.
#define LEMMINGS_DS_DIRECTORY_STANDARD_TEXTURE_ARCHIVES        "standard_texture_archives/"
#define LEMMINGS_DS_DIRECTORY_GRAPHICS                         "graphics/"
#define LEMMINGS_DS_DIRECTORY_STANDARD_GRAPHICAL_OBJECTS       "standard_graphical_objects/"
#define LEMMINGS_DS_DIRECTORY_MUSIC                            "music/"
#define LEMMINGS_DS_DIRECTORY_MUSIC_TITLE_SCREEN               "music/title_screen/"
#define LEMMINGS_DS_DIRECTORY_MUSIC_STANDARD_INGAME            "music/standard_ingame/"
#define LEMMINGS_DS_DIRECTORY_LEVELS                           "levels/"
#define LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER                   "levels/1player/"
#define LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_STANDARD          "levels/1player/standard/"
#define LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_CUSTOM            "levels/1player/custom/"

         const char *lemmings_ds_expected_directory_locations[NO_LEMMINGS_DS_EXPECTED_DIRECTORIES_ON_CARD] = {
                             LEMMINGS_DS_DIRECTORY_STANDARD_TEXTURE_ARCHIVES,
                             LEMMINGS_DS_DIRECTORY_GRAPHICS,
                             LEMMINGS_DS_DIRECTORY_STANDARD_GRAPHICAL_OBJECTS,
                             LEMMINGS_DS_DIRECTORY_MUSIC,
                             LEMMINGS_DS_DIRECTORY_MUSIC_TITLE_SCREEN,
                             LEMMINGS_DS_DIRECTORY_MUSIC_STANDARD_INGAME,
                             LEMMINGS_DS_DIRECTORY_LEVELS,
                             LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER,
                             LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_STANDARD,
                             LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_CUSTOM,
         };

         // Try to acquire each of the directories in turn.
         for (int d = 0; d < NO_LEMMINGS_DS_EXPECTED_DIRECTORIES_ON_CARD; d++) {
            // Use this string to construct the path to the directory to test.
            char test_directory_loading_destination[256];
            memset(test_directory_loading_destination, 0, 256);

            // Construct the path string.
            strcat(test_directory_loading_destination, lemmings_ds_root_dir);
            strcat(test_directory_loading_destination, lemmings_ds_expected_directory_locations[d]);

            DebugAppend("Constructed ");
            DebugAppend(test_directory_loading_destination);
            DebugAppend(". Checking: ");
            DebugWrite();

            // Attempt to acquire the directory.
            DIR_ITER *test_directory_loading_ptr = diropen(test_directory_loading_destination);

            // Test for failure
            if (test_directory_loading_ptr == NULL) {
               DebugAppend("Failure.\r\n");
               DebugWrite();

               successful_directory_loading = false;

               // Stop looking for directories.
               break;
            } else {
               DebugAppend("Success.\r\n");    
               DebugWrite();

               // Close this directory, and continue looking for directories.
               dirclose(test_directory_loading_ptr);
            }
         }

         // If we couldn't acquire each of the required directories, die.
         if (!successful_directory_loading) {
            fclose(lemmings_ds_config_file);

            return LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_DIRECTORIES;
         }
      }

      // SOUND PREFERENCE:
      DebugAppend("Now checking for sound preference - ");   
      DebugWrite();

      // Check if the user has specified a music preference.
      if (KeyValueC_KeyExists(lemmings_ds_config_file_info, config_parameter_sound_music_preference)
            == KEY_VALUE_CONTROLLER_KEY_FOUND) {

         DebugAppend("Found. Analysing: ");
         DebugWrite();

         // So, the user has specified a music/sound preference
         // Let's copy and convert this into the correct location for use:
         const char *read_config_parameter_sound_music_preference_parameter = KeyValueC_KeyLookup(lemmings_ds_config_file_info,
                                                                                                  config_parameter_sound_music_preference);

         // Compare the read key against all of the valid values:
         if (stricmp(read_config_parameter_sound_music_preference_parameter,
                     config_parameter_sound_music_preference_both          ) == 0) {
            // They've specified 'both sound and music'.
            DebugAppend("'Both Sound and Music'.\r\n");
            DebugWrite();

            // Set the music preference
            music_preference = MUSIC_PREFERENCE_MUSIC_AND_SOUND;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_sound_music_preference,
                                            config_parameter_sound_music_preference_both);
         } else
         if (stricmp(read_config_parameter_sound_music_preference_parameter,
                     config_parameter_sound_music_preference_music         ) == 0) {
            // They've specified 'music'.
            DebugAppend("'Music'.\r\n");
            DebugWrite();

            // Set the music preference
            music_preference = MUSIC_PREFERENCE_MUSIC;     

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_sound_music_preference,
                                            config_parameter_sound_music_preference_music);
         } else
         if (stricmp(read_config_parameter_sound_music_preference_parameter,
                     config_parameter_sound_music_preference_sound         ) == 0) {
            // They've specified 'sound'.
            DebugAppend("'Sound'.\r\n");
            DebugWrite();

            // Set the music preference
            music_preference = MUSIC_PREFERENCE_SOUND;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_sound_music_preference,
                                            config_parameter_sound_music_preference_sound);
         } else
         if (stricmp(read_config_parameter_sound_music_preference_parameter,
                     config_parameter_sound_music_preference_silence       ) == 0) {
            // They've specified 'silence'.
            DebugAppend("'Silence'.\r\n");
            DebugWrite();

            // Set the music preference
            music_preference = MUSIC_PREFERENCE_SILENCE;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_sound_music_preference,
                                            config_parameter_sound_music_preference_silence);
         } else {
            // Unexpected? Let's set a default.
            DebugAppend("Mangled. Applying default.\r\n");
            DebugWrite();

            // Set the music preference
            music_preference = MUSIC_PREFERENCE_MUSIC_AND_SOUND;   

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_sound_music_preference,
                                            config_parameter_sound_music_preference_both);
         }
      } else {
         // No value? Let's set one.
         DebugAppend("Absent. Applying default.\r\n");
         DebugWrite();

         // Set the music preference
         music_preference = MUSIC_PREFERENCE_MUSIC_AND_SOUND;

         // Reapply the standard string to the key pair (get rid of errant case mixups!)
         KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                         config_parameter_sound_music_preference,
                                         config_parameter_sound_music_preference_both);
      }

      // CONTROL STYLE PREFERENCE
      DebugAppend("Now checking for control style preference - ");
      DebugWrite();

      // Check if the user has specified a control style preference.
      if (KeyValueC_KeyExists(lemmings_ds_config_file_info, config_parameter_control_style_preference)
            == KEY_VALUE_CONTROLLER_KEY_FOUND) {

         DebugAppend("Found. Analysing: ");
         DebugWrite();

         // So, the user has specified a music/sound preference
         // Let's copy and convert this into the correct location for use:
         const char *read_config_parameter_control_style_preference_parameter = KeyValueC_KeyLookup(lemmings_ds_config_file_info,
                                                                                                  config_parameter_control_style_preference);

         // Compare the read key against all of the valid values:
         if (stricmp(read_config_parameter_control_style_preference_parameter,
                     config_parameter_control_style_preference_tapper_hold   ) == 0) {
            // They've specified 'tapper hold'.
            DebugAppend("'Tapper Hold'.\r\n");
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_TAPPER;
            control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_tapper_hold);
         } else
         if (stricmp(read_config_parameter_control_style_preference_parameter,
                     config_parameter_control_style_preference_tapper_toggle ) == 0) {
            // They've specified 'tapper toggle'.
            DebugAppend("'Tapper Toggle'.\r\n");
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_TAPPER;
            control_style_pause = CONTROL_STYLE_PAUSE_TOGGLE;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_tapper_toggle);
         } else
         if (stricmp(read_config_parameter_control_style_preference_parameter,
                     config_parameter_control_style_preference_holder        ) == 0) {
            // They've specified 'holder'.
            DebugAppend("'Holder'.\r\n");  
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_HOLDER;
            control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_holder);
         } else
         if (stricmp(read_config_parameter_control_style_preference_parameter,
                     config_parameter_control_style_preference_tactics_hold  ) == 0) {
            // They've specified 'tactics hold'.
            DebugAppend("'Tactics Hold'.\r\n");
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_TACTICS;
            control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_tactics_hold);
         } else
         if (stricmp(read_config_parameter_control_style_preference_parameter,
                     config_parameter_control_style_preference_tactics_toggle) == 0) {
            // They've specified 'tactics toggle'.
            DebugAppend("'Tactics Toggle'.\r\n");
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_TACTICS;
            control_style_pause = CONTROL_STYLE_PAUSE_TOGGLE;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_tactics_toggle);
         } else {
            // Unexpected? Let's set a default.
            DebugAppend("Mangled. Applying default.\r\n");
            DebugWrite();

            // Set the control style preference
            control_style_major = CONTROL_STYLE_MAJOR_TAPPER;
            control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

            // Reapply the standard string to the key pair (get rid of errant case mixups!)
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_control_style_preference,
                                            config_parameter_control_style_preference_tapper_hold);
         }
      } else {
         // No value? Let's set one.
         DebugAppend("Absent. Applying default.\r\n");
         DebugWrite();

         // Set the control style preference
         control_style_major = CONTROL_STYLE_MAJOR_TAPPER;
         control_style_pause = CONTROL_STYLE_PAUSE_HOLD;

         // Reapply the standard string to the key pair (get rid of errant case mixups!)
         KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                         config_parameter_control_style_preference,
                                         config_parameter_control_style_preference_tapper_hold);
      }

      // CURSOR EXTENSION PREFERENCE:
      DebugAppend("Now checking for cursor extension size preference - ");
      DebugWrite();

      // Check if the user has specified a music preference.
      if (KeyValueC_KeyExists(lemmings_ds_config_file_info, config_parameter_cursor_extension_size_preference)
            == KEY_VALUE_CONTROLLER_KEY_FOUND) {

         DebugAppend("Found. Analysing: ");
         DebugWrite();

         // So, the user has specified a cursor extesion size preference
         // Let's copy this into the correct location for use:
         const char *read_config_parameter_cursor_extension_preference_parameter = KeyValueC_KeyLookup(lemmings_ds_config_file_info,
                                                                                                       config_parameter_cursor_extension_size_preference);

         // Use sscanf to get an int version of the string retrieved from the config file
         int analysed_config_parameter_cursor_extension_preference_parameter_value;
         int config_parameter_cursor_extension_preference_parameter_analysis_result;

         config_parameter_cursor_extension_preference_parameter_analysis_result = sscanf(read_config_parameter_cursor_extension_preference_parameter,
                                                                                         "%d",
                                                                                         &analysed_config_parameter_cursor_extension_preference_parameter_value);

         // If the input was invalid, set a default
         if (config_parameter_cursor_extension_preference_parameter_analysis_result == EOF) {
            // Unexpected? Let's set a default.
            DebugAppend("Mangled. Applying default.\r\n");
            DebugWrite();

            // Set the default cursor extension size
            control_cursor_extension_size = 1;

            // Reapply the standard string to the key pair
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_cursor_extension_size_preference,
                                            "1");
         } else
         if ((analysed_config_parameter_cursor_extension_preference_parameter_value < 0)
          || (analysed_config_parameter_cursor_extension_preference_parameter_value > 9)) {
            // Invalid? Let's set a default.
            DebugAppend("Out of range. Applying default.\r\n");
            DebugWrite();

            // Set the default cursor extension size
            control_cursor_extension_size = 1;

            // Reapply the standard string to the key pair
            KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                            config_parameter_cursor_extension_size_preference,
                                            "1");
         } else {
            // They've specified a valid number
            DEBUG_SECTION {
               char output_text[4096];
               sprintf(output_text, "Found '%s', analysis shows it's valid as '%d'.\r\n",
                                    read_config_parameter_cursor_extension_preference_parameter,
                                    analysed_config_parameter_cursor_extension_preference_parameter_value);
               DebugAppend(output_text);
               DebugWrite();
            }

            // Set the cursor extension size
            control_cursor_extension_size = analysed_config_parameter_cursor_extension_preference_parameter_value;

            // Leave the string inside the key pair.
            // If the analysis liked the integer retrieved from the string enough to allow
            // execution to enter this branch, then it can rewrite that exact value to the
            // file when the options are changed, and we can be assured that sscanf will
            // not crash when it rereads the same (shown valid) number again.
         }
      } else {
         // No value? Let's set one.
         DebugAppend("Absent. Applying default.\r\n");
         DebugWrite();

         // Set the default cursor extension size
         control_cursor_extension_size = 1;

         // Reapply the standard string to the key pair
         KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                         config_parameter_cursor_extension_size_preference,
                                         "1");
      }
   }

   fclose(lemmings_ds_config_file);

   // Looks like everything went rather well.
   return LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_OK;
}

// This will rewrite the config file to the root of the flash media.
// All of the standard comments are in here too.
void LemmingsDSConfig_SaveConfigToFile() {
   // We will now attempt to open the Lemmings DS config file from
   // 'lemmings_ds_config_file_location' for writing.
   DebugAppend("Attempting to acquire Lemmings DS configuration file from:\r\n'");
   DebugAppend(lemmings_ds_config_file_location);
   DebugAppend("'\r\n");
   DebugWrite();

   FILE *lemmings_ds_config_file = fopen(lemmings_ds_config_file_location, "wb");

   // Check for file acquisition failure.
   if (lemmings_ds_config_file == NULL) {
      DebugAppend("Config file acquisition failed. (NULL returned from fopen)\r\n");
      DebugWrite();

      // Reset errno
      errno = 0;

      return;
   }

   DebugAppend("Config file acquired, loading.\r\n");
   DebugWrite();

   // Each config key pair will have a preamble.
   // I'm not sure of the best way to do this:

   const char *lemmings_ds_config_file_preamble_1 =
"# Change the following values to configure Lemmings DS!\r\n"
"\r\n"
"# Please keep this file in the root directory of your card.\r\n"
"\r\n"
"# Don't add any extra lines to this file:\r\n"
"# They'll disappear when Lemmings DS is next run.\r\n"
"\r\n"
"# 'Lemmings DS Directory' should be set to the location\r\n"
"# of the Lemmings DS files on your flash media.\r\n"
"# Please use Unix style directories (/games/blah/LemmingsDS)\r\n"
"# It should have a starting forwardslash.\r\n"
"# It should not have a trailing forwardslash.\r\n"
"# NOTE: If Lemmings DS lives in the root, LEAVE A BLANK LINE\r\n"
"# after 'Lemmings DS Directory'\r\n"
"\r\n";

   // Write this first preamble.
   fputs(lemmings_ds_config_file_preamble_1, lemmings_ds_config_file);

   // Then write the key value pair for the Lemmings DS root directory.
   KeyValueC_WritePairToFile(lemmings_ds_config_file_info, lemmings_ds_config_file, config_parameter_lemmings_ds_root_dir);

   const char *lemmings_ds_config_file_preamble_2 =
"\r\n"
"# 'Sound/Music Preference' controls whether sound or music will be played.\r\n"
"# Available options are:\r\n"
"\r\n"
"# Music and Sound for both music and sound.\r\n"
"# Music           for just music.\r\n"
"# Sound           for just sound.\r\n"
"# Silence         for nothing.\r\n"
"\r\n";

   // Write this second preamble.
   fputs(lemmings_ds_config_file_preamble_2, lemmings_ds_config_file);

   // Then write the key value pair for the sound and music preference.
   KeyValueC_WritePairToFile(lemmings_ds_config_file_info, lemmings_ds_config_file, config_parameter_sound_music_preference);

   const char *lemmings_ds_config_file_preamble_3 =
"\r\n"
"# 'Control Style' determines the default control style to use:\r\n"
"# Available options are:\r\n"
"\r\n"
"# Tapper Hold\r\n"
"# Tapper Toggle\r\n"
"# Holder\r\n"
"# Tactics Hold\r\n"
"# Tactics Toggle\r\n"
"\r\n";

   // Write this third preamble.
   fputs(lemmings_ds_config_file_preamble_3, lemmings_ds_config_file);

   // Then write the key value pair for the control style preference.
   KeyValueC_WritePairToFile(lemmings_ds_config_file_info, lemmings_ds_config_file, config_parameter_control_style_preference);

   const char *lemmings_ds_config_file_preamble_4 =
"\r\n"
"# 'Cursor Extension Size' affects the active range of the stylus:\r\n"
"# A larger number will make Lemmings easier to click, but reduces\r\n"
"# precision.\r\n"
"# Specify a whole number in the range 0 to 9. The default is 1.\r\n"
"\r\n";

   // Write this fourth preamble.
   fputs(lemmings_ds_config_file_preamble_4, lemmings_ds_config_file);

   // Then write the key value pair for the control style preference.
   KeyValueC_WritePairToFile(lemmings_ds_config_file_info, lemmings_ds_config_file, config_parameter_cursor_extension_size_preference);

   // Close file.
   fclose(lemmings_ds_config_file);

   DebugAppend("Updated configuration file with new keys.\r\n");
   DebugWrite();
}

// This function will update the values of the keys in the config file
// to reflect the current options state.
void LemmingsDSConfig_UpdateConfigKeyValues() {
   // Determine the correct keys to copy into the config key value controller:

   // SOUND PREFERENCE:
   DebugAppend("Analysing current sound preference - ");
   DebugWrite();

   if (music_preference == MUSIC_PREFERENCE_MUSIC_AND_SOUND) {
      // Current state is 'both sound and music'.
      DebugAppend("'Both Sound and Music'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_sound_music_preference,
                                      config_parameter_sound_music_preference_both);
   } else
   if (music_preference == MUSIC_PREFERENCE_MUSIC) {
      // Current state is 'music'.
      DebugAppend("'Music'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_sound_music_preference,
                                      config_parameter_sound_music_preference_music);
   } else
   if (music_preference == MUSIC_PREFERENCE_SOUND) {
      // Current state is 'sound'.
      DebugAppend("'Sound'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_sound_music_preference,
                                      config_parameter_sound_music_preference_sound);
   } else
   if (music_preference == MUSIC_PREFERENCE_SILENCE) {
      // Current state is 'silence'.
      DebugAppend("'Silence'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_sound_music_preference,
                                      config_parameter_sound_music_preference_silence);
   }

   // CONTROL STYLE PREFERENCE
   DebugAppend("Analysing control style preference - ");
   DebugWrite();

   if ((control_style_major == CONTROL_STYLE_MAJOR_TAPPER)
    && (control_style_pause == CONTROL_STYLE_PAUSE_HOLD  )) {
      // Current state is 'tapper hold'.
      DebugAppend("'Tapper Hold'.\r\n");   
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_control_style_preference,
                                      config_parameter_control_style_preference_tapper_hold);
   } else
   if ((control_style_major == CONTROL_STYLE_MAJOR_TAPPER)
    && (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE)) {
      // Current state is 'tapper toggle'.
      DebugAppend("'Tapper Toggle'.\r\n");  
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_control_style_preference,
                                      config_parameter_control_style_preference_tapper_toggle);
   } else
   if ((control_style_major == CONTROL_STYLE_MAJOR_HOLDER)
    && (control_style_pause == CONTROL_STYLE_PAUSE_HOLD  )) {
      // Current state is 'holder'.
      DebugAppend("'Holder'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_control_style_preference,
                                      config_parameter_control_style_preference_holder);
   } else
   if ((control_style_major == CONTROL_STYLE_MAJOR_TACTICS)
    && (control_style_pause == CONTROL_STYLE_PAUSE_HOLD  )) {
      // Current state is 'tactics hold'.
      DebugAppend("'Tactics Hold'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_control_style_preference,
                                      config_parameter_control_style_preference_tactics_hold);
   } else
   if ((control_style_major == CONTROL_STYLE_MAJOR_TACTICS)
    && (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE )) {
      // Current state is 'tactics toggle'.
      DebugAppend("'Tactics Toggle'.\r\n");
      DebugWrite();

      // Reapply the standard string to the key pair (get rid of errant case mixups!)
      KeyValueC_AddPairOrReplaceValue(lemmings_ds_config_file_info,
                                      config_parameter_control_style_preference,
                                      config_parameter_control_style_preference_tactics_toggle);
   }
}

// ---------------------------------------------------

// Song management:

// This is a pointer to the current song.
Song *current_song = NULL;

// This controls XM playback.
XMTransport xm_transport;

// This sets up the IP command core, and the NitroTracker frequency table.
void Music_SetupMusicEngine() {
   // Start the interprocess comms engine.
	CommandInit();
	// Set the song playback frequency table.
	CommandSetFreqTable(linear_freq_table);
   // Set up the command to be used for sound sample playback.
   DSSoundTriggerSetup(ds_sound_trigger_lemmings_ds_sfx_reference, CommandMRD_PlayOneShotSample);
   // Set default volume.
   CommandMRD_SetPlayerVolume(64);
}

// This struct holds a bunch of filenames for a specific music category.
typedef struct tagLEMMINGS_DS_MUSIC_CATEGORY_HANDLE {
   int no_music_files;     // This holds the number of music files within this category.
   char **music_filenames; // This is an array of char * holding the names of each music file in the category.
   // This should be fully qualified so that the pathname need not be constructed on demand.
} LEMMINGS_DS_MUSIC_CATEGORY_HANDLE;

LEMMINGS_DS_MUSIC_CATEGORY_HANDLE music_category_standard_ingame_songs = {0};
LEMMINGS_DS_MUSIC_CATEGORY_HANDLE music_category_title_screen_songs    = {0};

// This holds the current ingame music category.
// (This will either refer to the standard music category, or one of the
// custom level set music categories.)
LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *music_category_current_ingame;
                                     
// Interrogate a directory and populate the category's char ** with the names of all the .xm modules
// and place the number of songs into no_music_files.
void Music_PrepareMusicCategory(LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *music_category_handle, const char *location) {
   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Collating music tracks for the music category at PTR: %08X\r\n", (unsigned int)music_category_handle);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Reset the music category handle.
   music_category_handle->music_filenames = NULL;
   music_category_handle->no_music_files  = 0;
   // Open the directory specified:
   DIR_ITER *directory_scan = diropen(location);

   // If the directory could not be acquired...
   if (directory_scan == NULL) {
      DebugAppend("Could not acquire directory '");
      DebugAppend(location);
      DebugAppend("' for music track scanning.\r\n");
      DebugWrite();
      return;
   }

   // TODO: I think there's a bug with realloc, so I'm mallocing this here.
   music_category_handle->music_filenames = (char **)malloc(sizeof(char *) * 1024);
   DEBUG_SECTION {
      if (music_category_handle->music_filenames == NULL) {
         DebugAppend("Failed to acquire memory to store music filename array of (char *)s\r\n");
         DebugWrite();
         
         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_CHAR_PTR_ARRAY);
      }
   }

   DebugAppend("Scanning for available .xm modules in '");
   DebugAppend(location);
   DebugAppend("'.\r\n");
   DebugWrite();

   do {
      // String to store the read filename.
      char retrieved_filename_string[256];
      memset(retrieved_filename_string, 0, 256);

      // We are only looking for files, so
      // we will need to use a struct stat instance.
      struct stat file_stat;   
      
      // Reset errno
      errno = 0;

      // Scan for the next file.
      int d_r = dirnext(directory_scan, retrieved_filename_string, &file_stat);

      // Quit if we have reached the end of the directory, or there was an error.
      if ((d_r == -1) || (errno == ENOENT)) {
         DebugAppend("Available .xm modules scan concluded.\r\n");
         DebugWrite();

         // Reset errno
         errno = 0;

         break;
      }

      // If we've found the . or .., use continue.
      if ((strcmp(retrieved_filename_string, ".")  == 0)
       || (strcmp(retrieved_filename_string, "..") == 0)) continue;
       
      // Determine if it has the xm extension:
      int has_extension = strendsin(retrieved_filename_string, ".xm");

      // If the file does not have the extension, skip it.
      if (!has_extension) {
         DebugAppend("Wrong extension: Skipping.\r\n");
         DebugWrite();
         continue;
      }

      // Only interrogate files.
      if (!(file_stat.st_mode & S_IFDIR)) {
         DebugAppend(retrieved_filename_string);
         DebugAppend(" identified as a possible music track. Scanning...\r\n");
         DebugWrite();

         // Construct a string to hold the full path of this level:
         char possible_music_track_full_directory_location[256];
         memset(possible_music_track_full_directory_location, 0, 256);

         strcat(possible_music_track_full_directory_location, location);
         strcat(possible_music_track_full_directory_location, retrieved_filename_string);

         DebugAppend("Full music track location: ");
         DebugAppend(possible_music_track_full_directory_location);
         DebugAppend("\r\n");
         DebugWrite();

         // If no continue was called before now, it's a valid module.
         // Increase the number of valid modules, and add it to the list

         DebugAppend("Appears be a valid .xm module. Adding.\r\n");
         DebugWrite();

         // If we have identified a valid module...

         // Allocate memory for a char * for each level.
         // Use realloc to maintain the music filename pointers already specified:

         // TODO: I think there's a bug with realloc.
         //music_category_handle->music_filenames = (char **)realloc(music_category_handle->music_filenames, sizeof(char *) * music_category_handle->no_music_files);

         // Now allocate memory for the music filename string.
         music_category_handle->music_filenames[music_category_handle->no_music_files] = (char *)malloc(strlen(possible_music_track_full_directory_location) + 1);
         DEBUG_SECTION {
            if (music_category_handle->music_filenames[music_category_handle->no_music_files] == NULL) {
               DebugAppend("Failed to acquire memory to store a music filename.\r\n");
               DebugWrite();
               
               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_FILENAME_STRING);
            }
         }
         // Copy the level file location into this newly allocated memory.
         strcpy(music_category_handle->music_filenames[music_category_handle->no_music_files], possible_music_track_full_directory_location);

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "Copied music file location '%s' into music track slot #%d.\r\n", music_category_handle->music_filenames[music_category_handle->no_music_files], music_category_handle->no_music_files);
            DebugAppend(output_text);
            siprintf(output_text, "The char * for this music file location has value %08X\r\n", (unsigned int)music_category_handle->music_filenames[music_category_handle->no_music_files]);
            DebugAppend(output_text);
            DebugWrite();
         }

         // Increase the number of music files
         (music_category_handle->no_music_files)++;

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "RECAP: After %d music track identifications, the current state is:\r\n", music_category_handle->no_music_files);
            DebugAppend(output_text);

            // Output all of the level names as they appear in the list.
            for (int i = 0; i < music_category_handle->no_music_files; i++) {
               siprintf(output_text, "(At %08X) Music track %d of %d: %s\r\n", (unsigned int)music_category_handle->music_filenames[i], i, music_category_handle->no_music_files, music_category_handle->music_filenames[i]);
               DebugAppend(output_text);
            }

            DebugWrite();
         }
      } else continue;
      // Ignore non directories.

   } while (1);

   dirclose(directory_scan);

   DEBUG_SECTION {
      // Output all of the level names as they appear in the list.
      for (int i = 0; i < music_category_handle->no_music_files; i++) {
         char output_text[256];
         siprintf(output_text, "(At %08X) Music track %d of %d: %s\r\n", (unsigned int)music_category_handle->music_filenames[i], i, music_category_handle->no_music_files, music_category_handle->music_filenames[i]);
         DebugAppend(output_text);
         DebugWrite();
      }
   }
}

// This will properly destroy all the memory allocated during the
// initialistion of a LEMMINGS_DS_MUSIC_CATEGORY_HANDLE.
// (Don't use this on one that hasn't yet been initialised using the above function!)
// IT WON'T FREE THE STRUCT ITSELF.
void Music_DestroyMusicCategory(LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *music_category_handle) {
   for (int track = 0; track < music_category_handle->no_music_files; track++) {
      free(music_category_handle->music_filenames[track]);
   }

   free(music_category_handle->music_filenames);

   music_category_handle->music_filenames = NULL;
   music_category_handle->no_music_files  = 0;
}

// This function attempts to populate the music categories with available XM module music filenames
// This function uses DLDI FAT diropen and dirnext to find
// the names of all the level sets in the standard and custom
// directories.
void Music_SetUpXMMusicCategories() {
   // This function assumes that the music directories are in fact VALID.
   // This should be the case if the config file was properly parsed
   // and handled before this function was called.

   // Create a string to store the location of the music category.
   char music_category_location[256];
   // Blank it.
   memset(music_category_location, 0, 256);

   // Construct the path string.
   strcat(music_category_location, lemmings_ds_root_dir);
   strcat(music_category_location, LEMMINGS_DS_DIRECTORY_MUSIC_STANDARD_INGAME);

   DebugAppend("Now collating music files for the ingame music category.\r\n");
   DebugWrite();

   Music_PrepareMusicCategory(&music_category_standard_ingame_songs, music_category_location);

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Acquired %d ingame music .xm modules.\r\n", music_category_standard_ingame_songs.no_music_files);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Blank music category string.
   memset(music_category_location, 0, 256);

   // Construct the path string.
   strcat(music_category_location, lemmings_ds_root_dir);
   strcat(music_category_location, LEMMINGS_DS_DIRECTORY_MUSIC_TITLE_SCREEN);

   DebugAppend("Now collating music files for the title screen music category.\r\n");
   DebugWrite();

   Music_PrepareMusicCategory(&music_category_title_screen_songs, music_category_location);

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Acquired %d title screen music .xm modules.\r\n", music_category_title_screen_songs.no_music_files);
      DebugAppend(output_text);
      DebugWrite();
   }
}

// Function prototype for Notify Stop interprocessor event handler.
void MRD_NotifyStopHandler(MRD_NotifyStopCommand *c);

// Expose function pointer for the Notify Stop interprocessor event in command9.cpp:
extern void (*NotifyStopHandler)(MRD_NotifyStopCommand *c);

// This function pointer is required for the Notify Stop interprocessor event in command9.cpp:
void (*NotifyStopHandler)(MRD_NotifyStopCommand *c) = MRD_NotifyStopHandler;

// This holds the current state of the playing song as reported by the last
// song stop event. It is reset when a 'play a song now' call is used.
int ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING;

// Function definition
void MRD_NotifyStopHandler(MRD_NotifyStopCommand *c) {
   // When a song is stopped, report the reson in ingame_song_playing_reported_state
   ingame_song_playing_reported_state = c->stop_type;
}

// This is a lame abstraction for loading a new song from memory.
void Music_SwitchSongTo(const void *new_song_memory_location, u32 loops = 0) {
	// Stop the current song.
	CommandStopPlay();
   // Set up xm_transport to load the new song data from the specified memory location.
	xm_transport.load_memory(new_song_memory_location, &current_song);
	// Set the current song.
	CommandSetSong(current_song);
	// Play the current song the specified number of times.
	CommandStartPlay(0, loops);

   // Set song playing state report variable so we know the song is playing.
	ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING;
}

// This is a lame abstraction for loading a new song.
// This function expects to have a FULLY QUALIFIED location passed to it.
// Don't forget to make sure that any calling function constructs an appropriate path!
void Music_SwitchSongTo(const char *new_song_file_location, u32 loops = 0) {
   DebugAppend("SwitchSongTo attempting to load and start '");
   DebugAppend(new_song_file_location);
   DebugAppend("' from flash media.\r\n");
   DebugWrite();

   // Step one, try to get the filesize of the song: (if it exists)
   FILE *song_file = fopen(new_song_file_location, "rb");

   // Did we acquire the song?
   if (song_file == NULL) {
      DebugAppend("Acquisition of song from '");
      DebugAppend(new_song_file_location);
      DebugAppend("' failed.\r\n");
	
      // Set song playing state report variable to avoid critical megacrashes.
	   ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING;
	   // We're just going to have to make do without music until this is called again using
	   // a working song location argument.

      // Reset errno
      errno = 0;

      return;
   }

   // Wind the song to the end of the file, and retrieve the song file filesize.
   fseek(song_file, 0, SEEK_END);

   int song_filesize = ftell(song_file);

   // Rewind the file
   rewind(song_file);

   // Allocate memory for a memory instance of the song
   u8 *song_memory_instance = (u8 *)malloc(song_filesize);
   DEBUG_SECTION {
      if (song_memory_instance == NULL) {
         DebugAppend("Failed to acquire memory to store a loaded .xm file for analysis.\r\n");
         DebugWrite();
         
         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_XM_SONG);
      }
   }

   // Load the song into memory
   fread(song_memory_instance, song_filesize, 1, song_file);

   // Close the song file after song has been loaded.
   fclose(song_file);

   // Load the song into the NitroTracker core system using this memory chunk
   Music_SwitchSongTo(song_memory_instance, loops);

   // We don't need the memory instance of the song any more:
   free(song_memory_instance);
}

// Conditionally sets the volume based on the music preference
void Cond_CommandMRD_SetPlayerVolume(int volume) {
   // Execute normally if the music is enabled, else set it to silent.
   if (music_preference & MUSIC_PREFERENCE_MUSIC) {
      CommandMRD_SetPlayerVolume(volume);
   } else {
      CommandMRD_SetPlayerVolume(0);
   }
}

// This is the number of the currently playing song in
// the ingame level music jukebox.
int ingame_song_playing_number = 0;

// This is the length in frames after the end of one song before
// the next song in the ingame jukebox will begin playing.
#define INGAME_SONG_SILENCE_INTERMISSION_LENGTH  75

// This is the number of frames that have elapsed since the end of
// the last ingame song.
int ingame_song_silence_intermission_elapsed = 0;

#define INGAME_SONG_JUKEBOX_STATE_IDLE         0 // The jukebox is disabled.
#define INGAME_SONG_JUKEBOX_STATE_PLAYING      1 // The jukebox is playing.
#define INGAME_SONG_JUKEBOX_STATE_INTERMISSION 2 // The jukebox is changing songs.
#define INGAME_SONG_JUKEBOX_STATE_FADE         3 // The jukebox is fading from a song.

int ingame_song_jukebox_state = INGAME_SONG_JUKEBOX_STATE_IDLE;

// This function is called once per frame during the get ready, ingame,
// and results sequences. It checks to see if
void Music_HandleIngameSongJukeboxPlayback() {
   if (ingame_song_jukebox_state == INGAME_SONG_JUKEBOX_STATE_PLAYING) {
      // Has the current song reached its end?
      if (ingame_song_playing_reported_state == MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_REACHED_END_OF_SONG) {
         // Set the jukebox state to intermission.
         ingame_song_jukebox_state = INGAME_SONG_JUKEBOX_STATE_INTERMISSION;

         // Reset the intermission timer.
         ingame_song_silence_intermission_elapsed = 0;
      }
   } else
   if (ingame_song_jukebox_state == INGAME_SONG_JUKEBOX_STATE_INTERMISSION) {
      // Increase the intermission timer
      ingame_song_silence_intermission_elapsed++;

      // If the intermission timer has reached its limit:
      if (ingame_song_silence_intermission_elapsed == INGAME_SONG_SILENCE_INTERMISSION_LENGTH) {
         // Advance the song now playing, but loop it within the number of available songs.
         ingame_song_playing_number++;
         ingame_song_playing_number %= music_category_current_ingame->no_music_files;

         // Set the new song to play.                                  // Play all songs twice.
         Music_SwitchSongTo(music_category_current_ingame->music_filenames[ingame_song_playing_number], 2);

         // Set the jukebox state to playing.
         ingame_song_jukebox_state = INGAME_SONG_JUKEBOX_STATE_PLAYING;

         // Reset the song stop flag.
         ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING;
      }
   }
}

// Start up the jukebox:
void Music_KickIngameSongJukeboxPlayback(LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *music_category_to_kick_into, int track) {
   DebugAppend("Jukebox kicked.\r\n");
   DebugWrite();
   
   // The specified category is now the ingame jukebox category...
   // Intercept a kick to NULL as a call for the 'music_category_standard_ingame_songs' category
   if (music_category_to_kick_into == NULL) {
      music_category_current_ingame = &music_category_standard_ingame_songs;
   } else {
      music_category_current_ingame = music_category_to_kick_into;
   }

   // If the track given is minus one, set it to a random track from the jukebox range.
   if (track == -1) {
      track = rng_rand(music_category_current_ingame->no_music_files);
   }

   // Set the currently playing track to the parameter.
   ingame_song_playing_number = track;

   DEBUG_SECTION {
      char output_text[4096];
      siprintf(output_text, "Jukebox randomly landed on track %d.", ingame_song_playing_number);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Set the new song to play.                                                                // Play all songs twice.
   Music_SwitchSongTo(music_category_current_ingame->music_filenames[ingame_song_playing_number], 2);

   // Set the jukebox state to playing
   ingame_song_jukebox_state = INGAME_SONG_JUKEBOX_STATE_PLAYING;

   // Reset the song stop flag.
   ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING;

   // Set initial jukebox volume.
   Cond_CommandMRD_SetPlayerVolume(64);
}

// Shut down the jukebox:
void Music_RetireIngameSongJukeboxPlayback() {
   // Set the currently playing track to zero.
   ingame_song_playing_number = 0;

   // Set the jukebox state to idle
   ingame_song_jukebox_state = INGAME_SONG_JUKEBOX_STATE_IDLE;

   // Reset the song stop flag.
   ingame_song_playing_reported_state = MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_STOP_REQUESTED;

	// Stop the current song.
	CommandStopPlay();
}

// -----------------------------------------------------------

struct tagLEMMINGS_DS_LEVEL_SET_HANDLE;
typedef struct tagLEMMINGS_DS_LEVEL_SET_HANDLE LEMMINGS_DS_LEVEL_SET_HANDLE;

struct tagLEMMINGS_DS_LEVEL_HANDLE;
typedef struct tagLEMMINGS_DS_LEVEL_HANDLE LEMMINGS_DS_LEVEL_HANDLE;

typedef struct tagLEMMINGS_DS_LEVEL_CATEGORY_HANDLE {
   char *location;           // This is the full path of this category (malloced memory)

   int no_level_sets; // This holds the number of level sets within this category.
   LEMMINGS_DS_LEVEL_SET_HANDLE **level_sets; // This is a pointer to an array of (...LEVEL_SET_HANDLE *)
                                              // referencing each level set.
} LEMMINGS_DS_LEVEL_CATEGORY_HANDLE;

// This struct references a level set.
struct tagLEMMINGS_DS_LEVEL_SET_HANDLE {
   char *level_set_name;              // This is the name of the level set (malloced memory)

   int no_levels;                     // This holds the number of levels within this level set handle.
   LEMMINGS_DS_LEVEL_HANDLE **levels; // This is a pointer to an array of (...LEVEL_HANDLE *) referencing each level.
   
   LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *music_category; // This will point to a valid music category structure if this level set
                                                      // possesses a custom music soundtrack, and NULL if it doesn't.
};

// This is a function used to sort two (LEMMINGS_DS_LEVEL_SET_HANDLE *)
// It returns +1 if (a >  b)
// It returns  0 if (a == b)
// It returns -1 if (a <  b)
int LemmingsDSLevels_CompareTwoLevelSetHandlePointers(const void *a, const void *b) {
   LEMMINGS_DS_LEVEL_SET_HANDLE *one = (*((LEMMINGS_DS_LEVEL_SET_HANDLE **)(a)));
   LEMMINGS_DS_LEVEL_SET_HANDLE *two = (*((LEMMINGS_DS_LEVEL_SET_HANDLE **)(b)));

   return stricmp(one->level_set_name, two->level_set_name);
}

// This function will sort the pointers held within the level_sets array in
// a LEMMINGS_DS_LEVEL_CATEGORY_HANDLE so that the level sets appear in alphabetical order.
void LemmingsDSLevels_SortLevelSetsWithinCategory(LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *category_handle) {
   // This shouldn't happen
   if (category_handle == NULL) return;

   // Sort the pointers held within the level_sets array using qsort.
   qsort(category_handle->level_sets,
         category_handle->no_level_sets,
         sizeof(LEMMINGS_DS_LEVEL_SET_HANDLE *),
         LemmingsDSLevels_CompareTwoLevelSetHandlePointers);
}

// This struct references a single level.
struct tagLEMMINGS_DS_LEVEL_HANDLE {
   char *level_name;  // This is the name of the level (malloced memory)

   char *location;    // This should hold the full path of the level (so that a pathname need not be constructed on demand)
};                   

// This is a function used to sort two (LEMMINGS_DS_LEVEL_HANDLE *)
// It returns +1 if (a >  b)
// It returns  0 if (a == b)
// It returns -1 if (a <  b)
// It sorts their FILENAME, so we can keep the natural ordering of original and ONML levels!
int LemmingsDSLevels_CompareTwoLevelHandlePointers(const void *a, const void *b) {
   LEMMINGS_DS_LEVEL_HANDLE *one = (*((LEMMINGS_DS_LEVEL_HANDLE **)(a)));
   LEMMINGS_DS_LEVEL_HANDLE *two = (*((LEMMINGS_DS_LEVEL_HANDLE **)(b)));

   return stricmp(one->location, two->location);
}

// This function will sort the pointers held within the levels array in
// a LEMMINGS_DS_LEVEL_SET_HANDLE so that the levels appear in file-alphabetical order.
void LemmingsDSLevels_SortLevelsWithinLevelSet(LEMMINGS_DS_LEVEL_SET_HANDLE *level_set_handle) {
   // This shouldn't happen
   if (level_set_handle == NULL) return;

   // Sort the pointers held within the level_sets array using qsort.
   qsort(level_set_handle->levels,
         level_set_handle->no_levels,
         sizeof(LEMMINGS_DS_LEVEL_HANDLE *),
         LemmingsDSLevels_CompareTwoLevelHandlePointers);
}

// These handles will mastermind the storage and management of the standard and custom levels. 
LEMMINGS_DS_LEVEL_CATEGORY_HANDLE lemmings_ds_level_category_one_player_standard = {0};
LEMMINGS_DS_LEVEL_CATEGORY_HANDLE lemmings_ds_level_category_one_player_custom   = {0};

// These counters will show the total number of levels on the title screen!
unsigned int no_lemmings_ds_level_category_one_player_standard_levels = 0;
unsigned int no_lemmings_ds_level_category_one_player_custom_levels   = 0;

void LemmingsDSLevels_PrepareLevelCategory(LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *category_handle, const char *location);
void LemmingsDSLevels_PrepareLevelSet(LEMMINGS_DS_LEVEL_SET_HANDLE *level_set_handle, const char *location);

// This function checks the validity of a level on the flash media.

// It returns 0 for invalid levels
// and 1 for valid levels.
int LemmingsDSLevels_CheckValidityOfLevel(const char *location);

// This function uses DLDI FAT diropen and dirnext to find
// the names of all the level sets in the standard and custom
// directories.
void LemmingsDSLevels_GatherAndSortAllLevelCategories() {
   // This function assumes that the directories are in fact VALID.
   // This should be the case if the config file was properly parsed
   // and handled before this function was called.

   // Create a string to store the location of the level category.
   char level_category_location[256];
   // Blank it.
   memset(level_category_location, 0, 256);

   // Construct the path string.
   strcat(level_category_location, lemmings_ds_root_dir);
   strcat(level_category_location, LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_STANDARD);

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Now collating level sets for the standard category. (PTR: %08X)\r\n", (unsigned int)&lemmings_ds_level_category_one_player_standard);
      DebugAppend(output_text);
      DebugWrite();
   }

   LemmingsDSLevels_PrepareLevelCategory(&lemmings_ds_level_category_one_player_standard, level_category_location);

   // Total the number of levels in the standard level category:
   // Go through every set, adding the number of levels acquired.
   for (int set = 0; set < lemmings_ds_level_category_one_player_standard.no_level_sets; set++) {
      no_lemmings_ds_level_category_one_player_standard_levels += lemmings_ds_level_category_one_player_standard.level_sets[set]->no_levels;
   }

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Acquired %d standard category levels in %d level sets.\r\n", no_lemmings_ds_level_category_one_player_standard_levels, lemmings_ds_level_category_one_player_standard.no_level_sets);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Blank location of the level category string.
   memset(level_category_location, 0, 256);

   // Construct the path string.
   strcat(level_category_location, lemmings_ds_root_dir);
   strcat(level_category_location, LEMMINGS_DS_DIRECTORY_LEVELS_1PLAYER_CUSTOM);

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Now collating level sets for the custom category. (PTR: %08X)\r\n", (unsigned int)&lemmings_ds_level_category_one_player_custom);
      DebugAppend(output_text);
      DebugWrite();
   }

   LemmingsDSLevels_PrepareLevelCategory(&lemmings_ds_level_category_one_player_custom, level_category_location);

   // Total the number of levels in the custom level category:
   // Go through every set, adding the number of levels acquired.
   for (int set = 0; set < lemmings_ds_level_category_one_player_custom.no_level_sets; set++) {
      no_lemmings_ds_level_category_one_player_custom_levels += lemmings_ds_level_category_one_player_custom.level_sets[set]->no_levels;
   }   

   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Acquired %d custom category levels in %d level sets.\r\n", no_lemmings_ds_level_category_one_player_custom_levels, lemmings_ds_level_category_one_player_custom.no_level_sets);
      DebugAppend(output_text);
      DebugWrite();
   }
}


// This function will determine whether a certain directory
// is a good candidate for being a level set.
// Valid level sets are directories that contain at least one valid .lds level.

// Returns 0 for invalid level sets.
// Returns 1 for valid level sets.

int LemmingsDSLevels_CheckValidLevelSet(const char *location) {
   DebugAppend("Checking level set viability of '");
   DebugAppend(location);
   DebugAppend("'.\r\n");
   DebugWrite();

   // Acquire directory for scanning.
   DIR_ITER *level_set_directory_scan = diropen(location);

   // If the directory could not be acquired...
   if (level_set_directory_scan == NULL) {
      DebugAppend("Could not acquire directory '");
      DebugAppend(location);
      DebugAppend("' for level set scanning.\r\n");
      DebugWrite();
      return 0;
   }

   // Set this flag if and when a valid level file is found.
   int valid_level_found = 0;

   do {
      // String to store the read filename.
      char retrieved_filename_string[256];
      memset(retrieved_filename_string, 0, 256);

      // We are only looking for files, so
      // we will need to use a struct stat instance.
      struct stat file_stat;   
      
      // Reset errno
      errno = 0;

      // Scan for the next file.
      int d_r = dirnext(level_set_directory_scan, retrieved_filename_string, &file_stat);

      // Quit if we have reached the end of the directory, or there was an error.
      if ((d_r == -1) || (errno == ENOENT)) {
         DebugAppend("Level set viability scan concluded.\r\n");
         DebugWrite();

         // Reset errno
         errno = 0;

         break;
      }

      // If we've found the . or .., use continue.
      if ((strcmp(retrieved_filename_string, ".")  == 0)
       || (strcmp(retrieved_filename_string, "..") == 0)) continue;

      // Only interrogate files.
      if (!(file_stat.st_mode & S_IFDIR)) {
         DebugAppend(retrieved_filename_string);
         DebugAppend(" found. ");
         DebugWrite();

         // Determine if it has the lds extension:
         int has_extension = strendsin(retrieved_filename_string, ".lds");

         // If the level does not have the extension, skip it.
         if (!has_extension) {
            DebugAppend("Wrong extension: Skipping.\r\n");
            DebugWrite();
            continue;
         }

         DebugAppend("Has extension.\r\n");
         DebugWrite();

         // Construct a string to hold the full path of this file:
         char found_file_full_location[256];
         memset(found_file_full_location, 0, 256);

         strcat(found_file_full_location, location);
         strcat(found_file_full_location, retrieved_filename_string);

         DebugAppend("Testing for validity: '");
         DebugAppend(found_file_full_location);
         DebugAppend("'...\r\n");
         DebugWrite();

         // Now we test to see if the level is valid:
         if (LemmingsDSLevels_CheckValidityOfLevel(found_file_full_location)) {
            DebugAppend("File Valid.\r\n");
            DebugWrite();
         } else {
            DebugAppend("File Invalid.\r\n");
            DebugWrite();
            continue;
         }

         // If no continue was called before now, it's a valid level file.

         // Set flag to show that there's at least one valid level in here.
         valid_level_found = 1;
         // No need to loop any more.
         break;

      } else continue;
      // Ignore non files.
   } while (1);

   // Close directory.
   dirclose(level_set_directory_scan);

   return valid_level_found;
}

// This function will find all of the directories within the specified location that have valid levels,
// and populate the category handle with valid level set handles, and then prepare the level set handles.
// Then, everything will be sorted.

void LemmingsDSLevels_PrepareLevelCategory(LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *category_handle, const char *location) {
   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Collating level sets for the category at PTR: %08X\r\n", (unsigned int)category_handle);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Reset the category handle:
   category_handle->location      = NULL;

   category_handle->level_sets    = NULL;
   category_handle->no_level_sets = 0;

   // Acquire memory to store the location of this level category:
   category_handle->location = (char *)malloc(strlen(location) + 1);
   DEBUG_SECTION {
      if (category_handle->location == NULL) {
         DebugAppend("Failed to acquire memory for category location string.\r\n");
         DebugWrite();
         
         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_CATEGORY_LOCATION_STRING);
      }
   }
   // Copy the read level set directory name into this newly allocated memory.
   strcpy(category_handle->location, location);

   // TODO: I think there's a bug with realloc, so I'm mallocing this here.
   category_handle->level_sets = (LEMMINGS_DS_LEVEL_SET_HANDLE **)malloc(sizeof(LEMMINGS_DS_LEVEL_SET_HANDLE *) * 1024);
   DEBUG_SECTION {
      if (category_handle->level_sets == NULL) {
         DebugAppend("Failed to acquire memory for category 'level set' array of (LEMMINGS_DS_LEVEL_SET_HANDLE *)s.\r\n");
         DebugWrite();

         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE_PTR_ARRAY);
      }
   }

   // Open the directory specified:
   DIR_ITER *directory_scan = diropen(location);

   // If the directory could not be acquired...
   if (directory_scan == NULL) {
      DebugAppend("Could not acquire directory '");
      DebugAppend(location);
      DebugAppend("' for level set scanning.\r\n");
      DebugWrite();
      return;
   }

   DebugAppend("Scanning for available level sets in '");
   DebugAppend(location);
   DebugAppend("'.\r\n");
   DebugWrite();

   do {
      // String to store the read filename.
      char retrieved_filename_string[256];
      memset(retrieved_filename_string, 0, 256);

      // We are only looking for directories, so
      // we will need to use a struct stat instance.
      struct stat file_stat;
                       
      // Reset errno
      errno = 0;

      // Scan for the next directory.
      int d_r = dirnext(directory_scan, retrieved_filename_string, &file_stat);

      // Quit if we have reached the end of the directory, or there was an error.
      if ((d_r == -1) || (errno == ENOENT)) {
         DebugAppend("Available level set directories scan concluded.\r\n");
         DebugWrite();

         // Reset errno
         errno = 0;

         break;
      }

      // If we've found the . or .., use continue.
      if ((strcmp(retrieved_filename_string, ".")  == 0)
       || (strcmp(retrieved_filename_string, "..") == 0)) continue;

      // Only interrogate directories.
      if (file_stat.st_mode & S_IFDIR) {
         DebugAppend(retrieved_filename_string);
         DebugAppend(" identified as a possible level set directory. Scanning...\r\n");
         DebugWrite();

         // Construct a string to hold the full path of this level set:
         char possible_level_set_full_directory_location[256];
         memset(possible_level_set_full_directory_location, 0, 256);

         strcat(possible_level_set_full_directory_location, location);
         strcat(possible_level_set_full_directory_location, retrieved_filename_string);
         strcat(possible_level_set_full_directory_location, "/");

         DebugAppend("Full directory location: ");
         DebugAppend(possible_level_set_full_directory_location);
         DebugAppend("\r\n");
         DebugWrite();

         // Scan the directory to see if it contains any valid levels.
         if (!LemmingsDSLevels_CheckValidLevelSet(possible_level_set_full_directory_location)) continue;

         // If no continue was called before now, it's a valid level.
         // Increase the number of valid levels, and add it to the list

         DebugAppend("Appears to contain at least a single valid level. Adding.\r\n");
         DebugWrite();

         // If we have identified a valid directory...

         // Allocate memory for a LEMMINGS_DS_LEVEL_SET_HANDLE * for each level set.
         // Use realloc to maintain the level set pointers already specified:

         // TODO: I think there's a bug with realloc.
         //category_handle->level_sets = (LEMMINGS_DS_LEVEL_SET_HANDLE **)realloc(category_handle->level_sets, sizeof(LEMMINGS_DS_LEVEL_SET_HANDLE *) * category_handle->no_level_sets);

         // Allocate memory for the LEMMINGS_DS_LEVEL_SET_HANDLE
         // that will represent this newly found level set:
         category_handle->level_sets[category_handle->no_level_sets] = (LEMMINGS_DS_LEVEL_SET_HANDLE *)malloc(sizeof(LEMMINGS_DS_LEVEL_SET_HANDLE));
         DEBUG_SECTION {
            if (category_handle->level_sets[category_handle->no_level_sets] == NULL) {
               DebugAppend("Failed to acquire memory for a (LEMMINGS_DS_LEVEL_SET_HANDLE).\r\n");
               DebugWrite();

               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE);
            }
         }
         // Now allocate memory for this level set name string.
         category_handle->level_sets[category_handle->no_level_sets]->level_set_name = (char *)malloc(strlen(retrieved_filename_string) + 1);
         DEBUG_SECTION {
            if (category_handle->level_sets[category_handle->no_level_sets]->level_set_name == NULL) {
               DebugAppend("Failed to acquire memory for a level set name string.\r\n");
               DebugWrite();

               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_NAME_STRING);
            }
         }
         // Copy the read level set directory name into this newly allocated memory.
         strcpy(category_handle->level_sets[category_handle->no_level_sets]->level_set_name, retrieved_filename_string);

         // Gather the levels for this new level set:
         LemmingsDSLevels_PrepareLevelSet(category_handle->level_sets[category_handle->no_level_sets], possible_level_set_full_directory_location);

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "Copied level set title '%s' into level set slot #%d.\r\n", category_handle->level_sets[category_handle->no_level_sets]->level_set_name, category_handle->no_level_sets);
            DebugAppend(output_text);
            siprintf(output_text, "The char * for this level set title has value %08X\r\n", (unsigned int)category_handle->level_sets[category_handle->no_level_sets]->level_set_name);
            DebugAppend(output_text);
            DebugWrite();
         }

         // Increase the number of level sets
         (category_handle->no_level_sets)++;

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "RECAP: After %d level set identifications, the current state is:\r\n", category_handle->no_level_sets);
            DebugAppend(output_text);

            // Output all of the level names as they appear in the list.
            for (int i = 0; i < category_handle->no_level_sets; i++) {
               siprintf(output_text, "(At %08X) Item %d of %d: %s\r\n", (unsigned int)category_handle->level_sets[i]->level_set_name, i, category_handle->no_level_sets, category_handle->level_sets[i]->level_set_name);
               DebugAppend(output_text);
            }

            DebugWrite();
         }
      } else continue;
      // Ignore non directories.

   } while (1);

   dirclose(directory_scan);

   DEBUG_SECTION {
      // Output all of the level names as they appear in the list.
      for (int i = 0; i < category_handle->no_level_sets; i++) {
         char output_text[256];
         siprintf(output_text, "(At %08X) Item %d of %d: %s\r\n", (unsigned int)category_handle->level_sets[i]->level_set_name, i, category_handle->no_level_sets, category_handle->level_sets[i]->level_set_name);
         DebugAppend(output_text);
         DebugWrite();
      }
   }

   // Sort the list of level sets alphabetically.
   LemmingsDSLevels_SortLevelSetsWithinCategory(category_handle);

   DEBUG_SECTION {
      // Output all of the level names as they appear in the list.
      for (int i = 0; i < category_handle->no_level_sets; i++) {
         char output_text[256];
         siprintf(output_text, "(At %08X) Sorted Item %d of %d: %s\r\n", (unsigned int)category_handle->level_sets[i]->level_set_name, i, category_handle->no_level_sets, category_handle->level_sets[i]->level_set_name);
         DebugAppend(output_text);
         DebugWrite();
      }
      // The list should appear sorted.
   }
}

// This function will find all of the levels within the specified location that are valid,
// and use these to populate the level set handle. Then, everything will be sorted.
void LemmingsDSLevels_PrepareLevelSet(LEMMINGS_DS_LEVEL_SET_HANDLE *set_handle, const char *location) {
   DEBUG_SECTION {
      char output_text[256];
      siprintf(output_text, "Collating levels for the level set at PTR: %08X\r\n", (unsigned int)set_handle);
      DebugAppend(output_text);
      DebugWrite();
   }

   // Reset the level set handle: (except the name, which is handled externally)
   set_handle->levels         = NULL;
   set_handle->no_levels      = 0;
   set_handle->music_category = NULL;

   // TODO: I think there's a bug with realloc, so I'm mallocing this here.
   set_handle->levels    = (LEMMINGS_DS_LEVEL_HANDLE **)malloc(sizeof(LEMMINGS_DS_LEVEL_HANDLE *) * 1024);
   DEBUG_SECTION {
      if (set_handle->levels == NULL) {
         DebugAppend("Failed to acquire memory for category 'levels' array of (LEMMINGS_DS_LEVEL_HANDLE *)s.\r\n");
         DebugWrite();

         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE_PTR_ARRAY);
      }
   }

   // Open the directory specified:
   DIR_ITER *directory_scan = diropen(location);

   // If the directory could not be acquired...
   if (directory_scan == NULL) {
      DebugAppend("Could not acquire directory '");
      DebugAppend(location);
      DebugAppend("' for level scanning.\r\n");
      DebugWrite();
      return;
   }

   DebugAppend("Scanning for available levels in '");
   DebugAppend(location);
   DebugAppend("'.\r\n");
   DebugWrite();

   do {
      // String to store the read filename.
      char retrieved_filename_string[256];
      memset(retrieved_filename_string, 0, 256);

      // We are only looking for file, so
      // we will need to use a struct stat instance.
      struct stat file_stat;  
      
      // Reset errno
      errno = 0;

      // Scan for the next file.
      int d_r = dirnext(directory_scan, retrieved_filename_string, &file_stat);

      // Quit if we have reached the end of the directory, or there was an error.
      if ((d_r == -1) || (errno == ENOENT)) {
         DebugAppend("Available levels scan concluded.\r\n");
         DebugWrite();

         // Reset errno
         errno = 0;

         break;
      }

      // If we've found the . or .., use continue.
      if ((strcmp(retrieved_filename_string, ".")  == 0)
       || (strcmp(retrieved_filename_string, "..") == 0)) continue;

      // Only interrogate files.
      if (!(file_stat.st_mode & S_IFDIR)) {
         DebugAppend(retrieved_filename_string);
         DebugAppend(" identified as a possible level. Scanning...\r\n");
         DebugWrite();

         // Construct a string to hold the full path of this level:
         char possible_level_full_directory_location[256];
         memset(possible_level_full_directory_location, 0, 256);

         strcat(possible_level_full_directory_location, location);
         strcat(possible_level_full_directory_location, retrieved_filename_string);

         DebugAppend("Full level location: ");
         DebugAppend(possible_level_full_directory_location);
         DebugAppend("\r\n");
         DebugWrite();

         // Scan the level to see if it is valid.
         if (!LemmingsDSLevels_CheckValidityOfLevel(possible_level_full_directory_location)) continue;

         // Open the file (yes, again)
         FILE *level_file = fopen(possible_level_full_directory_location, "rb");

         // If the level file fails here, continue.
         if (level_file == NULL) {
            DebugAppend("Couldn't open the level file to read the level title.\r\n");
            DebugWrite();                           

            // Reset errno
            errno = 0;

            continue;
         }

         // If the file has been opened, lets read out the level name memory.
         char read_level_name_bytes[32];

         int bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.level_name));

         // First move the file pointer to the level name placement.
         fseek(level_file, bytes_offset, SEEK_SET);

         fread(read_level_name_bytes, 32, 1, level_file);

         // Don't forget to close the file after reading the necessary bytes.
         fclose(level_file);

         DebugAppend("Read level name successfully! Interpreting title... Got '");
         DebugAppend(read_level_name_bytes);
         DebugAppend("'\r\n");
         DebugWrite();

         // If no continue was called before now, it's a valid level.
         // Increase the number of valid levels, and add it to the list

         DebugAppend("Appears be a valid level. Adding.\r\n");
         DebugWrite();

         // If we have identified a valid level...

         // Allocate memory for a LEMMINGS_DS_LEVEL_HANDLE * for each level.
         // Use realloc to maintain the level pointers already specified:

         // TODO: I think there's a bug with realloc.
         //set_handle->levels = (LEMMINGS_DS_LEVEL_HANDLE **)realloc(set_handle->levels, sizeof(LEMMINGS_DS_LEVEL_HANDLE *) * set_handle->no_levels);

         // Allocate memory for the LEMMINGS_DS_LEVEL_HANDLE
         // that will represent this newly found level:
         set_handle->levels[set_handle->no_levels] = (LEMMINGS_DS_LEVEL_HANDLE *)malloc(sizeof(LEMMINGS_DS_LEVEL_HANDLE));
         DEBUG_SECTION {
            if (set_handle->levels[set_handle->no_levels] == NULL) {
               DebugAppend("Failed to acquire memory for a (LEMMINGS_DS_LEVEL_HANDLE).\r\n");
               DebugWrite();   
               
               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE);
            }
         }
         // Now allocate memory for the level location string.
         set_handle->levels[set_handle->no_levels]->location = (char *)malloc(strlen(possible_level_full_directory_location) + 1);
         DEBUG_SECTION {
            if (set_handle->levels[set_handle->no_levels]->location == NULL) {
               DebugAppend("Failed to acquire memory for a level location string.\r\n");
               DebugWrite();

               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOCATION_STRING);
            }
         }
         // Copy the level file location into this newly allocated memory.
         strcpy(set_handle->levels[set_handle->no_levels]->location, possible_level_full_directory_location);

         // Now allocate memory for the level name string.
         set_handle->levels[set_handle->no_levels]->level_name = (char *)malloc(strlen(read_level_name_bytes) + 1);
         DEBUG_SECTION {
            if (set_handle->levels[set_handle->no_levels]->level_name == NULL) {
               DebugAppend("Failed to acquire memory for a level name string.\r\n");
               DebugWrite();

               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_NAME_STRING);
            }
         }
         // Copy the level name into this newly allocated memory.
         strcpy(set_handle->levels[set_handle->no_levels]->level_name, read_level_name_bytes);

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "Copied level set title '%s' into level set slot #%d.\r\n", set_handle->levels[set_handle->no_levels]->level_name, set_handle->no_levels);
            DebugAppend(output_text);
            siprintf(output_text, "The char * for this level set title has value %08X\r\n", (unsigned int)set_handle->levels[set_handle->no_levels]->level_name);
            DebugAppend(output_text);
            DebugWrite();
         }

         // Increase the number of levels
         (set_handle->no_levels)++;

         DEBUG_SECTION {
            char output_text[256];
            siprintf(output_text, "RECAP: After %d level set identifications, the current state is:\r\n", set_handle->no_levels);
            DebugAppend(output_text);

            // Output all of the level names as they appear in the list.
            for (int i = 0; i < set_handle->no_levels; i++) {
               siprintf(output_text, "(At %08X) Item %d of %d: %s\r\n", (unsigned int)set_handle->levels[i]->level_name, i, set_handle->no_levels, set_handle->levels[i]->level_name);
               DebugAppend(output_text);
            }

            DebugWrite();
         }
      } else continue;
      // Ignore non directories.

   } while (1);

   dirclose(directory_scan);

   DEBUG_SECTION {
      // Output all of the level names as they appear in the list.
      for (int i = 0; i < set_handle->no_levels; i++) {
         char output_text[256];
         siprintf(output_text, "(At %08X) Item %d of %d: %s\r\n", (unsigned int)set_handle->levels[i]->level_name, i, set_handle->no_levels, set_handle->levels[i]->level_name);
         DebugAppend(output_text);
         DebugWrite();
      }
   }

   // Sort the list of levels alphabetically.
   LemmingsDSLevels_SortLevelsWithinLevelSet(set_handle);

   DEBUG_SECTION {
      // Output all of the level names as they appear in the list.
      for (int i = 0; i < set_handle->no_levels; i++) {
         char output_text[256];
         siprintf(output_text, "(At %08X) Sorted Item %d of %d: %s\r\n", (unsigned int)set_handle->levels[i]->level_name, i, set_handle->no_levels, set_handle->levels[i]->level_name);
         DebugAppend(output_text);
         DebugWrite();
      }
      // The list should appear sorted.
   }                                                           
   
   // Now we deal with this level's music category.     

   // Construct the location of the custom ingame music directory within the level set directory
   char custom_music_directory_location[256];
   memset(custom_music_directory_location, 0, 256);
   
   strcat(custom_music_directory_location, location);
   strcat(custom_music_directory_location, "custom_music_ingame/");

   DebugAppend("Constructed '");
   DebugAppend(custom_music_directory_location);
   DebugAppend("' in preparation for custom soundtrack scanning.\r\n");
   DebugWrite();

   // Create a temporary music category structure:
   LEMMINGS_DS_MUSIC_CATEGORY_HANDLE temporary_music_category;
   
   // Populate this temporary structure.
   Music_PrepareMusicCategory(&temporary_music_category, custom_music_directory_location);

   // If it detected some music, copy the music category into a persistent malloced structure
   // copy the pointer to this structure into this level set.
   if (temporary_music_category.no_music_files > 0) {
      set_handle->music_category = (LEMMINGS_DS_MUSIC_CATEGORY_HANDLE *)malloc(sizeof(LEMMINGS_DS_MUSIC_CATEGORY_HANDLE));

      *(set_handle->music_category) = temporary_music_category;
   } else {
      if (temporary_music_category.music_filenames != NULL) {
         // Deallocate the memory allocated by the temporary music category
         Music_DestroyMusicCategory(&temporary_music_category);
      }
   }
}

// This function checks the validity of a level on the flash media.

// It returns 0 for invalid levels
// and 1 for valid levels.
int LemmingsDSLevels_CheckValidityOfLevel(const char *location) {
   // This check will consist of these phases:
   //   1) Check that the file exists.
   //   2) Check that the file contains the correct validation_string.
   //   3) Check that the file holds the correct version number.
   //   4) Check that the file is of the correct size.
   //   5) Check that the texture archive needed to open the level exists.
   //   6) Check that the texture archive needed to open the level is valid.

   DebugAppend("Validating file:\r\n");
   DebugAppend(location);
   DebugWrite();

   // Open the file
   FILE *level_file = fopen(location, "rb");

   // Check 1, does the file exist?
   if (level_file == NULL) {
      DebugAppend("... Failure: Can't open.\r\n");
      DebugWrite();         

      // Reset errno
      errno = 0;
      
      return 0;
   }

   // If the file has been opened, lets read out the first sixteen bytes.
   u8 read_sixteen_bytes[16];
   fread(read_sixteen_bytes, 16, 1, level_file);

   DebugAppend("... Read Successfully.\r\n");
   DebugAppend("Scan results");
   DebugWrite();

   // We can use this to interrogate the read bytes properly:
   const LEMMINGS_LEVEL_LDS_FILE_V7 *read_sixteen_bytes_interpreted = (const LEMMINGS_LEVEL_LDS_FILE_V7 *)read_sixteen_bytes;

   // Check 2, do the read bytes hold the correct validation string?
   if (strcmp((const char *)read_sixteen_bytes_interpreted->validation_string, correct_validation_string) != 0) {
      DebugAppend("... Failure: Wrong validation string.\r\n");
      DebugWrite();

      fclose(level_file);
      return 0;
   }

   // Check 3, do the read bytes hold the correct version number?
   if (read_sixteen_bytes_interpreted->version_number != LEMMINGS_LEVEL_VERSION) {
      DebugAppend("... Failure: Wrong version number.\r\n");
      DebugWrite();

      fclose(level_file);
      return 0;
   }

   // Check 4, do the read bytes hold the correct filesize?
   fseek(level_file, 0, SEEK_END);
   unsigned int ftelled_level_filesize = ftell(level_file);

   if (read_sixteen_bytes_interpreted->lemmings_level_file_size != ftelled_level_filesize) {
      DebugAppend("... Failure: Wrong filesize.\r\n");
      DebugWrite();

      fclose(level_file);
      return 0;
   }
   
   // Check 5, does the texture archive necessary to render this level exist?

   // First, read the texture archive that's expected for this file
   fseek(level_file, (int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->texture_archive_using), SEEK_SET);

   char necessary_texture_archive[32];
   memset(necessary_texture_archive, 0, 32);
   fread(necessary_texture_archive, 32, 1, level_file);

   // Construct the texture archive full location string:
   char texture_archive_full_location[256];
   memset(texture_archive_full_location, 0, 256);

   // First, test for the custom texture archive...
   const char *last_slash_in_level_path = strfindlast("/", location);
   strncpy(texture_archive_full_location, location, 1 + (int)(((int)(last_slash_in_level_path)) - ((int)(location))));
   strcat(texture_archive_full_location, "custom_texture_archives/");
   strcat(texture_archive_full_location, necessary_texture_archive);
   strcat(texture_archive_full_location, ".LTA");     

   // Attempt to load the custom texture archive.
   FILE *texture_archive_file = fopen(texture_archive_full_location, "rb");

   if (texture_archive_file == NULL) {
      // Reset errno
      errno = 0;

      DebugAppend("... Working: Custom texture archive at\r\n'");
      DebugAppend(texture_archive_full_location);
      DebugAppend("' not accessible, checking standard");
      DebugWrite();

      memset(texture_archive_full_location, 0, 256);

      strcat(texture_archive_full_location, lemmings_ds_root_dir);
      strcat(texture_archive_full_location, LEMMINGS_DS_DIRECTORY_STANDARD_TEXTURE_ARCHIVES);
      strcat(texture_archive_full_location, necessary_texture_archive);
      strcat(texture_archive_full_location, ".LTA");

      // Load the texture archive.
      texture_archive_file = fopen(texture_archive_full_location, "rb");

      if (texture_archive_file == NULL) {
         DebugAppend("... Failure: Texture archive ");
         DebugAppend(texture_archive_full_location);
         DebugAppend(" not found.\r\n");
         DebugWrite();            

         // Reset errno
         errno = 0;

         fclose(level_file);
         return 0;
      }
   }

   // It's passed check 5...

   // Check 6, is the texture archive the correct filesize?
   unsigned int read_texture_archive_filesize;
   fread(&read_texture_archive_filesize, 4, 1, texture_archive_file);

   fseek(texture_archive_file, 0, SEEK_END);
   unsigned int ftelled_texture_archive_filesize = ftell(texture_archive_file);

   if (read_texture_archive_filesize != ftelled_texture_archive_filesize) {
      DebugAppend("... Failure: Wrong texture archive (");
      DebugAppend(texture_archive_full_location);
      DebugAppend(") filesize.\r\n");
      DebugWrite();

      fclose(level_file);
      fclose(texture_archive_file);
      return 0;
   }

   // If it's got all this way, it's gotta be valid.
   DebugAppend("... and it's valid!\r\n");
   DebugWrite();

   fclose(level_file);
   fclose(texture_archive_file);
   return 1;
}

// -------------------------------------------------------

const char lemmings_ds_level_set_completion_progress_file_name[]  = "progress.txt";

const char lemmings_ds_level_set_completion_progress_complete[]   = "Complete";
const char lemmings_ds_level_set_completion_progress_incomplete[] = "Incomplete";

// This function will search the specified level set's directory for 'progress.txt', malloc
// an array of u8s to store the 'won', 'not won' status of each level and populate it using
// key value pairs from 'progress.txt'
u8 *LemmingsDSSavedProgress_LoadProgressFromLevelSetFile(const LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *category_handle, const LEMMINGS_DS_LEVEL_SET_HANDLE *level_set_handle) {
   // Malloc the array we're going to populate
   u8 *progress_array = (u8 *)malloc(level_set_handle->no_levels);
   // Blank it
   memset(progress_array, 0, level_set_handle->no_levels);

   // Create a string to store the location of the level progress file.
   char level_set_progress_file_location[256];
   // Blank it.
   memset(level_set_progress_file_location, 0, 256);

   // Construct the path string.
   strcat(level_set_progress_file_location, category_handle->location);
   strcat(level_set_progress_file_location, level_set_handle->level_set_name);
   strcat(level_set_progress_file_location, "/");
   strcat(level_set_progress_file_location, lemmings_ds_level_set_completion_progress_file_name);

   // We will now attempt to load the level set completion progress file from
   // 'level_set_progress_file_location'
   DebugAppend("Attempting to load level set completion progress from:\r\n'");
   DebugAppend(level_set_progress_file_location);
   DebugAppend("'\r\n");
   DebugWrite();

   FILE *lemmings_ds_level_set_progress_file = fopen(level_set_progress_file_location, "rb");

   // Check for file loading failure.
   if (lemmings_ds_level_set_progress_file == NULL) {
      DebugAppend("Progress file loading failed. (NULL returned from fopen)\r\n");
      DebugWrite();

      // Reset errno
      errno = 0;

      return progress_array;
   }

   DebugAppend("Progress file acquired, loading.\r\n");
   DebugWrite();

   // Create the KEY_VALUE_CONTROLLER that will manage the data pairs.
   KEY_VALUE_CONTROLLER *key_value_progress = KeyValueC_Create();

   DebugAppend("Populating KEY_VALUE_CONTROLLER using file:\r\n");
   DebugWrite();

   // Populate the controller.
   if (KeyValueC_PopulateFromFile(key_value_progress, lemmings_ds_level_set_progress_file)
        == KEY_VALUE_CONTROLLER_POPULATION_FAILURE) {
      // If the file isn't found, then no levels are won.
      DebugAppend("Failed to populate.\r\n");
      DebugWrite();

      fclose(lemmings_ds_level_set_progress_file);

      KeyValueC_Destroy(key_value_progress);

      // Looks like we've got a bad file.
      return progress_array;
   } else {
      // Alright, now we should be able to retrieve the values
      // we want to know.

      // Iterate through each of the levels in the given level set, retrieve the
      // value for each level name and use these to populate the array.
      for (int level = 0; level < level_set_handle->no_levels; level++) {
         // Retrieve value
         const char *value = KeyValueC_KeyLookup(key_value_progress, level_set_handle->levels[level]->level_name);

         // If there was a value...
         if (value != NULL) {
            // Reading 'complete'
            if (strcmp(value, lemmings_ds_level_set_completion_progress_complete) == 0) {
               // Set the value in the array
               progress_array[level] = 1;
            }
         }
      }
   }              

   fclose(lemmings_ds_level_set_progress_file);

   KeyValueC_Destroy(key_value_progress);

   // Return the malloced array.
   return progress_array;
}

void LemmingsDSSavedProgress_SaveProgressToLevelSetFile(const LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *category_handle, const LEMMINGS_DS_LEVEL_SET_HANDLE *level_set_handle, const u8 *progress_array) {
   // Create the KEY_VALUE_CONTROLLER that will manage the data pairs.
   KEY_VALUE_CONTROLLER *key_value_progress = KeyValueC_Create();

   DebugAppend("Populating KEY_VALUE_CONTROLLER using progress array:\r\n");
   DebugWrite();

   // Iterate through each of the levels in the given level set,
   // using the values in progress_array, populate the key*value.
   for (int level = 0; level < level_set_handle->no_levels; level++) {
      if (progress_array[level] == 1) {
         DebugAppend("You've completed '");
         DebugAppend(level_set_handle->levels[level]->level_name);
         DebugAppend("': adding to key_value_progress.\r\n");

         KeyValueC_AddPairOrReplaceValue(key_value_progress, level_set_handle->levels[level]->level_name, lemmings_ds_level_set_completion_progress_complete);
      } else {
         DebugAppend("You've not yet completed '");
         DebugAppend(level_set_handle->levels[level]->level_name);
         DebugAppend("': adding to key_value_progress.\r\n");

         KeyValueC_AddPairOrReplaceValue(key_value_progress, level_set_handle->levels[level]->level_name, lemmings_ds_level_set_completion_progress_incomplete);
      }
   }

    // Create a string to store the location of the level progress file.
   char level_set_progress_file_location[256];
   // Blank it.
   memset(level_set_progress_file_location, 0, 256);

   // Construct the path string.
   strcat(level_set_progress_file_location, category_handle->location);
   strcat(level_set_progress_file_location, level_set_handle->level_set_name);
   strcat(level_set_progress_file_location, "/");
   strcat(level_set_progress_file_location, lemmings_ds_level_set_completion_progress_file_name);

   // We will now attempt to save the level set completion progress file to
   // 'level_set_progress_file_location'
   DebugAppend("Attempting to save level set completion progress to:\r\n'");
   DebugAppend(level_set_progress_file_location);
   DebugAppend("'\r\n");
   DebugWrite();

   FILE *lemmings_ds_level_set_progress_file = fopen(level_set_progress_file_location, "wb");

   // Check for file loading failure.
   if (lemmings_ds_level_set_progress_file == NULL) {
      DebugAppend("Progress file acquiring for save failed. (NULL returned from fopen)\r\n");
      DebugWrite();

      KeyValueC_Destroy(key_value_progress);

      // Reset errno
      errno = 0;
      
      return;
   }

   DebugAppend("Progress file acquired, saving.\r\n");
   DebugWrite();                                             

   const char *lemmings_ds_progress_file_preamble_1 =
"# Lemmings DS Progress File for Level Set: '";

   fputs(lemmings_ds_progress_file_preamble_1, lemmings_ds_level_set_progress_file);

   fputs(level_set_handle->level_set_name, lemmings_ds_level_set_progress_file);   

   const char *lemmings_ds_progress_file_preamble_2 =
"'.\r\n"
"# Modify this file manually if you really want to.\r\n"
"\r\n";

   fputs(lemmings_ds_progress_file_preamble_2, lemmings_ds_level_set_progress_file);
   
   KeyValueC_WriteAllToFile(key_value_progress, lemmings_ds_level_set_progress_file);
   
   fclose(lemmings_ds_level_set_progress_file);

   KeyValueC_Destroy(key_value_progress);
}

// -------------------------------------------------------

#define GRAPHICAL_OBJECT_ACQUISITION_FAILURE 0
#define GRAPHICAL_OBJECT_ACQUISITION_SUCCESS 1

int GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(LEMMINGS_GRAPHICAL_OBJECT_HEADER **destination_graphical_object_ptr,
                                                                                   DS_SPRITE **destination_sprite_array_ptr,
                                                                                  const char  *graphical_object_category_string,
                                                                                         u32   graphical_object_junction,
                                                     const LEMMINGS_DS_LEVEL_CATEGORY_HANDLE  *level_category_handle,
                                                          const LEMMINGS_DS_LEVEL_SET_HANDLE  *level_set_handle,
                                                                                         u16  *palette_to_set) {
   DEBUG_SECTION {
      char output_text[4096];
      siprintf(output_text, "Based on incoming: '%s', '%d' (%s); category at '%s', set '%s'.\r\n", graphical_object_category_string,
                                                                                                   graphical_object_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT,
                                                                                                (!(graphical_object_junction & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) ? "standard" : "custom",
                                                                                                   level_category_handle->location,
                                                                                                   level_set_handle->level_set_name);
                                                                                            
      DebugAppend(output_text);
      DebugWrite();
   }

   // This will store the full location of the object
   char graphical_object_full_location[256];
   // Blank it.
   memset(graphical_object_full_location, 0, 256);

   // The pathname construction is different for custom and standard graphical object loading:
   if (!(graphical_object_junction & LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT)) {
      // Construct the path string using the standard graphical objects directory
      siprintf(graphical_object_full_location, "%s%s%s_%d.lgo", lemmings_ds_root_dir,
                                                                LEMMINGS_DS_DIRECTORY_STANDARD_GRAPHICAL_OBJECTS,
                                                                graphical_object_category_string,
                                                                graphical_object_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);

      DebugAppend("Standard loading: Constructed '");
      DebugAppend(graphical_object_full_location);
      DebugAppend("'.\r\n");
      DebugWrite();
   } else {
      // Construct the path string using the custom graphical objects directory inside the level set
      // containing the level we're loading this graphcial object for.
      siprintf(graphical_object_full_location, "%s%s/custom_graphical_objects/%s_%d.lgo", level_category_handle->location,
                                                                                          level_set_handle->level_set_name,  
                                                                                          graphical_object_category_string,
                                                                                          graphical_object_junction & ~LEMMINGS_LEVEL_GENUS_JUNCTIONED_TO_CUSTOM_GENUS_BIT);
                                                                
      DebugAppend("Custom loading: Constructed '");
      DebugAppend(graphical_object_full_location);
      DebugAppend("'.\r\n");
      DebugWrite();
   }
   
   // Try to acquire the graphical object file
   FILE *graphical_object_file = fopen(graphical_object_full_location, "rb");

   // Has there been an error?
   if (graphical_object_file == NULL) {
      // There's not much we can do about this.
      DebugAppend("Graphical object loading error.\r\n");
      DebugWrite();                          

      // Reset errno
      errno = 0;

      return GRAPHICAL_OBJECT_ACQUISITION_FAILURE;
   }

   // Load the filesize from the graphical object
   u32 graphical_object_filesize;
   fread(&graphical_object_filesize, 4, 1, graphical_object_file);

   // Rewind the file
   rewind(graphical_object_file);

   // Allocate memory for the graphical object file
   (*destination_graphical_object_ptr) = (LEMMINGS_GRAPHICAL_OBJECT_HEADER *)malloc(graphical_object_filesize);
   DEBUG_SECTION {
      if ((*destination_graphical_object_ptr) == NULL) {
         DebugAppend("Failed to acquire memory for loading a graphical object file.\r\n");
         DebugWrite();

         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_GRAPHICAL_OBJECT_FILE);
      }
   }

   // Load the file into memory
   fread((*destination_graphical_object_ptr), graphical_object_filesize, 1, graphical_object_file);

   // Close the level file after graphical object has been loaded.
   fclose(graphical_object_file); 
   
   (*destination_sprite_array_ptr) = GraphicalObject_ConstructDSSpriteArray((*destination_graphical_object_ptr),
                                                                            palette_to_set);

   DEBUG_SECTION {
      DebugAppend("Graphical object loading successful, statistics:\r\n");
      char output_text[4096];
      siprintf(output_text, "Filesize: %d\r\n"
                            "Location: %s\r\n"
                            "Descriptive name: %s\r\n"
                            "Graphical object type number: %d\r\n"
                            "Dimensions: %d, %d\r\n"
                            "Frame count: %d. (%d / %d) R: %d\r\n"
                            "Handle: %d, %d\r\n"
                            "Active zone x: %d, %d\r\n"
                            "Active zone y: %d, %d\r\n", graphical_object_filesize,
                                                         graphical_object_full_location,
                                                         (*destination_graphical_object_ptr)->graphical_object_name,
                                                         (*destination_graphical_object_ptr)->graphical_object_type,
                                                         (*destination_graphical_object_ptr)->graphic_width,
                                                         (*destination_graphical_object_ptr)->graphic_height,
                                                         (*destination_graphical_object_ptr)->no_total_frames,
                                                         (*destination_graphical_object_ptr)->no_primary_frames,
                                                         (*destination_graphical_object_ptr)->no_secondary_frames,
                                                         (*destination_graphical_object_ptr)->representing_frame,
                                                         (*destination_graphical_object_ptr)->handle_x,
                                                         (*destination_graphical_object_ptr)->handle_y,
                                                         (*destination_graphical_object_ptr)->active_zone_x1,
                                                         (*destination_graphical_object_ptr)->active_zone_x2,
                                                         (*destination_graphical_object_ptr)->active_zone_y1,
                                                         (*destination_graphical_object_ptr)->active_zone_y2);

      DebugAppend(output_text);
      DebugWrite();
   }

   return GRAPHICAL_OBJECT_ACQUISITION_SUCCESS;
}

// -----------------------------------------------------------------------------

// This flag determines the display of various debug stuff. Most of it has been taken out before now.
int debug_lems = 0;

// A global copy of the runtime stats for the level currently being played.
// It's used for the lemming action handler, among other stuff.
LEMMINGS_LEVEL_RUNTIME_STATS_V7 current_level_runtime_stats = {{0}};
// This must be populated before PlayLemmingsDS_GoLemmingsLevel starts!

// level_data_prototype contains a copy of the level terrain before any changes are made to it
// When the level is rendered, it is rendered into the prototype.
// When the level is to be played, it is copied from the prototype into the actual level.
// (When it's in the actual level, it can mucked about like crazy, and just restored from the
// prototype when needed)
u8 level_data_prototype[LEVEL_X_SIZE][LEVEL_Y_SIZE] = {{0}};

// We need to malloc a LEVEL_X_SIZE * LEVEL_Y_SIZE memory area for this.
u8 (*level_data)[LEVEL_Y_SIZE] = {0};
// There is no need for a loaded level file and texture archive pair to be resident in memory
// at the same time as this level data memory chunk.

// These values are calculated when a level begins. No, Lemmings DS doesn't use the fade registers for the
// main screen ingame lemmings display fade.

// This function will return 1 if the specified pixel is within steel, or within a one-way area if the
// lemming direction is the wrong way.
inline int WithinSteel(int x, int y, int d) {
   if (x >= 0) {
      if (y >= 0) {
         if (x < LEVEL_X_SIZE) {
            if (y < LEVEL_Y_SIZE) {
               if (level_data[x][y]) {
                  for (u32 s = 0; s < current_level_runtime_stats.no_steel_areas; s++) {
                     if (x >= current_level_runtime_stats.steel_area_x1[s]) {
                        if (y >= current_level_runtime_stats.steel_area_y1[s]) {
                           if (x <= current_level_runtime_stats.steel_area_x2[s]) {
                              if (y <= current_level_runtime_stats.steel_area_y2[s]) {
                                 return 1;
                              }
                           }
                        }
                     }
                  }
                  if (d != -1) { // Ignore non directional attempts. They always succeed.
                     for (u32 o = 0; o < current_level_runtime_stats.no_one_way_areas; o++) {
                        if (((u8)(d)) != current_level_runtime_stats.one_way_area_d[o]) {
                          if (x >= current_level_runtime_stats.one_way_area_x1[o]) {
                             if (y >= current_level_runtime_stats.one_way_area_y1[o]) {
                                 if (x <= current_level_runtime_stats.one_way_area_x2[o]) {
                                    if (y <= current_level_runtime_stats.one_way_area_y2[o]) {
                                       return 1;
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return 0;
}

// These defines control what colours the interactive parts of the overview map display are.
// The lemming brick colour also appears here.
// One way walls are the responsibility of the level author, not Lemmings DS!
#define SPECIAL_COLOUR_BUILDER_BRICK   253
#define SPECIAL_COLOUR_MAP_LEMMING     254
#define SPECIAL_COLOUR_MAP_BORDER      255

// Safe level subtraction. Only subtracts if the target is inside the level and not steel.
inline void ssub(int x, int y, int d) {
   if (x >= 0) {
      if (y >= 0) {
         if (x < LEVEL_X_SIZE) {
            if (y < LEVEL_Y_SIZE) {     // Dont safesub outside the level!!!
               if (!WithinSteel(x, y, d)) {
                  level_data[x][y] = 0;
               }
            }
         }
      }
   }
}

// Safe Mask Away. Relative to the hotspot, try to subtract the level at these coordinates.
#define sma(xc, yc) ssub(((x) + (dx) * (xc)), ((y) + (yc)), d)
// Nop doesn't do anything
#define nop(xc, yc)
// The 'hotspot' is the coordinates (x, y). This is a macro, so we can do this!
// dx is the direction into which to extend the added coordinate.
// If it's 1, then we're working toward the right, so the x coordinate increases with xc.
// If it's -1, then we're working toward the left.

// Safe level addition. Only adds (eg. lemming bricks) if the target is inside the level and not steel.
inline void sadd(int x, int y, int c) {
   if (x >= 0) {
      if (y >= 0) {
         if (x < LEVEL_X_SIZE) {
            if (y < LEVEL_Y_SIZE) {     // Dont safeadd outside the level!!!
               if (!WithinSteel(x, y, -1)) {
                  level_data[x][y] = (u8)c;
               }
            }
         }
      }
   }
}

// -----------------------------------------------------------------------------

// This struct is used to hold information relating to a preview of a level.
typedef struct tagLEMMINGS_LEVEL_VITAL_STATISTICS {
   char  level_name[32]; // Thirty characters,  null terminated.
   char description[64]; // Sixty three characters,  null terminated.
   
   u32  lemmings;
   u32  to_be_saved;
   u32  release_rate;
   u32  time_in_minutes;
   char rating_description[16]; // Fifteen characters, null terminated
} LEMMINGS_LEVEL_VITAL_STATISTICS;

// This array holds the data loaded for the level preview from a level file
u8  level_preview_data_chunk[LEVEL_PREVIEW_DATA_X_SIZE * LEVEL_PREVIEW_DATA_Y_SIZE] = {0};
u16 level_preview_palette[256] = {0};

void VitalStatistics_ExtractLemmingsLevelVitalStatisticsAndMapPreviewFromLoadedLevel(LEMMINGS_LEVEL_VITAL_STATISTICS *vital_statistics, LEMMINGS_LEVEL_LDS_FILE_V7 *loaded_level_file) {
   vital_statistics->lemmings        = loaded_level_file->runtime_stats.lemmings;
   vital_statistics->to_be_saved     = loaded_level_file->runtime_stats.to_be_saved;
   vital_statistics->release_rate    = loaded_level_file->runtime_stats.release_rate;
   vital_statistics->time_in_minutes = loaded_level_file->runtime_stats.time_in_minutes;

   strcpy(vital_statistics->level_name,         loaded_level_file->runtime_stats.level_name);
   strcpy(vital_statistics->description,        loaded_level_file->runtime_stats.description);
   strcpy(vital_statistics->rating_description, loaded_level_file->runtime_stats.rating_description);

   memcpy(level_preview_data_chunk, loaded_level_file->preview_data,  LEVEL_PREVIEW_DATA_X_SIZE * LEVEL_PREVIEW_DATA_Y_SIZE);
   memcpy(level_preview_palette,    loaded_level_file->runtime_stats.level_palette, sizeof(u16) * 256);
}

void VitalStatistics_ExtractLemmingsLevelVitalStatisticsAndMapPreviewFromLevelFilename(LEMMINGS_LEVEL_VITAL_STATISTICS *vital_statistics, const char *level_location) {
   // Open the file
   FILE *level_file = fopen(level_location, "rb");

   // If the level file fails here, damn!
   if (level_file == NULL) {
      DebugAppend("Couldn't open ");
      DebugAppend(level_location);
      DebugAppend(" to extract vital statistics.\r\n");
      DebugWrite();

      LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_RETRIEVE_VITAL_STATISTICS,
                                    level_location + strlen(level_location) - 30);
   }

   DebugAppend("Opened ");
   DebugAppend(level_location);
   DebugAppend(" to extract vital statistics.\r\n");
   DebugWrite();

   int bytes_offset;

   // Wind the file to the preview data location, then read the preview data bytes
   bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->preview_data));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(level_preview_data_chunk, LEVEL_PREVIEW_DATA_X_SIZE * LEVEL_PREVIEW_DATA_Y_SIZE, 1, level_file);

   bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.level_palette));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(level_preview_palette, sizeof(u16) * 256, 1, level_file);

   // Next, wind the file to the level name, description and rating description locations
   // and read them.
   bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.level_name));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(vital_statistics->level_name, 32, 1, level_file);

   bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.description));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(vital_statistics->description, 64, 1, level_file);

   bytes_offset = ((int)(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.rating_description));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(vital_statistics->rating_description, 16, 1, level_file);

   // --------------

   bytes_offset = ((int)(&(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.lemmings)));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(&(vital_statistics->lemmings)       , 4, 1, level_file);

   bytes_offset = ((int)(&(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.to_be_saved)));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(&(vital_statistics->to_be_saved)    , 4, 1, level_file);

   bytes_offset = ((int)(&(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.release_rate)));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(&(vital_statistics->release_rate)   , 4, 1, level_file);

   bytes_offset = ((int)(&(((LEMMINGS_LEVEL_LDS_FILE_V7 *)(0))->runtime_stats.time_in_minutes)));
   fseek(level_file, bytes_offset, SEEK_SET);
   fread(&(vital_statistics->time_in_minutes), 4, 1, level_file);

   // --------------

   // Don't forget to close the file after reading the necessary bytes.
   fclose(level_file);
}

// -----------------------------------------------------------------------------

// These defines control the number of animation frames for each lemming animation.
// To add another animation, you have to add another one of these.

// These also affect how long a lemming can linger in a certain state for before changing
// to another.
#define NO_LEM_FRAMES_WALKER          8 // I'm walking
#define NO_LEM_FRAMES_DIGGER         16 // I'm digging
#define NO_LEM_FRAMES_BASHER         30 // I'm bashing
#define NO_LEM_FRAMES_MINER          22 // I'm mining
#define NO_LEM_FRAMES_BUILDER        16 // I'm building
#define NO_LEM_FRAMES_BUILDSHRUG      7 // I'm out of bricks. Click me quick.
#define NO_LEM_FRAMES_FALLER          4 // I'm falling
#define NO_LEM_FRAMES_UMBRELLAWHIP    4 // I'm getting out my umbrella
#define NO_LEM_FRAMES_FLOATER         8 // I'm floating
#define NO_LEM_FRAMES_CLIMBER         8 // I'm climbing
#define NO_LEM_FRAMES_ALLEYOOP        8 // I'm vaulting onto a platform
#define NO_LEM_FRAMES_BLOCKER        16 // I'm blocking
#define NO_LEM_FRAMES_SMOOSH         16 // I'm smooshed against the ground
#define NO_LEM_FRAMES_EARPULL        16 // I'm pulling my head(?)
#define NO_LEM_FRAMES_LEAVING         8 // I'm exiting the level
#define NO_LEM_FRAMES_POP            32 // I'm falling down as dust
#define NO_LEM_FRAMES_EXPLODE_NUMBER  5
#define NO_LEM_FRAMES_EXPLODE         1 // I'm pop
#define NO_LEM_FRAMES_DROWNING       16 // I'm drowning away
#define NO_LEM_FRAMES_BURNT          14 // I'm burning up

// These structures hold information about drawing a specific lemming animation frame to the backbuffer.
DS_SPRITE lemgfx_walker[NO_LEM_FRAMES_WALKER];
DS_SPRITE lemgfx_digger[NO_LEM_FRAMES_DIGGER];
DS_SPRITE lemgfx_basher[NO_LEM_FRAMES_BASHER];
DS_SPRITE lemgfx_miner[NO_LEM_FRAMES_MINER];
DS_SPRITE lemgfx_builder[NO_LEM_FRAMES_BUILDER];
DS_SPRITE lemgfx_buildshrug[NO_LEM_FRAMES_BUILDSHRUG];
DS_SPRITE lemgfx_faller[NO_LEM_FRAMES_FALLER];
DS_SPRITE lemgfx_umbrellawhip[NO_LEM_FRAMES_UMBRELLAWHIP];
DS_SPRITE lemgfx_floater[NO_LEM_FRAMES_FLOATER];
DS_SPRITE lemgfx_blocker[NO_LEM_FRAMES_BLOCKER];
DS_SPRITE lemgfx_climber[NO_LEM_FRAMES_CLIMBER];
DS_SPRITE lemgfx_alleyoop[NO_LEM_FRAMES_ALLEYOOP];
DS_SPRITE lemgfx_smoosh[NO_LEM_FRAMES_SMOOSH];
DS_SPRITE lemgfx_earpull[NO_LEM_FRAMES_EARPULL];
DS_SPRITE lemgfx_leaving[NO_LEM_FRAMES_LEAVING];
DS_SPRITE lemgfx_explode;
DS_SPRITE lemgfx_drowning[NO_LEM_FRAMES_DROWNING];
DS_SPRITE lemgfx_burnt[NO_LEM_FRAMES_BURNT];

DS_SPRITE lemgfx_explodenum[5];

// This is a debugging sprite the size of one square.
const u8  bonk_data   = 1;
const u16 bonk_pal[2] = {0xffff, RGB15A(31, 0, 0)};
const DS_SPRITE bonk = {&bonk_data, 1, 1, bonk_pal};

// This struct holds information about a single lemming.
typedef struct tagLEMMING_INFO_STRUCT {
   s32 x;
   s32 y;
   u32 d; // direction, 1 is right, 0 is left

#define LEMSTATE_WALKER              1
#define LEMSTATE_BLOCKER             2
#define LEMSTATE_BUILDER             3
#define LEMSTATE_BUILDSHRUG          4
#define LEMSTATE_BASHER              5
#define LEMSTATE_MINER               6
#define LEMSTATE_DIGGER              7

#define LEMSTATE_OFFGROUND_FLAG     512 /* The following states are off the ground. */
                                        // They can be tested for by anding with the offground flag.
#define LEMSTATE_CLIMBING           (LEMSTATE_OFFGROUND_FLAG | 1)
#define LEMSTATE_ALLEYOOP           (LEMSTATE_OFFGROUND_FLAG | 2)
#define LEMSTATE_FALLER             (LEMSTATE_OFFGROUND_FLAG | 3)
#define LEMSTATE_UMBRELLAWHIP       (LEMSTATE_OFFGROUND_FLAG | 4)
#define LEMSTATE_FLOATER            (LEMSTATE_OFFGROUND_FLAG | 5)

#define LEMSTATE_UNINTERACTIVE_FLAG 2048 /* The following states are immutable by the player. */
                                         // They can be tested for by anding with the uninteractive flag.
#define LEMSTATE_DONTEXIST          (LEMSTATE_UNINTERACTIVE_FLAG | 1)
#define LEMSTATE_SMOOSHED           (LEMSTATE_UNINTERACTIVE_FLAG | 2)
#define LEMSTATE_EARPULL            (LEMSTATE_UNINTERACTIVE_FLAG | 3)
#define LEMSTATE_POP                (LEMSTATE_UNINTERACTIVE_FLAG | 4)
#define LEMSTATE_LEAVING            (LEMSTATE_UNINTERACTIVE_FLAG | 5)
#define LEMSTATE_DROWNING           (LEMSTATE_UNINTERACTIVE_FLAG | 6)
#define LEMSTATE_BURNT              (LEMSTATE_UNINTERACTIVE_FLAG | 7)
   u32 state;

   u32 animframe;

#define LEMFLAG_CLIMBER_POWER   0x01
#define LEMFLAG_FLOATER_POWER   0x02
#define LEMFLAG_ATHLETE         (LEMFLAG_CLIMBER_POWER | LEMFLAG_FLOATER_POWER)
#define LEMFLAG_FUSELIT         0x04
#define LEMFLAG_BLOCKING        0x08 // A lemming can be blocking while earpull!
#define LEMFLAG_EXPLODE_BY_NUKE 0x10 // If this is set, then they won't go OH NO when they pull ears.
   u32 flags;

   s32 fusetimer; // In frames, not seconds.!

   u32 falldistance;

#define LEMMING_STANDARD_BRICK_COMPLEMENT 12
   u32 rembricks;
} LEMMING_INFO_STRUCT;

// This struct handles your status during *and after* playing a lemmings ds level.
// Only -persistent- stuff need live in here.
typedef struct tagLEVEL_STATUS_STRUCT {
   u32 lemmings_saved;
   u32 status_flags;

#define LEVEL_STATUS_FLAG_LEVEL_WON 1
#define LEVEL_STATUS_FLAG_RESTART   2  // If this is set, then the game won't go to status screen.
#define LEVEL_STATUS_FLAG_TIME_UP   4
} LEVEL_STATUS_STRUCT;

// These structs hold the state of active objects during gameplay:

// There's only genus of entrance, and each instance is identical to the rest:
typedef struct tagACTIVE_ENTRANCE_GENUS_STATUS_STRUCT {
   u32 animframe; // The currently displayed frame of all entrances.
} ACTIVE_ENTRANCE_GENUS_STATUS_STRUCT;

// There's only genus of exit, and each instance is identical to the rest:
typedef struct tagACTIVE_EXIT_GENUS_STATUS_STRUCT {
   u32 animframe; // The current animation frame of all exits. (If they're animated.)
   u32 animframe_subframe; // How many frames has the current frame been displayed for?
} ACTIVE_EXIT_GENUS_STATUS_STRUCT;

// This struct holds information about a -single trap-.
typedef struct tagACTIVE_TRAP_STATUS_STRUCT {
// These are the possible states of the trap.
#define TRAP_STATUS_STATE_IDLE    0 // Idle and invisible
#define TRAP_STATUS_STATE_WORKING 1 // Squishing some poor chap.
   u32 state;

   u32 animframe;
   u32 animframe_subframe; // How many frames has the current frame been displayed for?
} ACTIVE_TRAP_STATUS_STRUCT;

// This struct holds information about a single genus of hazard. (Each instance within a genus is identical)
typedef struct tagACTIVE_HAZARD_GENUS_STATUS_STRUCT {
   u32 animframe;
   u32 animframe_subframe; // How many frames has the current frame been displayed for?
} ACTIVE_HAZARD_GENUS_STATUS_STRUCT;

// This struct holds information about a single genus of uninteractive. (Each instance within a genus is identical)
typedef struct tagACTIVE_UNINTERACTIVE_GENUS_STATUS_STRUCT {
   u32 animframe;        
   u32 animframe_subframe; // How many frames has the current frame been displayed for?
} ACTIVE_UNINTERACTIVE_GENUS_STATUS_STRUCT;

// This struct holds information about all waters. (All waters are identical)
typedef struct tagACTIVE_WATER_GENUS_STATUS_STRUCT {
   u32 animframe;     
   u32 animframe_subframe; // How many frames has the current frame been displayed for?
} ACTIVE_WATER_GENUS_STATUS_STRUCT;

// --------------------------------------------------------------------

typedef struct tagLEVEL_INGAME_STATUS_STRUCT {
#define RELEASE_RATE_MINIMUM  1
#define RELEASE_RATE_MAXIMUM 99
   u32 release_rate_minimum;
   u32 release_rate;

#define TOOL_CLIMBER  0
#define TOOL_FLOATER  1
#define TOOL_EXPLODER 2
#define TOOL_BLOCKER  3
#define TOOL_BUILDER  4
#define TOOL_BASHER   5
#define TOOL_MINER    6
#define TOOL_DIGGER   7

#define NO_TOOLS      8
   s32 current_tool; // signed due to the way the tool selection code works.
   u32 remaining_tools[NO_TOOLS];

// This measures how long a second is in LEMMING LOGIC TICK FRAMES.
// ORIGINAL GUESS BASED ON WINUAE RECORDING #define LEMMING_FRAMES_PER_SECOND_REMAINING_TIME 16
// MORE RECENT ESTIMATE BASED ON COMPARISON OF LEVELS
#define LEMMING_FRAMES_PER_SECOND_REMAINING_TIME 17
   u32 time_remaining;

//#define MAX_LEMMINGS 100 // Historically 100.
#define MAX_LEMMINGS 999 // *Forget* 100.
   LEMMING_INFO_STRUCT lemming_info[MAX_LEMMINGS];    

   // This holds the number of lemmings that have been saved.
   u32 lemmings_exited;          

   // This holds the number of lemmings that have dropped from the trapdoors.
   u32 lemmings_dispensed;

   // This holds the ID of the next lemming that should be nuked.
   // If it holds the value NO_LEMMING_NUKE, then no nuke has been called.
#define NO_LEMMING_NUKE  -1
   int next_lemming_to_nuke;

   // This is a frame counter counting the number of frames since the last
   // lemming logic tick. The number of display frames necessary to update the
   // lemming logic will change depending on fast-forward, etc.
   u32 lemming_update_time;

#define LEMMING_UPDATE_FRAMES 3

   // This holds the number of frames (in lemming logic ticks) since the last lemming release.
   u32 lemming_release_timer;

   // This holds the number of the next entrance to create a lemming from.
   u32 next_entrance_to_use;

   // This is set to 1 if master pause (the PAWS) is enabled.
   u32 master_pause_flag;

   ACTIVE_ENTRANCE_GENUS_STATUS_STRUCT      entrance_state;
   ACTIVE_EXIT_GENUS_STATUS_STRUCT          exit_state;
   ACTIVE_TRAP_STATUS_STRUCT                trap_state[MAX_NO_TRAPS];
   ACTIVE_HAZARD_GENUS_STATUS_STRUCT        hazard_state[NO_HAZARD_GENUSES];
   ACTIVE_UNINTERACTIVE_GENUS_STATUS_STRUCT uninteractive_state[NO_UNINTERACTIVE_GENUSES];
   ACTIVE_WATER_GENUS_STATUS_STRUCT         water_state;
} LEVEL_INGAME_STATUS_STRUCT;

// These are the frame totals required for releasing the next lemming.
const u8 release_rate_value[]    = {53,53,52,52,51,51,50,50,49,49, // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
                                    48,48,47,47,46,46,45,45,44,44,
                                    43,43,42,42,41,41,40,40,39,39,
                                    38,38,37,37,36,36,35,35,34,34,
                                    33,33,32,32,31,31,30,30,29,29,
                                    28,28,27,27,26,26,25,25,24,24,
                                    23,23,22,22,21,21,20,20,19,19,
                                    18,18,17,17,16,16,15,15,14,14,
                                    13,13,12,12,11,11,10,10, 9, 9,
                                     8, 8, 7, 7, 6, 6, 5, 5, 4, 4, //90,91,92,93,94,95,96,97,98,99
                                     3, 1                        };//100.101

// --------------------------------------------------------------------

// These rules control how lemmings interact with their environment.

#define LEMMING_MOUNT_CAPABILITY          6 // How big a wall can a lemming instantly mount before turning around or climbing?
#define LEMMING_DESCEND_CAPABILITY        3 // How big a hole can a lemming instantly step down before turning into a faller?
#define LEMMING_BASHER_DESCEND_CAPABILITY 2 // How far can a basher 'slide' downwards before giving up.
#define LEMMING_FALL_SPEED                3 // How many pixels can a lemming fall in a single frame?
#define LEMMING_FLOATER_FALL_SPEED        2 // How fast does a floater fall per frame?
#define LEMMING_FALL_SMOOSH_THRESHHOLD   62 // How many pixels can a lemming fall before smooshing?
#define LEMMING_UMBRELLAWHIP_THRESHOLD   24 // How many pixels must a lemming fall before pulling out his brolly?
#define LEMMING_ALLEYOOP_REACH            7 // How many pixels must a climber be within the peak of his wall to vault over it?
#define LEMMING_CLIMB_HEADHIT_REACH       6 // Builder's collision with the ceiling range.

// These defines control the range through the ether that the blockers block effect works.
// Blockers can block only so many spaces above them, so we need to define the vertical range also.
#define LEMMING_BLOCKER_VERTICAL_TOP_RANGE     (-10)
#define LEMMING_BLOCKER_VERTICAL_BOTTOM_RANGE  2

#define LEMMING_BLOCKER_LEFT_RANGE_FAR    (-9)
#define LEMMING_BLOCKER_LEFT_RANGE_NEAR   (-3)
#define LEMMING_BLOCKER_RIGHT_RANGE_FAR   8
#define LEMMING_BLOCKER_RIGHT_RANGE_NEAR  2
// Right facing blockers will have their hotspot on the left hand side of the screen, because
// that's on their lower back.

// Basher remaining dirt to bash collision check box.
#define LEMMING_BASHER_CHECK_ARMS_LENGTH  2
#define LEMMING_BASHER_CHECK_X_SIZE       4
#define LEMMING_BASHER_CHECK_FAR_BOUND_X  ((LEMMING_BASHER_CHECK_ARMS_LENGTH + LEMMING_BASHER_CHECK_X_SIZE) - 1)
#define LEMMING_BASHER_CHECK_Y_SIZE       4

// Basher steel collision check box.
#define LEMMING_BASHER_STEEL_CHECK_ARMS_LENGTH  1
#define LEMMING_BASHER_STEEL_CHECK_X_SIZE       5
#define LEMMING_BASHER_STEEL_CHECK_FAR_BOUND_X  ((LEMMING_BASHER_STEEL_CHECK_ARMS_LENGTH + LEMMING_BASHER_STEEL_CHECK_X_SIZE) - 1)
#define LEMMING_BASHER_STEEL_CHECK_Y_SIZE       8

// Miner steel collision check box.
#define LEMMING_MINER_STEEL_CHECK_Y_SIZE       6 // This is the y size of the steel check performed for the miner
#define LEMMING_MINER_STEEL_CHECK_Y_DESCENT    3 // This is the y depression into the ground for the miner steel check.
#define LEMMING_MINER_STEEL_CHECK_ARMS_LENGTH  0
#define LEMMING_MINER_STEEL_CHECK_FAR_BOUND_X  7

// This is the size of the box in front of a builder to check for a headbutt.
// It starts at lemming_x + x_start and ends at lemming_x + x_end
#define LEMMING_BUILDER_HEADBUTT_X_START   1
#define LEMMING_BUILDER_HEADBUTT_X_END     3
#define LEMMING_BUILDER_HEADBUTT_Y_SIZE   9

// This is the size of the box for checking if a lemming is too close to steel to assign a basher to.
#define LEMMING_BASHER_ASSIGN_STEEL_CHECK_X_SIZE 4
#define LEMMING_BASHER_ASSIGN_STEEL_CHECK_Y_SIZE 6

// This is the size of the box for checking if a lemming is too close to steel to assign a miner to.
#define LEMMING_MINER_ASSIGN_STEEL_CHECK_Y_SIZE       6 // This is the y size of the steel check performed for the miner
#define LEMMING_MINER_ASSIGN_STEEL_CHECK_Y_DESCENT    3 // This is the y depression into the ground for the miner steel check.
#define LEMMING_MINER_ASSIGN_STEEL_CHECK_ARMS_LENGTH  0
#define LEMMING_MINER_ASSIGN_STEEL_CHECK_FAR_BOUND_X  7

// These defines control where the hotspot can be found on the lemming animation sprites.
// (It's almost always the lemmings feet on their back.)
// The animations are designed so the lemming logic is centered around this point.
// The lemming animation frame sizes are here also.
#define LEM_HANDLE_X_WALKER  2
#define LEM_HANDLE_Y_WALKER  9
#define LEM_WALKER_X_SIZE    6
#define LEM_WALKER_Y_SIZE   10

#define LEM_HANDLE_X_DIGGER  8
#define LEM_HANDLE_Y_DIGGER 11
#define LEM_DIGGER_X_SIZE   16
#define LEM_DIGGER_Y_SIZE   12

#define LEM_HANDLE_X_BASHER 11
#define LEM_HANDLE_Y_BASHER  8
#define LEM_BASHER_X_SIZE   17
#define LEM_BASHER_Y_SIZE    9

#define LEM_HANDLE_X_MINER   7
#define LEM_HANDLE_Y_MINER  12
#define LEM_MINER_X_SIZE    16
#define LEM_MINER_Y_SIZE    14

#define LEM_HANDLE_X_BUILDER  3
#define LEM_HANDLE_Y_BUILDER 12
#define LEM_BUILDER_X_SIZE    8
#define LEM_BUILDER_Y_SIZE   13

#define LEM_HANDLE_X_BUILDSHRUG  3
#define LEM_HANDLE_Y_BUILDSHRUG  9
#define LEM_BUILDSHRUG_X_SIZE    8
#define LEM_BUILDSHRUG_Y_SIZE   10

#define LEM_HANDLE_X_FALLER  2
#define LEM_HANDLE_Y_FALLER  8
#define LEM_FALLER_X_SIZE    6
#define LEM_FALLER_Y_SIZE   10

#define LEM_HANDLE_X_UMBRELLAWHIP  3
#define LEM_HANDLE_Y_UMBRELLAWHIP 13
#define LEM_UMBRELLAWHIP_X_SIZE    9
#define LEM_UMBRELLAWHIP_Y_SIZE   14

#define LEM_HANDLE_X_FLOATER  4
#define LEM_HANDLE_Y_FLOATER 13
#define LEM_FLOATER_X_SIZE    9
#define LEM_FLOATER_Y_SIZE   16

#define LEM_HANDLE_X_CLIMBER  7
#define LEM_HANDLE_Y_CLIMBER 10
#define LEM_CLIMBER_X_SIZE    9
#define LEM_CLIMBER_Y_SIZE   11

#define LEM_HANDLE_X_ALLEYOOP  4
#define LEM_HANDLE_Y_ALLEYOOP 15
#define LEM_ALLEYOOP_X_SIZE    7
#define LEM_ALLEYOOP_Y_SIZE   16

#define LEM_HANDLE_X_BLOCKER  4
#define LEM_HANDLE_Y_BLOCKER  9
#define LEM_BLOCKER_X_SIZE   10
#define LEM_BLOCKER_Y_SIZE   10

#define LEM_HANDLE_X_SMOOSH  7
#define LEM_HANDLE_Y_SMOOSH  9
#define LEM_SMOOSH_X_SIZE   16
#define LEM_SMOOSH_Y_SIZE   10

#define LEM_HANDLE_X_EARPULL  3
#define LEM_HANDLE_Y_EARPULL  9
#define LEM_EARPULL_X_SIZE    8
#define LEM_EARPULL_Y_SIZE   10

#define LEM_HANDLE_X_LEAVING  2
#define LEM_HANDLE_Y_LEAVING 12
#define LEM_LEAVING_X_SIZE    6
#define LEM_LEAVING_Y_SIZE   13

#define LEM_HANDLE_X_EXPLODE_NUMBER  2
#define LEM_HANDLE_Y_EXPLODE_NUMBER 15
#define LEM_EXPLODE_NUMBER_X_SIZE    6
#define LEM_EXPLODE_NUMBER_Y_SIZE    5

#define LEM_HANDLE_X_EXPLODE  17
#define LEM_HANDLE_Y_EXPLODE  20
#define LEM_EXPLODE_X_SIZE    32
#define LEM_EXPLODE_Y_SIZE    32

#define LEM_HANDLE_X_POP  17
#define LEM_HANDLE_Y_POP  20
#define LEM_POP_X_SIZE 32
#define LEM_POP_Y_SIZE 32

#define LEM_HANDLE_X_DROWNING  3
#define LEM_HANDLE_Y_DROWNING  7
#define LEM_DROWNING_X_SIZE    8
#define LEM_DROWNING_Y_SIZE    8

#define LEM_HANDLE_X_BURNT     5
#define LEM_HANDLE_Y_BURNT    13
#define LEM_BURNT_X_SIZE      12
#define LEM_BURNT_Y_SIZE      14

// This function uses a macro to populate the big list of DS_SPRITEs up there with
// the appropriate data, allowing the DS_SPRITE drawing functions to draw sprites to
// the main screen backbuffer thing.
// These sprites will always be in memory.
void SetUpPermanentLemmingSprites() {
   int frame = 0;

   #define LEMGFX_SPRITE_SET_UP_MACRO(array_small, defines_big)                                                   \
         for (frame = 0; frame < NO_LEM_FRAMES_##defines_big; frame++) {                                          \
            lemgfx_##array_small[frame].data    = (gfx_##array_small##Bitmap +                                    \
                                                 frame * LEM_##defines_big##_X_SIZE * LEM_##defines_big##_Y_SIZE);\
                                                                                                                  \
            lemgfx_##array_small[frame].width   = LEM_##defines_big##_X_SIZE;                                     \
            lemgfx_##array_small[frame].height  = LEM_##defines_big##_Y_SIZE;                                     \
            lemgfx_##array_small[frame].palette = lemgfx_palette;                                    \
         }                                                                                                        \

   // All of these use the same palette, the one in the macro, lemgfx_palette.
   LEMGFX_SPRITE_SET_UP_MACRO(      walker,         WALKER);
   LEMGFX_SPRITE_SET_UP_MACRO(      digger,         DIGGER);
   LEMGFX_SPRITE_SET_UP_MACRO(      basher,         BASHER);
   LEMGFX_SPRITE_SET_UP_MACRO(       miner,          MINER);
   LEMGFX_SPRITE_SET_UP_MACRO(     builder,        BUILDER);
   LEMGFX_SPRITE_SET_UP_MACRO(  buildshrug,     BUILDSHRUG);
   LEMGFX_SPRITE_SET_UP_MACRO(      faller,         FALLER);
   LEMGFX_SPRITE_SET_UP_MACRO(umbrellawhip,   UMBRELLAWHIP);
   LEMGFX_SPRITE_SET_UP_MACRO(     floater,        FLOATER);
   LEMGFX_SPRITE_SET_UP_MACRO(     climber,        CLIMBER);
   LEMGFX_SPRITE_SET_UP_MACRO(    alleyoop,       ALLEYOOP);
   LEMGFX_SPRITE_SET_UP_MACRO(     blocker,        BLOCKER);
   LEMGFX_SPRITE_SET_UP_MACRO(      smoosh,         SMOOSH);
   LEMGFX_SPRITE_SET_UP_MACRO(     earpull,        EARPULL);
   LEMGFX_SPRITE_SET_UP_MACRO(     leaving,        LEAVING);
   LEMGFX_SPRITE_SET_UP_MACRO(  explodenum, EXPLODE_NUMBER);
   LEMGFX_SPRITE_SET_UP_MACRO(    drowning,       DROWNING);
   LEMGFX_SPRITE_SET_UP_MACRO(       burnt,          BURNT);

   lemgfx_explode.data    = gfx_explodeBitmap;
   lemgfx_explode.width   = LEM_EXPLODE_X_SIZE;
   lemgfx_explode.height  = LEM_EXPLODE_Y_SIZE;
   lemgfx_explode.palette = lemgfx_palette;
}

// This draws a 'bonk' (a 1x1 red square) at the coordinates x, y.
void DrawDebugBonkAtLevelCoords(int x, int y) {
   int xd, yd;

   xd = (x << log2magnification) - camera_x_inset;
   yd = (y << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&bonk, xd, yd, 0, 0);
}

// --------------------------------------------------------------------

// These will point to malloced memory contaning a memory copy of the 
// junctioned graphical object files for the current level.
// Between levels, they will contain NULL.

LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_exit                                    =  NULL;
LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_entrance                                =  NULL;
LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_trap[NO_TRAP_GENUSES]                   = {NULL};
LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_hazard[NO_HAZARD_GENUSES]               = {NULL};
LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_uninteractive[NO_UNINTERACTIVE_GENUSES] = {NULL};
LEMMINGS_GRAPHICAL_OBJECT_HEADER *active_graphical_object_water                                   =  NULL;

DS_SPRITE *active_sprite_exit                                    =  NULL;
DS_SPRITE *active_sprite_entrance                                =  NULL;
DS_SPRITE *active_sprite_trap[NO_TRAP_GENUSES]                   = {NULL};
DS_SPRITE *active_sprite_hazard[NO_HAZARD_GENUSES]               = {NULL};
DS_SPRITE *active_sprite_uninteractive[NO_UNINTERACTIVE_GENUSES] = {NULL};
DS_SPRITE *active_sprite_water                                   =  NULL;

// --------------------------------------------------------------------

// The following functions draw various interactive level items to the screen.
// They also handle the relevant transformation to get their scrolled-magnified
// position relative to the camera_*_inset
void DrawEntrance(int frame, int x, int y) {
   int xd, yd;

   xd = ((x - active_graphical_object_entrance->handle_x) << log2magnification) - camera_x_inset;
   yd = ((y - active_graphical_object_entrance->handle_y) << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&active_sprite_entrance[frame], xd, yd, 0, 0);
}

void DrawExit(int frame, int x, int y) {
   int xd, yd;

   xd = ((x - active_graphical_object_exit->handle_x) << log2magnification) - camera_x_inset;
   yd = ((y - active_graphical_object_exit->handle_y) << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&active_sprite_exit[frame], xd, yd, 0, 0);
}

// This draws a water area between x1, x2 on the line y. (These measurements are in level coordinates.)
void DrawWaterArea(int frame, int x1, int x2, int y) {
   int xd, yd;

   int water_width = (x2 - x1);

   yd = ((y - active_graphical_object_water->handle_y) << log2magnification) - camera_y_inset;
   if ((yd > 0) && ((yd - ((active_graphical_object_water->handle_y << log2magnification) - 1)) < SCREEN_HEIGHT)) {
      u32 texture_position = 0;

      for (int x = 0; x <= water_width; x++) {
         xd = ((x + x1) << log2magnification) - camera_x_inset;

         texture_position++;

         if (texture_position == active_graphical_object_water->graphic_width) {
            texture_position = 0;
         }

         DS_DrawSpriteStripWMag(&active_sprite_water[frame], xd, yd, texture_position, current_level_runtime_stats.water_palette);
      }
   }
}

void DrawTrap(int genus, int frame, int x, int y, bool trap_working) {
   int xd, yd;

   xd = ((x - active_graphical_object_trap[genus]->handle_x) << log2magnification) - camera_x_inset;
   yd = ((y - active_graphical_object_trap[genus]->handle_y) << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&active_sprite_trap[genus][frame], xd, yd, 0, 0);
}

void DrawHazard(int genus, int frame, int x, int y) {
   int xd, yd;

   xd = ((x - active_graphical_object_hazard[genus]->handle_x) << log2magnification) - camera_x_inset;
   yd = ((y - active_graphical_object_hazard[genus]->handle_y) << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&active_sprite_hazard[genus][frame], xd, yd, 0, 0);
}

void DrawUninteractive(int genus, int frame, int x, int y) {
   int xd, yd;

   xd = ((x - active_graphical_object_uninteractive[genus]->handle_x) << log2magnification) - camera_x_inset;
   yd = ((y - active_graphical_object_uninteractive[genus]->handle_y) << log2magnification) - camera_y_inset;
   DS_DrawSpriteWMag(&active_sprite_uninteractive[genus][frame], xd, yd, 0, 0);
}

// This draws the ingame level_data that's visible within the current
// 'scroll window' (based on the current magnification and scroll)
// to the main screen backbuffer
void DrawLevel() {
   int xp, yp;
   int xd, yd;

   int ld;

   for (xp = 0; xp < GAME_DISPLAY_X_SIZE; xp++) {
      for (yp = 0; yp < GAME_DISPLAY_Y_SIZE; yp++) {
         xd = (camera_x_inset + xp) >> log2magnification;
         yd = (camera_y_inset + yp) >> log2magnification;

         ld = level_data[xd][yd];
         if (ld != 0) {
            DrawPixel(xp, yp, current_level_runtime_stats.level_palette[ld]);
         }
      }
   }
}


// This splatters a palette all over the screen for debugging.
void DebugDrawAPalette(u16 *palette_to_debug) {
   int xp, yp;

   for (xp = 0; xp < 256; xp++) {
      for (yp = 0; yp < 150; yp++) {
         DrawPixel(xp, yp, palette_to_debug[xp>>3]);
      }
   }
}

// -----------------------------------------------------------------------------

// This set of functions sends messages to the subscreen during the level select-play-result loop.
// This means that the select-play-result modules don't have to worry about the contents of the sub
// screen directly. (That could get messy. Fast.)
void SetUpSubScreenAsLevelInfo();
void UpdateSubScreenAsLevelInfo_WriteLevelInfo(const LEMMINGS_LEVEL_VITAL_STATISTICS *level_vital_statistics, bool level_won);
void UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(const u16 *palette);
void UpdateSubScreenAsLevelInfo_UpdateMap(int camera_x_inset, int camera_y_inset, bool draw_border, bool draw_lems, LEMMING_INFO_STRUCT *lemming_info);
void UpdateSubScreenAsLevelInfo_BlankMap();
void UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData();

#define UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_NOTHING     0x00
#define UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME 0x01
#define UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP         0x02
#define UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_OTHER_STUFF 0x04
#define UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING  0xFF
void UpdateSubScreenAsLevelInfo_SetFadeObjects(int objects);

void UpdateSubScreenAsLevelInfo_SetFadeValue(int fade_value);
void UpdateSubScreenAsLevelInfo_UpdateInOutTime(u32 out, u32 in, u32 no, u32 time_remaining);
void UpdateSubScreenAsLevelInfo_BlankInOutTime();

// -----------------------------------------------------------------------------

// How many colours of exploded lemming dust are there?
#define NO_LEMMING_DUST_COLOURS 18

// The lemming dust colours are determined by the current level palette, not the current lemming palette.
u16 lemming_dust_palette[NO_LEMMING_DUST_COLOURS] = {0};

// This controls how many particles of dust are shot out when a lemming explodes.
#define NO_LEMMING_DUST_PARTICLES (72)

// These control the position of a lemming dust particle as it flies along its path.
// Every lemming explodes in the same pattern for the duration of a level.
s32 lemming_dust_position_x[NO_LEM_FRAMES_POP][NO_LEMMING_DUST_PARTICLES] = {{0}};
s32 lemming_dust_position_y[NO_LEM_FRAMES_POP][NO_LEMMING_DUST_PARTICLES] = {{0}};
s32 lemming_dust_colour[NO_LEMMING_DUST_PARTICLES] = {0};

// This function runs a quick simulation of NO_LEMMING_DUST_PARTICLES being shot
// out from a central nexus. These results are then placed into the lemming_dust_position_*
// arrays so lemmings can have dust animations.
void LemmingDustSimulation() {
   // Copy the colours from the current level palette.
   lemming_dust_palette[ 0] = lemgfx_palette[1];
   lemming_dust_palette[ 1] = lemgfx_palette[2];

   lemming_dust_palette[ 2] = current_level_runtime_stats.level_palette[ 1] | 0x8000;
   lemming_dust_palette[ 3] = current_level_runtime_stats.level_palette[ 2] | 0x8000;
   lemming_dust_palette[ 4] = current_level_runtime_stats.level_palette[ 3] | 0x8000;
   lemming_dust_palette[ 5] = current_level_runtime_stats.level_palette[ 4] | 0x8000;
   lemming_dust_palette[ 6] = current_level_runtime_stats.level_palette[ 5] | 0x8000;
   lemming_dust_palette[ 7] = current_level_runtime_stats.level_palette[ 6] | 0x8000;
   lemming_dust_palette[ 8] = current_level_runtime_stats.level_palette[ 7] | 0x8000;
   lemming_dust_palette[ 9] = current_level_runtime_stats.level_palette[ 8] | 0x8000;
   lemming_dust_palette[10] = current_level_runtime_stats.level_palette[ 9] | 0x8000;
   lemming_dust_palette[11] = current_level_runtime_stats.level_palette[10] | 0x8000;
   lemming_dust_palette[12] = current_level_runtime_stats.level_palette[11] | 0x8000;
   lemming_dust_palette[13] = current_level_runtime_stats.level_palette[12] | 0x8000;
   lemming_dust_palette[14] = current_level_runtime_stats.level_palette[13] | 0x8000;
   lemming_dust_palette[15] = current_level_runtime_stats.level_palette[14] | 0x8000;
   lemming_dust_palette[16] = current_level_runtime_stats.level_palette[15] | 0x8000;
   lemming_dust_palette[17] = current_level_runtime_stats.level_palette[16] | 0x8000;

   // Set the colours.
   for (int p = 0; p < NO_LEMMING_DUST_PARTICLES; p++) {
      lemming_dust_colour[p] = RandomInt(NO_LEMMING_DUST_COLOURS);
   }

// The lemming dust simluation coordinates are in 64ths of pixel!!

#define LEMMING_DUST_SIMULATION_GRAVITY       37
#define LEMMING_DUST_X_STARTING_VELOCITY_MAX 270
#define LEMMING_DUST_Y_STARTING_VELOCITY_MAX 660

#define LEMMING_DUST_HI_TO_LOW_RES_SHIFT 6

   s32 hi_res_particle_pos_x;
   s32 hi_res_particle_pos_y;
   s32 hi_res_particle_vel_x;
   s32 hi_res_particle_vel_y;

   for (int p = 0; p < NO_LEMMING_DUST_PARTICLES; p++) {
      hi_res_particle_pos_x = hi_res_particle_pos_y = 0;

      // Random velocity.
      hi_res_particle_vel_x = ((RandomInt(LEMMING_DUST_X_STARTING_VELOCITY_MAX * 2)) - LEMMING_DUST_X_STARTING_VELOCITY_MAX);
      hi_res_particle_vel_y = (0 - RandomInt(LEMMING_DUST_Y_STARTING_VELOCITY_MAX));

      for (int f = 0; f < NO_LEM_FRAMES_POP; f++) {
         hi_res_particle_vel_y += LEMMING_DUST_SIMULATION_GRAVITY;

         hi_res_particle_pos_x += hi_res_particle_vel_x;
         hi_res_particle_pos_y += hi_res_particle_vel_y;

         // Store the high res particle positions into the arrays, low detail.
         lemming_dust_position_x[f][p] = hi_res_particle_pos_x >> LEMMING_DUST_HI_TO_LOW_RES_SHIFT;
         lemming_dust_position_y[f][p] = hi_res_particle_pos_y >> LEMMING_DUST_HI_TO_LOW_RES_SHIFT;
      }
   }
}

// This draws a single lemmings dust frame onto the backbuffer, using the specified palette (for fading).
void DrawLemmingDustFrame(LEMMING_INFO_STRUCT *lemming) {
   u32     xd ,     yd ; // Current position of lemming dust.
   s32     xdm,     ydm; // Counters for iterating through the pixels in magnification.
   u32 dst_x,   dst_y;   // Destination pixel on screen.
   u32 p; // Particle counter
   u32 particle_colour;

   for (p = 0; p < NO_LEMMING_DUST_PARTICLES; p++) {
      // Get the upper left pixel position on screen for this dust particle.
      xd = ((lemming->x + lemming_dust_position_x[lemming->animframe][p]) << log2magnification) - camera_x_inset;
      yd = ((lemming->y + lemming_dust_position_y[lemming->animframe][p]) << log2magnification) - camera_y_inset;

      particle_colour = lemming_dust_palette[lemming_dust_colour[p]];

      // While on screen.
      if ((((xd + magnification - 1) > 0) && (xd < SCREEN_WIDTH ))
       && (((yd + magnification - 1) > 0) && (yd < SCREEN_HEIGHT))) {
         // Draw the dust particle (with loops for magnification)
         for (ydm = 0; ydm < magnification; ydm++) {
            dst_y = yd + ydm;
            if ((dst_y > 0) && (dst_y < SCREEN_HEIGHT)) {
               for (xdm = 0; xdm < magnification; xdm++) {
                  dst_x = xd + xdm;
                  if ((dst_x > 0) && (dst_x < SCREEN_WIDTH)) {
                     DrawPixel(xd + xdm, yd + ydm, particle_colour);
                  }
               }
            }
         }
      }
   }
}

// This draws a single lemming onto the backbuffer using the specified palette.
// It will change the palette used if the lemming is currently selected.
// (It'll go red and yellow, using lemgfx_palette_selected)
void DrawLemming(LEMMING_INFO_STRUCT *lemming, int lemming_selected) {
   const u16 *palette;

   if (lemming_selected) {
      palette = lemgfx_palette_selected;
   } else {
      palette = lemgfx_palette;
   }

   // Find the state the lemming is in, do the appropriate transformation to find the upper-left
   // SCREEN COORDINATE pixel position for the sprite, and then draw the sprite as magnified by
   // the current magnification.
   if (lemming->state != LEMSTATE_DONTEXIST) {
      int xd, yd;

      if (lemming->state == LEMSTATE_WALKER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_WALKER) << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_WALKER - LEM_WALKER_X_SIZE + 1) << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_WALKER)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_walker[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_walker[lemming->animframe], xd, yd, palette, 1);
         }

      } else if (lemming->state == LEMSTATE_BLOCKER) {
         xd = ((lemming->x - LEM_HANDLE_X_BLOCKER + lemming->d - 1) << log2magnification) - camera_x_inset;
         yd = ((lemming->y - LEM_HANDLE_Y_BLOCKER) << log2magnification) - camera_y_inset;
         DS_DrawSpriteWMag(&lemgfx_blocker[lemming->animframe], xd, yd, palette, 0);

      } else if (lemming->state == LEMSTATE_DIGGER) {
         xd = ((lemming->x - LEM_HANDLE_X_DIGGER) << log2magnification) - camera_x_inset;
         yd = ((lemming->y - LEM_HANDLE_Y_DIGGER) << log2magnification) - camera_y_inset;
         DS_DrawSpriteWMag(&lemgfx_digger[lemming->animframe], xd, yd, palette, 0);

      } else if (lemming->state == LEMSTATE_BASHER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_BASHER) << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_BASHER - LEM_BASHER_X_SIZE + 1) << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_BASHER) << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_basher[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_basher[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_MINER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_MINER) << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_MINER - LEM_MINER_X_SIZE + 1) << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_MINER) << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_miner[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_miner[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_BUILDER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_BUILDER)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_BUILDER - LEM_BUILDER_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_BUILDER)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_builder[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_builder[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_BUILDSHRUG) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_BUILDSHRUG)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_BUILDSHRUG - LEM_BUILDSHRUG_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_BUILDSHRUG)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_buildshrug[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_buildshrug[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_FALLER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_FALLER)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_FALLER - LEM_FALLER_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_FALLER)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_faller[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_faller[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_UMBRELLAWHIP) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_UMBRELLAWHIP)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_UMBRELLAWHIP - LEM_UMBRELLAWHIP_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_UMBRELLAWHIP)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_umbrellawhip[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_umbrellawhip[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_FLOATER) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_FLOATER)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_FLOATER - LEM_FLOATER_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_FLOATER)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_floater[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_floater[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_CLIMBING) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_CLIMBER)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_CLIMBER - LEM_CLIMBER_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_CLIMBER)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_climber[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_climber[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_ALLEYOOP) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_ALLEYOOP)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_ALLEYOOP - LEM_ALLEYOOP_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_ALLEYOOP)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_alleyoop[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_alleyoop[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_SMOOSHED) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_SMOOSH)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_SMOOSH - LEM_SMOOSH_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_SMOOSH)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_smoosh[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_smoosh[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_EARPULL) {
         xd = ((lemming->x - LEM_HANDLE_X_EARPULL + lemming->d - 1)  << log2magnification) - camera_x_inset;
         yd = ((lemming->y - LEM_HANDLE_Y_EARPULL)  << log2magnification) - camera_y_inset;
         DS_DrawSpriteWMag(&lemgfx_earpull[lemming->animframe], xd, yd, palette, 0);

      } else if (lemming->state == LEMSTATE_LEAVING) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_LEAVING)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_LEAVING - LEM_LEAVING_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_LEAVING)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_leaving[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_leaving[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_POP) {
         if (lemming->animframe == 0) {
            xd = ((lemming->x - LEM_HANDLE_X_POP)  << log2magnification) - camera_x_inset;
            yd = ((lemming->y - LEM_HANDLE_Y_POP)  << log2magnification) - camera_y_inset;
            DS_DrawSpriteWMag(&lemgfx_explode, xd, yd, palette, 0);
         } else {
            DrawLemmingDustFrame(lemming);
         }
      } else if (lemming->state == LEMSTATE_DROWNING) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_DROWNING)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_DROWNING - LEM_DROWNING_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_DROWNING)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_drowning[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_drowning[lemming->animframe], xd, yd, palette, 1);
         }
      } else if (lemming->state == LEMSTATE_BURNT) {
         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_BURNT)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_BURNT - LEM_BURNT_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_BURNT)  << log2magnification) - camera_y_inset;

         if (lemming->d == 1) {
            DS_DrawSpriteWMag(&lemgfx_burnt[lemming->animframe], xd, yd, palette, 0);
         } else {
            DS_DrawSpriteWMag(&lemgfx_burnt[lemming->animframe], xd, yd, palette, 1);
         }
      }

      // If the fuse is lit on a lemming, draw its number also.
      if ((lemming->flags & LEMFLAG_FUSELIT) && (lemming->state != LEMSTATE_EARPULL)
                                             && (lemming->state != LEMSTATE_POP)) {
         u32 timer_showing = IntDiv(lemming->fusetimer, LEMMING_FRAMES_PER_SECOND_REMAINING_TIME) + 1;

         (timer_showing == 6) ? timer_showing = 5 : 0;

         if (lemming->d == 1) {
            xd = ((lemming->x - LEM_HANDLE_X_EXPLODE_NUMBER)  << log2magnification) - camera_x_inset;
         } else {
            xd = ((lemming->x + LEM_HANDLE_X_EXPLODE_NUMBER - LEM_EXPLODE_NUMBER_X_SIZE + 1)  << log2magnification) - camera_x_inset;
         }
         yd = ((lemming->y - LEM_HANDLE_Y_EXPLODE_NUMBER)  << log2magnification) - camera_y_inset;

         DS_DrawSpriteWMag(&lemgfx_explodenum[timer_showing - 1], xd, yd, palette, 0);
      }
   }
}

// -----------------------------------------------------------------------------

// Calculate the number of lemmings currently within the field (including popped lemmings)
int LemmingsOut(const LEVEL_INGAME_STATUS_STRUCT *ingame_status) {
   u32 lemmings = 0;
   for (u32 l = 0; l < current_level_runtime_stats.lemmings; l++) {
      if (ingame_status->lemming_info[l].state != LEMSTATE_DONTEXIST) {
         lemmings++;
      }
   }
   return lemmings;
}

// Try and figure out if the specified tool can be used on the specified lemming.
int ToolAllowedOnLemming(const LEVEL_INGAME_STATUS_STRUCT *ingame_status, LEMMING_INFO_STRUCT *lemming, int tool) {
   // Get a pointer to the lemming info struct
   const LEMMING_INFO_STRUCT *lemming_info = ingame_status->lemming_info;

   // You can only use stuff on interactive lemmings.
   if (!(lemming->state & LEMSTATE_UNINTERACTIVE_FLAG)) {
      // Attempting to use tools that aren't exploder on the blocker is goofy.
      if (lemming->state != LEMSTATE_BLOCKER) {
         // Only climb, float and explode skills work on falling lemmings.
         if (!(lemming->state & LEMSTATE_OFFGROUND_FLAG)) {
            if (tool == TOOL_BLOCKER) {
               // Do blocker applicable check.
               if (lemming->state != LEMSTATE_BLOCKER) {
                  // You can't apply a blocker to a lemming which is inside the reach of an existing blocker.

                  const LEMMING_INFO_STRUCT *blocker_lem;
                  s32 left_facing, yp, xp;

                  for (u32 blocker_lem_index = 0; blocker_lem_index < current_level_runtime_stats.lemmings; blocker_lem_index++) {
                     // Get ptr to blocker lem to check
                     blocker_lem = &lemming_info[blocker_lem_index];
                     if (blocker_lem->state != LEMSTATE_DONTEXIST) { // Better safe than sorry...
                        // Left facing holds the offset to add if the blocker is facing to the left and no the right.
                       left_facing = (blocker_lem->d == 1) ? 1 : 0;
                        // Is this lem a blocker?
                        if (blocker_lem->flags & LEMFLAG_BLOCKING) {
                           xp = lemming->x;
                           yp = lemming->y;
                           if ((yp >= LEMMING_BLOCKER_VERTICAL_TOP_RANGE    + blocker_lem->y)
                            && (yp <= LEMMING_BLOCKER_VERTICAL_BOTTOM_RANGE + blocker_lem->y)) {
                              if ((xp >= LEMMING_BLOCKER_LEFT_RANGE_FAR   + blocker_lem->x + left_facing)
                               && (xp <= LEMMING_BLOCKER_RIGHT_RANGE_FAR  + blocker_lem->x + left_facing)) {
                                 return 0; // No dice, you're inside a blocker.
                                 // Notice the difference here, the near range is never mentioned!
                                 // In lemmings, the walkers can walk through the middle of the blocker
                                 // and pop out the other side, and then the blocker acts as a one way wall.
                                 // But, you can't make a blocker inside another... not even within the two
                                 // thin areas!
                              }
                           }
                        }
                     }
                  }

                  return 1;
               } else {
                  return 0;
               }
            }
            if (tool == TOOL_BUILDER) {
               // Do builder applicable check.
               if (lemming->state != LEMSTATE_BUILDER) {
                  return 1;
               } else {
                  return 0;
               }
            }
            if (tool == TOOL_BASHER) {
               // Do basher applicable check.
               if (lemming->state != LEMSTATE_BASHER) {
                  s32 bash_check_abs_x;
                  s32 bash_check_abs_y;

                  // The direction into which the check extends horizontally depends on the
                  // direction the lemming is facing.
                  s32 dx = ((lemming->d == 1) ? (1) : (-1));

                  // This is the ZERO IN FRONT CHECK.
                  for (int bash_check_rel_y = 0; bash_check_rel_y < LEMMING_BASHER_ASSIGN_STEEL_CHECK_Y_SIZE; bash_check_rel_y++) {
                     bash_check_abs_y = lemming->y - bash_check_rel_y;

                     for (int bash_check_rel_x = 0; bash_check_rel_x <= LEMMING_BASHER_ASSIGN_STEEL_CHECK_X_SIZE; bash_check_rel_x++) {
                        bash_check_abs_x = lemming->x + (dx * bash_check_rel_x);

                        if (WithinSteel(bash_check_abs_x, bash_check_abs_y, lemming->d)) {
                           // Can't assign a basher to this guy.

                           // SOUND REQUEST : steel hit
                           DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                                         0,
                                                                         lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                                         camera_x_inset);                  // This is the position of the left of the screen in maggies.
                           // Extra added bonus: if this click propogates through multiple lemmings,
                           // the dupe request elimination together with the pan snapping will
                           // mean that no more than two steel hit sounds will be played for any one
                           // assignment run. Cool, eh?

                           return 0;
                        }
                     }
                  }

                  return 1;
               } else {
                  return 0;
               }
            }
            if (tool == TOOL_MINER) {
               // Do miner applicable check.
               if (lemming->state != LEMSTATE_MINER) {
                  // The direction into which the check extends horizontally depends on the
                  // direction the lemming is facing.
                  s32 dx = ((lemming->d == 1) ? (1) : (-1));

                  // Do the steel check here!
                  // This is the steel check
                  s32 mine_check_abs_x;
                  s32 mine_check_abs_y;

                  for (int mine_check_rel_y = 0; mine_check_rel_y < LEMMING_MINER_ASSIGN_STEEL_CHECK_Y_SIZE; mine_check_rel_y++) {
                     mine_check_abs_y = lemming->y - mine_check_rel_y + LEMMING_MINER_STEEL_CHECK_Y_DESCENT;

                     for (int mine_check_rel_x = LEMMING_MINER_ASSIGN_STEEL_CHECK_ARMS_LENGTH; mine_check_rel_x <= LEMMING_MINER_ASSIGN_STEEL_CHECK_FAR_BOUND_X; mine_check_rel_x++) {
                        mine_check_abs_x = lemming->x + (dx * mine_check_rel_x);

                        if (mine_check_abs_x >= 0) {
                           if (mine_check_abs_x < LEVEL_X_SIZE) {
                              if (mine_check_abs_y >= 0) {
                                 if (mine_check_abs_y < LEVEL_Y_SIZE) {
                                    if (WithinSteel(mine_check_abs_x, mine_check_abs_y, lemming->d)) {
                                       // Can't assign a miner to this guy.

                                       // SOUND REQUEST : steel hit
                                       DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                                                     0,
                                                                                     lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                                                     camera_x_inset);                  // This is the position of the left of the screen in maggies.s
                                       // Extra added bonus: if this click propogates through multiple lemmings,
                                       // the dupe request elimination together with the pan snapping will
                                       // mean that no more than two steel hit sounds will be played for any one
                                       // assignment run. Cool, eh?

                                       return 0;
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  return 1;
               } else {
                  return 0;
               }
            }
            if (tool == TOOL_DIGGER) {
               // Do digger applicable check.
               if (lemming->state != LEMSTATE_DIGGER) {
                  if (WithinSteel(lemming->x, lemming->y+1, lemming->d)) {
                     // Can't assign a digger to this guy.

                     // SOUND REQUEST : steel hit
                     DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                                   0,
                                                                   lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                                   camera_x_inset);                  // This is the position of the left of the screen in maggies.
                     // Extra added bonus: if this click propogates through multiple lemmings,
                     // the dupe request elimination together with the pan snapping will
                     // mean that no more than two steel hit sounds will be played for any one
                     // assignment run. Cool, eh?

                     return 0;
                  }
                  return 1;
               } else {
                  return 0;
               }
            }
         }

         if (tool == TOOL_CLIMBER) {
            // Do climber applicable check.
            if (!(lemming->flags & LEMFLAG_CLIMBER_POWER)) {
               return 1;
            } else {
               return 0;
            }
         }
         if (tool == TOOL_FLOATER) {
            // Do floater applicable check.
            if (!(lemming->flags & LEMFLAG_FLOATER_POWER)) {
               return 1;
            } else {
               return 0;
            }
         }
      }

      if (tool == TOOL_EXPLODER) {
         // Do exploder applicable check.
         if (!(lemming->flags & LEMFLAG_FUSELIT)) {
            return 1;
         } else {
            return 0;
         }
      }
   }
   return 0;
}

// Try to assign the specified tool to the specified lemming.
int AssignLemmingTool(const LEVEL_INGAME_STATUS_STRUCT *ingame_status, LEMMING_INFO_STRUCT* lemming, int tool) {
   if (ToolAllowedOnLemming(ingame_status, lemming, tool) == 1) {
      if (tool == TOOL_CLIMBER) {
         lemming->flags |= LEMFLAG_CLIMBER_POWER;
         return 1;
      }
      if (tool == TOOL_FLOATER) {
         lemming->flags |= LEMFLAG_FLOATER_POWER;
         return 1;
      }
      if (tool == TOOL_EXPLODER) {
         lemming->flags     |= LEMFLAG_FUSELIT;
         // The lemming explosion time is five seconds.
         lemming->fusetimer  = 5 * LEMMING_FRAMES_PER_SECOND_REMAINING_TIME;
         return 1;
      }
      if (tool == TOOL_BLOCKER) {
         lemming->state      = LEMSTATE_BLOCKER;
         lemming->animframe  = 0;
         lemming->flags     |= LEMFLAG_BLOCKING;
         return 1;
      }
      if (tool == TOOL_BUILDER) {
         lemming->animframe = NO_LEM_FRAMES_BUILDER - 1;
         lemming->state     = LEMSTATE_BUILDER;
         lemming->rembricks = LEMMING_STANDARD_BRICK_COMPLEMENT;
         return 1;
      }
      if (tool == TOOL_BASHER) {
         lemming->animframe = NO_LEM_FRAMES_BASHER - 1;
         lemming->state     = LEMSTATE_BASHER;
         return 1;
      }
      if (tool == TOOL_MINER) {
         lemming->animframe = NO_LEM_FRAMES_MINER - 1;
         lemming->state     = LEMSTATE_MINER;
         return 1;
      }
      if (tool == TOOL_DIGGER) {
         lemming->animframe = 0;
         lemming->state     = LEMSTATE_DIGGER;
         return 1;
      }
   }

   return 0;
}

// Would this particular point (in screen space!!!!!!) count as clicking on the specified lemming?
int CursorColDecLemming(POINT *p, LEMMING_INFO_STRUCT *l) {
   s32 maggies_x = (p->x + camera_x_inset);
   s32 maggies_y = (p->y + camera_y_inset);

   // Figure out what direction 'forwards' is for the current lemming.
   s32 dx = ((l->d == 1) ? (1) : (-1));

   s32 fb = 0, bb = 0, ub = 0, lb = 0; // front back upper and lower bounds.

#define LEMMING_COLDEC_WALKER_U_BOUND        7 // How many pixels lemspace above the lemming for coldec.
#define LEMMING_COLDEC_WALKER_L_BOUND        0 // How many pixels lemspace below the lemming for coldec.
#define LEMMING_COLDEC_WALKER_F_BOUND        1 // How many pixels lemspace in front of the lemming for coldec.
#define LEMMING_COLDEC_WALKER_B_BOUND        0 // How many pixels lemspace behind the lemming for coldec.

#define LEMMING_COLDEC_BLOCKER_U_BOUND       8
#define LEMMING_COLDEC_BLOCKER_L_BOUND       0
#define LEMMING_COLDEC_BLOCKER_F_BOUND       3
#define LEMMING_COLDEC_BLOCKER_B_BOUND       2

#define LEMMING_COLDEC_BUILDER_U_BOUND       7
#define LEMMING_COLDEC_BUILDER_L_BOUND       0
#define LEMMING_COLDEC_BUILDER_F_BOUND       1
#define LEMMING_COLDEC_BUILDER_B_BOUND       0

#define LEMMING_COLDEC_BUILDSHRUG_U_BOUND    7
#define LEMMING_COLDEC_BUILDSHRUG_L_BOUND    0
#define LEMMING_COLDEC_BUILDSHRUG_F_BOUND    1
#define LEMMING_COLDEC_BUILDSHRUG_B_BOUND    0

#define LEMMING_COLDEC_BASHER_U_BOUND        7
#define LEMMING_COLDEC_BASHER_L_BOUND        0
#define LEMMING_COLDEC_BASHER_F_BOUND        2
#define LEMMING_COLDEC_BASHER_B_BOUND        2

#define LEMMING_COLDEC_MINER_U_BOUND         7
#define LEMMING_COLDEC_MINER_L_BOUND         0
#define LEMMING_COLDEC_MINER_F_BOUND         2
#define LEMMING_COLDEC_MINER_B_BOUND         0

#define LEMMING_COLDEC_DIGGER_U_BOUND        4
#define LEMMING_COLDEC_DIGGER_L_BOUND        0
#define LEMMING_COLDEC_DIGGER_F_BOUND        3
#define LEMMING_COLDEC_DIGGER_B_BOUND        3

#define LEMMING_COLDEC_CLIMBING_U_BOUND      7
#define LEMMING_COLDEC_CLIMBING_L_BOUND      (0-1)
#define LEMMING_COLDEC_CLIMBING_F_BOUND      0
#define LEMMING_COLDEC_CLIMBING_B_BOUND      4

#define LEMMING_COLDEC_ALLEYOOP_U_BOUND     11
#define LEMMING_COLDEC_ALLEYOOP_L_BOUND      (0-3)
#define LEMMING_COLDEC_ALLEYOOP_F_BOUND      1
#define LEMMING_COLDEC_ALLEYOOP_B_BOUND      3

#define LEMMING_COLDEC_FALLER_U_BOUND        6
#define LEMMING_COLDEC_FALLER_L_BOUND        0
#define LEMMING_COLDEC_FALLER_F_BOUND        1
#define LEMMING_COLDEC_FALLER_B_BOUND        0

#define LEMMING_COLDEC_UMBRELLAWHIP_U_BOUND  9
#define LEMMING_COLDEC_UMBRELLAWHIP_L_BOUND  0
#define LEMMING_COLDEC_UMBRELLAWHIP_F_BOUND  1
#define LEMMING_COLDEC_UMBRELLAWHIP_B_BOUND  0

#define LEMMING_COLDEC_FLOATER_U_BOUND       9
#define LEMMING_COLDEC_FLOATER_L_BOUND       1
#define LEMMING_COLDEC_FLOATER_F_BOUND       2
#define LEMMING_COLDEC_FLOATER_B_BOUND       1


#define LEMMING_COLDEC_MACRO(T) \
      if (l->state == LEMSTATE_##T) {                                               \
         fb = ((l->x + ((dx * LEMMING_COLDEC_##T##_F_BOUND))) << log2magnification); \
         bb = ((l->x - ((dx * LEMMING_COLDEC_##T##_B_BOUND))) << log2magnification); \
         ub = ((l->y - (      LEMMING_COLDEC_##T##_U_BOUND )) << log2magnification); \
         lb = ((l->y + (      LEMMING_COLDEC_##T##_L_BOUND )) << log2magnification); \
      }

// Use the nifty macro to codec for all of the possible clickable states.
   LEMMING_COLDEC_MACRO(WALKER)       else
   LEMMING_COLDEC_MACRO(BLOCKER)      else
   LEMMING_COLDEC_MACRO(BUILDER)      else
   LEMMING_COLDEC_MACRO(BUILDSHRUG)   else
   LEMMING_COLDEC_MACRO(BASHER)       else
   LEMMING_COLDEC_MACRO(MINER)        else
   LEMMING_COLDEC_MACRO(DIGGER)       else
   LEMMING_COLDEC_MACRO(CLIMBING)     else
   LEMMING_COLDEC_MACRO(ALLEYOOP)     else
   LEMMING_COLDEC_MACRO(FALLER)       else
   LEMMING_COLDEC_MACRO(UMBRELLAWHIP) else
   LEMMING_COLDEC_MACRO(FLOATER) else return 0; // If we couldn't find a lemming that's clickable, then exit.

   // By now, we should have a complete set of coordinate bounds to check for collision.
   // (These will all be in screen space)

   // Throw the bottom bound down (m-1) so it compares the bottom edge of the mag
   // pixel it resides inside.
   lb += magnification - 1;

   if ((maggies_y >= (ub - control_cursor_extension_size))
    && (maggies_y <= (lb + control_cursor_extension_size))) {
      if (l->d == 1) {
         // Throw the right bound right (m-1) so it compares the right edge of the mag
         // pixel it resides inside.
         fb += magnification - 1;

         if ((maggies_x >= (bb - control_cursor_extension_size))
          && (maggies_x <= (fb + control_cursor_extension_size))) {
            return 1;
         } else return 0;
      } else {
         bb += magnification - 1;
         // Throw the right bound right (m-1) so it compares the right edge of the mag
         // pixel it resides inside.

         if ((maggies_x >= (fb - control_cursor_extension_size))
          && (maggies_x <= (bb + control_cursor_extension_size))) {
            return 1;
         } else return 0;
      }
   }

   return 0;
}

// Try to take an action on the lemming below POINT (screen space) using the current tool.
int ClickAgainstLemming(LEVEL_INGAME_STATUS_STRUCT *ingame_status, POINT *clickpos, LEMMING_INFO_STRUCT* lemming) {
   // Gonna return whether this click had any effect!

   if (CursorColDecLemming(clickpos, lemming)) {
      if (AssignLemmingTool(ingame_status, lemming, ingame_status->current_tool) == 1) {
         return 1;
      }
   }

   // One means the click did something, and the click should not propogate.
   // Zero means the click was ineffective and the click should propogate to
   // rest of the lemmings.
   return 0;
}

// This is a prototype for the function which takes the slice out of the landscape
// every time the lemming bashes.
void SubtractionBasher(int frame, int d, int x, int y);
void SubtractionMiner(int frame, int d, int x, int y);
// Same for the explosion egg shape.
void SubtractionExplosion(int d, int x, int y);

// -----------------------------------------------------------------------------

int UpdateLemming(LEVEL_INGAME_STATUS_STRUCT *ingame_status, LEMMING_INFO_STRUCT *lemming, const LEMMINGS_LEVEL_RUNTIME_STATS_V7 *level) {
   // This function should do everything to do with updating a single lemming.

   // The state handlers that live outside here have no handler for the timer decrement:
   // this means that they DONT EXPLODE when the timer runs out. (In fact, it's frozen.)


   // This handler is easy. When a lemming hits the limit of smoosh frames, it wont exist any more.
   if (lemming->state == LEMSTATE_SMOOSHED) {
      lemming->animframe++;

      if (lemming->animframe == NO_LEM_FRAMES_SMOOSH) {
         lemming->state = LEMSTATE_DONTEXIST;
      }
   } // END OF LEMMING SMOOSH HANDLER.
   else if (lemming->state == LEMSTATE_EARPULL) {
      // To advance a falling earpull lemming by LEMMING_FALL_SPEED pixels, run this loop
      // LEMMING_FALL_SPEED times.
      // It advances the lemming, checks for edge of level deaths.
      // If it survives, it has to check for the water and flametrap hazards.
      // If it survives, it has to run its normal earpull behaviour.
      for (int lem_fall = 0; lem_fall < LEMMING_FALL_SPEED; lem_fall++) {
         // Has he fallen off the level?
         if (lemming->y == (LEVEL_Y_SIZE - 1)) {
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,               // This is the lemmings position in maggies.
                                                          camera_x_inset);                               // This is the position of the left of the screen in maggies.

         } else if (!level_data[lemming->x][lemming->y + 1]) {
            // If the blank space below the lemming is empty, make him fall.
            lemming->y++;
         } // Note there's no else here. This means that an earpull lemming that has fallen and hit ground
           // will just sit there like a lemon. (Or a lemming, even.)
           // Still, we have to check if this falling pixel has meant that the daft sod has fallen into a hazard.

         // Check lemming state again. He might have been killed by the level falling off handler.
         if (lemming->state != LEMSTATE_DONTEXIST) {
            // Check the lemming against all of the water hazard areas on the level.
            for (u32 w = 0; w < level->no_waters; w++) {
               if ((lemming->x >= level->water_x1[w]                                               )
                && (lemming->x <= level->water_x2[w]                                               )
                && (lemming->y >= level->water_y[w] + active_graphical_object_water->active_zone_y1)
                && (lemming->y <= level->water_y[w] + active_graphical_object_water->active_zone_y2)) {
                  // I'm drowning!!
                  lemming->state     = LEMSTATE_DROWNING;

                  // SOUND REQUEST : lemming hit water
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER,
                                                                0,
                                                                lemming->x << log2magnification,           // This is the lemmings position in maggies.
                                                                camera_x_inset);                           // This is the position of the left of the screen in maggies.

                  // Reset the lemming animation frame and place him on the surface of the water.
                  lemming->y         = level->water_y[w];
                  lemming->animframe = 0;
                  break;
               }
            }
         }

         // Check lemming state again. He might have been killed by a hazard.
         if (lemming->state != LEMSTATE_DONTEXIST) {
            // Check the lemming against all of the hazards on the level.
            for (u32 f = 0; f < level->no_hazards; f++) {
               if ((lemming->x >= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x1))
                && (lemming->x <= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x2))
                && (lemming->y >= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y1))
                && (lemming->y <= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y2))) {
                  // I'm burning!
                  lemming->state     = LEMSTATE_BURNT;

                  // SOUND REQUEST : lemming in flames
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,
                                                                0,
                                                                lemming->x << log2magnification,    // This is the lemmings position in maggies.
                                                                camera_x_inset);                    // This is the position of the left of the screen in maggies.

                  // Reset the lemming animation frame.
                  lemming->animframe = 0;
                  break;
               }
            }
         }
      }

      // If the lemming wasn't DONTEXIST by all of the hazards, we have to advance his
      // normal earpull behaviour.
      if (lemming->state == LEMSTATE_EARPULL) {
         lemming->animframe++;

         // Has the lemming reached the number of earpull frames?
         if (lemming->animframe == NO_LEM_FRAMES_EARPULL) {
            // Reset my animation frame, because I'm about to pop.
            lemming->state = LEMSTATE_POP;
            lemming->animframe = 0;

            // SOUND REQUEST : lemming explode
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE,
                                                          0,
                                                          lemming->x << log2magnification,    // This is the lemmings position in maggies.
                                                          camera_x_inset);                    // This is the position of the left of the screen in maggies.

            // Exploding lemmings don't block no more.
            lemming->flags &= ~LEMFLAG_BLOCKING;
         }
      }
   } // END OF LEMMING EARPULL HANDLER
   else if (lemming->state == LEMSTATE_POP) {
      // Popping lemmings that are transferring from their zeroth frame to their
      // 1th frame take an egg shaped chunk out of the landscape.
      if (lemming->animframe == 0) {
         SubtractionExplosion(lemming->d, lemming->x, lemming->y);
      }

      // When a popping lemming reaches its frame quota, it doesn't exist.
      lemming->animframe++;
      if (lemming->animframe == NO_LEM_FRAMES_POP) {
         lemming->state = LEMSTATE_DONTEXIST;
      }
   } // END OF LEMMING POP HANDLER.
   else if (lemming->state == LEMSTATE_LEAVING) {
      // Leaving lemmings can't explode
      lemming->animframe++;

      // When a leaving lemming reaches its frame quota, it doesn't exist.
      if (lemming->animframe == NO_LEM_FRAMES_LEAVING) {
         lemming->state = LEMSTATE_DONTEXIST;
         ingame_status->lemmings_exited++; // Here's where we register a leaving lemming score.
      }
   } // END OF LEMMING LEAVING HANDLER.
   else if (lemming->state == LEMSTATE_DROWNING) {
      // Drowning lemmings can't explode
      lemming->animframe++;

      s32 xcoord;
      if ((lemming->animframe ==  1)
       || (lemming->animframe ==  2)
       || (lemming->animframe ==  5)
       || (lemming->animframe ==  9)
       || (lemming->animframe == 11)
       || (lemming->animframe == 14)) { // If the lemming is on one of the special frames
                                        // then it should advance forward a single pixel.
         // Calculate the pixel in front of the lemming. It's useful.
         xcoord = lemming->x + ((lemming->d == 1) ? (1) : (-1));

         // Has the lemming drifted into a piece of level geometry?
         if (level_data[xcoord][lemming->y]) {
            // Turn him around.
            lemming->d = lemming->d == 1 ? 0 : 1;

            // Calculate the new lemming destination pixel based on the turning around.
            xcoord = lemming->x + ((lemming->d == 1) ? (1) : (-1));
         }
         lemming->x = xcoord;
      }

      if ((lemming->animframe ==  6)
       || (lemming->animframe ==  7)
       || (lemming->animframe ==  8)
       || (lemming->animframe == 10)
       || (lemming->animframe == 15)) { // If the lemming is on one of the special frames
                                        // then it should advance forward a single pixel.
         // Calculate the pixel in front of the lemming. It's useful.
         xcoord = lemming->x + ((lemming->d == 1) ? (1) : (-1));

         if (level_data[xcoord][lemming->y]) {
            // Turn him around.
            lemming->d = lemming->d == 1 ? 0 : 1;

            xcoord = lemming->x + ((lemming->d == 1) ? (1) : (-1));
         }
         lemming->x = xcoord;
      }

      // I apologize greatly for this. There may be a reason why these are seperate.
      // Although, I cannot recall it at this time.

      // Is the lemming out of drowning frames?
      if (lemming->animframe == NO_LEM_FRAMES_DROWNING) {
         // SOUND REQUEST : lemming drowning
         DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_DROWN,
                                                       0,
                                                       lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                       camera_x_inset);                  // This is the position of the left of the screen in maggies.

         lemming->state = LEMSTATE_DONTEXIST;
      }
   } // END OF LEMMING DROWNING HANDLER.
   else if (lemming->state == LEMSTATE_BURNT) {
      // Burnt lemmings can't explode

      lemming->animframe++;

      // Is the lemming out of burning frames?
      if (lemming->animframe == NO_LEM_FRAMES_BURNT) {
         lemming->state = LEMSTATE_DONTEXIST;
      }
   } // END OF LEMMING BURNING HANDLER.
   else if (lemming->state != LEMSTATE_DONTEXIST) {
      if ((lemming->state == LEMSTATE_WALKER )
       || (lemming->state == LEMSTATE_BUILDER)
       || (lemming->state == LEMSTATE_BASHER )
       || (lemming->state == LEMSTATE_MINER  )
       || (lemming->state == LEMSTATE_DIGGER )) {
         // This next set of handlers is to do with active and useful lemmings.

         // Check for exit! Exit rectangular box collision detection.
         for (u32 exit = 0; exit < level->no_exits; exit++) {
            if ((lemming->x >= (level->exit_x[exit] + active_graphical_object_exit->active_zone_x1))
             && (lemming->x <= (level->exit_x[exit] + active_graphical_object_exit->active_zone_x2))) {
               if ((lemming->y >= (level->exit_y[exit] + active_graphical_object_exit->active_zone_y1))
                && (lemming->y <= (level->exit_y[exit] + active_graphical_object_exit->active_zone_y2))) {
                  // SOUND REQUEST : lemming save
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_SAVE,
                                                                0,
                                                                lemming->x << log2magnification,   // This is the lemmings position in maggies.
                                                                camera_x_inset);                   // This is the position of the left of the screen in maggies.

                  // Set lemming state to leaving and reset animation frame.
                  lemming->state     = LEMSTATE_LEAVING;
                  lemming->animframe = 0;
                  return 1;
               }
            }
         }

         // Check for trap using rectangular box collision detection.
         for (u32 trap = 0; trap < level->no_traps; trap++) {
            // Only allow traps to hit these kind of lemmings if the trap targets grounded lemmings:
            if (!(active_graphical_object_trap[level->trap_genus[trap]]->active_flags
                   & LEMMINGS_GRAPHICAL_OBJECT_ACTIVE_FLAG_TRAP_HURTS_GROUNDED)) {
               continue;
            }

            // Only allow lemmings to interface with idle traps.
            if (ingame_status->trap_state[trap].state == TRAP_STATUS_STATE_IDLE) {
               if ((lemming->x >= (level->trap_x[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_x1))
                && (lemming->x <= (level->trap_x[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_x2))) {
                  if ((lemming->y >= (level->trap_y[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_y1))
                   && (lemming->y <= (level->trap_y[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_y2))) {
                     // SOUND REQUEST : lemming trap trigger
                     DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER,
                                                                   0,
                                                                   lemming->x << log2magnification,   // This is the lemmings position in maggies.
                                                                   camera_x_inset);                   // This is the position of the left of the screen in maggies.

                     // Set lemming state to don't exist and reset animation frame.
                     lemming->state     = LEMSTATE_DONTEXIST;
                     lemming->animframe = 0;

                     // Set the trap state to working.
                     ingame_status->trap_state[trap].state     = TRAP_STATUS_STATE_WORKING;
                     // Set the trap frame to minus (with the update call bringing it to zero later).
                     ingame_status->trap_state[trap].animframe          = active_graphical_object_trap[level->trap_genus[trap]]->no_primary_frames;
                     ingame_status->trap_state[trap].animframe_subframe = 1;

                     return 1;
                  }
               }
            }
         }
      }

      // If the lemming has a lit fuse, then run the lemming explode handler.
      // This handler does not apply to the previously handled lemming states
      // because those lemmings cannot explode.
      if (lemming->flags & LEMFLAG_FUSELIT) {
         lemming->fusetimer--; // Remember, a lemming fuse is in frames, not seconds.

         // Are we out of time.
         if (lemming->fusetimer == 0) {
            lemming->animframe = 0;

            // This flag will be set to one if the lemming should pop instantly.
            u32 pop_instant = 0;

            // If the lemming is climbing or vaulting over a wall, then they will be moved one pixel
            // INTO the wall they were vaulting, and will not have an instant pop.
            if ((lemming->state == LEMSTATE_CLIMBING)
             || (lemming->state == LEMSTATE_ALLEYOOP)) {
               lemming->x += ((lemming->d == 1) ? (1) : (-1));

               // Only do OH NO sound request for lemming that wasn't explode triggered by nuke.
               if (!(lemming->flags & LEMFLAG_EXPLODE_BY_NUKE)) {
                  // SOUND REQUEST : lemming oh no
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_OH_NO,
                                                                                           0,
                                                                                  lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                                              camera_x_inset);                      // This is the position of the left of the screen in maggies.
               }
               lemming->state = LEMSTATE_EARPULL;
            } else {
               // If the lemming wasn't climing or vaulting, then run more checks.

               // Lemmings that are on the bottom pixel of the level will explode instantly.
               if (lemming->y == (LEVEL_Y_SIZE - 1)) {
                  pop_instant = 1;
               } else if (!level_data[lemming->x][lemming->y + 1]) {
                  // Lemmings that aren't on solid ground will pop instantly.
                  pop_instant = 1;
               }

               // If the lemming wont pop instantly, change their state into the earpull state.
               if (pop_instant == 0) {
                  // Only do OH NO sound request for lemming that wasn't explode triggered by nuke.
                  if (!(lemming->flags & LEMFLAG_EXPLODE_BY_NUKE)) {
                     // SOUND REQUEST : lemming oh no
                     DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_OH_NO,
                                                                                              0,
                                                                                     lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                                                 camera_x_inset);                     // This is the position of the left of the screen in maggies.
                  }

                  lemming->state = LEMSTATE_EARPULL;
               } else {
                  // This handles lemmings that will pop instantly.

                  lemming->state = LEMSTATE_POP;

                  // SOUND REQUEST : lemming explode
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_EXPLODE,
                                                                0,
                                                                lemming->x << log2magnification,      // This is the lemmings position in maggies.
                                                                camera_x_inset);                      // This is the position of the left of the screen in maggies.
                  // Exploding lemmings don't block no more.
                  lemming->flags &= ~LEMFLAG_BLOCKING;
               }
            }
         }
      } // END OF LEMMING FUSELIT HANDLER.

      // If the lemming isn't pulling their head, popping or drowning, then they're
      // still in a useful lemming state.
      if ((lemming->state != LEMSTATE_EARPULL)
       && (lemming->state != LEMSTATE_POP)
       && (lemming->state != LEMSTATE_DROWNING)) {
         // Hmm. Let's see if the lemming is on solid ground.
         // If they're not, they should be falling!
         if ((!level_data[lemming->x][lemming->y + 1])
          && (!((lemming->state == LEMSTATE_FALLER      )
             || (lemming->state == LEMSTATE_FLOATER     )
             || (lemming->state == LEMSTATE_UMBRELLAWHIP)
             || (lemming->state == LEMSTATE_CLIMBING    )
             || (lemming->state == LEMSTATE_DIGGER      )
             || (lemming->state == LEMSTATE_ALLEYOOP    )))) {
            // Set the state!
            lemming->state = LEMSTATE_FALLER;

            // Fallen lemmings don't block no more.
            lemming->flags &= ~LEMFLAG_BLOCKING;

            // Reset the fall timer.
            lemming->falldistance = 0;
         }

         // This handler will turn around any lemming that isn't a blocker or a miner.
         if ((lemming->state != LEMSTATE_BLOCKER)
          && (lemming->state != LEMSTATE_MINER  )) { // Can't turn a blocker... or a miner
            LEMMING_INFO_STRUCT *blocker_lem;
            s32 left_facing, yp, xp;

            for (u32 blocker_lem_index = 0; blocker_lem_index < current_level_runtime_stats.lemmings; blocker_lem_index++) {
               // Get ptr to blocker lem to check
               blocker_lem = &(ingame_status->lemming_info[blocker_lem_index]);
               if (blocker_lem->state != LEMSTATE_DONTEXIST) { // Better safe than sorry...
                  // left_facing holds the offset to add if the blocker is facing to the left and not the right.
                  // This offset is necesarry because left facing and right facing blockers have different hotspot positions.
                  left_facing = (blocker_lem->d == 1) ? 1 : 0;

                  // Is this lem a blocker?
                  if (blocker_lem->flags & LEMFLAG_BLOCKING) {
                     xp = lemming->x;
                     yp = lemming->y;

                     // Collide the current lemming against the current blocker.
                     if ((yp >= LEMMING_BLOCKER_VERTICAL_TOP_RANGE    + blocker_lem->y)
                      && (yp <= LEMMING_BLOCKER_VERTICAL_BOTTOM_RANGE + blocker_lem->y)) {
                        // Different x coordinate checks depending on what direction the lemming is facing.
                        if (((lemming->d == 1) && ((xp >= LEMMING_BLOCKER_LEFT_RANGE_FAR   + blocker_lem->x + left_facing)
                                                && (xp <= LEMMING_BLOCKER_LEFT_RANGE_NEAR  + blocker_lem->x + left_facing)))
                         || ((lemming->d == 0) && ((xp <= LEMMING_BLOCKER_RIGHT_RANGE_FAR  + blocker_lem->x + left_facing)
                                                && (xp >= LEMMING_BLOCKER_RIGHT_RANGE_NEAR + blocker_lem->x + left_facing)))) {
                           // Turn him around.
                           lemming->d = lemming->d == 1 ? 0 : 1;
                        }
                     }
                  }
               }
            }
         }
      } // END OF SUDDEN FALLER AND BLOCKER TURN AROUND HANDLERS

      if (lemming->state == LEMSTATE_WALKER) {
         // Calculate the pixel in front of the lemming.
         // This is where the lemming will walk to.
         s32 xcoord   = lemming->x + ((lemming->d == 1) ? (1) : (-1));

         // Before all of the walker handling... has the lemming fell off the map?
         if ((lemming->y >= (LEVEL_Y_SIZE - 1))
          || (lemming->y <  (0               ))) {
            // The daft blighter's walked off the top or bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                          camera_x_inset);                  // This is the position of the left of the screen in maggies.
         } else
         // Hmm. Let's see if the lemming is on solid ground.
         if (level_data[lemming->x][lemming->y + 1]) {
            // If he is, then he can walk.

            if ((xcoord >= LEVEL_X_SIZE)
             || (xcoord <  0           )) {
               // The daft blighter's walked off the edge of the map!
               // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
               lemming->state = LEMSTATE_DONTEXIST;

               // SOUND REQUEST : lemming fall off level
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                             0,
                                                             lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                             camera_x_inset);                  // This is the position of the left of the screen in maggies.
            } else {
               // Measure the wall in from of the lemming, if there is one.
               u32 wallhigh = 0;

               for (u32 ycoord = lemming->y; level_data[xcoord][ycoord]; ycoord--) {
                  wallhigh++;
               }

               // Is there some kind of (insurmountable) obstruction in the way?
               if (wallhigh > LEMMING_MOUNT_CAPABILITY) {
                  // This wall is way too high for the lemming to try to mount.

                  // Can the lemming climb it?
                  // Check his powers.
                  if (!(lemming->flags & LEMFLAG_CLIMBER_POWER)) {
                     // He can't climb it.

                     // Turn him around.
                     lemming->d = lemming->d == 1 ? 0 : 1;

                     // Recalc xcoord
                     xcoord   = lemming->x + ((lemming->d == 1) ? (1) : (-1));

                     // Recalc the wallhigh.
                     wallhigh = 0;

                     for (u32 ycoord = lemming->y; level_data[xcoord][ycoord]; ycoord--) {
                        wallhigh++;
                     }
                  } else {
                     // The lemming can climb it after all.
                     lemming->state = LEMSTATE_CLIMBING;

                     lemming->animframe = 0;
                  }
               }

               // Is the lemming still a walker? (He didn't turn into a climber?)
               if (lemming->state == LEMSTATE_WALKER) {
                  // Can the lemming instantly mount the wall in front of him?
                  if (wallhigh <= LEMMING_MOUNT_CAPABILITY) {
                     // If the wall wasn't too high, then it must be okay for the
                     // lemming to walk into the next cell, raising or falling if it must.

                     // Is there something for the lemming to walk onto.
                     if ((level_data[xcoord][lemming->y+1])
                      || (level_data[xcoord][lemming->y  ])) {
                        // Move the lemming forward.
                        lemming->x = xcoord;

                        // Up you go.
                        lemming->y -= wallhigh;

                        // Advance his frame
                        lemming->animframe++;
                        lemming->animframe %= NO_LEM_FRAMES_WALKER;
                     } else {
                        // There appears to be a hole in the floor!

                        // Get the depth of the hole into wallhigh
                        wallhigh = 0;
                        for (u32 ycoord = lemming->y+1; !level_data[xcoord][ycoord]; ycoord++) {
                           wallhigh++;
                        }

                        // Can the lemming instantly fall down this hole?
                        if (wallhigh > LEMMING_DESCEND_CAPABILITY) {
                           // This fall is way too high for the lemming to try to step down.

                           lemming->x = xcoord;

                           // You are now falling.
                           lemming->state = LEMSTATE_FALLER;
                           // Fall!
                           lemming->y += LEMMING_FALL_SPEED;

                           // Reset the fall timer.
                           lemming->falldistance = LEMMING_FALL_SPEED;

                           // Reset frame for falling-ness
                           lemming->animframe = 0;
                        } else {
                           // The lemming can step down this hole with ease.

                           lemming->x = xcoord;

                           // Down you go.
                           lemming->y += wallhigh;

                           // Move his frame
                           lemming->animframe++;
                           lemming->animframe %= NO_LEM_FRAMES_WALKER;
                        }
                     }
                  } else {
                     // Walk backwards
                     lemming->x = xcoord;

                     // Move his frame
                     lemming->animframe++;
                     lemming->animframe %= NO_LEM_FRAMES_WALKER;
                  }
               }
            }
         }

         // After all of the walker handling... has the lemming fell off the map?
         if ((lemming->y >= (LEVEL_Y_SIZE - 1))
          || (lemming->y <  (0               ))) {
            // The daft blighter's walked off the top or bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                          camera_x_inset);                  // This is the position of the left of the screen in maggies.
         }
      } // END OF LEMMING WALKER HANDLER
      else if (lemming->state == LEMSTATE_BLOCKER) {
         // Move his frame
         lemming->animframe++;
         lemming->animframe %= NO_LEM_FRAMES_BLOCKER;
      } // END OF LEMMING BLOCKER HANDLER
      else if (lemming->state == LEMSTATE_DIGGER) {
         // This checks whether or not the next dig action (frames 0 and 8)
         // will result in the digger falling out of his current dig.
         if ((lemming->animframe == 0)
          || (lemming->animframe == 8)) {
            // Get the total of the values of the cells beneath the digger.
            // If there's a line of zero, then he falls. Otherwise, he digs.
            u32 dig_space_total = 0;
            s32 dig_check_abs_x = 0;
            for (int dig_check_rel_x = -3; dig_check_rel_x <= 3; dig_check_rel_x++) {
               dig_check_abs_x = lemming->x + dig_check_rel_x;
               if (dig_check_abs_x >= 0) {
                  if (dig_check_abs_x < LEVEL_X_SIZE) {
                     dig_space_total += level_data[dig_check_abs_x][lemming->y + 1];
                  }
               }
            }
            if (dig_space_total == 0) {
               lemming->state = LEMSTATE_FALLER;
               lemming->animframe = 0;
               return 1;
            }
         }

         // This performs the next dig action (frames 0 and 8)
         if ((lemming->animframe == 0)
          || (lemming->animframe == 8)) {
            // Will the lemmings next dig action hit steel?
            // Only check the pixel directly beneath the lemming's hotspot.
            if (WithinSteel(lemming->x, lemming->y+1, lemming->d)) {
               // SOUND REQUEST : steel hit
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                             0,
                                                             lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                             camera_x_inset);                 // This is the position of the left of the screen in maggies.

               // You are now a walker
               lemming->state = LEMSTATE_WALKER;
               lemming->animframe = 0;

               return 1;
            } else if (lemming->y == (LEVEL_Y_SIZE - 1)) {
               // The daft blighter's dug off the bottom edge of the map!
               // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
               lemming->state = LEMSTATE_DONTEXIST;
               lemming->animframe = 0;

               // SOUND REQUEST : lemming fall off level
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                             0,
                                                             lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                             camera_x_inset);                 // This is the position of the left of the screen in maggies.
               return 1;
            } else {
               // Destroy the ground underneath the lemming
               for (int xblast = Max(lemming->x - 4, 0); xblast <= Min(lemming->x + 4, LEVEL_X_SIZE - 1); xblast++) {
                  level_data[xblast][lemming->y + 1] = 0;
               }
               // Move the lemming down.
               lemming->y++;
            }
         }

         // Move his frame
         lemming->animframe++;
         // Loop the lemmings frames
         lemming->animframe %= NO_LEM_FRAMES_DIGGER;
      } // END OF LEMMING DIGGER HANDLER
      else if (lemming->state == LEMSTATE_BASHER) {
         // Calculate the pixel in front of the lemming.
         // This is where the lemming will walk to.
         s32 xcoord   = lemming->x + ((lemming->d == 1) ? (1) : (-1));

         // Hmm. Let's see if the lemming is on solid ground.
         if (level_data[lemming->x][lemming->y + 1]) {
            // Move his frame
            lemming->animframe++;
            lemming->animframe %= NO_LEM_FRAMES_BASHER;

            u32 frametest = ((lemming->animframe >= 15) ? (lemming->animframe - 15) : (lemming->animframe));

            // Test for the specific frames on which the basher moves forward.
            if ((frametest ==  0)
             || (frametest ==  5)
             || (frametest ==  9)
             || (frametest == 11)
             || (frametest == 14)) {

               // If he is, then he can advance.

               if ((xcoord >= LEVEL_X_SIZE)
                || (xcoord <  0           )) {
                  // The daft blighter's bashed off the edge of the map!
                  // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
                  lemming->state = LEMSTATE_DONTEXIST;

                  // SOUND REQUEST : lemming fall off level
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                                0,
                                                                lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                                camera_x_inset);                 // This is the position of the left of the screen in maggies.
               } else {
                  // First, check for holes in the floor.
                  // get the depth of the hole into wallhigh
                  u32 wallhigh = 0;
                  for (u32 ycoord = lemming->y+1; !level_data[xcoord][ycoord]; ycoord++) {
                     wallhigh++;
                  }

                  if (wallhigh > LEMMING_BASHER_DESCEND_CAPABILITY) {
                     // This fall is way too high for the lemming to try to step down.

                     lemming->x = xcoord;

                     // You are now falling.
                     lemming->state = LEMSTATE_FALLER;
                     // Fall!
                     lemming->y += LEMMING_FALL_SPEED;

                     // Reset the fall timer.
                     lemming->falldistance = LEMMING_FALL_SPEED;

                     // Reset falling animation frame
                     lemming->animframe = 0;
                  } else {
                     // Test if the lemming is about to give up
                     if (frametest == 5) {
                        // The direction into which the check extends horizontally depends on the
                        // direction the lemming is facing.
                        s32 dx = ((lemming->d == 1) ? (1) : (-1));

                        // Get the total of the values of the cells in front of the basher.
                        // If they're all zero, then no bash. If he hits steel, then stop bash.

                        s32 bash_check_abs_x;
                        s32 bash_check_abs_y;

                        // This is the steel check
                        for (int bash_check_rel_y = 0; bash_check_rel_y < LEMMING_BASHER_STEEL_CHECK_Y_SIZE; bash_check_rel_y++) {
                           bash_check_abs_y = lemming->y - bash_check_rel_y;

                           for (int bash_check_rel_x = LEMMING_BASHER_STEEL_CHECK_ARMS_LENGTH; bash_check_rel_x <= LEMMING_BASHER_STEEL_CHECK_FAR_BOUND_X; bash_check_rel_x++) {
                              bash_check_abs_x = lemming->x + (dx * bash_check_rel_x);

                              // Is the lemming still a basher?
                              if (lemming->state == LEMSTATE_BASHER) {
                                 if (bash_check_abs_x >= 0) {
                                    if (bash_check_abs_x < LEVEL_X_SIZE) {
                                       if (bash_check_abs_y >= 0) {
                                          if (bash_check_abs_y < LEVEL_Y_SIZE) {
                                             // Is the pixel within steel?
                                             if (WithinSteel(bash_check_abs_x, bash_check_abs_y, lemming->d)) {
                                                // He hit steel, he's now a walker.
                                                lemming->state = LEMSTATE_WALKER;
                                                lemming->animframe = 0;

                                                // Turn him around.
                                                lemming->d = lemming->d == 1 ? 0 : 1;

                                                // SOUND REQUEST : steel hit
                                                DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                                                              0,
                                                                                              lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                                                              camera_x_inset);                 // This is the position of the left of the screen in maggies.
                                             }
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }

                        // This counts the amount of dirt (any occluded) pixels in front of the basher.
                        u32 bash_space_total = 0;

                        // Is the lemming still a basher?
                        if (lemming->state == LEMSTATE_BASHER) {
                           // This is the ZERO DIRT IN FRONT CHECK.
                           for (int bash_check_rel_y = 0; bash_check_rel_y < LEMMING_BASHER_CHECK_Y_SIZE; bash_check_rel_y++) {
                              bash_check_abs_y = lemming->y - bash_check_rel_y;

                              for (int bash_check_rel_x = LEMMING_BASHER_CHECK_ARMS_LENGTH; bash_check_rel_x <= LEMMING_BASHER_CHECK_FAR_BOUND_X; bash_check_rel_x++) {
                                 bash_check_abs_x = lemming->x + (dx * bash_check_rel_x);

                                 if (bash_check_abs_x >= 0) {
                                    if (bash_check_abs_x < LEVEL_X_SIZE) {
                                       if (bash_check_abs_y >= 0) {
                                          if (bash_check_abs_y < LEVEL_Y_SIZE) {
                                             bash_space_total += level_data[bash_check_abs_x][bash_check_abs_y];
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                           // The entire space in front of the basher is blank. That's enough bashing out of you!
                           if (bash_space_total == 0) {
                              // You're now a walker.
                              lemming->state = LEMSTATE_WALKER;
                              lemming->animframe = 0;
                           }
                        }
                     }

                     // If the lemming is still a basher, move them forward into their next pixel.
                     if (lemming->state == LEMSTATE_BASHER) {
                        lemming->x = xcoord;
                     }
                     // Lemmings that changed state don't move forward.

                     // Move the lemming up or down depending on the next pixels.
                     lemming->y += wallhigh;
                  }
               }
            }
            if ((frametest == 2)
             || (frametest == 3)
             || (frametest == 4)
             || (frametest == 5)) { // Check for a specific bash-dirt-subtraction frame.
               // If it's a basher bashing frame then do the bash!
               SubtractionBasher(frametest, lemming->d, lemming->x, lemming->y);
               //printf("bash frame: FT = %d, animframe = %d\n", frametest, lemming->animframe);
            }
         }

         // Has the lemming fallen off the map?
         if ((lemming->y >= (LEVEL_Y_SIZE - 1))
          || (lemming->y <  (0               ))) {
            // The daft blighter's walked off the top or bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                          camera_x_inset);                  // This is the position of the left of the screen in maggies.
         }
      } // END OF LEMMING BASHER HANDLER
      else if (lemming->state == LEMSTATE_BUILDER) {
         // Hmm. Let's see if the lemming is on solid ground.
         if (level_data[lemming->x][lemming->y + 1]) {
            // Find the direction that the lemming is facing.
            s32 dx = ((lemming->d == 1) ? (1) : (-1));

            // Move his frame
            lemming->animframe++;
            lemming->animframe %= NO_LEM_FRAMES_BUILDER;

            // Calculate the destination pixel in front of the lemming. It's useful.
            // This is the pixel the lemming will walk to if it's time to walk forward.
            // Note that builders move two pixels at a time.
            s32 xcoord   = lemming->x + ((lemming->d == 1) ? (2) : (-2));

            if (lemming->animframe == 8) { // Frame 8 is the frame to place the brick on.
               for (int brick_x = 0; brick_x < 6; brick_x++) {
                  sadd(lemming->x + (dx * brick_x), lemming->y, SPECIAL_COLOUR_BUILDER_BRICK);
               } // Safely add the new brick to the level data array.

               // Decrement the lemming remaining brick count.
               lemming->rembricks--;

               // ^_^ You should know exactly what this does.
               if (lemming->rembricks < 3) {
                  // SOUND REQUEST : builder chink
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_BUILDER_CHINK,
                                                                0,
                                                                lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                                camera_x_inset);                  // This is the position of the left of the screen in maggies.
               }
            }

            if (lemming->animframe == 12) { // This is the frame that the builder may walk into a bit of the level and turn around.
               u32 build_space_total = 0;

               s32 build_check_abs_x;
               s32 build_check_abs_y;

               // Check the rectangle in front of the builder.
               // Count the number of debris pieces.
               for (int build_check_rel_x = 1; build_check_rel_x <= 2; build_check_rel_x++) {
                  build_check_abs_x = lemming->x + (dx * build_check_rel_x);

                  for (int build_check_rel_y = 1; build_check_rel_y < 4; build_check_rel_y++) {
                     build_check_abs_y = lemming->y - build_check_rel_y;

                     if (build_check_abs_x >= 0) {
                        if (build_check_abs_x < LEVEL_X_SIZE) {
                           if (build_check_abs_y >= 0) {
                              if (build_check_abs_y < LEVEL_Y_SIZE) {
                                 build_space_total += level_data[build_check_abs_x][build_check_abs_y];
                              }
                           }
                        }
                     }
                  }
               }

               // If there's a single speck of dirt in the lemmings check space, they've collided with
               // something, so turn them into a walker and turn them around.
               if (build_space_total != 0) {
                  lemming->state = LEMSTATE_WALKER;
                  lemming->animframe = 0;

                  // Turn him around.
                  lemming->d = lemming->d == 1 ? 0 : 1;
               }
            }

            // Test for the specific frame on which the builder moves forward.
            if (lemming->animframe == 15) {

               // Test for walk to doom.
               if ((xcoord >= LEVEL_X_SIZE)
                || (xcoord <  0           )) {
                  // The daft blighter's bashed off the edge of the map!
                  // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
                  lemming->state = LEMSTATE_DONTEXIST;

                  // SOUND REQUEST : lemming fall off level
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                                0,
                                                                lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                                camera_x_inset);                 // This is the position of the left of the screen in maggies.
               } else {
                  // DO THE HEADBUTT CHECK HERE.

                  u32 build_space_total = 0;

                  s32 build_check_abs_x;
                  s32 build_check_abs_y;

                  // Check the rectangle in front of the builder.
                  // Count the number of debris pieces.
                  for (int build_check_rel_x = LEMMING_BUILDER_HEADBUTT_X_START; build_check_rel_x <= LEMMING_BUILDER_HEADBUTT_X_END; build_check_rel_x++) {
                     build_check_abs_x = lemming->x + (dx * build_check_rel_x);

                     for (int build_check_rel_y = 1; build_check_rel_y <= LEMMING_BUILDER_HEADBUTT_Y_SIZE; build_check_rel_y++) {
                        build_check_abs_y = lemming->y - build_check_rel_y;

                        if (build_check_abs_x >= 0) {
                           if (build_check_abs_x < LEVEL_X_SIZE) {
                              if (build_check_abs_y >= 0) {
                                 if (build_check_abs_y < LEVEL_Y_SIZE) {
                                    build_space_total += level_data[build_check_abs_x][build_check_abs_y];
                                 }
                              }
                           }
                        }
                     }
                  }

                 // If there's a single speck of dirt in the lemmings check space, they've collided with
                 // something, so turn them into a walker and turn them around.
                  if (build_space_total != 0) {
                     lemming->state = LEMSTATE_WALKER;
                     lemming->animframe = 0;

                     // Turn him around.
                     lemming->d = lemming->d == 1 ? 0 : 1;
                  }

                  // If they're still a builder, then they can advance.
                  if (lemming->state == LEMSTATE_BUILDER) {
                     lemming->x = xcoord;
                     lemming->y--;

                     // If they're out of bricks, they will shrug.
                     if (lemming->rembricks == 0) {
                        lemming->state = LEMSTATE_BUILDSHRUG;
                        lemming->animframe = 0;
                     }
                  }
               }
            }
         }

         // Has the lemming fallen off the top or bottom of the map?
         if ((lemming->y >= (LEVEL_Y_SIZE - 1))
          || (lemming->y <  (0               ))) {
            // The daft blighter's walked off the top or bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                          camera_x_inset);                  // This is the position of the left of the screen in maggies.
         }
      } // END OF LEMMING BUILDER HANDLER
      else if (lemming->state == LEMSTATE_MINER) {

         // Hmm. Let's see if the lemming is on solid ground.
         if (level_data[lemming->x][lemming->y + 1]) {
            // Find the direction that the lemming is facing.
            s32 dx = ((lemming->d == 1) ? (1) : (-1));

            // Move his frame, and loop.
            lemming->animframe++;
            lemming->animframe %= NO_LEM_FRAMES_MINER;

            // Calculate the destination pixel in front of the lemming. It's useful.
            s32 xcoord   = lemming->x + ((lemming->d == 1) ? (2) : (-2));
            // Miners move forward two pixels at once!

            // Test for the specific frames on which the miner moves forward.
            if ((lemming->animframe ==  2)
             || (lemming->animframe == 16)) {

               // Test for walk to doom.
               if ((xcoord >= LEVEL_X_SIZE)
                || (xcoord <  0           )) {
                  // The daft blighter's bashed off the edge of the map!
                  // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
                  lemming->state = LEMSTATE_DONTEXIST;

                  // SOUND REQUEST : lemming fall off level
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                                0,
                                                                lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                                camera_x_inset);                 // This is the position of the left of the screen in maggies.
               } else {
                  // Do the steel check here!
                  // This is the steel check
                  s32 mine_check_abs_x;
                  s32 mine_check_abs_y;

                  // Check all of the pixels in front of the miner for steel.

                  // First we calculate the coordinates of the pixel which we're going to check against.
                  // Use the constants defined above!
                  for (int mine_check_rel_y = 0; mine_check_rel_y < LEMMING_MINER_STEEL_CHECK_Y_SIZE; mine_check_rel_y++) {
                     mine_check_abs_y = lemming->y - mine_check_rel_y + LEMMING_MINER_STEEL_CHECK_Y_DESCENT;

                     for (int mine_check_rel_x = LEMMING_MINER_STEEL_CHECK_ARMS_LENGTH; mine_check_rel_x <= LEMMING_MINER_STEEL_CHECK_FAR_BOUND_X; mine_check_rel_x++) {
                        mine_check_abs_x = lemming->x + (dx * mine_check_rel_x);

                        // At this point, we need to make sure that the check hasn't transformed him from a miner
                        // into a walker due to steel collision!
                        if (lemming->state == LEMSTATE_MINER) {
                           // Make sure that the check pixel is within the map area.
                           // Attempting to check outside the map will result in overflow and bad.
                           if (mine_check_abs_x >= 0) {
                              if (mine_check_abs_x < LEVEL_X_SIZE) {
                                 if (mine_check_abs_y >= 0) {
                                    if (mine_check_abs_y < LEVEL_Y_SIZE) {
                                       // If the pixel we're checking for is within steel:
                                       if (WithinSteel(mine_check_abs_x, mine_check_abs_y, lemming->d)) {
                                          // He's hit steel while mining, you're now a walker. Reset state and frame.
                                          lemming->state = LEMSTATE_WALKER;
                                          lemming->animframe = 0;
                                          // Turn him around.
                                          lemming->d = lemming->d == 1 ? 0 : 1;

                                          // SOUND REQUEST : steel hit
                                          DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_STEEL_HIT,
                                                                                        0,
                                                                                        lemming->x << log2magnification,   // This is the lemmings position in maggies.
                                                                                        camera_x_inset);                   // This is the position of the left of the screen in maggies.
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  // If the lemming is still a miner after the check, then they get to advance as a miner.
                  // (Two spaces forward, and one space down)
                  if (lemming->state == LEMSTATE_MINER) {
                     lemming->x = xcoord;
                     lemming->y++;
                  }
               }
            }

            // If the lemming is still a miner here, we need to remove any dirt in front of the miner
            // (if the miner is on an animation frame)
            if (lemming->state == LEMSTATE_MINER) {
               if ((lemming->animframe == 1)
                || (lemming->animframe == 2)) {
                  // If it's a miner mining frame then do the mine!
                  SubtractionMiner(lemming->animframe, lemming->d, lemming->x, lemming->y);
               }
            }
         }

         // Has the lemming fallen off the top or bottom of the map?
         if ((lemming->y >= (LEVEL_Y_SIZE - 1))
          || (lemming->y <  (0               ))) {
            // The daft blighter's walked off the top or bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                          camera_x_inset);                  // This is the position of the left of the screen in maggies.
         }
      } // END OF LEMMING MINER HANDLER
      else if (lemming->state == LEMSTATE_BUILDSHRUG) {
         // The buildshrug is a simple animation, advance and switch to walker when out of frames.
         lemming->animframe++;
         if (lemming->animframe == NO_LEM_FRAMES_BUILDSHRUG) {
            lemming->state = LEMSTATE_WALKER;
            lemming->animframe = 0;
         }
      } // END OF LEMMING BUILDSHRUG HANDLER
      else if (lemming->state == LEMSTATE_CLIMBING) {
         // Advance and loop his frame
         lemming->animframe++;
         lemming->animframe %= NO_LEM_FRAMES_CLIMBER;

         // The frames above and equal to four have the climber moving up one frame.
         if (lemming->animframe >= 4) {
            // Move the lemming up.
            lemming->y--;

            // Measure the amount of holes directly above the climber, to see if they can
            // continue to climb without hitting their head.
            u32 wallhigh = 0;
            for (u32 ycoord = lemming->y; (ycoord >= 0) && (!level_data[lemming->x][ycoord]); ycoord--) {
               wallhigh++;
            }

            // If the ceiling is too low, the lemming hits his head, and falls down.
            if (wallhigh <= LEMMING_CLIMB_HEADHIT_REACH) {
               // Turn around
               lemming->d = ((lemming->d == 1) ? 0 : 1);

               // You're now a fresh faller, reset stats, frame and go.
               lemming->state = LEMSTATE_FALLER;
               lemming->animframe = 0;
               lemming->falldistance = 0;

               return 1;
            }

            // Check to see if the lemming has climbed off the top of the map.
            if (lemming->y < 0) {
               // The daft blighter's climbed off the top edge of the map!
               // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
               lemming->state = LEMSTATE_DONTEXIST;

               // SOUND REQUEST : lemming fall off level
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                             0,
                                                             lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                             camera_x_inset);                 // This is the position of the left of the screen in maggies.
            } else {
               // If the lemming has not climbed off the top of the map:

               // This is the coordinate the lemming will inhabit if they reach the top
               // and advance by one pixel.
               s32 xcoord = lemming->x + ((lemming->d == 1) ? 1 : -1);

               // Measure the wall the lemming is climbing
               wallhigh = 0;
               for (u32 ycoord = lemming->y; level_data[xcoord][ycoord]; ycoord--) {
                  wallhigh++;
               }
               // This includes the block which the lemming is meant to be climbing on.

               // If the wall is close enough to alleyoop:
               if (wallhigh <= LEMMING_ALLEYOOP_REACH) {
                  // Position the lemming on top of the wall they're mounting.
                  lemming->y = lemming->y - wallhigh + LEMMING_ALLEYOOP_REACH;

                  // Set frame to alleyoop and reset counter.
                  lemming->state = LEMSTATE_ALLEYOOP;
                  lemming->animframe = 0;
               }
            }
         }
      } // END OF LEMMING CLIMBING HANDLER
      else if (lemming->state == LEMSTATE_ALLEYOOP) {
         // Move his frame
         lemming->animframe++;

         // If the lemming has reached the end of their alleyoop:
         if (lemming->animframe == NO_LEM_FRAMES_ALLEYOOP) {
            // The lemming is now on the ground. (of the wall they're mounting)
            lemming->y -= LEMMING_ALLEYOOP_REACH;

            // Advance the lemming by one pixel.
            lemming->x += ((lemming->d == 1) ? 1 : -1);

            // Change them to a walker and reset their frame counter
            lemming->state = LEMSTATE_WALKER;
            lemming->animframe = 0;
         }
      } // END OF LEMMING ALLEYOOP HANDLER
      else if ((lemming->state == LEMSTATE_FALLER      )
            || (lemming->state == LEMSTATE_UMBRELLAWHIP)
            || (lemming->state == LEMSTATE_FLOATER     )) {
            // We're managing all off-ground descending lemmings now.

         // Check to see if the lemming has fallen off the bottom of the map.
         if (lemming->y >= (LEVEL_Y_SIZE - 1)) {
            // The daft blighter's fell off the bottom edge of the map!
            // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
            lemming->state = LEMSTATE_DONTEXIST;

            // SOUND REQUEST : lemming fall off level
            DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                          0,
                                                          lemming->x << log2magnification, // This is the lemmings position in maggies.
                                                          camera_x_inset);                 // This is the position of the left of the screen in maggies.

            return 1;
         }

         // This flag will hold the status of the lemming
         // (have they hit the ground on this loop?)
         int hit_ground = 0;

         // This is the coordinate the lemming will inhabit after falling.
         int xcoord = lemming->x;

         // If the lemming is one pixel above some solid
         // ground, then they've hit it.
         if (level_data[xcoord][lemming->y+1]) {
            hit_ground = 1;
         } // If they're still in the air, then we need to proceed with either
           // LEMMING_FALL_SPEED or LEMMING_FLOATER_FALL_SPEED ticks of a 'fall one pixel
           // test for ground hit + smoosh' loop.
         else for (int f = 0; f < ((lemming->state == LEMSTATE_FALLER)
                                     ? LEMMING_FALL_SPEED
                                     : LEMMING_FLOATER_FALL_SPEED); f++) {

            // Move the lemming down by one pixel, and increase the counter of pixels fallen.
            lemming->y++;
            lemming->falldistance++;

            // Has the lemming fallen off the bottom of the map?
            if (lemming->y >= (LEVEL_Y_SIZE - 1)) {
               // The daft blighter's fell off the bottom edge of the map!
               // Dead! Eeeeeoooooooouuuuuuuuurrrrrg. (lemmingslevel!)
               lemming->state = LEMSTATE_DONTEXIST;

               // SOUND REQUEST : lemming fall off level
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_FALL_OFF_LEVEL,
                                                             0,
                                                             lemming->x << log2magnification,               // This is the lemmings position in maggies.
                                                             camera_x_inset);                               // This is the position of the left of the screen in maggies.

               return 1;
            }

            // If the lemming posesses floater power, and is still a faller
            // (hasn't had his state changed in any way), then try to open umbrella.
            if ((lemming->flags & LEMFLAG_FLOATER_POWER) && (lemming->state == LEMSTATE_FALLER)) {
               // Has the lemming fallen the number of pixels necessary to open umbrella.
               if (lemming->falldistance >= LEMMING_UMBRELLAWHIP_THRESHOLD) {
                  // Change state to umbrella whip and reset frame counter.
                  lemming->state = LEMSTATE_UMBRELLAWHIP;
                  lemming->animframe = -1;
               }
            }
            
            // Check for trap using rectangular box collision detection.
            for (u32 trap = 0; trap < level->no_traps; trap++) {
               // Only allow traps to hit falling lemmings if the trap targets ungrounded lemmings:
               if (!(active_graphical_object_trap[level->trap_genus[trap]]->active_flags
                      & LEMMINGS_GRAPHICAL_OBJECT_ACTIVE_FLAG_TRAP_HURTS_UNGROUNDED)) {
                  continue;
               }

               // Only allow lemmings to interface with idle traps.
               if (ingame_status->trap_state[trap].state == TRAP_STATUS_STATE_IDLE) {
                  if ((lemming->x >= (level->trap_x[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_x1))
                   && (lemming->x <= (level->trap_x[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_x2))) {
                     if ((lemming->y >= (level->trap_y[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_y1))
                      && (lemming->y <= (level->trap_y[trap] + active_graphical_object_trap[level->trap_genus[trap]]->active_zone_y2))) {
                        // SOUND REQUEST : lemming trap trigger
                        DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_TRAP_TRIGGER,
                                                                      0,
                                                                      lemming->x << log2magnification,   // This is the lemmings position in maggies.
                                                                      camera_x_inset);                   // This is the position of the left of the screen in maggies.

                        // Set lemming state to don't exist and reset animation frame.
                        lemming->state     = LEMSTATE_DONTEXIST;
                        lemming->animframe = 0;

                        // Set the trap state to working.
                        ingame_status->trap_state[trap].state     = TRAP_STATUS_STATE_WORKING;
                        // Set the trap frame to minus (with the update call bringing it to zero later).
                        ingame_status->trap_state[trap].animframe          = active_graphical_object_trap[level->trap_genus[trap]]->no_primary_frames;
                        ingame_status->trap_state[trap].animframe_subframe = 1;

                        return 1;
                     }
                  }
               }
            }
            
            // Check lemming state again. He might have been killed by a hazard.
            if (lemming->state != LEMSTATE_DONTEXIST) {
               // Check the lemming against all of the hazards on the level.
               for (u32 f = 0; f < level->no_hazards; f++) {
                  if ((lemming->x >= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x1))
                   && (lemming->x <= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x2))
                   && (lemming->y >= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y1))
                   && (lemming->y <= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y2))) {
                     // I'm burning!
                     lemming->state     = LEMSTATE_BURNT;

                     // SOUND REQUEST : lemming in flames
                     DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,
                                                                   0,
                                                                   lemming->x << log2magnification,    // This is the lemmings position in maggies.
                                                                   camera_x_inset);                    // This is the position of the left of the screen in maggies.

                     // Reset the lemming animation frame.
                     lemming->animframe = 0;
                     
                     return 1;
                  }
               }
            }

            // If the lemming has hit the ground on this tick, set the hit_ground flag.
            if (level_data[xcoord][lemming->y+1]) {
               hit_ground = 1;
               break;
            }
         }

         // If the lemming has hit the ground after the tick sequence:
         if (hit_ground) {
            // If the lemming hasn't fallen too far, or posesses floater power,
            // then they won't be smooshed by the fall.
            if ((lemming->falldistance < LEMMING_FALL_SMOOSH_THRESHHOLD)
             || (lemming->flags & LEMFLAG_FLOATER_POWER))  {
               // Change their state to walker and reset the frame.
               lemming->state = LEMSTATE_WALKER;
               lemming->animframe = 0;

               // Reset fall counter
               lemming->falldistance = 0;
            } else {
               // The lemming fell too far and didn't have floater power, so they
               // get smooshed.

               // Change the state, and reset the frame.
               lemming->state = LEMSTATE_SMOOSHED;
               lemming->animframe = 0;

               // SOUND REQUEST : Lemming smoosh against ground
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_SQUISH,
                                                             0,
                                                             lemming->x << log2magnification,  // This is the lemmings position in maggies.
                                                             camera_x_inset);                  // This is the position of the left of the screen in maggies.

               // Reset fall counter
               lemming->falldistance = 0;
            }
         } else {
            // If the lemming didn't hit the ground, they're still falling

            // Advance the lemming's frame
            lemming->animframe++;

            // If they're a faller, loop to the faller frame limit:
            if (lemming->state == LEMSTATE_FALLER) {
               lemming->animframe %= NO_LEM_FRAMES_FALLER;
            } else if (lemming->state == LEMSTATE_UMBRELLAWHIP) {
               // If they're an umbrellawhip lemming, and they're at the limit
               // of umbrellawhip frames, then change to a floater.
               if (lemming->animframe == NO_LEM_FRAMES_UMBRELLAWHIP) {
                  // Change state and reset frame counter.
                  lemming->state = LEMSTATE_FLOATER;
                  lemming->animframe = 0;
               }
               // If they're a floater, loop to the floater frame limit:
            } else if (lemming->state == LEMSTATE_FLOATER) {
               lemming->animframe %= NO_LEM_FRAMES_FLOATER;
            }
         }
      } // END OF LEMMING FALLER/UMBRELLAWHIP/FLOATER HANDLER

      // If the lemming is still alive after that gargantuan handler, they've got more
      // trials to content with!
      if (lemming->state != LEMSTATE_DONTEXIST) {
         // Check the lemming against each water area!
         // (The lemming hotspot must lie within the water area rectangle if they are to drown)
         for (u32 w = 0; w < level->no_waters; w++) {
            if ((lemming->x >= level->water_x1[w]                                               )
             && (lemming->x <= level->water_x2[w]                                               )
             && (lemming->y >= level->water_y[w] + active_graphical_object_water->active_zone_y1)
             && (lemming->y <= level->water_y[w] + active_graphical_object_water->active_zone_y2)) {
               // Change state to drowning, place lemming on surface of the water and reset frame.
               lemming->state     = LEMSTATE_DROWNING;
               lemming->y         = level->water_y[w];
               lemming->animframe = 0;

               // SOUND REQUEST : lemming hit water
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_HIT_WATER,
                                                             0,
                                                             lemming->x << log2magnification,           // This is the lemmings position in maggies.
                                                             camera_x_inset);                           // This is the position of the left of the screen in maggies.

               break;
            }
         }
      }
      if (lemming->state != LEMSTATE_DONTEXIST) {
         // Check the lemming against all of the hazards on the level.
         for (u32 f = 0; f < level->no_hazards; f++) {
            if ((lemming->x >= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x1))
             && (lemming->x <= (level->hazard_x[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_x2))
             && (lemming->y >= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y1))
             && (lemming->y <= (level->hazard_y[f] + active_graphical_object_hazard[level->hazard_genus[f]]->active_zone_y2))) {
               // I'm burning!
               lemming->state     = LEMSTATE_BURNT;

               // SOUND REQUEST : lemming in flames
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_IN_FLAMES,
                                                             0,
                                                             lemming->x << log2magnification,    // This is the lemmings position in maggies.
                                                             camera_x_inset);                    // This is the position of the left of the screen in maggies.

               // Reset the lemming animation frame.
               lemming->animframe = 0;
               break;
            }
         }
      }

      return 1;
   }
   return 0;
}

// Congratualations. If you understood all of the above, you've mastered the
// autonomous reactions of a lemming based on its own state and the state of
// the level in which it resides.

// Its not exactly perfect, but I think it's a pretty good job.

// Not bad for a clean-room implementation, eh? :P



// This is a function which takes the slice out of the landscape every time the lemming bashes.

// x and y are the lemming hotspot, d is the lemmings direction, and the frame
// is the frame into the bash animation to effect the bash mask from.
// Each frame has it's own subtraction mask to effect. (That's what makes the dirt gradually
// disintergrate)
void SubtractionBasher(int frame, int d, int x, int y) {
   // The direction into which the mask extends horizontally depends on the
   // direction the lemming is facing.
   s32 dx = ((d == 1) ? (1) : (-1));

   // These are hard-coded bash masks.
   // The nops are used to show pixels where the bash will already have been subtracted.
   if (frame == 2) {
         sma( 0,-8); sma( 1,-8); sma( 2,-8); sma( 3,-8); sma( 4,-8);
         sma( 0,-7); sma( 1,-7); sma( 2,-7); sma( 3,-7); sma( 4,-7); sma( 5,-7); sma( 6,-7);
         sma( 0,-6); sma( 1,-6); sma( 2,-6); sma( 3,-6); sma( 4,-6);
         sma( 0,-5); sma( 1,-5); sma( 2,-5);
         sma( 0,-4); sma( 1,-4); sma( 2,-4);
         sma( 0,-3); sma( 1,-3); sma( 2,-3);
         sma( 0,-2); sma( 1,-2); sma( 2,-2);
         sma( 0,-1); sma( 1,-1); sma( 2,-1);
   }
   else if (frame == 3) {
                     sma( 1,-8); nop( 2,-8); nop( 3,-8); nop( 4,-8);
                                 nop( 2,-7); nop( 3,-7); nop( 4,-7); nop( 5,-7); nop( 6,-7);
                                 nop( 2,-6); nop( 3,-6); nop( 4,-6); sma( 5,-6); sma( 6,-6);
                                 nop( 2,-5); sma( 3,-5); sma( 4,-5); sma( 5,-5);
                                 nop( 2,-4); sma( 3,-4); sma( 4,-4); sma( 5,-4);
                                 nop( 2,-3);
                                 nop( 2,-2);
                                 nop( 2,-1);
   }
   else if (frame == 4) {
                                 nop( 2,-8); nop( 3,-8); nop( 4,-8);
                                 nop( 2,-7); nop( 3,-7); nop( 4,-7); nop( 5,-7); nop( 6,-7);
                                 nop( 2,-6); nop( 3,-6); nop( 4,-6); nop( 5,-6); nop( 6,-6); sma( 7,-6);
                                 nop( 2,-5); nop( 3,-5); nop( 4,-5); nop( 5,-5); sma( 6,-5); sma( 7,-5);
                                 nop( 2,-4); nop( 3,-4); nop( 4,-4); nop( 5,-4); sma( 6,-4);
                                 nop( 2,-3); sma( 3,-3); sma( 4,-3); sma( 5,-3);
                                 nop( 2,-2); sma( 3,-2); sma( 4,-2); sma( 5,-2);
                                 nop( 2,-1); sma( 3,-1); sma( 4,-1); sma( 5,-1);
   }
   else if (frame == 5) {
                     nop( 1,-8); nop( 2,-8); nop( 3,-8); sma( 4,-8);
                     nop( 1,-7); nop( 2,-7); nop( 3,-7); nop( 4,-7); nop( 5,-7);
                     nop( 1,-6); nop( 2,-6); nop( 3,-6); nop( 4,-6); nop( 5,-6); nop( 6,-6);
                     nop( 1,-5); nop( 2,-5); nop( 3,-5); nop( 4,-5); nop( 5,-5); nop( 6,-5);
                     nop( 1,-4); nop( 2,-4); nop( 3,-4); nop( 4,-4); nop( 5,-4); sma( 6,-4);
                     nop( 1,-3); nop( 2,-3); nop( 3,-3); nop( 4,-3); sma( 5,-3); sma( 6,-3);
                     nop( 1,-2); nop( 2,-2); nop( 3,-2); nop( 4,-2); sma( 5,-2); sma( 6,-2);
                     nop( 1,-1); nop( 2,-1); nop( 3,-1); nop( 4,-1); sma( 5,-1); sma( 6,-1);
                     sma( 1, 0); sma( 2, 0); sma( 3, 0); sma( 4, 0); sma( 5, 0);
   }
}

// This is a function which takes the slice out of the landscape every time the lemming mines.

// x and y are the lemming hotspot, d is the lemmings direction, and the frame
// is the frame into the mine animation to effect the mine mask from.
// Each frame has it's own subtraction mask to effect. (That's what makes the dirt gradually
// disintergrate)
void SubtractionMiner(int frame, int d, int x, int y) {
   // The direction into which the mask extends horizontally depends on the
   // direction the lemming is facing.
   s32 dx = ((d == 1) ? (1) : (-1));

   // These are hard-coded mine masks.
   // The nops are used to show pixels where the bash will already have been subtracted.
   if (frame == 1) {
      sma(  0,-11); sma(  1,-11); sma(  2,-11); sma(  3,-11); sma(  4,-11); sma(  5,-11);
      sma(  0,-10); sma(  1,-10); sma(  2,-10); sma(  3,-10); sma(  4,-10); sma(  5,-10); sma(  6,-10);
      sma(  0, -9); sma(  1, -9); sma(  2, -9); sma(  3, -9); sma(  4, -9); sma(  5, -9); sma(  6, -9); sma(  7, -9);
      sma(  0, -8); sma(  1, -8); sma(  2, -8); sma(  3, -8); sma(  4, -8); sma(  5, -8); sma(  6, -8); sma(  7, -8);
      sma(  0, -7); sma(  1, -7); sma(  2, -7); sma(  3, -7); sma(  4, -7); sma(  5, -7); sma(  6, -7); sma(  7, -7);
      sma(  0, -6); sma(  1, -6); sma(  2, -6); sma(  3, -6); sma(  4, -6); sma(  5, -6);
      sma(  0, -5); sma(  1, -5); sma(  2, -5);
      sma(  0, -4); sma(  1, -4);
      sma(  0, -3); sma(  1, -3);
      sma(  0, -2); sma(  1, -2);
      sma(  0, -1); sma(  1, -1);
      sma(  0,  0); sma(  1,  0);
   } else if (frame == 2) {
                                                                                          sma(  6, -9);
                                                                                          sma(  6, -8);
                    sma(  1, -7); sma(  2, -7); sma(  3, -7); sma(  4, -7); sma(  5, -7); sma(  6, -7); sma(  7, -7);
      sma(  0, -6); sma(  1, -6); sma(  2, -6); sma(  3, -6); sma(  4, -6); sma(  5, -6); sma(  6, -6); sma(  7, -6);
      sma(  0, -5); sma(  1, -5); sma(  2, -5); sma(  3, -5); sma(  4, -5); sma(  5, -5); sma(  6, -5); sma(  7, -5);
      sma(  0, -4); sma(  1, -4); sma(  2, -4); sma(  3, -4); sma(  4, -4); sma(  5, -4); sma(  6, -4); sma(  7, -4);
      sma(  0, -3); sma(  1, -3); sma(  2, -3); sma(  3, -3); sma(  4, -3); sma(  5, -3); sma(  6, -3); sma(  7, -3);
      sma(  0, -2); sma(  1, -2); sma(  2, -2); sma(  3, -2); sma(  4, -2); sma(  5, -2); sma(  6, -2); sma(  7, -2);
      sma(  0, -1); sma(  1, -1); sma(  2, -1); sma(  3, -1); sma(  4, -1); sma(  5, -1); sma(  6, -1); sma(  7, -1);
      sma(  0,  0); sma(  1,  0); sma(  2,  0); sma(  3,  0); sma(  4,  0); sma(  5,  0); sma(  6,  0);
                                  sma(  2,  1); sma(  3,  1); sma(  4,  1); sma(  5,  1);
   }
}

// Gouge out an egg-shaped hole in the landscape for an explosion happing around a lemming with hotspot x, y.
void SubtractionExplosion(int d, int x, int y) {
   // The direction into which the mask extends horizontally depends on the
   // direction the lemming is facing.
   s32 dx = ((d == 1) ? (1) : (-1));

                                                                                       sma(-1, -13); sma( 0, -13); sma( 1, -13); sma( 2, -13);
                                                           sma(-3, -12); sma(-2, -12); sma(-1, -12); sma( 0, -12); sma( 1, -12); sma( 2, -12); sma( 3, -12); sma( 4, -12);
                                             sma(-4, -11); sma(-3, -11); sma(-2, -11); sma(-1, -11); sma( 0, -11); sma( 1, -11); sma( 2, -11); sma( 3, -11); sma( 4, -11); sma( 5, -11);
                                             sma(-4, -10); sma(-3, -10); sma(-2, -10); sma(-1, -10); sma( 0, -10); sma( 1, -10); sma( 2, -10); sma( 3, -10); sma( 4, -10); sma( 5, -10);
                               sma(-5,  -9); sma(-4,  -9); sma(-3,  -9); sma(-2,  -9); sma(-1,  -9); sma( 0,  -9); sma( 1,  -9); sma( 2,  -9); sma( 3,  -9); sma( 4,  -9); sma( 5,  -9); sma( 6,  -9);
                               sma(-5,  -8); sma(-4,  -8); sma(-3,  -8); sma(-2,  -8); sma(-1,  -8); sma( 0,  -8); sma( 1,  -8); sma( 2,  -8); sma( 3,  -8); sma( 4,  -8); sma( 5,  -8); sma( 6,  -8);
                 sma(-6,  -7); sma(-5,  -7); sma(-4,  -7); sma(-3,  -7); sma(-2,  -7); sma(-1,  -7); sma( 0,  -7); sma( 1,  -7); sma( 2,  -7); sma( 3,  -7); sma( 4,  -7); sma( 5,  -7); sma( 6,  -7); sma( 7,  -7);
                 sma(-6,  -6); sma(-5,  -6); sma(-4,  -6); sma(-3,  -6); sma(-2,  -6); sma(-1,  -6); sma( 0,  -6); sma( 1,  -6); sma( 2,  -6); sma( 3,  -6); sma( 4,  -6); sma( 5,  -6); sma( 6,  -6); sma( 7,  -6);
                 sma(-6,  -5); sma(-5,  -5); sma(-4,  -5); sma(-3,  -5); sma(-2,  -5); sma(-1,  -5); sma( 0,  -5); sma( 1,  -5); sma( 2,  -5); sma( 3,  -5); sma( 4,  -5); sma( 5,  -5); sma( 6,  -5); sma( 7,  -5);
   sma(-7,  -4); sma(-6,  -4); sma(-5,  -4); sma(-4,  -4); sma(-3,  -4); sma(-2,  -4); sma(-1,  -4); sma( 0,  -4); sma( 1,  -4); sma( 2,  -4); sma( 3,  -4); sma( 4,  -4); sma( 5,  -4); sma( 6,  -4); sma( 7,  -4); sma( 8,  -4);
   sma(-7,  -3); sma(-6,  -3); sma(-5,  -3); sma(-4,  -3); sma(-3,  -3); sma(-2,  -3); sma(-1,  -3); sma( 0,  -3); sma( 1,  -3); sma( 2,  -3); sma( 3,  -3); sma( 4,  -3); sma( 5,  -3); sma( 6,  -3); sma( 7,  -3); sma( 8,  -3);
   sma(-7,  -2); sma(-6,  -2); sma(-5,  -2); sma(-4,  -2); sma(-3,  -2); sma(-2,  -2); sma(-1,  -2); sma( 0,  -2); sma( 1,  -2); sma( 2,  -2); sma( 3,  -2); sma( 4,  -2); sma( 5,  -2); sma( 6,  -2); sma( 7,  -2); sma( 8,  -2);
   sma(-7,  -1); sma(-6,  -1); sma(-5,  -1); sma(-4,  -1); sma(-3,  -1); sma(-2,  -1); sma(-1,  -1); sma( 0,  -1); sma( 1,  -1); sma( 2,  -1); sma( 3,  -1); sma( 4,  -1); sma( 5,  -1); sma( 6,  -1); sma( 7,  -1); sma( 8,  -1);
   sma(-7,   0); sma(-6,   0); sma(-5,   0); sma(-4,   0); sma(-3,   0); sma(-2,   0); sma(-1,   0); sma( 0,   0); sma( 1,   0); sma( 2,   0); sma( 3,   0); sma( 4,   0); sma( 5,   0); sma( 6,   0); sma( 7,   0); sma( 8,   0);
   sma(-7,   1); sma(-6,   1); sma(-5,   1); sma(-4,   1); sma(-3,   1); sma(-2,   1); sma(-1,   1); sma( 0,   1); sma( 1,   1); sma( 2,   1); sma( 3,   1); sma( 4,   1); sma( 5,   1); sma( 6,   1); sma( 7,   1); sma( 8,   1);
   sma(-7,   2); sma(-6,   2); sma(-5,   2); sma(-4,   2); sma(-3,   2); sma(-2,   2); sma(-1,   2); sma( 0,   2); sma( 1,   2); sma( 2,   2); sma( 3,   2); sma( 4,   2); sma( 5,   2); sma( 6,   2); sma( 7,   2); sma( 8,   2);
                 sma(-6,   3); sma(-5,   3); sma(-4,   3); sma(-3,   3); sma(-2,   3); sma(-1,   3); sma( 0,   3); sma( 1,   3); sma( 2,   3); sma( 3,   3); sma( 4,   3); sma( 5,   3); sma( 6,   3); sma( 7,   3);
                 sma(-6,   4); sma(-5,   4); sma(-4,   4); sma(-3,   4); sma(-2,   4); sma(-1,   4); sma( 0,   4); sma( 1,   4); sma( 2,   4); sma( 3,   4); sma( 4,   4); sma( 5,   4); sma( 6,   4); sma( 7,   4);
                 sma(-6,   5); sma(-5,   5); sma(-4,   5); sma(-3,   5); sma(-2,   5); sma(-1,   5); sma( 0,   5); sma( 1,   5); sma( 2,   5); sma( 3,   5); sma( 4,   5); sma( 5,   5); sma( 6,   5); sma( 7,   5);
                               sma(-5,   6); sma(-4,   6); sma(-3,   6); sma(-2,   6); sma(-1,   6); sma( 0,   6); sma( 1,   6); sma( 2,   6); sma( 3,   6); sma( 4,   6); sma( 5,   6); sma( 6,   6);
                                             sma(-4,   7); sma(-3,   7); sma(-2,   7); sma(-1,   7); sma( 0,   7); sma( 1,   7); sma( 2,   7); sma( 3,   7); sma( 4,   7); sma( 5,   7);
                                                                         sma(-2,   8); sma(-1,   8); sma( 0,   8); sma( 1,   8); sma( 2,   8); sma( 3,   8);

}

// These define the metrics used to display the interface at the bottom of the screen.
#define INTERFACE_TOOL_NUMBER_SPACING   4

#define INTERFACE_SCREEN_POSITION_X        0
#define INTERFACE_SCREEN_POSITION_Y      168

#define INTERFACE_TOOL_WIDTH              16
#define INTERFACE_TOOL_HEIGHT             24

// These control how many pixels inset the tool numbers should be displayed.
#define INTERFACE_TOOL_NUMBERS_INSET_X     5
#define INTERFACE_TOOL_NUMBERS_INSET_Y     2

// This function places a tool number sprite to the main screen at the specified location
// If 'number' is -1, then the sprite is invisibled
// Placing number 10 gives you the infinity symbol.
void PlaceInterfaceNumberSpriteNumber(int sprite_no, int number, int x, int y) {
   if (number != -1) {
      // Place the number sprite at the specified location.
      sprite_shadow_m[sprite_no].attribute[0] = y;
      sprite_shadow_m[sprite_no].attribute[1] = x;
      sprite_shadow_m[sprite_no].attribute[2] = 9 + number;
   } else {
      sprite_shadow_m[sprite_no].attribute[0] = 0;
      sprite_shadow_m[sprite_no].attribute[1] = 0;
      sprite_shadow_m[sprite_no].attribute[2] = 0;
   }
}

// These are the limits (_I = inside of tool, _O = outside of tool)
// for the on-screen coordinates of each tool.
#define INTERFACE_FF_LEFT_X_I                   (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 0))
#define INTERFACE_FF_RIGHT_X_O                  (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 1))

#define INTERFACE_ZOOM_PAIR_LEFT_X_I            (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 1))
#define INTERFACE_ZOOM_IN_LEFT_X_I              (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 2))
#define INTERFACE_ZOOM_PAIR_RIGHT_X_O           (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 3))

#define INTERFACE_RELEASE_RATE_MINIMUM_LEFT_X_I (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 4))
#define INTERFACE_RELEASE_RATE_LEFT_X_I         (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 4))
#define INTERFACE_RELEASE_RATE_LEFT_X_O         (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 5))
#define INTERFACE_RELEASE_RATE_RIGHT_X_I        (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 5))
#define INTERFACE_RELEASE_RATE_RIGHT_X_O        (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 6))

#define INTERFACE_TOOL_FAR_LEFT_X_I             (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 6))
#define INTERFACE_TOOL_FAR_RIGHT_X_O            (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 14))

#define INTERFACE_PAUSE_LEFT_X_I                (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 14))
#define INTERFACE_PAUSE_RIGHT_X_O               (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 15))

#define INTERFACE_NUKE_LEFT_X_I                 (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 15))
#define INTERFACE_NUKE_RIGHT_X_O                (INTERFACE_SCREEN_POSITION_X + (INTERFACE_TOOL_WIDTH * 16))


// This updates the interface to the bottom of the main screen
void UpdateIngameInterface(LEVEL_INGAME_STATUS_STRUCT *ingame_status, bool draw_tool_rect) {
   // Only draw currently selected tool rectangle if instructed.
   if (draw_tool_rect) {
      // Place the selected tool sprite (sprite 20) at the correct location.
      sprite_shadow_m[20].attribute[0] = ATTR0_TALL | INTERFACE_SCREEN_POSITION_Y;
      sprite_shadow_m[20].attribute[1] = INTERFACE_SCREEN_POSITION_X +
                                             (INTERFACE_TOOL_WIDTH * (6 + ingame_status->current_tool)) | (1 << 15);
      sprite_shadow_m[20].attribute[2] = 1;
   } else {
      // Get rid of the selected tool sprite.
      sprite_shadow_m[20].attribute[0] = 0;
      sprite_shadow_m[20].attribute[1] = 0;
      sprite_shadow_m[20].attribute[2] = 0;
   }

   // Variables used for parsing integer variables into their component parts.
   u32 tens, units;

   // Parse the minimum release rate value into its component numbers.
   ParseIntIntoComponents(ingame_status->release_rate_minimum, &tens, &units);
   // Place the numbers for the release rate minimum.
   // Sprites 0 and 1 are the release rate minimum numbers,
   // sprites 2 and 3 are the current release rate numbers.
   PlaceInterfaceNumberSpriteNumber(    0,
                                     tens,
                                    INTERFACE_RELEASE_RATE_MINIMUM_LEFT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X,
                                    INTERFACE_SCREEN_POSITION_Y             + INTERFACE_TOOL_NUMBERS_INSET_Y);
   PlaceInterfaceNumberSpriteNumber(    1,
                                    units,
                                    INTERFACE_RELEASE_RATE_MINIMUM_LEFT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X + INTERFACE_TOOL_NUMBER_SPACING,
                                    INTERFACE_SCREEN_POSITION_Y             + INTERFACE_TOOL_NUMBERS_INSET_Y);

   // Parse the current release rate value into its component numbers.
   ParseIntIntoComponents(ingame_status->release_rate, &tens, &units);
   // Place the numbers for the current release.
   PlaceInterfaceNumberSpriteNumber(    2,
                                     tens,
                                    INTERFACE_RELEASE_RATE_RIGHT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X,
                                    INTERFACE_SCREEN_POSITION_Y      + INTERFACE_TOOL_NUMBERS_INSET_Y);
   PlaceInterfaceNumberSpriteNumber(    3,
                                    units,
                                    INTERFACE_RELEASE_RATE_RIGHT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X + INTERFACE_TOOL_NUMBER_SPACING,
                                    INTERFACE_SCREEN_POSITION_Y      + INTERFACE_TOOL_NUMBERS_INSET_Y);

   // Draw the numbers for all of the tools.
   for (int tool = 0; tool < 8; tool++) {
      // Make them blank if there's none of this tool available.
      if (ingame_status->remaining_tools[tool] == 0) {
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 4, -1, 0, 0);
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 5, -1, 0, 0);
      } else if (ingame_status->remaining_tools[tool] == 100) {
         // You have infinite of this tool
         // Draw the infinity symbol using the left sprite, make the right sprite invisible.
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 4,
                                                    10,
                                          INTERFACE_TOOL_FAR_LEFT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X + (tool * INTERFACE_TOOL_WIDTH),
                                          INTERFACE_SCREEN_POSITION_Y      + INTERFACE_TOOL_NUMBERS_INSET_Y);
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 5, -1, 0, 0);
      } else {
         // Parse the remaining number of this tool into its component numbers.
         ParseIntIntoComponents(ingame_status->remaining_tools[tool], &tens, &units);
         // Draw the numbers for the tool now.
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 4,
                                                  tens,
                                          INTERFACE_TOOL_FAR_LEFT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X + (tool * INTERFACE_TOOL_WIDTH),
                                          INTERFACE_SCREEN_POSITION_Y      + INTERFACE_TOOL_NUMBERS_INSET_Y);
         PlaceInterfaceNumberSpriteNumber(tool * 2 + 5,
                                                 units,
                                          INTERFACE_TOOL_FAR_LEFT_X_I + INTERFACE_TOOL_NUMBERS_INSET_X + (tool * INTERFACE_TOOL_WIDTH) + INTERFACE_TOOL_NUMBER_SPACING,
                                          INTERFACE_SCREEN_POSITION_Y      + INTERFACE_TOOL_NUMBERS_INSET_Y);
      }
   }
}

// These are the maximum and minimum values for magnification (not log2!)
#define MAGNIFICATION_MAX  8
#define MAGNIFICATION_MIN  1

// This changes the camera inset so that the camera will stay focused around the same central point
// when changing from zoom level log2mold to log2mnew
void ZoomToFromLogMag(int log2mnew, int log2mold) {
   camera_x_inset = ((camera_x_inset + GAME_DISPLAY_X_SIZE2) >> (log2mold - log2mnew)) - GAME_DISPLAY_X_SIZE2;
   camera_y_inset = ((camera_y_inset + GAME_DISPLAY_Y_SIZE2) >> (log2mold - log2mnew)) - GAME_DISPLAY_Y_SIZE2;
}
// It doesn't work... it's left in for fun, I guess.
// (Hey, I'm a lot more helpful than other documentation!)

// This function alters the zoom variables and the camera inset
// so that the camera is zoomed in one more 'level'.
// (Up to MAGNIFICATION_MAX)
int ZoomIn() {
   if (magnification != MAGNIFICATION_MAX) {
      //ZoomToFromLogMag(log2magnification+1, log2magnification);
      camera_x_inset = (camera_x_inset + GAME_DISPLAY_X_SIZE4) << 1;
      camera_y_inset = (camera_y_inset + GAME_DISPLAY_Y_SIZE4) << 1;
      log2magnification++;
      magnification <<= 1;
      return 1; // success
   } else {
      return 0; // failure
   }
}

// This function alters the zoom variables and the camera inset
// so that the camera is zoomed out one more 'level'.
// (Up to MAGNIFICATION_MIN)
int ZoomOut() {
   if (magnification != MAGNIFICATION_MIN) {
      //ZoomToFromLogMag(log2magnification-1, log2magnification);
      camera_x_inset = (camera_x_inset >> 1) - GAME_DISPLAY_X_SIZE4;
      camera_y_inset = (camera_y_inset >> 1) - GAME_DISPLAY_Y_SIZE4;
      log2magnification--;
      magnification >>= 1;
      return 1; // success
   } else {
      return 0; // failure
   }
}

// This focuses the camera around a specific point on the level by altering
// the camera inset variables.
void CameraFocusTo(int x, int y) {
   camera_x_inset = x - (GAME_DISPLAY_X_SIZE >> (log2magnification+1));
   camera_y_inset = y - (GAME_DISPLAY_Y_SIZE >> (log2magnification+1));

   if (camera_x_inset < 0) {
      camera_x_inset = 0;
   }
   if (camera_x_inset > ((s32)((LEVEL_X_SIZE  << log2magnification)) - GAME_DISPLAY_X_SIZE)) {
      camera_x_inset = ((LEVEL_X_SIZE  << log2magnification)) - GAME_DISPLAY_X_SIZE;
   }
   if (camera_y_inset < 0) {
      camera_y_inset = 0;
   }
   if (camera_y_inset > ((s32)((LEVEL_Y_SIZE  << log2magnification)) - GAME_DISPLAY_Y_SIZE)) {
      camera_y_inset = ((LEVEL_Y_SIZE  << log2magnification)) - GAME_DISPLAY_Y_SIZE;
   }
}

// -----------------------------------------------------------------------------

// & the BG_CR register with this to get rid of the bmp 16bit colour data source offset
// which is stored on bits 8, 9, 10, 11, and 12
#define BG_CR_BITMAP_BASE_BIT_MASK (BIT( 0) | \
                                    BIT( 1) | \
                                    BIT( 2) | \
                                    BIT( 3) | \
                                    BIT( 4) | \
                                    BIT( 5) | \
                                    BIT( 6) | \
                                    BIT( 7) | \
                                    BIT(13) | \
                                    BIT(14) | \
                                    BIT(15))

// Flip the display IO to set the exrot bitmap to use the alternate page of the bitmap memory.
void FlipDisplayPages() {
   // Get rid of any bitmap base data, and just leave the rest.
   BG2_CR &= BG_CR_BITMAP_BASE_BIT_MASK;

   // If we just wrote to 0x6020000, display from that base (that's  eight  of 0x4000)
   // if we just wrote to 0x6040000, display from that base (that's sixteen of 0x4000)
   BG2_CR |= (drawbuffer == (u16*)0x06020000) ? BG_BMP_BASE(8) : BG_BMP_BASE(16);

   drawbuffer = (drawbuffer == (u16*)0x06020000) ? (u16*)0x06040000 : (u16*)0x06020000;
}

// Copies a series of no_colours colours from palette_original to destination
// while reducing each component of each colour by fading_by.
void CopyPaletteFromToFadingBy(const u16 *palette_original, u16 *destination, u32 no_colours, u32 fading_by) {
   int r, g, b, c;

   for (u32 colour = 0; colour < no_colours; colour++) {
      c = palette_original[colour] & 0x7fff; // ditch alpha bit

      // Compute the new component values by splitting and subtracting.
      r = (c >>  0) & 31; r -= fading_by; if (r<0) r = 0;
      g = (c >>  5) & 31; g -= fading_by; if (g<0) g = 0;
      b = (c >> 10) & 31; b -= fading_by; if (b<0) b = 0;

      destination[colour] = RGB15A(r, g, b);
   }
}

// This sets up the display IO for the main Lemmings gameplay.
void PlayLemmingsDS_SetUpGoLemmingsLevelGFXModeMainScreen() {
   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // The MAIN screen is Mode 5...
   videoSetMode(MODE_5_2D | DISPLAY_BG1_ACTIVE      // Interface GFX are on BG 1
                          | DISPLAY_BG2_ACTIVE      // Main ingame GFX are on BG 2
                          | DISPLAY_SPR_ACTIVE      // Sprites for the interface...
                          | DISPLAY_SPR_1D_LAYOUT); // are linear! :)

   // Background 2 is a 16 bit 256x256 bitmap, priority 3.
   BG2_CR = BG_BMP16_256x256 | BG_PRIORITY_3;

   // Map VRAM A ram space as 0x06020000, and map
   // VRAM B ram space as 0x06040000 next to it for our back page.
   vramSetMainBanks(VRAM_A_MAIN_BG_0x06020000,
                    VRAM_B_MAIN_BG_0x06040000,
                    VRAM_C_SUB_BG            ,
                    VRAM_D_SUB_SPRITE        );

   // Map banks F and G for main screen backgrounds and sprites.
   vramSetBankF(VRAM_F_MAIN_BG);
   vramSetBankG(VRAM_G_MAIN_SPRITE);

   // Set affine matrix for the exrot BG2
   BG2_XDX = 0x100;
   BG2_XDY = 0;
   BG2_YDX = 0;
   BG2_YDY = 0x100;
   BG2_CY = 0;
   BG2_CX = 0;

   // Copy the graphics for the lower main screen interface, the number sprites
   // and the selected tool sprite:
   // Copy the selected tool sprite graphics into sprite tile 1.
	for (int d = 0; d < (gfx_interface_selected_toolTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (SPRITE_GFX+(S_TILE_SIZE_u16 *   1))[d] = gfx_interface_selected_toolTiles[d];
   }

	// Copy the two palette entries for the sprite part of the interface to the sprite palette
   for (int pe = 0; pe < 2; pe++) {
      SPRITE_PALETTE[pe + 1] = gfx_interface_selected_toolPal[pe];
   }

   // Copy the interface number graphics into sprite tile 9.
	for (int d = 0; d < (gfx_interface_numbersTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (SPRITE_GFX+(S_TILE_SIZE_u16 *   9))[d] = gfx_interface_numbersTiles[d];
   }

   // Copy the interface graphic tiles into tiles 1 onwards:
	for (int d = 0; d < (gfx_interfaceTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = gfx_interfaceTiles[d];
   }

	// Copy the palette entries for the interface to the background palette
   for (int pe = 0; pe < 16; pe++) {
      BG_PALETTE[pe] = gfx_interfacePal[pe];
   }

   // Set up the background register for background one.
   // Tegel base 4, priority 2.
	BG1_CR = BG_COLOR_16 | (4 << 8) | 2;

   // Clear translation registers for BG1
   BG1_X0 = 0;
   BG1_Y0 = 0;

   // Set up the tegels for the lower interface:
   for (int tegelc = 0; tegelc < (32 * 3); tegelc++) {
      (*((vu16*)(SCREEN_BASE_BLOCK(4)) + (tegelc) + (21 * 32))) = tegelc + 1;
   }

   // We want to blend everything on the main screen with black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;
   BLEND_Y = 16;                                     

   // Clear the framebuffer level display pages.
   memset((void *)0x06020000, 0, SCREEN_WIDTH * INTERFACE_SCREEN_POSITION_Y * 2);
   memset((void *)0x06040000, 0, SCREEN_WIDTH * INTERFACE_SCREEN_POSITION_Y * 2);
}

// These constants define the possible values of the 'past_shown' parameter to the PlayLemmingsDS_GoLemmingsLevel function
#define PLAY_LEMMINGS_DS_GO_LEMMINGS_LEVEL_PAST_SHOWN_LEVEL_INFO 0 // We last saw the level info screen
#define PLAY_LEMMINGS_DS_GO_LEMMINGS_LEVEL_PAST_SHOWN_INGAME     1 // We last saw a level in progress.

// Depending on the layout of the screen when this function is called, there is
// a different response. If we're moving FROM the level info screen INTO the Lemmings level,
// then the top map doesn't need to fade on (although the in/out and the time still need to).

// If we're fading from a level in progress, the map will have faded to black (because it's
// invalid: we want a nice fresh map if we're going to restart the level!)
// If we're coming into the Lemmings level from a restart, we need to fade in the map together
// with the in/out and time display.

// This will run a complete level of Lemmings DS using the runtime stats contained in the
// malloced LEMMINGS_LEVEL_RUNTIME_STATS_V7 *current_level_runtime_stats and the prerendered
// level contained within the malloced u8 (*)[LEVEL_Y_SIZE] level data.
//
// status is a pointer to a LEVEL_STATUS_STRUCT into which
// the result of playing the level can be stored.
// The past_shown parameter tells the function what phase was displayed directly before this
// one.
void PlayLemmingsDS_GoLemmingsLevel(LEVEL_STATUS_STRUCT *status, int past_shown) {  
   DebugAppend("Before instantiation\r\n");
   DebugWrite();

   // Use this to keep track of the game in progress:
   LEVEL_INGAME_STATUS_STRUCT *ingame_status_struct = (LEVEL_INGAME_STATUS_STRUCT *)malloc(sizeof(LEVEL_INGAME_STATUS_STRUCT));
   DEBUG_SECTION {
      if (ingame_status_struct == NULL) {
         DebugAppend("Failed to instantiate LEVEL_INGAME_STATUS_STRUCT structure for gameplay.\r\n");
         DebugWrite();

         // Crash
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_INGAME_STATUS_STRUCT);
      }
   }

   DebugAppend("Just instantiated.\r\n");
   DebugWrite();

   DEBUG_SECTION {   
      char output_text[4096];
      siprintf(output_text, "ingame_status_struct lies at %08X and is %d big.\r\n", (int)ingame_status_struct, sizeof(LEVEL_INGAME_STATUS_STRUCT));
      DebugAppend(output_text);
      DebugWrite();
   }

   // Blank it.
   memset(ingame_status_struct, 0, sizeof(LEVEL_INGAME_STATUS_STRUCT));

   DebugAppend("Blanked new status struct memory.\r\n");
   DebugWrite();

   // Get pointer to lemming info array
   LEMMING_INFO_STRUCT *lemming_info = ingame_status_struct->lemming_info;

   DebugAppend("Lemming info pointer acquired.\r\n");
   DebugWrite();

   // Reset all lemmings.
   for (u32 l = 0; l < MAX_LEMMINGS; l++) {
      lemming_info[l].state        = LEMSTATE_DONTEXIST;
      lemming_info[l].animframe    = 0;
      lemming_info[l].rembricks    = 0;
      lemming_info[l].falldistance = 0;
   }

   // Reset all of the variables for a new level run.
   status->status_flags   = 0;
   status->lemmings_saved = 0;

   ingame_status_struct->lemmings_exited = 0;

   // Reset tools.
   ingame_status_struct->current_tool = TOOL_CLIMBER;

   for (int t = 0; t < NO_TOOLS; t++) {
      ingame_status_struct->remaining_tools[t] = current_level_runtime_stats.tool_complement[t];
   }

   // This holds the ID of the next lemming that should be nuked.
   // If it holds the value NO_LEMMING_NUKE, then no nuke has been called.
   ingame_status_struct->next_lemming_to_nuke   = NO_LEMMING_NUKE;

   // This is a frame counter counting the number of frames since the last
   // lemming logic tick. The number of display frames necessary to update the
   // lemming logic will change depending on fast-forward, etc.
   ingame_status_struct->lemming_update_time    = 0;

   // This holds the number of frames (in lemming logic ticks) since the last lemming release.
   ingame_status_struct->lemming_release_timer  =  0;
   // This holds the number of the next entrance to create a lemming from.
   ingame_status_struct->next_entrance_to_use   =  0;
   // This holds the number of lemmings that have dropped from the trapdoors.
   ingame_status_struct->lemmings_dispensed     =  0;

   // This is set to 1 if master pause (the PAWS) is enabled.
   ingame_status_struct->master_pause_flag      =  0;

   // Trapdoors start closed
   ingame_status_struct->entrance_state.animframe = 1;

   // Manually reset animframes of all objects:
   ingame_status_struct->exit_state.animframe          = 0;
   ingame_status_struct->exit_state.animframe_subframe = 0;

   for (u32 trap = 0; trap < current_level_runtime_stats.no_traps; trap++) {
      ingame_status_struct->trap_state[trap].state              = TRAP_STATUS_STATE_IDLE;
      ingame_status_struct->trap_state[trap].animframe          = 0;
      ingame_status_struct->trap_state[trap].animframe_subframe = 0;
   }

   for (u32 hazard_genus = 0; hazard_genus < NO_HAZARD_GENUSES; hazard_genus++) {
      ingame_status_struct->hazard_state[hazard_genus].animframe          = 0;
      ingame_status_struct->hazard_state[hazard_genus].animframe_subframe = 0;
   }

   for (u32 uninteractive_genus = 0; uninteractive_genus < NO_UNINTERACTIVE_GENUSES; uninteractive_genus++) {
      ingame_status_struct->uninteractive_state[uninteractive_genus].animframe          = 0;
      ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe = 0;
   }

   ingame_status_struct->water_state.animframe          = 0;
   ingame_status_struct->water_state.animframe_subframe = 0;


   // Reset camera variables.
   magnification = 1;
   log2magnification = 0;
   CameraFocusTo(current_level_runtime_stats.camera_x, current_level_runtime_stats.camera_y);
                                                    
   DebugAppend("Lemming dust simulation.\r\n");
   DebugWrite();

   // Generate a unique dust pattern for exploding lemmings.
   LemmingDustSimulation();

   // Set up the time remaining and release rate variables.
   ingame_status_struct->time_remaining                                            = current_level_runtime_stats.time_in_minutes*LEMMING_FRAMES_PER_SECOND_REMAINING_TIME*60;
   ingame_status_struct->release_rate = ingame_status_struct->release_rate_minimum = current_level_runtime_stats.release_rate;

   // These are the different states which the level can be in:
#define LEVEL_STATE_FADE_IN   0 // The level is fading in from black.
#define LEVEL_STATE_OPENWAIT  1 // The level is in progress, but the trapdoor hasn't fully opened yet.
#define LEVEL_STATE_STABLE    2 // The level is in progress, the trapdoor is open and there are lemmings active.
#define LEVEL_STATE_FADE_OUT  3 // The level is fading out to black.

   // The level starts by fading in. (This state holds the pause between level start and trapdoor open)
   int level_state = LEVEL_STATE_FADE_IN;

   // These are the different fade states the level can be in:
#define LEVEL_FADE_STATE_NO_FADE  0
#define LEVEL_FADE_STATE_FADE_IN  1
#define LEVEL_FADE_STATE_FADE_OUT 2

   // This controls the actual FADE state.
   int level_fade_state = LEVEL_FADE_STATE_FADE_IN;

   // Blank the top time strip.
   UpdateSubScreenAsLevelInfo_BlankInOutTime();

   // If the last phase shown was the level information screen, then:
   if (past_shown == PLAY_LEMMINGS_DS_GO_LEMMINGS_LEVEL_PAST_SHOWN_LEVEL_INFO) {
      // Set the sub screen so that only the in-out timer fades in from black.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME);
   } else {
      // If the last phase wasn't the level info, we need to fade the map as well as the in-out timer.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME
                                              | UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);
   }

   // Tell the top screen to set the blend registers to 16.
   UpdateSubScreenAsLevelInfo_SetFadeValue(16);

   // This controls game logic like the opening of the trapdoor, and the first lemming timer.
   int level_game_logic_frame =  0;
   // This variable indirectly controls the current fade frame
   int level_fade_logic_frame =  0;
   // This variable controls the current fade value
   int fade_value             =  0;

   // This holds the ID of the current lemming that the stylus is hovering over
   // (current_level_runtime_stats.lemmings) means that no lemming is selected.
   u32 selected_hover_lemming = (current_level_runtime_stats.lemmings);
   // (HOLDER style only.)

   // Length in real-time display ticks for a bonk of the release rate.
#define RELEASE_RATE_BONK_LENGTH 2
   // This holds the total number of frames that the release rate buttons have been held down.
   // When it reaches the value RELEASE_RATE_BONK_LENGTH it will be reset to zero and the
   // release rate will be affected.
   int release_rate_bonk_timer = 0;

   // These control how fast you must double tap SELECT to trigger the restart level flag.
#define SELECT_PRESS_TOO_SOON_BEFORE 3  // You must leave at least this number of frames - 1 before tapping again.
#define SELECT_PRESS_TOO_LATE_AFTER  25 // You must tap before this number of frames have elapsed.

   u8  select_press_timer        = 255; // This is the time since the select press just gone.
   u8  second_select_press_timer = 255; // This is the time since the select press two buttons ago.

   // These control how fast you must double tap SELECT to trigger the restart level flag.
#define NUKE_PRESS_TOO_SOON_BEFORE 5    // You must leave at least this number of frames - 1 before tapping again.
#define NUKE_PRESS_TOO_LATE_AFTER  15   // You must tap before this number of frames have elapsed.

   // This holds the time in real-time display frames since the last nuke button press.
   u8  nuke_press_timer          = 255;

   // When this is set to true, the main Lemmings loop will exit.
   int toexit = 0;

   // This holds a combination of the following flags.
   u32 game_flags = 0;

#define GameFlag(x) (bits(game_flags, x))

   // These flags tell the game that certain events have happened that need to be
   // handled.
#define GAME_FLAG_ZOOM_OUT         BIT( 1) // Must zoom out.
#define GAME_FLAG_ZOOM_IN          BIT( 2) // Must zoom in.
#define GAME_FLAG_RR_UP            BIT( 3) // Must increase release rate.
#define GAME_FLAG_RR_DOWN          BIT( 4) // Must decrease release rate.
#define GAME_FLAG_NUKE_TRIGGER     BIT( 5) // Must trigger nuke.
#define GAME_FLAG_FF               BIT( 6) // Must FF this frame.
#define GAME_FLAG_CAM_L            BIT( 7) // Must move camera left.
#define GAME_FLAG_CAM_R            BIT( 8) // Must move camera right.
#define GAME_FLAG_CAM_U            BIT( 9) // Must move camera up.
#define GAME_FLAG_CAM_D            BIT(10) // Must move camera down.
#define GAME_FLAG_PAUSE            BIT(11) // Must not allow lemming logic to proceed.
#define GAME_FLAG_RESTART_TRIGGER  BIT(12) // Must begin level restart sequence.

   //DebugAppend("Entering main loop.\r\n");
   //DebugWrite();

   // Main loop.
   while (!toexit) {
      // Update joy and joyp keycodes.
      ScanKeypad();

      // Read new touchscreen pen coordinates if available.
		ScanPenCoords();

		// ------------------------------------------------------------

      // Reset flags for this frame.
		game_flags = 0;

		// Let's analyse the game and input to come up with some flags.

      // Elementary camera controls:

      // If the player is pressing one camera control and
      // not the opposite one, then the camera should move
      // in that direction.
		if ((joy & KEY_LEFT) && !(joy & KEY_RIGHT)) {
         game_flags |= GAME_FLAG_CAM_L;
      } else if ((joy & KEY_RIGHT) && !(joy & KEY_LEFT )) {
         game_flags |= GAME_FLAG_CAM_R;
      }
      if ((joy & KEY_UP   ) && !(joy & KEY_DOWN )) {
         game_flags |= GAME_FLAG_CAM_U;
      } else if ((joy & KEY_DOWN ) && !(joy & KEY_UP   )) {
         game_flags |= GAME_FLAG_CAM_D;
      }
      if ((joy & KEY_Y    ) && !(joy & KEY_A    )) {
         game_flags |= GAME_FLAG_CAM_L;
      } else if ((joy & KEY_A    ) && !(joy & KEY_Y    )) {
         game_flags |= GAME_FLAG_CAM_R;
      }
      if ((joy & KEY_X    ) && !(joy & KEY_B    )) {
         game_flags |= GAME_FLAG_CAM_U;
      } else if ((joy & KEY_B    ) && !(joy & KEY_X    )) {
         game_flags |= GAME_FLAG_CAM_D;
      }

      // If Start is pressed, we're in fast forward for this frame.
      if (joy & KEY_START) {
         game_flags |= GAME_FLAG_FF;
      }

      // Reset selected hover lemming if we don't want it to be persistent between frames:
      if (control_style_major != CONTROL_STYLE_MAJOR_TACTICS) {
         // Reset the lemming hover variable.
         selected_hover_lemming = (current_level_runtime_stats.lemmings);
      }

      // This code allows the player to select lemmings, and pause:
      // You can't select lemmings or pause (both control setups) while fading out.
      if (level_state != LEVEL_STATE_FADE_OUT) {
         // The different control styles need different handlers:
         if (control_style_major == CONTROL_STYLE_MAJOR_TAPPER) { // Tapper style!
            // First, we manage the pause controls:
            if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) { // Holder pause!
               // Tapper hold allows you to do one of two things:
               // 1) If you hold one shoulder button and tap the other, you skip a single lemming logic frame.
               if (((  joy & KEY_R ) && (KeyDown(KEY_L)))
                || ((KeyDown(KEY_R)) && (  joy & KEY_L) )) { // Hold a shoulder, tap the other
                  game_flags |= GAME_FLAG_FF; // Skip a frame!
               } else
               // 2) If you're holding a single shoulder button, the game is paused.
               if ((joy & KEY_R) || (joy & KEY_L)) { // Hold any shoulder button.
                  game_flags |= GAME_FLAG_PAUSE;
               }
            } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) { // Toggle pause!
               // Tapper toggle allows you to do one thing:
               // If you tap a single shoulder button while leaving the other one alone,
               // you can toggle the master pause.
               if ((KeyDown(KEY_R) && !(joy & KEY_L))      // Tap a single shoulder button,
                || (KeyDown(KEY_L) && !(joy & KEY_R))) {   // while leaving the other one alone!
                  ingame_status_struct->master_pause_flag = (ingame_status_struct->master_pause_flag == 0) ? 1 : 0; // Togle the master pause.
               }
            }

            // Second, we manage the lemming selection controls.
            // You can't select lemmings if we don't have any tools. (of the current type)
            if ((ingame_status_struct->remaining_tools[ingame_status_struct->current_tool] != 0) || (debug_lems)) {
               // If the pen has been tapped against the screen this frame:
               if (KeyDown(KEY_TOUCH)) {
                  // Check to see if the pen is actually inside the game area, and not the interface.
                  if (touchXY.py < GAME_DISPLAY_Y_SIZE) {
                     // Make a POINT from the current click location -in camera space-.
                     POINT click_location_cameraspace = {touchXY.px, touchXY.py};

                     // Use lem to try to find a lemming below the pen
                     // to which the current tool can be assigned.
                     u32 lem;
                     for (lem = 0; lem < (current_level_runtime_stats.lemmings); lem++) {
                        // Test the pens location against all lemmings, and break if
                        // one of the lemmings is below the pen and can be clicked on.
                        if (ClickAgainstLemming(ingame_status_struct, &click_location_cameraspace, &lemming_info[lem])) break;
                     }
                     // At this point, lem will be < (current_level_runtime_stats.lemmings) if a lemming could be found,
                     // else it will have run until (current_level_runtime_stats.lemmings).
                     if (lem != (current_level_runtime_stats.lemmings)) {
                        // Decrement the current tool complement if we're not debugging
                        // or the player has infinite of that tool
                        if (!(debug_lems || (ingame_status_struct->remaining_tools[ingame_status_struct->current_tool] == 100))) {
                           ingame_status_struct->remaining_tools[ingame_status_struct->current_tool]--;
                        }

                        // SOUND REQUEST : Lemming assign tool.
                        DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,
                                                                      0,
                                                                      127,  // Center.
                                                                      0);
                     }
                  }
               }
            }
         } // END OF CONTROL STYLE MAJOR TAPPER HANDLER
         else if (control_style_major == CONTROL_STYLE_MAJOR_HOLDER) {
            // Nothing will happen for this style unless the pen is down on the game.
            if ((joy & KEY_TOUCH) && (touchXY.py < GAME_DISPLAY_Y_SIZE)) {
               // When the pen is against the screen, the game will not advance.
               game_flags |= GAME_FLAG_PAUSE;

               // Make a POINT from the current pen location -in camera space-.
               POINT hold_location_cameraspace = {touchXY.px, touchXY.py};

               // Use selected_hover_lemming to try to find a lemming below the pen
               for (selected_hover_lemming = 0; selected_hover_lemming < (current_level_runtime_stats.lemmings); selected_hover_lemming++) {
                  if (CursorColDecLemming(&hold_location_cameraspace, &lemming_info[selected_hover_lemming])) break;
               }
               // At this point, selected_hover_lemming will be < (current_level_runtime_stats.lemmings) if a lemming could be found,
               // else it will have run until (current_level_runtime_stats.lemmings).

               // If a lemming is selected:
               if (selected_hover_lemming != (current_level_runtime_stats.lemmings)) {
                  // You can effect the current tool on the lemming under the pen
                  // by tapping a shoulder button while leaving the other one alone.
                  if ((KeyDown(KEY_R) && !(joy & KEY_L))      // Tap a single shoulder button,
                   || (KeyDown(KEY_L) && !(joy & KEY_R))) {   // while leaving the other one alone!
                     // You can't assign lemmings if we don't have any tools. (of the current type)
                     if ((ingame_status_struct->remaining_tools[ingame_status_struct->current_tool] != 0) || (debug_lems)) {
                        // Try to assign the current tool to the current lemming.
                        if (AssignLemmingTool(ingame_status_struct, &lemming_info[selected_hover_lemming], ingame_status_struct->current_tool)) {
                           // Decrement the current tool complement if we're not debugging
                           // or the player has infinite of that tool
                           if (!(debug_lems || (ingame_status_struct->remaining_tools[ingame_status_struct->current_tool] == 100))) {
                              ingame_status_struct->remaining_tools[ingame_status_struct->current_tool]--;
                           }

                           // SOUND REQUEST : Lemming assign tool.
                           DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,
                                                                         0,
                                                                         127,  // Center.
                                                                         0);
                        }
                     }
                  }
               }
            }
         } // END OF CONTROL STYLE MAJOR HOLDER HANDLER
         else if (control_style_major == CONTROL_STYLE_MAJOR_TACTICS) {
            // First, we manage the pause controls:
            if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) { // Holder pause!
               // Tapper hold allows you to do one of two things:
               // 1) If you hold one shoulder button and tap the other, you skip a single lemming logic frame.
               if (((  joy & KEY_R ) && (KeyDown(KEY_L)))
                || ((KeyDown(KEY_R)) && (  joy & KEY_L) )) { // Hold a shoulder, tap the other
                  game_flags |= GAME_FLAG_FF; // Skip a frame!
               } else
               // 2) If you're holding a single shoulder button, the game is paused.
               if ((joy & KEY_R) || (joy & KEY_L)) { // Hold any shoulder button.
                  game_flags |= GAME_FLAG_PAUSE;
               }
            } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) { // Toggle pause!
               // Tapper toggle allows you to do one thing:
               // If you tap a single shoulder button while leaving the other one alone,
               // you can toggle the master pause.
               if ((KeyDown(KEY_R) && !(joy & KEY_L))      // Tap a single shoulder button,
                || (KeyDown(KEY_L) && !(joy & KEY_R))) {   // while leaving the other one alone!
                  ingame_status_struct->master_pause_flag = (ingame_status_struct->master_pause_flag == 0) ? 1 : 0; // Togle the master pause.
               }
            }

            // Second, we manage the lemming selection controls.
            // If the pen has been tapped against the screen this frame:
            if (KeyDown(KEY_TOUCH)) {
               // Check to see if the pen is actually inside the game area, and not the interface.
               if (touchXY.py < GAME_DISPLAY_Y_SIZE) {
                  // Make a POINT from the current click location -in camera space-.
                  POINT click_location_cameraspace = {touchXY.px, touchXY.py};

                  // Use selected_hover_lemming to try to find a lemming below the pen
                  for (selected_hover_lemming = 0; selected_hover_lemming < (current_level_runtime_stats.lemmings); selected_hover_lemming++) {
                     if (CursorColDecLemming(&click_location_cameraspace, &lemming_info[selected_hover_lemming])) break;
                  }
                  // At this point, selected_hover_lemming will be < (current_level_runtime_stats.lemmings) if a lemming could be found,
                  // else it will have run until (current_level_runtime_stats.lemmings).

                  // For TACTICS, play the TOOL SELECT noise if a lemming was selected.
                  // (Only play the lemming select when a tool is cast!)
                  if (selected_hover_lemming != (current_level_runtime_stats.lemmings)) {
                     // SOUND REQUEST : tool select
                     DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT,
                                                                   0,
                                                                   127,  // Center.
                                                                   0);
                  }
               }
            }
         }  // END OF CONTROL STYLE MAJOR TACTICS HANDLER
      }
      
      // If the lid is closed, force the master pause flag:
      if (joy & KEY_LID) {
         ingame_status_struct->master_pause_flag = 1;
      }

      // This code allows the player to use the interface to change the release rate and change tools
      // The interface is useless if the game is fading out.
      if (level_state != LEVEL_STATE_FADE_OUT) {
         // If the pen is against the screen while over the interface:
         if ((joy & KEY_TOUCH) && (touchXY.py >= INTERFACE_SCREEN_POSITION_Y)) {
            // Hold the pen over the release rate icons to change the release rate:
            if ((touchXY.px >= INTERFACE_RELEASE_RATE_MINIMUM_LEFT_X_I)
             && (touchXY.px <  INTERFACE_RELEASE_RATE_RIGHT_X_O       )) {
               // If the pen is on the left of the icon on the right:
               // (that's a longwinded way of saying 'its on the plus')
               if (touchXY.px < INTERFACE_RELEASE_RATE_LEFT_X_O) {
                  // Tell the game to try to increase the release rate.
                  game_flags |= GAME_FLAG_RR_UP;
               } else {
                  // Tell the game to try to decrease the release rate.
                  game_flags |= GAME_FLAG_RR_DOWN;
               }
            }

            // Hold the pen over the fast forward icon to fast forward.
            if ((touchXY.px >= INTERFACE_FF_LEFT_X_I)
             && (touchXY.px <  INTERFACE_FF_RIGHT_X_O)) {
               game_flags |= GAME_FLAG_FF;
            }

            // Tap on the zoom icons to change zoom.
            if (KeyDown(KEY_TOUCH)) {
               if ((touchXY.px >= INTERFACE_ZOOM_PAIR_LEFT_X_I )
                && (touchXY.px <  INTERFACE_ZOOM_PAIR_RIGHT_X_O)) {
                  // If the pen is on the left of the icon on the right:
                  // (that's a longwinded way of saying 'its on the small lemming')
                  if (touchXY.px < INTERFACE_ZOOM_IN_LEFT_X_I) {
                     // Tell the game to zoom out.
                     game_flags |= GAME_FLAG_ZOOM_OUT;
                  } else {
                     // Tell the game to zoom in.
                     game_flags |= GAME_FLAG_ZOOM_IN;
                  }
               }
            }

            // Tap on the tools for tool change.
            if (KeyDown(KEY_TOUCH)) {
               if ((touchXY.px >= INTERFACE_TOOL_FAR_LEFT_X_I )
                && (touchXY.px <  INTERFACE_TOOL_FAR_RIGHT_X_O)) {
                  // Find the coordinate of the pen relative to the left side
                  // of the interface section dealing with lemming skills.
                  int tool_coord = touchXY.px - INTERFACE_TOOL_FAR_LEFT_X_I;

                  // Calculate the tool the pen is over by using division.
                  s32 clicked_tool = IntDiv(tool_coord, INTERFACE_TOOL_WIDTH);

                  // TACTICS mode has a unique way of handling the tool interface:
                  if (control_style_major == CONTROL_STYLE_MAJOR_TACTICS) {
                     // Only attempt assignment if selected_hover_lemming is set.
                     if ((selected_hover_lemming >= 0) && (selected_hover_lemming < (current_level_runtime_stats.lemmings))) {
                        // You can't assign lemmings if we don't have any tools. (of the current type)
                        if ((ingame_status_struct->remaining_tools[clicked_tool] != 0) || (debug_lems)) {
                           // Try to assign the current tool to the current lemming.
                           if (AssignLemmingTool(ingame_status_struct, &lemming_info[selected_hover_lemming], clicked_tool)) {
                              // Decrement the current tool complement if we're not debugging
                              // or the player has infinite of that tool
                              if (!(debug_lems || (ingame_status_struct->remaining_tools[clicked_tool] == 100))) {
                                 ingame_status_struct->remaining_tools[clicked_tool]--;
                              }

                              // SOUND REQUEST : Lemming assign tool.
                              DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LEMMING_SELECT,
                                                                            0,
                                                                            127,  // Center.
                                                                            0);
                           }
                        }
                     }
                  } else {
                     // END OF TACTICS MODE tool interface handling.

                     // Only switch to a tool if it's not the current tool.
                     if (ingame_status_struct->current_tool != clicked_tool) {
                        ingame_status_struct->current_tool = clicked_tool;

                        // SOUND REQUEST : tool select
                        DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_TOOL_SELECT,
                                                                      ingame_status_struct->current_tool,
                                                                      0,
                                                                      0);
                     }
                  }
               }
            }

            // Tap on the pause for pause toggle.
            if (KeyDown(KEY_TOUCH)) {
               if ((touchXY.px >= INTERFACE_PAUSE_LEFT_X_I )
                && (touchXY.px <  INTERFACE_PAUSE_RIGHT_X_O)) {
                  // toggle the master pause.
                  ingame_status_struct->master_pause_flag = (ingame_status_struct->master_pause_flag == 0) ? 1 : 0;
               }
            }

            // This code deals with the nuke trigger system:
            // Only allow a nuke if the nuke hasn't already been triggered, and don't nuke while paused!
            if ((ingame_status_struct->next_lemming_to_nuke == NO_LEMMING_NUKE) && (!GameFlag(GAME_FLAG_PAUSE))) {
               // Tap on the nuke to trigger it.
               if (KeyDown(KEY_TOUCH)) {
                  if ((touchXY.px >= INTERFACE_NUKE_LEFT_X_I )
                   && (touchXY.px <  INTERFACE_NUKE_RIGHT_X_O)) {
                     // If the last time the player tapped on the nuke was
                     // long enough, but not too short, the nuke is triggered.
                     if ((nuke_press_timer <= NUKE_PRESS_TOO_LATE_AFTER )
                      && (nuke_press_timer >= NUKE_PRESS_TOO_SOON_BEFORE)) {
                        game_flags |= GAME_FLAG_NUKE_TRIGGER;
                     } else {
                        // If the nuke tap was too fast, or too slow, reset
                        // the nuke timer.
                        nuke_press_timer = 0;
                     }
                  }
               }
            }
         }
      }
      
      // Every frame, increment the nuke timer up to the maximum.
      if (nuke_press_timer != 255) {
         nuke_press_timer++;
      }
      // This causes nuke_press_timer to record the number
      // of frames since the last tap of the nuke button.

      // You can only try to trigger a restart when the game is stable.
      if (level_state == LEVEL_STATE_STABLE) {
         // A restart is triggered by tapping Select three times in quick succession.
         if (KeyDown(KEY_SELECT)) {
            // When Select is tapped, calculate the delay between
            // the second to last and the last press,
            u32 select_press_delay_far  = second_select_press_timer - select_press_timer;
            // and the delay between the last press and the current press.
            u32 select_press_delay_near = select_press_timer;

            // If the delay between the second to last press and the last press
            // was within the suitable range, and the the delay between the last
            // press and the current press was also within a suitable range.
            if ((select_press_delay_far  <= SELECT_PRESS_TOO_LATE_AFTER )
             && (select_press_delay_far  >= SELECT_PRESS_TOO_SOON_BEFORE)
             && (select_press_delay_near <= SELECT_PRESS_TOO_LATE_AFTER )
             && (select_press_delay_near >= SELECT_PRESS_TOO_SOON_BEFORE)) {
               // Signal the game to begin the restart sequence.
               game_flags |= GAME_FLAG_RESTART_TRIGGER;
            } else {
               // Move the last press timer into the second to last press timer.
               second_select_press_timer = select_press_timer;
               // Set the last press timer to the current time.
               select_press_timer = 0;
            }
         }
      }

      // Increment both timers up to their limit.
      if (second_select_press_timer != 255) {
         second_select_press_timer++;
      }
      if (select_press_timer != 255) {
         select_press_timer++;
      }
      // This causes select_press_timer to record the number
      // of frames since the last tap of the Select button, and
      // second_select_press_timer to record the number of
      // frames since the second to last tap of the Select button.

		// ------------------------------------------------------------

      //DebugAppend("Reached flag analysis.\r\n");
      //DebugWrite();

		// Let's analyse the flags that are set, and respond accordingly:

      // Respond to zoom in/out signals by zooming in and out.
      if (GameFlag(GAME_FLAG_ZOOM_OUT)) ZoomOut();
      if (GameFlag(GAME_FLAG_ZOOM_IN))  ZoomIn();

      // Respond to release rate flags:
      // If the rate UP flag is set, but not the rate DOWN flag:
      if (GameFlag(GAME_FLAG_RR_UP) && !GameFlag(GAME_FLAG_RR_DOWN)) {
         // Increase the release rate bonk timer.
         release_rate_bonk_timer++;

         // If the release rate timer is equal to the number of frames
         // necessary for a release rate change, then:
         if (release_rate_bonk_timer == RELEASE_RATE_BONK_LENGTH) {
            // If the current release rate is not at the minimum:
            if (ingame_status_struct->release_rate != ingame_status_struct->release_rate_minimum) {
               // Decrease the release rate.
               ingame_status_struct->release_rate--;

               // SOUND REQUEST : release rate tick
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE,
                                                             ingame_status_struct->release_rate,
                                                             0,
                                                             0);
            }
            // Reset the timer.
            release_rate_bonk_timer = 0;
         }
      } else if (GameFlag(GAME_FLAG_RR_DOWN) && !GameFlag(GAME_FLAG_RR_UP)) {
        // Increase the release rate bonk timer.
         release_rate_bonk_timer++;

         // If the release rate timer is equal to the number of frames
         // necessary for a release rate change, then:
         if (release_rate_bonk_timer == RELEASE_RATE_BONK_LENGTH) {
            // If the current release rate is not at the minimum:
            if (ingame_status_struct->release_rate != RELEASE_RATE_MAXIMUM) {
               // Increase the release rate.
               ingame_status_struct->release_rate++;

               // SOUND REQUEST : release rate tick
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_RELEASE_RATE_CHANGE,
                                                             ingame_status_struct->release_rate,
                                                             0,
                                                             0);
            }
            // Reset the timer.
            release_rate_bonk_timer = 0;
         }
      } else {
         // If no release rate flags are set, set the release rate timer to zero.
         release_rate_bonk_timer = 0;
      }

      // Respond to the nuke flag (if the nuke has not already began)
      if (GameFlag(GAME_FLAG_NUKE_TRIGGER) && (ingame_status_struct->next_lemming_to_nuke == NO_LEMMING_NUKE)) {
         // Set next_lemming_to_nuke to the earliest existing lemming.
         for (ingame_status_struct->next_lemming_to_nuke = 0; ((ingame_status_struct->next_lemming_to_nuke < (int)current_level_runtime_stats.lemmings) && (lemming_info[ingame_status_struct->next_lemming_to_nuke].state == LEMSTATE_DONTEXIST)); ingame_status_struct->next_lemming_to_nuke++) {

         }
         // (If we didn't do this, then we'd wait for ages before the first '5' appeared)

         // SOUND REQUEST : Oh no! (Nuke trigger)
         DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_OH_NO,
                                                       0,
                                                       127,  // Centre.
                                                       0);
      }

      // Make sure the current tool is within the valid range of tools.
      // Shouldn't happen, but I'll keep it anyway.
      if (ingame_status_struct->current_tool == NO_TOOLS) ingame_status_struct->current_tool = 0;
      if (ingame_status_struct->current_tool ==       -1) ingame_status_struct->current_tool = NO_TOOLS - 1;

      // If the pause flag is set, the game should not allow any more lemming logic ticks to occur.
      // This is done by jamming the lemming_update_time timer at 1.
      if (GameFlag(GAME_FLAG_PAUSE) || ingame_status_struct->master_pause_flag) ingame_status_struct->lemming_update_time = 1;
                      
      //DebugAppend("Responding to a tick:\r\n");
      //DebugWrite();

      // If lemming_update_time is such that a new lemming logic tick has elapsed...
      // Note that the use of the ternary operator to make the divisor 1 when the FF flag is set.
      if ((ingame_status_struct->lemming_update_time++) >= (GameFlag(GAME_FLAG_FF) ? 1 : LEMMING_UPDATE_FRAMES)) {
         // -----------------------------------------------------------------------------------------
         // A lemming logic tick has elapsed!

         ingame_status_struct->lemming_update_time = 1; // No embarrasing overflow.

// These constants define specific level_game_logic_frame frames where special events take place:
#define LEVEL_FRAME_FIRST_FULL_FRAME        (19) // This is the first frame where there is no fade effect.
#define LEVEL_FRAME_LETS_GO_SOUND_FRAME       0  // The 'Let's Go!' sound effect is played on this frame.
                                                 // This is measured from the start of the OPENWAIT state.
#define LEVEL_FRAME_ENTRANCE_OPEN_BEGIN      18  // The entrances start to open on this frame.
#define LEVEL_FRAME_ENTRANCE_OPEN_DURATION    8  // The length of the entrance opening animation
#define LEVEL_FRAME_ENTRANCE_OPEN_LAST_FRAME (LEVEL_FRAME_ENTRANCE_OPEN_BEGIN + \
                                               + LEVEL_FRAME_ENTRANCE_OPEN_DURATION) //// The entrances stop opening on this frame.

#define LEVEL_FRAME_FIRST_LEMMING_APPEAR     (LEVEL_FRAME_ENTRANCE_OPEN_LAST_FRAME + 7)
                                                 // The first lemming is created on this frame.
                                                 // New lemmings are freely created after this.
                                                 // This frame is measured from the start of the OPENWAIT
                                                 // state, not the fade in state.

#define LEVEL_FRAME_FADE_OUT_FRAMES          32  // This is the number of frames used to fade out.

         // This deals with the 'fade in' state:
         if (level_state == LEVEL_STATE_FADE_IN) {
            // This state exists to increase level_game_logic_frame until it hits LEVEL_FRAME_FIRST_FULL_FRAME,
            // signifying the end of the fade-in sequence.

            // Keep increasing level_game_logic_frame until it hits LEVEL_FRAME_FIRST_FULL_FRAME
            if (level_game_logic_frame == LEVEL_FRAME_FIRST_FULL_FRAME) {
               // Reset frame counter.
               level_game_logic_frame = 0;
               // When it hits LEVEL_FRAME_FIRST_FULL_FRAME, then switch state to LEVEL_STATE_OPENWAIT.
               level_state = LEVEL_STATE_OPENWAIT;
            } else {
               ++level_game_logic_frame;
            }

         } // END OF LEVEL FADE IN STATE HANDLER
         else if (level_state == LEVEL_STATE_OPENWAIT) {
            // This state exists to increase level_game_logic_frame so that the game waits,
            // opens the entrances, and then switches to the stable state.

            // Check to see if this is the 'Let's Go!' sound frame.
            if (level_game_logic_frame == LEVEL_FRAME_LETS_GO_SOUND_FRAME) {
               // SOUND REQUEST : Let's Go!
               DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_LETS_GO,
                                                             0,
                                                             0,
                                                             0);
            }

            // Check to see if this is the trapdoor start open frame...
            if (level_game_logic_frame == LEVEL_FRAME_ENTRANCE_OPEN_BEGIN) {
               // If so, check the number of trapdoors there are in the level.
               if (current_level_runtime_stats.no_entrances == 1) {
                  // If there's only one trapdoor, the sound is going to come
                  // from the trapdoor's location.

                  // SOUND REQUEST : trapdoor open, at a specific location!
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN,
                                                                0,
                                                                current_level_runtime_stats.entrance_x[0] << log2magnification,  // This is the entrances position in maggies.
                                                                camera_x_inset);                                               // This is the position of the left of the screen in maggies.
               } else {
                  // If there's more than one trapdoor, it's easier just to play the
                  // sound effect in the centre of the screen.
                  // SOUND REQUEST : trapdoor open, centre of screen
                  DSSoundTriggerLemmingsDS_RequestStandardSound(LEMMINGS_DS_SOUND_TYPE_TRAPDOOR_OPEN,
                                                                0,
                                                                127,  // Center.
                                                                0);
               }
            }

            // Between the two extreme entrance frames:
            if ((level_game_logic_frame >= LEVEL_FRAME_ENTRANCE_OPEN_BEGIN)
             && (level_game_logic_frame <= LEVEL_FRAME_ENTRANCE_OPEN_LAST_FRAME)) {
               // Calculate the entrance frame from the current logic frame.
               ingame_status_struct->entrance_state.animframe = 1 + level_game_logic_frame - (LEVEL_FRAME_ENTRANCE_OPEN_BEGIN - 1);
               
               // The graphical object for entrances starts with the trapdoor fully open.
               // Change the order of frames thusly:
               if (ingame_status_struct->entrance_state.animframe == 10) {
                  ingame_status_struct->entrance_state.animframe = 0;
               }
            }

            // Keep increasing level_game_logic_frame until it hits LEVEL_FRAME_FIRST_LEMMING_APPEAR:
            if (level_game_logic_frame == LEVEL_FRAME_FIRST_LEMMING_APPEAR) {
               // At which point switch to the stable state.
               level_state = LEVEL_STATE_STABLE;
               level_game_logic_frame = 0;
            } else {
               ++level_game_logic_frame;
            }

         } // END OF LEVEL OPENWAIT STATE HANDLER
         else if (level_state == LEVEL_STATE_STABLE) {
            // This state exists to create new lemmings and manage the nuke and restart systems.

            // This code block creates new lemmings.
            // No new lemmings are created while a nuke is in progress.
            if (ingame_status_struct->next_lemming_to_nuke == NO_LEMMING_NUKE) {
               // The first lemming is generated immediately.
               if (ingame_status_struct->lemmings_dispensed == 0) {
                  // This line, combined with the increment below, will ensure
                  // that the first lemming is generated instantly.
                  ingame_status_struct->lemming_release_timer = release_rate_value[ingame_status_struct->release_rate] - 1;
               }

               // Only generate lemmings when the number created
               // is unequal to the number to create in total
               if (ingame_status_struct->lemmings_dispensed != current_level_runtime_stats.lemmings) {
                  // Increase the release timer.
                  ingame_status_struct->lemming_release_timer++;

                  // Use the values in the release_rate_value to determine
                  // whether the timer is 'full of frames' enough to generate
                  // a new lemming.
                  if (ingame_status_struct->lemming_release_timer >= release_rate_value[ingame_status_struct->release_rate]) {
                     // If there are enough frames in the timer, a new lemming
                     // can be created.

                     // First, remove the number of frames 'used up' from the release timer.
                     ingame_status_struct->lemming_release_timer -= release_rate_value[ingame_status_struct->release_rate];

                     // Get a pointer to the 'new' lemmings struct.
                     LEMMING_INFO_STRUCT *newlem = &lemming_info[ingame_status_struct->lemmings_dispensed];
                     // Notice that the lemming_info array refers to them in the order
                     // that they're released, so we don't have to search for an inactive slot.

                     // Reset the new lemming based on the paramters of the next_entrance_to_use
                     newlem->x = current_level_runtime_stats.entrance_x[ingame_status_struct->next_entrance_to_use];
                     newlem->y = current_level_runtime_stats.entrance_y[ingame_status_struct->next_entrance_to_use];
                     newlem->d = current_level_runtime_stats.entrance_d[ingame_status_struct->next_entrance_to_use];
                     newlem->state = LEMSTATE_FALLER;
                     newlem->flags = 0;
                     newlem->falldistance = 0;

                     // Work out the next entrance to use:
                     ingame_status_struct->next_entrance_to_use = Modulo(ingame_status_struct->next_entrance_to_use + 1, current_level_runtime_stats.no_entrances);

                     // Increment the lemmings dispensed counter.
                     ingame_status_struct->lemmings_dispensed++;
                  }
               }
            }
         }

         // The following logic handlers are in effect for all states.

         // Advance and loop the active object frames:

         if (GetGraphicalObjectGraphic(active_graphical_object_exit, ingame_status_struct->exit_state.animframe)->frame_length == 0) {
            if (ingame_status_struct->exit_state.animframe_subframe > 0) {
               (ingame_status_struct->exit_state.animframe)++;
               ingame_status_struct->exit_state.animframe_subframe = 0;
            }
         } else {
            if (ingame_status_struct->exit_state.animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_exit, ingame_status_struct->exit_state.animframe)->frame_length) {
               (ingame_status_struct->exit_state.animframe)++;
               ingame_status_struct->exit_state.animframe_subframe = 0;
            }
         }

         if (ingame_status_struct->exit_state.animframe == active_graphical_object_exit->no_primary_frames) {
            ingame_status_struct->exit_state.animframe = 0;
         }

         (ingame_status_struct->exit_state.animframe_subframe)++;

         if (GetGraphicalObjectGraphic(active_graphical_object_water, ingame_status_struct->water_state.animframe)->frame_length == 0) {
            if (ingame_status_struct->water_state.animframe_subframe > 0) {
               (ingame_status_struct->water_state.animframe)++;
               ingame_status_struct->water_state.animframe_subframe = 0;
            }
         } else {
            if (ingame_status_struct->water_state.animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_water, ingame_status_struct->water_state.animframe)->frame_length) {
               (ingame_status_struct->water_state.animframe)++;
               ingame_status_struct->water_state.animframe_subframe = 0;
            }
         }

         if (ingame_status_struct->water_state.animframe == active_graphical_object_water->no_primary_frames) {
            ingame_status_struct->water_state.animframe = 0;
         }

         (ingame_status_struct->water_state.animframe_subframe)++;

         // Advance and manage the traps.
         for (u32 trap = 0; trap < current_level_runtime_stats.no_traps; trap++) {
            // Advance the frames of working traps...
            if (ingame_status_struct->trap_state[trap].state == TRAP_STATUS_STATE_WORKING) {  
               if (GetGraphicalObjectGraphic(active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]], ingame_status_struct->trap_state[trap].animframe)->frame_length == 0) {
                  if (ingame_status_struct->trap_state[trap].animframe_subframe > 0) {
                  (ingame_status_struct->trap_state[trap].animframe)++;
                  ingame_status_struct->trap_state[trap].animframe_subframe = 0;
                  }
               } else {
                  if (ingame_status_struct->trap_state[trap].animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]], ingame_status_struct->trap_state[trap].animframe)->frame_length) {
                     (ingame_status_struct->trap_state[trap].animframe)++;
                     ingame_status_struct->trap_state[trap].animframe_subframe = 0;
                  }
               }

               // Set traps that reach their secondary (working) frame limit to idle.
               if (ingame_status_struct->trap_state[trap].animframe == ((active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]]->no_primary_frames  )
                                                                       +(active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]]->no_secondary_frames))) {
                  ingame_status_struct->trap_state[trap].state = TRAP_STATUS_STATE_IDLE;

                  ingame_status_struct->trap_state[trap].animframe = 0;
               }

               (ingame_status_struct->trap_state[trap].animframe_subframe)++;
            } else {
            // Advance the frame of idle traps.
               if (GetGraphicalObjectGraphic(active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]], ingame_status_struct->trap_state[trap].animframe)->frame_length == 0) {
                  if (ingame_status_struct->trap_state[trap].animframe_subframe > 0) {
                  (ingame_status_struct->trap_state[trap].animframe)++;
                  ingame_status_struct->trap_state[trap].animframe_subframe = 0;
                  }
               } else {
                  if (ingame_status_struct->trap_state[trap].animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]], ingame_status_struct->trap_state[trap].animframe)->frame_length) {
                     (ingame_status_struct->trap_state[trap].animframe)++;
                     ingame_status_struct->trap_state[trap].animframe_subframe = 0;
                  }
               }

               if (ingame_status_struct->trap_state[trap].animframe == active_graphical_object_trap[current_level_runtime_stats.trap_genus[trap]]->no_primary_frames) {
                  ingame_status_struct->trap_state[trap].animframe = 0;
               }

               (ingame_status_struct->trap_state[trap].animframe_subframe)++;
            }
         }

         // Advance the frames of all hazard genuses
         for (u32 hazard_genus = 0; hazard_genus < NO_HAZARD_GENUSES; hazard_genus++) {
            if (GetGraphicalObjectGraphic(active_graphical_object_hazard[hazard_genus], ingame_status_struct->hazard_state[hazard_genus].animframe)->frame_length == 0) {
               if (ingame_status_struct->hazard_state[hazard_genus].animframe_subframe > 0) {
                  (ingame_status_struct->hazard_state[hazard_genus].animframe)++;
                  ingame_status_struct->hazard_state[hazard_genus].animframe_subframe = 0;
               }
            } else {
               if (ingame_status_struct->hazard_state[hazard_genus].animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_hazard[hazard_genus], ingame_status_struct->hazard_state[hazard_genus].animframe)->frame_length) {
                  (ingame_status_struct->hazard_state[hazard_genus].animframe)++;
                  ingame_status_struct->hazard_state[hazard_genus].animframe_subframe = 0;
               }
            }

            if (ingame_status_struct->hazard_state[hazard_genus].animframe == active_graphical_object_hazard[hazard_genus]->no_primary_frames) {
               ingame_status_struct->hazard_state[hazard_genus].animframe = 0;
            }

            (ingame_status_struct->hazard_state[hazard_genus].animframe_subframe)++;
         }

         // Advance the frames of all uninteractive genuses
         for (u32 uninteractive_genus = 0; uninteractive_genus < NO_UNINTERACTIVE_GENUSES; uninteractive_genus++) {
            if (GetGraphicalObjectGraphic(active_graphical_object_uninteractive[uninteractive_genus], ingame_status_struct->uninteractive_state[uninteractive_genus].animframe)->frame_length == 0) {
               if (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe > 0) {
                  (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe)++;
                  ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe = 0;
               }
            } else {
               if (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe >= GetGraphicalObjectGraphic(active_graphical_object_uninteractive[uninteractive_genus], ingame_status_struct->uninteractive_state[uninteractive_genus].animframe)->frame_length) {
                  (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe)++;
                  ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe = 0;
               }
            }

            if (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe == active_graphical_object_uninteractive[uninteractive_genus]->no_primary_frames) {
               ingame_status_struct->uninteractive_state[uninteractive_genus].animframe = 0;
            }

            (ingame_status_struct->uninteractive_state[uninteractive_genus].animframe_subframe)++;
         }

         // Update every lemming that has been dispensed.
         for (u32 lem = 0; lem < ingame_status_struct->lemmings_dispensed; lem++) {
            // This function will only try to update lemmings who exist.
            UpdateLemming(ingame_status_struct, &lemming_info[lem], &current_level_runtime_stats);
         }

         // If the nuke is currently in progress:
         if (ingame_status_struct->next_lemming_to_nuke != NO_LEMMING_NUKE) {
            // And the nuke has not reached the total number of lemmings:
            if (ingame_status_struct->next_lemming_to_nuke != (int)current_level_runtime_stats.lemmings) {
               // For every lemming that hasn't already had their fuse lit:
               if (!(lemming_info[ingame_status_struct->next_lemming_to_nuke].flags & LEMFLAG_FUSELIT)) {
                  // Set the Exploder Triggered By Nuke flag.
                  lemming_info[ingame_status_struct->next_lemming_to_nuke].flags |= LEMFLAG_EXPLODE_BY_NUKE;
                  // This means that nuke triggered lemmings will not yell
                  // 'Oh no' when it's their time to explode.
               }

               // Try to make every lemming an exploder.
               AssignLemmingTool(ingame_status_struct, &lemming_info[ingame_status_struct->next_lemming_to_nuke], TOOL_EXPLODER);

               // Advance the next lemming to nuke counter.
               ingame_status_struct->next_lemming_to_nuke++;
            }
         }

         // Decrement the time remaining, but not below zero.
         if (ingame_status_struct->time_remaining > 0) {
            --ingame_status_struct->time_remaining;
         }

         // This deals with the 'stable' state:
         if (level_state == LEVEL_STATE_STABLE) {
            // Determine whether the level should fade to black:
            // If the entrances have dispensed all of the lemmings, (or the nuke is in effect, so
            // no more lemmings could be created anyway) and there are no more lemmings wandering
            // the level, then the level will end due to a lack of lemmings:
            if (((ingame_status_struct->lemmings_dispensed   == current_level_runtime_stats.lemmings)
              || (ingame_status_struct->next_lemming_to_nuke != NO_LEMMING_NUKE                     ))
             && (LemmingsOut(ingame_status_struct) == 0                                            )) {
               // Change the state to fade to black state, and reset frame controller.
               level_state      = LEVEL_STATE_FADE_OUT;
               level_fade_state = LEVEL_FADE_STATE_FADE_OUT;
               level_fade_logic_frame = 0;

               // Signal the subscreen so that the map and the in-out timer panel fade to black.
               UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME
                                                       | UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);

            } // End of 'lack of lemmings' level end detection.
            else if ((ingame_status_struct->time_remaining == 0) && (current_level_runtime_stats.time_in_minutes != 0)) {
               // If the timer has reached zero...
               // (But not reached zero FROM zero. Zero minutes is infinite time.)
               // Change the state to fade to black state, and reset frame controller.
               level_state      = LEVEL_STATE_FADE_OUT;
               level_fade_state = LEVEL_FADE_STATE_FADE_OUT;
               level_fade_logic_frame = 0;

               // Record the fact that the timer has reached zero.
               status->status_flags |= LEVEL_STATUS_FLAG_TIME_UP;

               // Signal the subscreen so that the map and the in-out timer panel fade to black.
               UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME
                                                       | UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);

            } // End of 'out of time' level end detection.
            else if (GameFlag(GAME_FLAG_RESTART_TRIGGER)) {
               // Has the restart been triggered?
               // Change the state to fade to black state, and reset frame controller.
               level_state      = LEVEL_STATE_FADE_OUT;
               level_fade_state = LEVEL_FADE_STATE_FADE_OUT;
               level_fade_logic_frame = 0;

               // Signal the subscreen so that the map and the in-out timer panel fade to black.
               UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME
                                                       | UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);

               // Depending on whether you've met the quota of lemmings for this level,
               // one of two things will happen:
               if (ingame_status_struct->lemmings_exited >= current_level_runtime_stats.to_be_saved) {
                  // If you've rescued enough lemmings, the level will fade to black
                  // as if there were no more lemmings left on the level.
                  // You don't want to restart when you've won! That's fair enough, right?
               } else {
                  // If you haven't saved enough lemmings, then you must want to restart:
                  // set the flag here.
                  status->status_flags |= LEVEL_STATUS_FLAG_RESTART;
               }
            }
         }

         // If debug mode is specified, place the lemming counters into the climber and floater tools.
         if (debug_lems) ingame_status_struct->remaining_tools[TOOL_CLIMBER] = ingame_status_struct->lemmings_exited;
         if (debug_lems) ingame_status_struct->remaining_tools[TOOL_FLOATER] = LemmingsOut(ingame_status_struct);
         // -----------------------------------------------------------------------------------------
         //  End of handler for a single lemming logic tick.
         // -----------------------------------------------------------------------------------------
      }   
                         
      //DebugAppend("Finished tick handler.\r\n");
      //DebugWrite();

      // This code will handle the fade in and fade out of the screen.
      // It is completely independent of the entrance opening 'fade in'
      // You could fast forward so that the trapdoor opens before the
      // screen has faded from black.

      // These control how long the fade in and fade out sequences last.
#define LEVEL_FADE_LOGIC_FRAME_LAST_FADE_IN_FRAME    32
#define LEVEL_FADE_LOGIC_FRAME_LAST_FADE_OUT_FRAME   32

      // This deals with the 'fade in' (from black) state:
      if (level_fade_state == LEVEL_FADE_STATE_FADE_IN) {
         // If this level_fade_logic_frame has not exceeded the last fade in frame:
         if (level_fade_logic_frame <= LEVEL_FADE_LOGIC_FRAME_LAST_FADE_IN_FRAME) {
            // Set the current fade value to fade in the level, objects and interface from black.
            fade_value = 32 - level_fade_logic_frame;
            
            // Advance the level fade logic frame.
            level_fade_logic_frame++;

            // Inform the subscreen of the value it should place into the blend registers
            // in order to fade the map and in-out timers to match the main screen.
            UpdateSubScreenAsLevelInfo_SetFadeValue(fade_value >> 1);
         }
         if (level_fade_logic_frame == LEVEL_FADE_LOGIC_FRAME_LAST_FADE_IN_FRAME) {
            // Set the fade state to no fade.
            level_fade_state = LEVEL_FADE_STATE_NO_FADE;

            // Set the fade value to no fade.
            fade_value = 0;
         }
      } // End of 'fade in' state handler.
      // This deals with the 'fade out' (to black) state:
      else if (level_fade_state == LEVEL_FADE_STATE_FADE_OUT) {
         if (level_fade_logic_frame <= LEVEL_FADE_LOGIC_FRAME_LAST_FADE_OUT_FRAME) {
            // Set the current fade value to fade out the level, objects and interface to black.
            fade_value = level_fade_logic_frame;

            // Advance the level fade logic frame.
            level_fade_logic_frame++;

            // Inform the subscreen of the value it should place into the blend registers
            // in order to fade the map and in-out timers to match the main screen.
            UpdateSubScreenAsLevelInfo_SetFadeValue(fade_value >> 1);
         }
         if (level_fade_logic_frame == LEVEL_FADE_LOGIC_FRAME_LAST_FADE_OUT_FRAME) {
            // Set the fade state to no fade.
            level_fade_state = LEVEL_FADE_STATE_NO_FADE;
         }
      } // End of 'fade out' state handler.

      // If debug mode is specified, place some debugging information into the lemming tool displays.
      if (debug_lems) ingame_status_struct->remaining_tools[TOOL_EXPLODER] = IntDiv(camera_x_inset, 100);
      if (debug_lems) ingame_status_struct->remaining_tools[TOOL_BLOCKER]  = Modulo(camera_x_inset, 100);

      if (debug_lems) ingame_status_struct->remaining_tools[TOOL_DIGGER] = level_game_logic_frame;

      // Handle the camera motion flags:
      if (GameFlag(GAME_FLAG_CAM_L)) camera_x_inset -= 10;
      if (GameFlag(GAME_FLAG_CAM_R)) camera_x_inset += 10;
      if (GameFlag(GAME_FLAG_CAM_U)) camera_y_inset -= 10;
      if (GameFlag(GAME_FLAG_CAM_D)) camera_y_inset += 10;

      // Lock the camera inset variables so that the camera does
      // not attempt to show any area outside of the level.
      if (camera_x_inset < 0) {
         camera_x_inset = 0;
      }
      if (camera_x_inset > ((s32)((LEVEL_X_SIZE  << log2magnification)) - GAME_DISPLAY_X_SIZE)) {
         camera_x_inset = ((LEVEL_X_SIZE  << log2magnification)) - GAME_DISPLAY_X_SIZE;
      }
      if (camera_y_inset < 0) {
         camera_y_inset = 0;
      }
      if (camera_y_inset > ((s32)((LEVEL_Y_SIZE  << log2magnification)) - GAME_DISPLAY_Y_SIZE)) {
         camera_y_inset = ((LEVEL_Y_SIZE  << log2magnification)) - GAME_DISPLAY_Y_SIZE;
      }

      // ---------------------------------------------------

      // This is the rendering part of this display frame:

      // Clear the level camera part of the screen to black.
      memset(drawbuffer, 0, SCREEN_WIDTH * INTERFACE_SCREEN_POSITION_Y * 2);

      // The camera view of the level is drawn in the following sequence:
      // - Background traps / hazards / uninteractives / water areas
      // - Entrances
      // - Exits
      // - Level scenery
      // - Foreground traps / hazards / uninteractives / water areas
      // - Unselected lemmings
      // - Selected lemming

      //DebugAppend("Drawing background objects:\r\n");
      //DebugWrite();

      // Draw all of the background traps to the screen.
      for (u32 t = 0; t < current_level_runtime_stats.no_traps; t++) {
         if (current_level_runtime_stats.trap_z[t] == TRAP_Z_BACKGROUND) {
            // Tell function whether trap is idle or not.
            DrawTrap(current_level_runtime_stats.trap_genus[t], ingame_status_struct->trap_state[t].animframe, current_level_runtime_stats.trap_x[t], current_level_runtime_stats.trap_y[t], ingame_status_struct->trap_state[t].state == TRAP_STATUS_STATE_WORKING);
         }
      }

      // Draw all of the background hazards to the screen.
      for (u32 h = 0; h < current_level_runtime_stats.no_hazards; h++) {
         if (current_level_runtime_stats.hazard_z[h] == HAZARD_Z_BACKGROUND) {
            DrawHazard(current_level_runtime_stats.hazard_genus[h], ingame_status_struct->hazard_state[current_level_runtime_stats.hazard_genus[h]].animframe, current_level_runtime_stats.hazard_x[h], current_level_runtime_stats.hazard_y[h]);
         }
      }

      // Draw all of the background uninteractives to the screen.
      for (u32 u = 0; u < current_level_runtime_stats.no_uninteractives; u++) {
         if (current_level_runtime_stats.uninteractive_z[u] == UNINTERACTIVE_Z_BACKGROUND) {
            DrawUninteractive(current_level_runtime_stats.uninteractive_genus[u], ingame_status_struct->uninteractive_state[current_level_runtime_stats.uninteractive_genus[u]].animframe, current_level_runtime_stats.uninteractive_x[u], current_level_runtime_stats.uninteractive_y[u]);
         }
      }

      // Draw all of the background water areas to the screen.
      for (u32 w = 0; w < current_level_runtime_stats.no_waters; w++) {
         if (current_level_runtime_stats.water_z[w] == WATER_Z_BACKGROUND) {
            DrawWaterArea(ingame_status_struct->water_state.animframe, current_level_runtime_stats.water_x1[w], current_level_runtime_stats.water_x2[w], current_level_runtime_stats.water_y[w]);
         }
      }

      //DebugAppend("Drawing entrances:\r\n");
      //DebugWrite();

      // Draw all of the entrances to the screen.
      for (u32 e = 0; e < current_level_runtime_stats.no_entrances; e++) {
         DrawEntrance(ingame_status_struct->entrance_state.animframe, current_level_runtime_stats.entrance_x[e], current_level_runtime_stats.entrance_y[e]);
      }

      //DebugAppend("Drawing exits:\r\n");
      //DebugWrite();

      //DEBUG_SECTION {
      //   char output_text[4096];
      //   siprintf(output_text, "Current exit frame is %d.\r\n", ingame_status_struct->exit_state.animframe);
      //   DebugAppend(output_text);
      //   DebugWrite();
      //}

      // Draw all of the exits to the screen.
      for (u32 e = 0; e < current_level_runtime_stats.no_exits; e++) {
         DrawExit(ingame_status_struct->exit_state.animframe, current_level_runtime_stats.exit_x[e], current_level_runtime_stats.exit_y[e]);
      }

      //DebugAppend("Drawing level:\r\n");
      //DebugWrite();

      // Draw the level scenery to the screen
      DrawLevel();

      //DebugAppend("Drawing foreground objects:\r\n");
      //DebugWrite();

      // Draw all of the foreground traps to the screen.
      for (u32 t = 0; t < current_level_runtime_stats.no_traps; t++) {
         if (current_level_runtime_stats.trap_z[t] == TRAP_Z_FOREGROUND) {
            // Tell function whether trap is idle or not.
            DrawTrap(current_level_runtime_stats.trap_genus[t], ingame_status_struct->trap_state[t].animframe, current_level_runtime_stats.trap_x[t], current_level_runtime_stats.trap_y[t], ingame_status_struct->trap_state[t].state == TRAP_STATUS_STATE_WORKING);
         }
      }

      // Draw all of the foreground hazards to the screen.
      for (u32 h = 0; h < current_level_runtime_stats.no_hazards; h++) {
         if (current_level_runtime_stats.hazard_z[h] == HAZARD_Z_FOREGROUND) {
            DrawHazard(current_level_runtime_stats.hazard_genus[h], ingame_status_struct->hazard_state[current_level_runtime_stats.hazard_genus[h]].animframe, current_level_runtime_stats.hazard_x[h], current_level_runtime_stats.hazard_y[h]);
         }
      }

      // Draw all of the foreground uninteractives to the screen.
      for (u32 u = 0; u < current_level_runtime_stats.no_uninteractives; u++) {
         if (current_level_runtime_stats.uninteractive_z[u] == UNINTERACTIVE_Z_FOREGROUND) {
            DrawUninteractive(current_level_runtime_stats.uninteractive_genus[u], ingame_status_struct->uninteractive_state[current_level_runtime_stats.uninteractive_genus[u]].animframe, current_level_runtime_stats.uninteractive_x[u], current_level_runtime_stats.uninteractive_y[u]);
         }
      }

      // Draw all of the foreground water areas to the screen.
      for (u32 w = 0; w < current_level_runtime_stats.no_waters; w++) {
         if (current_level_runtime_stats.water_z[w] == WATER_Z_FOREGROUND) {
            DrawWaterArea(ingame_status_struct->water_state.animframe, current_level_runtime_stats.water_x1[w], current_level_runtime_stats.water_x2[w], current_level_runtime_stats.water_y[w]);
         }
      }


      //DebugAppend("Drawing lemmings:\r\n");
      //DebugWrite();

      // Draw all the unselected lemmings:
      for (u32 lem = 0; lem < current_level_runtime_stats.lemmings; lem++) {
         if (lem != selected_hover_lemming) DrawLemming(&lemming_info[lem], 0);
      }
      // Draw the selected lemming, if there is a lemming selected.
      if (selected_hover_lemming != (current_level_runtime_stats.lemmings)) {
         DrawLemming(&lemming_info[selected_hover_lemming], 1);
      }

      //DebugAppend("Drawing complete.\r\n");
      //DebugWrite();

      // Update the lower screen interface
      UpdateIngameInterface(ingame_status_struct, control_style_major != CONTROL_STYLE_MAJOR_TACTICS);

      // Update the sub screen minimap.
      UpdateSubScreenAsLevelInfo_UpdateMap(camera_x_inset, camera_y_inset, true, true, lemming_info);

      // Exit the main loop if we've reached the last frame of the fade out sequence.
      if ((level_state == LEVEL_STATE_FADE_OUT)
       && (level_fade_logic_frame == LEVEL_FADE_LOGIC_FRAME_LAST_FADE_OUT_FRAME)) {
         toexit = true;
      }

      // Use the fade_value to fade the screen to or from black.
      BLEND_Y = fade_value >> 1; // fade_value runs from 0 to 32.

      // Copy the sprite shadows to the OAM for each screen.
      updateOAM(OAM    , sprite_shadow_m);
      updateOAM(OAM_SUB, sprite_shadow_s);

      // Set the background to be 'lemmings eerie blue'
      BG_PALETTE[0] = RGB15A(0, 0, 6);

      // Play all sounds in the queue.
      DSSoundTrigger_ExecuteRequestQueue();

      // Automatically switch the ingame music tracks using the jukebox.
      Music_HandleIngameSongJukeboxPlayback();

      // Wait, and low power until VBlank.
      swiWaitForVBlank();

      // Use DS haxor to change the visible page, and the page to which the writes are committed.
      FlipDisplayPages();
      // Inform the sub screen of the current in-out statistics
      UpdateSubScreenAsLevelInfo_UpdateInOutTime(LemmingsOut(ingame_status_struct), ingame_status_struct->lemmings_exited, current_level_runtime_stats.lemmings, ingame_status_struct->time_remaining);
   }

   // Store the number of lemmings saved into the result struct.
   status->lemmings_saved = ingame_status_struct->lemmings_exited;

   // Set the 'won level' flag in the result struct
   if (ingame_status_struct->lemmings_exited >= current_level_runtime_stats.to_be_saved) {
      status->status_flags |= LEVEL_STATUS_FLAG_LEVEL_WON;
   }
   
   // Free the game status structure before exiting the function
   free(ingame_status_struct);
}

// And that's it. The lemmings machine.

// WriteText will write some text to a tiled background using the 1x2 tile lemmings font.
// It only works for a 32x32 tiled background.
void WriteText(const char *text, int tegel_start, int x, int y, int colour, int letter_image_base_start) {
   // tegel_start should be an int of this style: SCREEN_BASE_BLOCK(14)
   // letter_image_base_start should be the tile number where the font is loaded.
   // You should give in the number you told the font to be copied to.
   // You've gotta have two tiles empty for the space char!
   // The specified tile should have the -upper half of the exclamation mark- contained in it.

   // The y coordinate you give is the TOP OF THE LETTER.

   // DONT EVEN TRY TO WRITE SOME HALF LETTERS OFF THE TOP OR BOTTOM OF THE SCREEN!!!!

   // The colour is the palette line you want your text to use.
   
   // Catch the obvious one
   if (text == NULL) return;

   // Get a pointer to the tegel we're going to use to store the top half of the first character.
   vu16 *dst_tegel = ((vu16 *)tegel_start) + x + (y * 32);

   // Advance through the string, drawing each letter one by one.
   // Ignore letters that are off the left or right of the screen.
   for (const char *cur_letter = text; ((*cur_letter) != 0) && (x < 32); dst_tegel++, x++, cur_letter++) {
      if (x < 0) continue;

      // Calculate top half tegel and store.
      *(dst_tegel     ) = (letter_image_base_start + (((*cur_letter) - 32) * 2) + 0 - 2) | (colour << 12);
      // Calculate bottom half tegel and store.
      *(dst_tegel + 32) = (letter_image_base_start + (((*cur_letter) - 32) * 2) + 1 - 2) | (colour << 12);
   }
}

// WriteTextCen will write some centered text to a tiled background using the 1x2 tile lemmings font.
// It only works for a 32x32 tiled background.
void WriteTextCen(const char *text, int tegel_start, int y, int colour, int letter_image_base_start) {
   // tegel_start should be an int of this style: SCREEN_BASE_BLOCK(14)
   // letter_image_base_start should be the tile number where the font is loaded.
   // You should give in the number you told the font to be copied to.
   // You've gotta have two tiles empty for the space char!
   // The specified tile should have the -upper half of the exclamation mark- contained in it.

   // The y coordinate you give is the TOP OF THE LETTER.

   // DONT EVEN TRY TO WRITE SOME HALF LETTERS OFF THE TOP OR BOTTOM OF THE SCREEN!!!!

   // The colour is the palette line you want your text to use.

   // Anticlimactic function delegation!
   WriteText(text, tegel_start, 16-(strlen(text)>>1), y, colour, letter_image_base_start);
}

// WriteTextMegaCen will write some centered text to one of two tiled backgrounds using the 1x2 tile lemmings font.
// It only works for 32x32 tiled backgrounds.
void WriteTextMegaCen(const char *text, int tegel_start_even, int tegel_start_odd, int y, int colour, int letter_image_base_start) {
   // tegel_start_even and tegel_start_odd are integers referencing two background tegel tables:
   // tegel_start_odd will be used if text has an odd number of characters (including space)
   // tegel_start_even will be used if text has an even number of characters (including space)
   // They should be an int of this style: SCREEN_BASE_BLOCK(14)

   // The tegel_start_odd background should be 4 pixels to the left of the
   // the tegel_start_even background.

   // This gives the illusion of a single tiled background which can somehow
   // use 'half tiles' to display text.

   // letter_image_base_start should be the tile number where the font is loaded.
   // You should give in the number you told the font to be copied to.
   // You've gotta have two tiles empty for the space char!
   // The specified tile should have the -upper half of the exclamation mark- contained in it.

   // The y coordinate you give is the TOP OF THE LETTER.

   // DONT EVEN TRY TO WRITE SOME HALF LETTERS OFF THE TOP OR BOTTOM OF THE SCREEN!!!!

   // The colour is the palette line you want your text to use.

   // Anticlimactic function delegation!
   WriteText(text, (strlen(text)&1) ? tegel_start_odd : tegel_start_even, 16 - (strlen(text)>>1), y, colour, letter_image_base_start);
}

// These are the possible return values from the title screen function:
#define PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_STANDARD 0 // The player wants to play a one player game.
#define PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_CUSTOM   1 // The player wants to play a one player custom level.
#define PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_TWO_PLAYER          2 // The player wants to play a two player game.

// This function will take control of both screens and manage the full title screen sequence.
int PlayLemmingsDS_GoTitleScreenMenu() {
   // First, set up the vram and the other stuff so we have the main screen ready for the menu,
   // and the sub screen ready for other interesting things.

   // Wow, hehehehe, killer comment from the devkitpro guys here:
   //   //vram banks are somewhat complex
   //   vramSetMainBanks(VRAM_A_LCD, VRAM_B_LCD, VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);
   // Helpful as always. >:)

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE_0x06400000,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);
                    
   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);

   // Main screen, turn on all the backgrounds.
   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Scrolly letters.
                          | DISPLAY_BG1_ACTIVE   // scrollybar
                          | DISPLAY_BG2_ACTIVE   // Lemmings logo
                          | DISPLAY_BG3_ACTIVE   // Background texture.
                          | DISPLAY_SPR_ACTIVE   // Display sprites
                          | DISPLAY_SPR_1D_LAYOUT);  // Linear sprites

   // Sub screen, turn the first three backgrounds.
   videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE   // Info texts
                             | DISPLAY_BG2_ACTIVE   // Lemmings logo
                             | DISPLAY_BG3_ACTIVE   // Background texture
                             | DISPLAY_SPR_ACTIVE   // Display sprites
                             | DISPLAY_SPR_1D_LAYOUT);  // Linear sprites

   // Initially, we want to blend everything on both screens with black.
   SUB_BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                                   | BLEND_SRC_BG1
                                   | BLEND_SRC_BG2
                                   | BLEND_SRC_BG3
                                   | BLEND_SRC_SPRITE
                                   | BLEND_SRC_BACKDROP;

   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;

   // Set both screens to fully blend with black.
   SUB_BLEND_Y = 16;
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Reset everything and wait for a single frame.
   BlankAllPalettes();
   BlankVRAMPagesABCD();
   ResetBackgroundTranslationRegistersMain();
   ResetBackgroundTranslationRegistersSub();
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
	BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007000) Priority 3

	// Move the lemmings logo down four pixels.
	BG2_Y0 = -4;

	BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06006800) Priority 3

	// Move the scrollerbar up one pixel
	BG1_Y0 = 1;

	BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06006000) Priority 3

	// Move the text on the scroller up two pixels
	BG0_Y0 = 1;

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   }

	// Copy the eight palette entries for the brown title screen texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe+8];
   }

	for (int d = 0; d < (menugfx_lemmings_logoTilesLen >> 1); d++) {
	   // This refers to the tile number 241. (not 240!. that's the last texture tile!)
      (BG_GFX+(S_TILE_SIZE_u16 * 241))[d] = menugfx_lemmings_logoTiles[d];
   }

	// Copy the sixteen palette entries for the lemmings logo to the BG palette.
   for (int pe = 0; pe < 16; pe++) {
      BG_PALETTE[16+pe] = menugfx_lemmings_logoPal[pe];
   }

   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the main screen during the title screen sequence.
#define TITLE_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX 513

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * 513))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }

	// Now, copy the scroller backdrop into the VRAM bank.
	for (int d = 0; d < (menugfx_scroller_backdropTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * 701))[d] = menugfx_scroller_backdropTiles[d];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   // Fill in the tegel entries for the two background.
   // It's a 30x9 tile 16 colour image, not looping.
   for (int tegely = 0; tegely < 9; tegely++) {
      for (int tegelx = 0; tegelx < 30; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK(14)) + (tegelx + 1) + (tegely + 1) * 32)) = (tegelx + tegely * 30 + 241)
                                                                                 | (1 << 12);
         // These tiles are using 16 colour palette line 1.
      }
   } // That's the lemmings logo.

   // Fill in the tegel entries for the scroller bar
   for (int tegelx = 2; tegelx < 32; tegelx++) {
      (*((vu16*)(SCREEN_BASE_BLOCK(13)) + (tegelx) + (22) * 32)) = 701 | (1 << 12);
      (*((vu16*)(SCREEN_BASE_BLOCK(13)) + (tegelx) + (23) * 32)) = 701 | (1 << 12) | (1 << 11);
   }
   // The scroller bar consists of two complete horizontal lines of the scroller tile.
   // The bottom line uses the tile flipped vertically.

   // The little lemmings with the signs for the menu are five 64 by 64 sprites

   // There's different main menu layouts depending on whether the two player lemmings option
   // is visible or not
#ifdef SHOW_TWO_PLAYER_LEMS_OPTION
   // This defines the distance between each lemming.
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE 50

   // These are the lemming's positions.
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X  4
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 1) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 2) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 3) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 4) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
#else
    // This defines the distance between each lemming.
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE 60

   // These are the lemming's positions.
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X  14
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 1) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X  0
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 2) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X  ((TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_X_DISTANCE * 3) + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X)
#endif

   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y    95

   // These define the collision rectangle within a sign lemming within which
   // you must tap in order for the tap to be registered.
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1  3
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_Y1 16
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2 44
   #define TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_Y2 46

   // Copy the sign lemmings data to the sprite tile data.
   for (int d = 0; d < (menugfx_sign_lemmingsTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not 0!)
      (SPRITE_GFX+(S_TILE_SIZE_u16 * 1))[d] = menugfx_sign_lemmingsTiles[d];
   }

   // Copying the lemmings palette data to the sprite palette (line 0)
   for (int pe = 0; pe < 16; pe++) {
      SPRITE_PALETTE[pe] = menugfx_sign_lemmingsPal[pe];
   }

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1                       769

	// Copy the lemming blink frames data to the sprite tile data.
   for (int d = 0; d < (menugfx_lemming_eye_framesTilesLen >> 1); d++) {
      (SPRITE_GFX+(S_TILE_SIZE_u16 * TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1))[d] = menugfx_lemming_eye_framesTiles[d];
   }

	// Copying the lemmings palette data to the sprite palette (line 1)
   for (int pe = 0; pe < 3; pe++) {
      SPRITE_PALETTE[16+pe] = menugfx_lemming_eye_framesPal[pe];
   }

#define TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1 + 31)

	// Copy the scrolly lemming frames data to the sprite tile data.
   for (int d = 0; d < (menugfx_scroller_lemmingTilesLen >> 1); d++) {
      (SPRITE_GFX+(S_TILE_SIZE_u16 * TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1))[d] = menugfx_scroller_lemmingTiles[d];
   }

	// Copying the scrolly lemmings palette data to the sprite palette (line 2)
   for (int pe = 0; pe < 16; pe++) {
      SPRITE_PALETTE[32+pe] = menugfx_scroller_lemmingPal[pe];
   }

   // Ditch all garbage data stored in the main sprite shadow.
   initSprites(sprite_shadow_m, 128, 0);

   // The sprites 0 1 2 3 and 4 are the five sign holding lemmings on the title screen.
   // Set up the attribute crunchies for the sign lemming sprites

   // Attribute zero controls the Y coordinate.
   sprite_shadow_m[0].attribute[0] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y;
   // Attribute one controls the X coordinate and the sprite's shape.
   sprite_shadow_m[0].attribute[1] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X | ATTR1_SIZE_64;
   // Attribute two sets the tile index, the priority, and the 16 bit subpalette to use.
   sprite_shadow_m[0].attribute[2] = (1 + 64*0) | (3 << 10) | 0;

   sprite_shadow_m[1].attribute[0] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y;
   sprite_shadow_m[1].attribute[1] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X | ATTR1_SIZE_64;
   sprite_shadow_m[1].attribute[2] = (1 + 64*1) | (3 << 10) | 0;

// Only show two player lems option if explicitly specified.
#ifdef SHOW_TWO_PLAYER_LEMS_OPTION
   sprite_shadow_m[2].attribute[0] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y;
   sprite_shadow_m[2].attribute[1] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X | ATTR1_SIZE_64;
   sprite_shadow_m[2].attribute[2] = (1 + 64*2) | (3 << 10) | 0;
#endif

   sprite_shadow_m[3].attribute[0] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y;
   sprite_shadow_m[3].attribute[1] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X | ATTR1_SIZE_64;
   if (music_preference == MUSIC_PREFERENCE_MUSIC_AND_SOUND) {
      sprite_shadow_m[3].attribute[2] = (1 + 64*3) | (3 << 10) | 0;
   } else if (music_preference == MUSIC_PREFERENCE_MUSIC) {
      sprite_shadow_m[3].attribute[2] = (1 + 64*4) | (3 << 10) | 0;
   } else if (music_preference == MUSIC_PREFERENCE_SOUND) {
      sprite_shadow_m[3].attribute[2] = (1 + 64*5) | (3 << 10) | 0;
   } else if (music_preference == MUSIC_PREFERENCE_SILENCE) {
      sprite_shadow_m[3].attribute[2] = (1 + 64*6) | (3 << 10) | 0;
   }

   sprite_shadow_m[4].attribute[0] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y;
   sprite_shadow_m[4].attribute[1] = TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X | ATTR1_SIZE_64;
   if (control_style_major == CONTROL_STYLE_MAJOR_TAPPER) {
      if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) {
         sprite_shadow_m[4].attribute[2] = (1 + 64*7) | (3 << 10) | 0;
      } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) {
         sprite_shadow_m[4].attribute[2] = (1 + 64*8) | (3 << 10) | 0;
      }
   } else if (control_style_major == CONTROL_STYLE_MAJOR_HOLDER) {
      sprite_shadow_m[4].attribute[2] = (1 + 64*9) | (3 << 10) | 0;
   } else if (control_style_major == CONTROL_STYLE_MAJOR_TACTICS) {
      if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) {
         sprite_shadow_m[4].attribute[2] = (1 + 64*10) | (3 << 10) | 0;
      } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) {
         sprite_shadow_m[4].attribute[2] = (1 + 64*11) | (3 << 10) | 0;
      }
   }

   // Now, we do the sprites for the lemming eyes.
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH                  3

   // All of the lemming eye properties are listed here so that they can be managed
   // by calling a mega mega macro. It's not the best method, but it works.
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_1                          109
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_1                           44
//#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1                       641
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_1             ATTR0_SQUARE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_1              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_1                          1
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1            30
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_1     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_1     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_1     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

// The frame boundary is the frame number (based on the lemming eye frame counter)
// where the eyes will change from open, to half closed, to closed and to half open.

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_2                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X + 20)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_2                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + 10)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_2                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1 + 3)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_2               ATTR0_WIDE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_2              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_2                          2
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2            60
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_2     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_2     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_2     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_3                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X + 16)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_3                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + 11)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_3                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_2 + 6)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_3               ATTR0_WIDE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_3              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_3                          2
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3            90
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_3     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_3     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_3     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_4                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X + 19)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_4                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + 11)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_4                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_3 + 9)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_4             ATTR0_SQUARE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_4              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_4                          1
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_4           120
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_4     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_4 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_4     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_4 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_4     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_4 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_5                          196
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_5                           38
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_5                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_4 - 3)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_5             ATTR0_SQUARE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_5              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_5                          1
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_5           150
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_5     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_5 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_5     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_5 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_5     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_5 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_6                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X + 19)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_6                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + 11)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_6                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_5 + 3)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_6             ATTR0_SQUARE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_6              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_6                          1
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_6           180
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_6     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_6 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_6     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_6 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_6     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_6 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_7                           20
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_7                           43
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_7                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_6 + 3)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_7             ATTR0_SQUARE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_7              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_7                          1
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_7           210
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_7     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_7 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_7     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_7 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_7     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_7 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_8                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X + 18)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_8                          (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + 11)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_8                       (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_7 + 4)
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_8               ATTR0_WIDE
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_8              ATTR1_SIZE_8
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_8                          2
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_8           240
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_8     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_8 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_8     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_8 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_8     (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_8 + (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(l) \
         sprite_shadow_m[l + 4].attribute[0] = TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_Y_##l | TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SHAPE_##l;          \
         sprite_shadow_m[l + 4].attribute[1] = TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_X_##l | TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_SIZE_##l;           \
         sprite_shadow_m[l + 4].attribute[2] = (1 << 12) | (2 << 10);                                                                    \
         if (lemming_eyes_blink_frame < TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_##l) {                                           \
            sprite_shadow_m[l + 4].attribute[2] |= TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_##l;                                              \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_##l) {                                    \
            sprite_shadow_m[l + 4].attribute[2] |= TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_##l * 1; \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_##l) {                                    \
            sprite_shadow_m[l + 4].attribute[2] |= TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_##l * 2; \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_##l) {                                    \
            sprite_shadow_m[l + 4].attribute[2] |= TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_INC_##l * 1; \
         } else {                                                                                                                        \
            sprite_shadow_m[l + 4].attribute[2] |= TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_##l;                                              \
         }

   // This counter controls the eye blink animations of all the lemmings on both screens
   int lemming_eyes_blink_frame = 0;

   // These control the position of the little scrolly lemmings in the bottom corners of the bottom screen.
#define TITLE_SCREEN_SCROLLER_LEMMINGS_Y    (192-17)
#define TITLE_SCREEN_SCROLLER_LEMMINGS_L_X   0
#define TITLE_SCREEN_SCROLLER_LEMMINGS_R_X  (256-16)
//#define TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 (TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_TILE_1 + 31)

   // Set up the sprite attributes for the scrolly lemmings.
   // 13 is the first available sprite after the lemming eyes.
   sprite_shadow_m[13].attribute[0] = TITLE_SCREEN_SCROLLER_LEMMINGS_Y;
   sprite_shadow_m[13].attribute[1] = TITLE_SCREEN_SCROLLER_LEMMINGS_L_X | ATTR1_SIZE_16;
   sprite_shadow_m[13].attribute[2] = TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 | (2 << 12);
   sprite_shadow_m[14].attribute[0] = TITLE_SCREEN_SCROLLER_LEMMINGS_Y;
   sprite_shadow_m[14].attribute[1] = TITLE_SCREEN_SCROLLER_LEMMINGS_R_X | ATTR1_SIZE_16 | ATTR1_FLIP_X;
   sprite_shadow_m[14].attribute[2] = TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 | (2 << 12);

   // Now to set up all of the sub screen stuff!

   // First, we set up the background control registers so they know what to do.
	SUB_BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06207800) Priority 3
	SUB_BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06207000) Priority 3

	// Move the lemmings logo down four pixels.
	SUB_BG2_Y0 = -4;

	SUB_BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06206800) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX_SUB+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   } // That's the background texture.

	// Copy the eight palette entries for the brown title screen texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE_SUB[pe] = menugfx_background_texturePal[pe+8];
   }

	for (int d = 0; d < (menugfx_lemmings_logoTilesLen >> 1); d++) {
	   // This refers to the tile number 241. (not 240!. that's the last texture tile!)
      (BG_GFX_SUB+(S_TILE_SIZE_u16 * 241))[d] = menugfx_lemmings_logoTiles[d];
   } // That's the lemmings logo.

	// Copy the sixteen palette entries for the lemmings logo to the BG palette.
   for (int pe = 0; pe < 16; pe++) {
      BG_PALETTE_SUB[16+pe] = menugfx_lemmings_logoPal[pe];
   }

   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the sub screen during the title screen sequence.
#define TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX 513

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX_SUB+(S_TILE_SIZE_u16 * 513))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE_SUB[160+pe] = menugfx_lemmings_fontPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE_SUB last)
   BG_PALETTE_SUB[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the sub screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         // Plus one because the first tile has to be transparent.
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // These tiles are using 16 colour palette line zero.
      }
   } // That's the background texture.

   // Fill in the tegel entries for the two background.
   // It's a 30x9 tile 16 colour image
   for (int tegely = 0; tegely < 9; tegely++) {
      for (int tegelx = 0; tegelx < 30; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(14)) + (tegelx + 1) + (tegely + 1) * 32)) = (tegelx + tegely * 30 + 241)
                                                                                 | (1 << 12);
         // These tiles are using 16 colour palette line one.
      }
   } // That's the lemmings logo.

   // Clear the tegels for the one background.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(13)) + tegelx + tegely * 32)) = 0;
      }
   }

   // If there is a two player lemmings option:
#ifdef SHOW_TWO_PLAYER_LEMS_OPTION
   char title_screen_text_row_1[32];
   char title_screen_text_row_2[32];

   siprintf(title_screen_text_row_1, "%d standard one player level%s",  no_lemmings_ds_level_category_one_player_standard_levels,
                                                                       (no_lemmings_ds_level_category_one_player_standard_levels == 1) ? global_gamephrase_nullstring
                                                                                                                                       : global_gamephrase_plural);
   siprintf(title_screen_text_row_2, "detected across %d level set%s",  lemmings_ds_level_category_one_player_standard.no_level_sets,
                                                                       (lemmings_ds_level_category_one_player_standard.no_level_sets == 1) ? global_gamephrase_nullstring
                                                                                                                                           : global_gamephrase_plural);

   // Write the 'x standard one player levels' caption to the one background.
   WriteText(                                   title_screen_text_row_1,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                      1,
                                                                     11,
                                                                     14,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'detected across x level sets' caption to the one background.
   WriteText(                                   title_screen_text_row_2,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                      1,
                                                                     13,
                                                                     14,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);
#else
   char title_screen_text_row_1[32];
   char title_screen_text_row_2[32];

   siprintf(title_screen_text_row_1, "%d standard one player level%s",  no_lemmings_ds_level_category_one_player_standard_levels,
                                                                       (no_lemmings_ds_level_category_one_player_standard_levels == 1) ? global_gamephrase_nullstring
                                                                                                                                       : global_gamephrase_plural);
   siprintf(title_screen_text_row_2, "across %d level set%s",           lemmings_ds_level_category_one_player_standard.no_level_sets,
                                                                       (lemmings_ds_level_category_one_player_standard.no_level_sets == 1) ? global_gamephrase_nullstring
                                                                                                                                           : global_gamephrase_plural);

   // Write the 'x standard one player levels' caption to the one background.
   WriteTextCen(                                title_screen_text_row_1,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                     11,
                                                                     14,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'detected across x level sets' caption to the one background.
   WriteTextCen(                                title_screen_text_row_2,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                     13,
                                                                     14,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   char title_screen_text_row_3[32];
   char title_screen_text_row_4[32];

   siprintf(title_screen_text_row_3, "%d custom one player level%s",   no_lemmings_ds_level_category_one_player_custom_levels,
                                                                      (no_lemmings_ds_level_category_one_player_custom_levels == 1) ? global_gamephrase_nullstring
                                                                                                                                    : global_gamephrase_plural);
   siprintf(title_screen_text_row_4, "across %d level set%s",          lemmings_ds_level_category_one_player_custom.no_level_sets,
                                                                      (lemmings_ds_level_category_one_player_custom.no_level_sets == 1) ? global_gamephrase_nullstring
                                                                                                                                        : global_gamephrase_plural);

   // Write the 'x custom one player levels' caption to the one background.
   WriteTextCen(                                title_screen_text_row_3,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                     16,
                                                                     12,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'detected across x level sets' caption to the one background.
   WriteTextCen(                                title_screen_text_row_4,
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                     18,
                                                                     12,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);
#endif

   // Write the version string string to the one background.
   WriteText(                   global_gamephrase_titlescreensubtext[0],
                                              SCREEN_BASE_BLOCK_SUB(13),
                                                                      6,
                                                                     21,
                                                                     11,
             TITLE_SCREEN_MENU_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Blank the in-out tiemr strip on the sub screen (clear any debris left over)
   UpdateSubScreenAsLevelInfo_BlankInOutTime();

   // Now, we manage the sprites for the lemming eyes on the sub screen.
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH                  3

#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_X_1                           20
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_Y_1                           43
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_1                       662
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SHAPE_1             ATTR0_SQUARE
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SIZE_1              ATTR1_SIZE_8
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_1                          1
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1            60
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_1     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_1     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_1     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_1 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_X_2                          109
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_Y_2                           44
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_2                       641
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SHAPE_2             ATTR0_SQUARE
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SIZE_2              ATTR1_SIZE_8
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_2                          1
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2           120
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_2     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_2     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_2     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_2 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_X_3                          196
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_Y_3                           38
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_3                       656
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SHAPE_3             ATTR0_SQUARE
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SIZE_3              ATTR1_SIZE_8
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_3                          1
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3           180
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_3     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 1))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_3     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 2))
#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_3     (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_3 + (TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_LENGTH * 3))

#define TITLE_SCREEN_SUB_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(l) \
         sprite_shadow_s[l-1].attribute[0] = TITLE_SCREEN_SUB_MENU_LEMMING_EYES_Y_##l | TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SHAPE_##l;          \
         sprite_shadow_s[l-1].attribute[1] = TITLE_SCREEN_SUB_MENU_LEMMING_EYES_X_##l | TITLE_SCREEN_SUB_MENU_LEMMING_EYES_SIZE_##l;           \
         sprite_shadow_s[l-1].attribute[2] = (1 << 12) | (2 << 10);                                                                            \
         if (lemming_eyes_blink_frame < TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_1_##l) {                                           \
            sprite_shadow_s[l-1].attribute[2] |= TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_##l;                                                  \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_2_##l) {                                    \
            sprite_shadow_s[l-1].attribute[2] |= TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_##l * 1; \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_3_##l) {                                    \
            sprite_shadow_s[l-1].attribute[2] |= TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_##l * 2; \
         } else if (lemming_eyes_blink_frame < TITLE_SCREEN_SUB_MENU_LEMMING_EYES_FRAME_BOUNDARY_4_##l) {                                    \
            sprite_shadow_s[l-1].attribute[2] |= TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_##l + TITLE_SCREEN_SUB_MENU_LEMMING_EYES_INC_##l * 1; \
         } else {                                                                                                                            \
            sprite_shadow_s[l-1].attribute[2] |= TITLE_SCREEN_SUB_MENU_LEMMING_EYES_TILE_##l;                                                  \
         }

	// Copy the lemming blink frames data to the sub screen sprite tile data.
   for (int d = 0; d < (menugfx_lemming_eye_framesTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not 0!)
      (SPRITE_GFX_SUB+(S_TILE_SIZE_u16 * 641))[d] = menugfx_lemming_eye_framesTiles[d];
   }

	// Copying the lemmings eye palette data to the sprite palette (line 1)
   for (int pe = 0; pe < 3; pe++) {
      SPRITE_PALETTE_SUB[16+pe] = menugfx_lemming_eye_framesPal[pe];
   }

   // Ditch all garbage data stored in the sub sprite shadow.
   initSprites(sprite_shadow_s, 128, 0);
///////////////////////////////////////////////////////////

   // This controls the fade register values for both the main screen and sub screen
   int fade_frame = 32;

   // These are the possible states for the title screen loop:
#define TITLE_SCREEN_STATE_FADE_IN  0 // The title screens are fading in from black.
#define TITLE_SCREEN_STATE_STEADY   1 // The title screens are steady and responding to input.
#define TITLE_SCREEN_STATE_FADE_OUT 2 // The title screens are fading to black.
   int title_screen_state       = TITLE_SCREEN_STATE_FADE_IN;

   // This holds the value which will be returned by this function when it ends.
   int title_screen_result      = PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_STANDARD;
   // These values are defined above the function.

   // These variables hold the current frame of the scroller (scrolling background) animation
   int scroller_frame           = 0;
   // and the current frame for the lemmings in the bottom corners.
   int scroller_lemmings_frame  = 0;

   // These variables hold the index of the previous and ensuing
   int scroller_leaving_phrase  = 0;
   int scroller_upcoming_phrase = 1;

   // The title screen loop will continue until this value is set to true.
   int title_screen_exit       = 0;

   // This will increment constantly while the title screen is displayed. It is used to seed the random number generator.
   int number_of_frames_elapsed_here = 0;

   while (!title_screen_exit) {
      // Update joy and joyp keycodes.
      ScanKeypad();

      // Read new touchscreen pen coordinates if available.
		ScanPenCoords();

		// ------------------------------------------------------------

      // This code block handles user interaction against the sign lemmings:
      // You can only click on the sign lemmings when the state is steady.
      if (title_screen_state == TITLE_SCREEN_STATE_STEADY) {
         // Only respond to a tap against the touch screen:
         if (KeyDown(KEY_TOUCH)) {
            // Seed the random number generator using the current time and stylus position:
            srand(number_of_frames_elapsed_here * touchXY.px * touchXY.py);

            // Test to see if the pen lies on the horizontal band where all
            // five sign lemmings reside.
            if ((touchXY.py >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_Y1))
             && (touchXY.py <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_Y + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_Y2))) {
               // Now the input must be tested against each lemming sign:
               // Test the pen lies within the vertical strip bounded by the
               // horizontal coordinates of each lemming sign.

               // The first lemming sign is the 'One Player Standard' start game lemming.
                    if ((touchXY.px >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1))
                     && (touchXY.px <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_1_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2))) {
                  // Only respond to this event if at least one custom level was detected upon startup.
                  if (no_lemmings_ds_level_category_one_player_standard_levels != 0) {
                     // Set the appropriate result variable and change the state to 'fading out'.
                     title_screen_result = PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_STANDARD;
                     title_screen_state  = TITLE_SCREEN_STATE_FADE_OUT;
                  }
               }
               // The second lemming sign is the 'One Player Custom' start game lemming.
               else if ((touchXY.px >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1))
                     && (touchXY.px <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_2_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2))) {
                  // Only respond to this event if at least one custom level was detected upon startup.
                  if (no_lemmings_ds_level_category_one_player_custom_levels != 0) {
                     // Set the appropriate result variable and change the state to 'fading out'.
                     title_screen_result = PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_CUSTOM;
                     title_screen_state  = TITLE_SCREEN_STATE_FADE_OUT;
                  }
               }
// Only handle two player lems option if explicitly specified.
#ifdef SHOW_TWO_PLAYER_LEMS_OPTION
               // The third lemming sign is the 'Two Player' start game lemming.
               else if ((touchXY.px >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1))
                     && (touchXY.px <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_3_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2))) {
                  // Set the appropriate result variable and change the state to 'fading out'.
                  title_screen_result = PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_TWO_PLAYER;
                  title_screen_state  = TITLE_SCREEN_STATE_FADE_OUT;
               }
#endif
               // The fourth lemming sign is the 'Music / FX' start game lemming.
               else if ((touchXY.px >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1))
                     && (touchXY.px <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_4_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2))) {
                  // Cycle to the next music preference based on the current preference
                  // and change the sprite used to render the fourth sign lemming.
                  if (music_preference == MUSIC_PREFERENCE_MUSIC_AND_SOUND) {
                     music_preference = MUSIC_PREFERENCE_MUSIC;
                     sprite_shadow_m[3].attribute[2] = (1 + 64*4) | (3 << 10) | 0;
                  } else if (music_preference == MUSIC_PREFERENCE_MUSIC) {
                     music_preference = MUSIC_PREFERENCE_SOUND;
                     sprite_shadow_m[3].attribute[2] = (1 + 64*5) | (3 << 10) | 0;
                  } else if (music_preference == MUSIC_PREFERENCE_SOUND) {
                     music_preference = MUSIC_PREFERENCE_SILENCE;
                     sprite_shadow_m[3].attribute[2] = (1 + 64*6) | (3 << 10) | 0;
                  } else if (music_preference == MUSIC_PREFERENCE_SILENCE) {
                     music_preference = MUSIC_PREFERENCE_MUSIC_AND_SOUND;
                     sprite_shadow_m[3].attribute[2] = (1 + 64*3) | (3 << 10) | 0;
                  }

                  Cond_CommandMRD_SetPlayerVolume(64);

                  LemmingsDSConfig_UpdateConfigKeyValues();
                  LemmingsDSConfig_SaveConfigToFile();
               }
               // The fifth lemming sign is the 'Control Style' start game lemming.
               else if ((touchXY.px >= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X1))
                       && (touchXY.px <= (TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_5_X + TITLE_SCREEN_MAIN_MENU_SIGN_LEMMING_SIGN_INSET_X2))) {
                  // Cycle to the next control style based on the current preference
                  // and change the sprite used to render the fifth sign lemming.
                  if (control_style_major == CONTROL_STYLE_MAJOR_TAPPER) {
                     if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) {
                        control_style_pause = CONTROL_STYLE_PAUSE_TOGGLE;
                        sprite_shadow_m[4].attribute[2] = (1 + 64*8) | (3 << 10) | 0;
                     } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) {
                        control_style_major = CONTROL_STYLE_MAJOR_HOLDER;
                        control_style_pause = CONTROL_STYLE_PAUSE_HOLD;
                        sprite_shadow_m[4].attribute[2] = (1 + 64*9) | (3 << 10) | 0;
                     }
                  } else if (control_style_major == CONTROL_STYLE_MAJOR_HOLDER) {
                     control_style_major = CONTROL_STYLE_MAJOR_TACTICS;
                     control_style_pause = CONTROL_STYLE_PAUSE_HOLD;
                     sprite_shadow_m[4].attribute[2] = (1 + 64*10) | (3 << 10) | 0;
                  } else if (control_style_major == CONTROL_STYLE_MAJOR_TACTICS) {
                     if (control_style_pause == CONTROL_STYLE_PAUSE_HOLD) {
                        control_style_pause = CONTROL_STYLE_PAUSE_TOGGLE;
                        sprite_shadow_m[4].attribute[2] = (1 + 64*11) | (3 << 10) | 0;
                     } else if (control_style_pause == CONTROL_STYLE_PAUSE_TOGGLE) {
                        control_style_major = CONTROL_STYLE_MAJOR_TAPPER;
                        control_style_pause = CONTROL_STYLE_PAUSE_HOLD;
                        sprite_shadow_m[4].attribute[2] = (1 + 64*7) | (3 << 10) | 0;
                     }
                  }

                  LemmingsDSConfig_UpdateConfigKeyValues();
                  LemmingsDSConfig_SaveConfigToFile();
               }
            }
         }
      }

      // Refresh the lemming eyes sprties for all lemmings based on the current lemming_eyes_blink_frame.
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(1);
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(2);
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(3);
      
// Only show the eyes on the two player lems option if explicitly specified.
#ifdef SHOW_TWO_PLAYER_LEMS_OPTION
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(4);
#endif

      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(5);
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(6);
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(7);
      TITLE_SCREEN_MAIN_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(8);

      TITLE_SCREEN_SUB_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(1);
      TITLE_SCREEN_SUB_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(2);
      TITLE_SCREEN_SUB_MENU_LEMMING_EYES_REFRESH_SPRITE_MACRO(3);

      // Advance and loop lemming_eyes_blink_frame
      lemming_eyes_blink_frame++;
      if (lemming_eyes_blink_frame == 450) {
         lemming_eyes_blink_frame = 0;
      }

      // This defines how long the horizontal distance between
      // two phrases on the lower scroller will be (in tegels)
#define TITLE_SCREEN_MENU_SCROLLER_TEXT_DISTANCE_TEGELS 34

      // The defines the Y tegel used to render the scroller text.
#define TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL 22

      // The framestop zone is the number of frames it takes to scroll
      // away an existing phrase and replace it with a new one.
#define TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE (TITLE_SCREEN_MENU_SCROLLER_TEXT_DISTANCE_TEGELS * 8)
      // If scroller_frame is greater than or equal to this, then
      // it's within the waiting period.

      // The waiting time is the number of frames the title screen will
      // wait before moving on to the next phrase.
#define TITLE_SCREEN_MENU_SCROLLER_WAITING_TIME   128

      // This is the combined length in frames of displaying a phrase on screen.
#define TITLE_SCREEN_MENU_SCROLLER_FRAME_SEQUENCE_LENGTH (TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE + TITLE_SCREEN_MENU_SCROLLER_WAITING_TIME)

      // Increment scroller frame.
      scroller_frame++;

      // When scroller frame hits the frame sequence length.
      // Loop it to zero.
      if (scroller_frame == TITLE_SCREEN_MENU_SCROLLER_FRAME_SEQUENCE_LENGTH) {
         scroller_frame = 0;
      }

      // When scroller hits the end of the framestop zone (when the
      // scroller has shifted a phrase into the centre of the screen)
      if (scroller_frame == TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE) {
         // Copy the upcoming phrase reference into the leaving phrase reference.
         // (The phrase that will be leaving to the left is the phrase
         // that just came onto the right)
         scroller_leaving_phrase = scroller_upcoming_phrase;

         // Increment the upcoming phrase.
         scroller_upcoming_phrase++;
         // If this would throw the upcoming phrase reference off the end of the array
         if (scroller_upcoming_phrase == MENUTEXT_MAINSCROLLER_TEXT_LINES) {
            // Loop it to 1.
            // Don't loop it to zero. Zero is a blank phrased used when the system is first turned on.
            scroller_upcoming_phrase = 1;
         }
      }

      // Clear the two lines of tegels used by the text. (on main screen background zero)
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         ( *((vu16*)(SCREEN_BASE_BLOCK(12)) + (tegelx) + (TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL    ) * 32)) =
          (*((vu16*)(SCREEN_BASE_BLOCK(12)) + (tegelx) + (TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL + 1) * 32)) = 0;
      }

      // If scroller_frame is still trying to scroll a new phrase onto the screen
      if (scroller_frame < TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE) {
         // Write the phrase leaving on the left based on the current scroller_frame.
         WriteText(                                menutext_mainscroller_text[scroller_leaving_phrase],
                                                                                 SCREEN_BASE_BLOCK(12),
                                                                         ((2 - (scroller_frame >> 3))),
                                                               TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL,
                                                                                                    10,
                                           TITLE_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

         // Write the phrase incoming from the right based on the current scroller_frame.
         WriteText(                               menutext_mainscroller_text[scroller_upcoming_phrase],
                                                                                 SCREEN_BASE_BLOCK(12),
                       (TITLE_SCREEN_MENU_SCROLLER_TEXT_DISTANCE_TEGELS + (2 - (scroller_frame >> 3))),
                                                               TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL,
                                                                                                    10,
                                           TITLE_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
      } else {
         // Write the phrase leaving on the left in the centre of the screen.
         WriteText(                                menutext_mainscroller_text[scroller_leaving_phrase],
                                                                                 SCREEN_BASE_BLOCK(12),
                                                                                                     2,
                                                               TITLE_SCREEN_MENU_SCROLLER_TEXT_Y_TEGEL,
                                                                                                    10,
                                           TITLE_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
      }

      // Scroll the background zero (the text scrolly background) based on scroller_frame.
      // Don't scroll it any more than 7. We move the text rendering location, not the background.
      BG0_X0 = (scroller_frame < TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE) ?
                    (scroller_frame & 7) : 0;

      // Scroll the background one (the yellow scrolly thing) based on scroller_frame.
      // Don't scroll it any more than 3. It loops!
      BG1_X0 = (scroller_frame < TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE) ?
                    (scroller_frame & 3) : 0;

      // Calculate the new scroller_lemmings_frame based on scroller_frame.
      // If the text within the waiting period, set the lemming frames to zero.
      scroller_lemmings_frame = (scroller_frame < TITLE_SCREEN_MENU_SCROLLER_FRAMESTOP_ZONE) ?
                                        (scroller_frame & 15) : 0;

      // This variable is used to calculate the new frame used to draw the lemmings
      // on the bottom left and right of the screen.
      sprite_shadow_m[13].attribute[2] = (TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 + (scroller_lemmings_frame * 4)) | (2 << 12);
      sprite_shadow_m[14].attribute[2] = (TITLE_SCREEN_SCROLLER_LEMMINGS_TILE_1 + (scroller_lemmings_frame * 4)) | (2 << 12);

      // Copy the sprite shadows to the OAM for each screen.
      updateOAM(OAM    , sprite_shadow_m);
      updateOAM(OAM_SUB, sprite_shadow_s);

      // This block of code advances the fade frame based on the current state:
      if (title_screen_state == TITLE_SCREEN_STATE_FADE_IN) {
         // If the state is 'fade in', then it will decrement the fade_frame
         // until it hits zero and then switch to 'steady'.
         if (fade_frame == 0) {
            title_screen_state = TITLE_SCREEN_STATE_STEADY;
         } else {
            fade_frame--;
         }
      } else if (title_screen_state == TITLE_SCREEN_STATE_FADE_OUT) {
         // If the state is 'fade out', then it will increment the fade_frame
         // until it hits 32 and then switch title_screen_exit to 1.
         if (fade_frame == 32) {
            title_screen_exit = 1;
         } else {
            fade_frame++;
         }
      }

      // During the fade states:
      if (title_screen_state != TITLE_SCREEN_STATE_STEADY) {
         // Place (fade_frame >> 1) into the blend value registers for
         // both the main and sub screens.
         SUB_BLEND_Y = fade_frame >> 1;
         BLEND_Y = fade_frame >> 1;
      }

      //// Manage background music: fadeout when fading to black.
      //if (title_screen_state == TITLE_SCREEN_STATE_FADE_OUT) {
      //   // Only set the volume on every fourth frame, otherwise, weird clickety pops.
      //   if ((fade_frame & 3) == 0) {
      //      Cond_CommandMRD_SetPlayerVolume((32 - fade_frame) << 1);
      //   }
      //}
      // Originally, the main menu and the level select had different music, but now they don't.
      // So, we don't need to fade out the music here. :)

      // Increment the number of frames spent on the title screen counter.
      number_of_frames_elapsed_here++;

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }

   // When this function ends, the screen should be black with totally black palettes.
   BlankAllPalettes();

   // Blank the sprite shadows.
   initSprites(sprite_shadow_m, 128, 0);
   initSprites(sprite_shadow_s, 128, 0);
   // Copy the sprite shadows to the OAM.
   updateOAM(OAM    , sprite_shadow_m);
   updateOAM(OAM_SUB, sprite_shadow_s);

   // Blank all tiles, tegels, etc.
   BlankVRAMPagesABCD();

   // Set the blend registers to no blend, no value.
   BLEND_CR = 0;
   BLEND_Y = 0;

   // Return the result.
   return title_screen_result;
}

// This function sets up the sub screen so it displays level info, the in-out timer strip
// and the minimap.
void SetUpSubScreenAsLevelInfo() {
   // Sub screen, turn the first three backgrounds.
   videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Level text and wooden frame.
                             | DISPLAY_BG1_ACTIVE   // Level info text.
                             | DISPLAY_BG2_ACTIVE   // In/out time text
                             | DISPLAY_BG3_ACTIVE   // Background Texture
                             | DISPLAY_SPR_ACTIVE   // Display sprites
                             | DISPLAY_SPR_1D_LAYOUT);  // Linear sprites

   // Blank all the vram used for the sub screen's tiles, tegels and sprites.
   BlankVRAMPagesCD();

   // Resets all of the translation registers for the sub screen background.
   ResetBackgroundTranslationRegistersSub();

   // First, we set up the background control registers so they know what to do.
	SUB_BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	SUB_BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06007000) Priority 3
	SUB_BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06006800) Priority 3

	// Move the level info text up four pixels.
	SUB_BG1_Y0 = +5;

	SUB_BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX_SUB+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   } // That's the background texture.

	// Copy the eight palette entries for the green level info texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE_SUB[pe] = menugfx_background_texturePal[pe];
   }

   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the sub screen while the sub screen is used to display level info.
#define SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX 243

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX_SUB+(S_TILE_SIZE_u16 * 243))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE_SUB[160+pe] = menugfx_lemmings_fontPal[pe];
   }

	// Copy the info panel tiles into the VRAM bank.
	for (int d = 0; d < (levelinfogfx_wooden_panelTilesLen >> 1); d++) {
      (BG_GFX_SUB+(S_TILE_SIZE_u16 * 431))[d] = levelinfogfx_wooden_panelTiles[d];
   } // These are used to surround the map and in-out timer strip.

	// Copy the info panel palette entries to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE_SUB[16+pe] = levelinfogfx_wooden_panelPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE_SUB last)
   BG_PALETTE_SUB[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         // Plus one because the first tile has to be transparent.
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   // Set the tiles used for the in-out timer strip to black.
   for (int tegely = 16; tegely < 18; tegely++) {
      for (int tegelx = 1; tegelx < 31; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(15)) + tegelx + tegely * 32)) = 0;
      }
   }

   // Set the tiles used for the map strip to black.
   for (int tegely = 19; tegely < 23; tegely++) {
      for (int tegelx = 1; tegelx < 31; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(15)) + tegelx + tegely * 32)) = 0;
      }
   }

   // Draw the wooden strip around the in-out timer strip:
   // First, the top line, the middle line, and the lower line.
   for (int tegelx = 1; tegelx < 31; tegelx++) {
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + tegelx + 15 * 32)) = 432 | (1 << 12);
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + tegelx + 18 * 32)) = 438 | (1 << 12);
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + tegelx + 23 * 32)) = 444 | (1 << 12);
   }
   // Then the sides of the in-out timer strip.
   for (int tegely = 16; tegely < 18; tegely++) {
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) +  0 + tegely * 32)) = 434 | (1 << 12);
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + 31 + tegely * 32)) = 436 | (1 << 12);
   }
   // Then the sides of the map strip.
   for (int tegely = 19; tegely < 23; tegely++) {
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) +  0 + tegely * 32)) = 440 | (1 << 12);
     (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + 31 + tegely * 32)) = 442 | (1 << 12);
   }
   // Now the corners:
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) +  0 + 15 * 32)) = 431 | (1 << 12);
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + 31 + 15 * 32)) = 433 | (1 << 12);
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) +  0 + 18 * 32)) = 437 | (1 << 12);
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + 31 + 18 * 32)) = 439 | (1 << 12);
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) +  0 + 23 * 32)) = 443 | (1 << 12);
   (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + 31 + 23 * 32)) = 445 | (1 << 12);

   // Clear all the sprites in the sub screen sprite shadow.
   initSprites(sprite_shadow_s, 128, 0);

   // The map is rendered as a 30x4 set of 256 colour 8x8 sprites.
   // The sprites are numbered in vertical columns (0-3, 4-7, etc.)
   // This code block will place the necesary 120 sprites:
   for (int spritex = 0; spritex < 30; spritex++) {
      for (int spritey = 0; spritey < 4; spritey++) {
         sprite_shadow_s[spritey + spritex*4].attribute[0] = (19+spritey)*8 | ATTR0_COLOR_256;
         sprite_shadow_s[spritey + spritex*4].attribute[1] = ( 1+spritex)*8;
         sprite_shadow_s[spritey + spritex*4].attribute[2] = 2*(spritey + spritex*4 + 1);
      }
   }

	// This refers to the area of memory used to draw the sprites making up the sub screens
	// miniature map.
	// It's a reference to 'TILE 1' (not zero) of the sub screens 256 colour sprite tile memory.
#define SUB_SCREEN_AS_LEVEL_INFO_SPRITE_DATA_BEGIN ((u8*)(SPRITE_GFX_SUB+(D_TILE_SIZE_u16 *   1)))

   // Update the sub screen using the data from the sprite shadow.
   updateOAM(OAM_SUB, sprite_shadow_s);
}

// This is an array used to hold a miniature version of the main map.
u8 sub_screen_as_level_info_map_data[240*32] = {0};

// This function will write the standard level info page onto the sub screen using
// the level data from 'level_description_struct'.
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
void UpdateSubScreenAsLevelInfo_WriteLevelInfo(const LEMMINGS_LEVEL_VITAL_STATISTICS *level_statistics, bool level_won) {
   // Clear the one and zero backgrounds.
   for (int tegely = 0; tegely < 15; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(12)) + tegelx + tegely * 32)) = 0;
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(13)) + tegelx + tegely * 32)) = 0;
      }
   }

   // Write the level name to the sub screen using red text if the level
   // has not already been won, and cyan text if it has.
   WriteText(                 (const char *)level_statistics->level_name,
                                               SCREEN_BASE_BLOCK_SUB(12),
                                                                       1,
                                                                       1,
                                                     level_won ? 14 : 11,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'number of lemmings' caption.
   WriteText(                    global_gamephrase_levelinfosubscreen[0],
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6,
                                                                       5,
                                                                      10,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the number of lemmings into number_of_lemmings_text as a null-terminated string.
   char number_of_lemmings_text[4];
   siprintf(number_of_lemmings_text, "%d", level_statistics->lemmings);

   // Write the number of lemmings to the sub screen after the number of lemmings caption.
   WriteText(                                    number_of_lemmings_text,
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6 + 19,
                                                                       5,
                                                                      10,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Work out the number of lemmings to be saved, as a percentage.
   int to_be_saved_number = IntDivR(level_statistics->to_be_saved * 100,
                                    level_statistics->lemmings);
   // We need to get the actual value of the number to display.

   // Write the percentage into to_be_saved_text as a null-terminated string.
   char to_be_saved_text[4];
   siprintf(to_be_saved_text, "%d", to_be_saved_number);

   // Write the percentage to the sub screen.
   WriteText(                                           to_be_saved_text,
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6,
                                                                       7,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'to be saved' caption to the sub screen after the percentage.
   WriteText(                    global_gamephrase_levelinfosubscreen[1],
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6 + strlen(to_be_saved_text),
                                                                       7,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'release rate' caption to the sub screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[2],
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6,
                                                                       9,
                                                                      13,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the release rate into release_rate_text as a null-terminated string.
   char release_rate_text[4];
   siprintf(release_rate_text, "%d", level_statistics->release_rate);


   // Write the release rate to the sub screen after the release rate caption.
   WriteText(                                          release_rate_text,
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6 + 13,
                                                                       9,
                                                                      13,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'time' caption to the sub screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[(level_statistics->time_in_minutes == 1) ? 4 : 3],
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6,
                                                                      11,
                                                                      14,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the time in minutes into time_in_minutes_text as a null-terminated string.
   char time_in_minutes_text[4];
   siprintf(time_in_minutes_text, "%d", level_statistics->time_in_minutes);

   // Write the time to the sub screen after the time caption.
   WriteText(                                       time_in_minutes_text,
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6 + 8,
                                                                      11,
                                                                      14,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the rating caption to the sub screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[5],
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6,
                                                                      13,
                                                                      15,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the rating description caption to the sub screen.
   WriteText(                       level_statistics->rating_description,
                                               SCREEN_BASE_BLOCK_SUB(13),
                                                                       6 + 8,
                                                                      13,
                                                                      15,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);
}

// This function copies a full palette of 256 from the palette referenced by 'palette'
// into the sub screen sprite palette.
// The purpose of this function is to copy the a level palette
// into the sub screen so it can rendered using the sprites.
void UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(const u16 *palette) {
   for (int pe = 0; pe < 256; pe++) {
      SPRITE_PALETTE_SUB[pe] = palette[pe];
   }
}

// This function copies the map data from level_data into the sub screen's map sprite system.
// The camera_*_inset parameters give the position of the camera in maggies.
// The draw_* paramters control whether or not the lemmings or camera border should be drawn.
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
void UpdateSubScreenAsLevelInfo_UpdateMap(int camera_x_inset, int camera_y_inset, bool draw_border, bool draw_lems, LEMMING_INFO_STRUCT *lemming_info) {
   // Blank the sub screens miniature version of the map:
   memset(sub_screen_as_level_info_map_data, 0, 240*32);

   s32 column ,column2; // 'column' is the map sprite column currently being used to draw to.
                        // It's also used to draw the left side of the map border.
                        // 'column2' is the column used to draw the right hand side of the map border.
   s32   mapx ,   mapy; // 'map*' controls the current pixel within the miniature map sprite strip.
   s32 levelx , levely; // These control the current pixel within the real level_data map.

   // For each sprite column of the map strip:
   for (column = 0; column < 30; column++) {
      // For each row down a specific map sprite column:
      for (mapy = 0; mapy < 32; mapy++) {
         // Calculate the real map y pixel to grab the pixel data from.
         levely = (mapy * 21) >> 2; // This formula maps the 32 pixels of the miniature map
                                    // to 168 pixels of the real map screen.

         // For each pixel across the specific map sprite row:
         for (mapx = 0; mapx < 8; mapx++) {
            // Calculate the real map x pixel to grab the pixel data from.
            levelx = ((mapx + (column*8)) * 13653) >> 11; // This formula maps the 240 pixels of the miniature map
                                                          // to 1599 pixels of the real map screen.

            // Grab the pixel from the large map and store it into the miniature map.
            sub_screen_as_level_info_map_data[(mapx) + (mapy*8) + (column * (32*8))] = level_data[levelx][levely];
         }
      }
   }

   // Draw the border:
   if (draw_border) {
      // These store the miniature map position of the extremes of the camera.
      s32 mapx2, mapy2;

      // This stores the current inset of the current drawing pixel against the minimap.
      s32    ix;

      // These store the current location within the current map sprite column.
      s32    dx,    dy;

      // ix is the position through the map, dx is the position through the sprite column.
      // dx is worked out from ix.

      // Calculate the minimap position of the camera box extremes.
      mapx  =         ((camera_x_inset      >> log2magnification) *  307) >> 11;
      mapy  =         ((camera_y_inset      >> log2magnification) *  195) >> 10;
      mapx2 = mapx + (((GAME_DISPLAY_X_SIZE >> log2magnification) *  307) >> 11);
      mapy2 = mapy + (((GAME_DISPLAY_Y_SIZE >> log2magnification) *  195) >> 10);
      // These map the 1600 by 168 level world into the 240 by 32 minimap.

      // Clamp the far coordinate to the egde of the minimap.
      if (mapx2 > 239) mapx2 = 239;

      // Draw the top and bottom edges of the camera rectangle.
      // For each pixel between mapx and mapx2:
      for (ix = mapx; ix <= mapx2; ix++) {
         // Calculate the pixel column for the current pixel.
         column = ix >> 3;
         // Calulcate the pixel within the current column.
         dx     = ix &  7;

         // Draw the top and bottom pixels of the current column using the special map colour.
         sub_screen_as_level_info_map_data[(dx) + (mapy *8) + (column  * (32*8))] = SPECIAL_COLOUR_MAP_BORDER;
         sub_screen_as_level_info_map_data[(dx) + (mapy2*8) + (column  * (32*8))] = SPECIAL_COLOUR_MAP_BORDER;
      }

      // Calculate the pixel column and pixel within column for the
      // left and right extremes of the camera rectangle.
      column   = mapx >> 3;
      mapx    &= 7;
      column2  = mapx2 >> 3;
      mapx2   &= 7;

      // For each pixel down the sides of the camera box:
      for (dy = mapy; dy <= mapy2; dy++) {
         // Draw the pixel at the left and right using the special map colour.
         sub_screen_as_level_info_map_data[(mapx ) + (dy*8) + (column  * (32*8))] = SPECIAL_COLOUR_MAP_BORDER;
         sub_screen_as_level_info_map_data[(mapx2) + (dy*8) + (column2 * (32*8))] = SPECIAL_COLOUR_MAP_BORDER;
      }
   }

   // Draw the lemming dots:
   if (draw_lems) {
      for (u32 lem = 0; lem < current_level_runtime_stats.lemmings; lem++) {
         if (lemming_info[lem].state != LEMSTATE_DONTEXIST) {
            mapx = (lemming_info[lem].x *  307) >> 11;
            mapy = (lemming_info[lem].y *  195) >> 10;
            // These map the 1584 by 168 level world into the 240 by 32 minimap.

            // Clamp the x coordinate to the egde of the minimap.
            if (mapx > 239) mapx = 239;

            // Calculate the pixel column and pixel within column for the
            // lemming x coordinate.
            column  = mapx >> 3;
            mapx   &= 7;

            // Draw the lemming pixel at the coordinates using the special lemming colour.
            sub_screen_as_level_info_map_data[(mapx) + (mapy*8) + (column * (32*8))] = SPECIAL_COLOUR_MAP_LEMMING;
         }
      }
   }

   // Copy the new minimap into the VRAM sprite area of the sub screen.
   dmaCopy(sub_screen_as_level_info_map_data, SUB_SCREEN_AS_LEVEL_INFO_SPRITE_DATA_BEGIN, 240*32);
}

// Blanks the sub screen minimap (from SetUpSubScreenAsLevelInfo())
void UpdateSubScreenAsLevelInfo_BlankMap() {
   // Blank the minimap.
   memset(sub_screen_as_level_info_map_data, 0, 240*32);

   // Copy this blank minimap into the VRAM sprite area of the sub screen.
   dmaCopy(sub_screen_as_level_info_map_data, SUB_SCREEN_AS_LEVEL_INFO_SPRITE_DATA_BEGIN, 240*32);
}

// Updates the subscreen minimap using the preview data stored in the level_preview_data_chunk array.
void UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData() {
   // Copy the minimap from the global array into the VRAM sprite area of the sub screen.
   dmaCopy(level_preview_data_chunk, SUB_SCREEN_AS_LEVEL_INFO_SPRITE_DATA_BEGIN, 240*32);
}

// This function will set the blend registers so that various objects are
// affected by the current blend value.
// The objects affects are determined by 'objects', a set of flags. (Definitions
// are earlier in the file next to the UpdateSubScreenAsLevelInfo* prototypes.)
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
void UpdateSubScreenAsLevelInfo_SetFadeObjects(int objects) {
   // Set the sub screen blend type to blend-with-black.
   SUB_BLEND_CR = BLEND_FADE_BLACK;

   // Set the background two (in-out time strip) to blend, if the flag is set.
   if (objects & UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_IN_OUT_TIME) {
      SUB_BLEND_CR |= BLEND_SRC_BG2;
   }

   // Set the sprite layer (minimap) to blend, if the flag is set.
   if (objects & UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP) {
      SUB_BLEND_CR |= BLEND_SRC_SPRITE;
   }

   // Set the rest of the items (background texture, level info) to blend, if the flag is set.
   if (objects & UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_OTHER_STUFF) {
      SUB_BLEND_CR |= BLEND_SRC_BG0
                    | BLEND_SRC_BG1
                    | BLEND_SRC_BG3
                    | BLEND_SRC_BACKDROP;
   }
}

// This function sets the fade value for the sub screen.
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
void UpdateSubScreenAsLevelInfo_SetFadeValue(int fade_value) {
   SUB_BLEND_Y = fade_value;
}

// This function will blank the strip of sub screen background containing the in-out time display.
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
void UpdateSubScreenAsLevelInfo_BlankInOutTime() {
   for (int tegely = 16; tegely < 18; tegely++) {
      for (int tegelx = 1; tegelx < 31; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK_SUB(14)) + tegelx + tegely * 32)) = 0;
      }
   }
}

// This function will update the strip of sub screen background containing the in-out time display.
// This function relies on the sub screen being in the state left by SetUpSubScreenAsLevelInfo().
// The paramters are: 'out', the number of active lemmings, 'in', the number of saved lemmings,
// and 'no', the total number of lemmings.
void UpdateSubScreenAsLevelInfo_UpdateInOutTime(u32 out, u32 in, u32 no, u32 time_remaining) {
   // Erase any text already displayed on the in-out time strip.
   UpdateSubScreenAsLevelInfo_BlankInOutTime();

   // Write the 'out:' caption to the sub screen.
   WriteText(                        global_gamephrase_inoutsubscreen[0],
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                       1,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the out number into out_text as a null-terminated string.
   char out_text[4];
   siprintf(out_text, "%3d", out);

   // Write the out number to the sub screen.
   WriteText(                                                   out_text,
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                       6,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Write the 'in:' caption to the sub screen.
   WriteText(                        global_gamephrase_inoutsubscreen[1],
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                      11,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Work out the number of lemmings saved, as a percentage.
   int in_number = IntDivR(in * 100, no);
   // We need to get the actual value of the number to display.

   // Write the percentage into in_text as a null-terminated string.
   char in_text[4];
   siprintf(in_text, "%3d", in_number);

   // Write the percentage to the sub screen.
   WriteText(                                                    in_text,
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                      15,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);


   // Write the 'time:' caption to the sub screen.
   WriteText(                        global_gamephrase_inoutsubscreen[2],
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                      21,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Calculate how many minutes are remaining:
   int minutes_number = IntDiv(time_remaining, LEMMING_FRAMES_PER_SECOND_REMAINING_TIME*60);

   // Write the number of minutes into minutes_number as a null-terminated string.
   char minutes_text[4];
   siprintf(minutes_text, "%d", minutes_number);

   // Write the number of minutes to the sub screen.
   WriteText(                                               minutes_text,
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                      27,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);

   // Calculate how many seconds are remaining:
   int seconds_number = Modulo(time_remaining, LEMMING_FRAMES_PER_SECOND_REMAINING_TIME*60);
   seconds_number = IntDiv(seconds_number, LEMMING_FRAMES_PER_SECOND_REMAINING_TIME);

   // Write the number of minutes into seconds_text as a null-terminated string.
   char seconds_text[4];
   siprintf(seconds_text, "%02d", seconds_number);

   // Write the number of seconds to the sub screen.
   WriteText(                                               seconds_text,
                                               SCREEN_BASE_BLOCK_SUB(14),
                                                                      29,
                                                                      16,
                                                                      12,
             SUB_SCREEN_LEVEL_INFO_LEMMINGS_FONT_SUB_SCREEN_TILE_DATA_INDEX);
}

// This controls how many levels can be displayed on the main screen level selection list.
#define UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES 8

// This define holds the tile index of the EXCLAMATION MARK character
// used on the main screen during the level select sequence.
#define UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX 243

// This function will write a list of levels (based on the given level set pointer) to the main screen.
// The parameters: 'level_set' is a pointer to the level set to refer to
//                 'list_start' is the absolute index of the level to begin the list with
//                 'selected' is the absolute index of the level that should be highlighted as selected.
void UpdateMainScreenAsLevelSelect_WriteLevelList(LEMMINGS_DS_LEVEL_SET_HANDLE *level_set, int list_start, int selected, u8 *level_won_array) {
   // line is the current line.
   // level_index is the current level number.
   int line, level_index = list_start;

   // For each line to be written:
   for (line = 0; line < UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES; line++) {
      // Draw a blank string to the main screen to erase the previous text.
      WriteText(                              global_gamephrase_blank_line,
                                                     SCREEN_BASE_BLOCK(14),
                                                                         0,
                                                                         0 + (2 * line),
                                                                         0,
                UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);
   }

   // For each line to be written:
   for (line = 0; ((line < UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES) && (level_index < level_set->no_levels)); line++, level_index++) {
      // Has this level been won?
      int won = level_won_array[level_index];

      // Is this level selected?
      int sel = (level_index == selected);

      // Write the level name to the main screen:
      WriteText(                level_set->levels[level_index]->level_name,               // Write the level name.
                                                     SCREEN_BASE_BLOCK(14),
                                                                         0,               // At the far left edge of the two background.
                                                                         0 + (2 * line),  // Position is based on line number, two tegels per line.
                                                                         (won ?
                                                                           (sel ?
                                                                              ( 9)        //     Won and     selected = light cyan.
                                                                            : (14))       //     Won and not selected = dark cyan.
                                                                         : (sel ?
                                                                              ( 8)        // Not won and     selected = light green.
                                                                            : (12))),     // Not won and not selected = dark green.
                UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);
   }
}

// These constants define the possible values of the 'past_shown' parameter to the GoLevelSelectStandard function
#define PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_TITLE_SCREEN  0 // We're heading into the level select from the title screen.
#define PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_LEVEL_INFO    1 // We're heading into the level select from a cancelled 'get ready' screen.
#define PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_STATUS_SCREEN 2 // We're heading into the level select from the status ('result') screen.

// These are the possible return values from the level select function:
#define PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL   0 // The player wants to play a level. (returned through selected_level)
#define PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_TO_MAIN_MENU 1 // The player wants to return to the main menu.

// This function will execute the level selection sequence.
// If a level is selected, the active level set and level number will be placed in
// selected_level_set and selected_level.
int PlayLemmingsDS_GoLevelSelect(const LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *level_category, LEMMINGS_LEVEL_VITAL_STATISTICS *level_vital_statistics, int *selected_level_set, int *selected_level, u8 **level_won_array, int past_shown) {
   DebugAppend("Entered PlayLemmingsDS_GoLevelSelect():\r\n");
   DebugWrite();

   // Based on the last shown parameter, set the sub screen fade objects to different things:
   if (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_TITLE_SCREEN) {
      // If the last shown parameter is the title screen, then we need to fade everything on the sub screen.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);
   } else
   if (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_LEVEL_INFO) {
      // If the last shown parameter is the level info screen, then we don't need to fade anything.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_NOTHING);
   } else
   if (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_STATUS_SCREEN) {
      // If the last shown parameter is the status (result) screen, we need to fade in the map.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);
   }

   // Set the blend value register for the sub screen to 16 (fully blend to black)
   UpdateSubScreenAsLevelInfo_SetFadeValue(16);

   // Blank the in-out time strip.
   UpdateSubScreenAsLevelInfo_BlankInOutTime();

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);

   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);

   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Top of screen captions
                          | DISPLAY_BG1_ACTIVE   // Scrolling instruction prompt.
                          | DISPLAY_BG2_ACTIVE   // Scrolling level list.
                          | DISPLAY_BG3_ACTIVE); // Background texture

   // Enable the zero, one and sprite windows for the main screen.
   DISPLAY_CR |= DISPLAY_WIN0_ON;
   DISPLAY_CR |= DISPLAY_WIN1_ON;
   DISPLAY_CR |= DISPLAY_SPR_WIN_ON;

   // Enable blending on everything for the main screen.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;

   // Fade everything fully to black on the main screen.
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Blank all tiles, tegels, etc.
   BlankVRAMPagesAB();
   // Resets all of the translation registers for the main screen backgrounds.
   ResetBackgroundTranslationRegistersMain();
   // Wait for a frame.
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
   BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
   // The three background is a 16 colour background using the tegel data from
   // base block 15. (that's 0x06007800) Priority 3
   BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
   // The two background is a 16 colour background using the tegel data from
   // base block 14. (that's 0x06007000) Priority 3
   BG2_X0 = -8;
   // Move background two to the left by 8.
   BG2_Y0 = -40;
   // Move background two down by 24.

   BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
   // The one background is a 16 colour background using the tegel data from
   // base block 13. (that's 0x06006800) Priority 3

   BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
   // The zero background is a 16 colour background using the tegel data from
   // base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
   for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
      // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   } // That's the background texture.

   // This tracks the current and next palette sets for the background
   int background_palette_index_current = 8*rng_rand(4); // (4 random background texture palettes)
   int background_palette_index_next = 0;

   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe + background_palette_index_current];
   }

	// Now, copy the lemmings font into the VRAM bank.
   for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 96; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }
   // Special highlight palette placement for the lemmings font.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[160-32+pe   ] = BG_PALETTE[160+40+pe];
      BG_PALETTE[160-32+pe+16] = BG_PALETTE[160+72+pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   DebugAppend("About to perform 'Should I update the subscreen?' check.\r\n");
   DebugWrite();

   // The sub screen map needs updating if the past shown variable is anything but
   // level info.
   if ((past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_TITLE_SCREEN )
    || (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_STATUS_SCREEN)) {
      VitalStatistics_ExtractLemmingsLevelVitalStatisticsAndMapPreviewFromLevelFilename(level_vital_statistics, level_category->level_sets[*selected_level_set]->levels[*selected_level]->location);
      
      UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData();
      UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(level_preview_palette);

      UpdateSubScreenAsLevelInfo_WriteLevelInfo(level_vital_statistics, ((*level_won_array)[(*selected_level)] != 0));
   }

   DebugAppend("'Should I update the subscreen?' check complete.\r\n");
   DebugWrite();

   int level_list_start = 0; // This is the start of the list, which is in the range 0-29.

   // These are the currently selected level, and the previously currently selected level.
   int old_selected_level = *selected_level;

   // These are the current level shown on top, and the previous level shown on top.
   int level_shown_on_top = *selected_level;
   int old_level_shown_on_top = level_shown_on_top;

   // Position the list so that the currently selected item is the second item, if possible.
   if (*selected_level > 0) {
      level_list_start = *selected_level - 1;
   }
   if (level_list_start > (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1))) {
      level_list_start = (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1));
   }
   if (level_list_start < 0) {
      level_list_start = 0;
   }

   // Write the level list to the main screen.
   UpdateMainScreenAsLevelSelect_WriteLevelList(level_category->level_sets[*selected_level_set], level_list_start, (*selected_level), (*level_won_array));

   // -------------------------------------------

   // This part is copied from the code in the loop which updates the top info when
   // the level selected is changed.

         // Erase the text on the top of the main screen on background one.
         WriteText(                               global_gamephrase_blank_line,
                                                         SCREEN_BASE_BLOCK(12),
                                                                             0,
                                                                             1,
                                                                             0,
             UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the 'Level' caption on the main screen.
         WriteText(                 global_gamephrase_levelselectmainscreen[0],
                                                         SCREEN_BASE_BLOCK(12),
                                                                             1,
                                                                             1,
                                                                            11,
            UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the level number into level_number_text as a null-terminated string.
         char level_number_text[16];
         siprintf(          level_number_text, "% 3d", min(999, (*selected_level) + 1));
         // It's very rare for there to be more than 999 levels.
         // IF that happens, then just lock the number to 999.

         // Write the level number to the main screen after the 'Level:' caption.
         WriteText(                                          level_number_text,
                                                         SCREEN_BASE_BLOCK(12),
                                                                             7,
                                                                             1,
                                                                            11,
            UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the relevant difficulty string after the 'Rating' caption on the main screen.
         WriteText(level_category->level_sets[*selected_level_set]->level_set_name,
                                                         SCREEN_BASE_BLOCK(12),
                                                                            14,
                                                                             1,
                                                                            15,
            UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

   // -------------------------------------------

   // Set up the windowing stuff for the main screen:
   // We want everything to be displayed as normal, except BG1 must only be displayed in window 0s

   // Define size and position of window 0, which will be used to show the level list.
#define LEVEL_SELECT_MAIN_SCREEN_LEVEL_LIST_WINDOW_Y_START  (32 + 8)
#define LEVEL_SELECT_MAIN_SCREEN_LEVEL_LIST_WINDOW_Y_END    (152)
   WIN0X = (0 << 8) | 255;
   WIN0Y = (LEVEL_SELECT_MAIN_SCREEN_LEVEL_LIST_WINDOW_Y_START << 8) | LEVEL_SELECT_MAIN_SCREEN_LEVEL_LIST_WINDOW_Y_END;

   // Define size and position of window 1, which will be used to show the controls prompt.
#define LEVEL_SELECT_MAIN_SCREEN_AVAILABLE_CONTROLS_WINDOW_Y_START  (168)
#define LEVEL_SELECT_MAIN_SCREEN_AVAILABLE_CONTROLS_WINDOW_Y_END    (184)
   WIN1X = (0 << 8) | 255;
   WIN1Y = (LEVEL_SELECT_MAIN_SCREEN_AVAILABLE_CONTROLS_WINDOW_Y_START << 8) | LEVEL_SELECT_MAIN_SCREEN_AVAILABLE_CONTROLS_WINDOW_Y_END;

   // Don't show backgrounds 0, 1 within window 0 and background 2 within window 1.   !?!
   WIN_IN  = (~(0)) & (~(BIT(1)))                             & (~(BIT(10)));
   WIN_OUT = (~(0)) & (~(BIT(1))) & (~(BIT(2))) & (~(BIT(9))) & (~(BIT(10)));       // God damnit... Dualis would have to not support windowing, eh?

   // These are the possible states for the level select screen loop:
#define LEVEL_SELECT_SCREEN_STATE_FADE_IN               0 // The level select is fading in.
#define LEVEL_SELECT_SCREEN_STATE_STEADY                1 // The level select is steady and handling input.
#define LEVEL_SELECT_SCREEN_STATE_FADE_OUT              2 // The level select is fading in.
   int level_select_screen_state       = LEVEL_SELECT_SCREEN_STATE_FADE_IN;

   // Set the result variable to play level.
   int level_select_screen_result      = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL;

   // Keep looping the level select loop until this is set.
   int level_select_screen_exit        = 0;

   // This controls the fading of the main screen.
   int main_screen_fade_frame          = 32;

   // These are the possible states for the SUB SCREEN during the main loop:
#define LEVEL_SELECT_SUBSCREEN_STATE_START_FADE_IN  0 // The sub screen is fading in from the previous state
#define LEVEL_SELECT_SUBSCREEN_STATE_STEADY         1 // The sub screen is steady
#define LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_IN  2 // The sub screen is fading in from changing levels
#define LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_OUT 3 // The sub screen is fading out to change level
#define LEVEL_SELECT_SUBSCREEN_STATE_END_FADE_OUT   4 // The sub screen is fading out to the next state
   int level_select_subscreen_state;
   int level_select_subscreen_frame = 0;

   // If the past shown state was the title screen or the status screen
   if ((past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_TITLE_SCREEN )
    || (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_STATUS_SCREEN)) {
      // Tell the subscreen to fade in from black.
      level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_START_FADE_IN;
      // Set the fade frame for the sub screen.
      level_select_subscreen_frame = 32;
   } else {
      // Any other state should be valid: the sub screen does not need to be altered.
      level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_STEADY;
   }

   // There are the possible states for the LEVEL LIST during the main loop:
#define LEVEL_SELECT_LEVEL_LIST_STATE_STEADY                 0 // The level list is steady.
#define LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_LEFT  1 // The level list is shifting to the left.
#define LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_RIGHT 2 // The level list is shifting to the right.
#define LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_UP        3 // The level list is shifting levels up.
#define LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_DOWN      4 // The level list is shifting levels down.

   // The level list will start off steady.
   int level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_STEADY;
   int level_select_level_list_frame = 0;

   // Reset the controls text frame.
   int level_select_controls_text_frame        = 0;
   int level_select_controls_text_current_line = 0;

   // This is the number of frames that the main screen blue text controls prompt will stay steady for.
#define LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH 180

   // Based on the text in the global_gamephrase_levelselectcontrolstext array:
   // Make sure the text is centered by insetting background one by 4 pixels if there is an odd
   // number of characters.
	if (strlen(global_gamephrase_levelselectcontrolstext[level_select_controls_text_current_line]) & 1) {
	   // Move background one to the left by 4.
      BG1_X0 = 4;
   } else {
	   // Move background one back to normal position.
      BG1_X0 = 0;
   }

   // Write the current line of the main screen controls prompt.
   WriteTextCen(global_gamephrase_levelselectcontrolstext[level_select_controls_text_current_line],
                                                                             SCREEN_BASE_BLOCK(13),
                                                                                                21,
                                                                                                10,
                                  UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

   // Loop until level_select_screen_exit is set.
   while (!level_select_screen_exit) {
      // Update joy and joyp keycodes.
      ScanKeypad();

      // Read new touchscreen pen coordinates if available.
		ScanPenCoords();

		// ------------------------------------------------------------

      // These control the size of the area where the pen can be used to select levels.
#define LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_START   0
#define LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_END   255
#define LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START  40
#define LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_END    ((LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START + (16 * (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1))) - 1)

      // If the main state is steady:
      if (level_select_screen_state == LEVEL_SELECT_SCREEN_STATE_STEADY) {
         // Handle the pen release event:
         if (KeyRelease(KEY_TOUCH)) {
            // When the pen is released, the level to be shown on the top screen
            // is the selected level. (This is so that you can drag the pen
            // about on the main screen,  and only have the sub screen
            // change when you release the pen).
            level_shown_on_top = *selected_level;
         } // The next handler handles when the level list is steady.
           // No pen interactions are possible when the list is moving.
         else if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_STEADY) {
            // If the pen has tapped against the screen this frame:
            if (KeyDown(KEY_TOUCH)) {
               if ((touchXY.px >= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_START)
                && (touchXY.px <= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_END  )
                && (touchXY.py >= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START)
                && (touchXY.py <= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_END  )) {
                  // Calculate the line which the player has touched:
                  int touch_y_inset_into_region = touchXY.py - LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START;
                  int touched_line = touch_y_inset_into_region >> 4;
                  // Calculate the LEVEL (within the current set) which the player has touched.
                  int touched_level = touched_line + level_list_start;

                  // If the player has tapped the level that's currently highlighted:
                  if (touched_level == (*selected_level)) {
                     // Change the state to fade out, and set the result.
                     level_select_screen_state = LEVEL_SELECT_SCREEN_STATE_FADE_OUT;
                     level_select_screen_result = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL;
                     // Because we're transitioning to play level, we don't need to
                     // change anything about the sub screen.
                  }
               }
            } else if (Key(KEY_TOUCH)) {
               // If the pen has been held against the screen from frame to frame:
               if ((touchXY.px >= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_START)
                && (touchXY.px <= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_X_END  )
                && (touchXY.py >= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START)
                && (touchXY.py <= LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_END  )) {
                  // Calculate the line which the player has touched:
                  int touch_y_inset_into_region = touchXY.py - LEVEL_SELECT_MAIN_SCREEN_TOUCHSCREEN_LEVEL_SELECT_REGION_Y_START;
                  int touched_line = touch_y_inset_into_region >> 4;
                  // Calculate the LEVEL (within the current set) which the player has touched.
                  int touched_level = touched_line + level_list_start;

                  // Because the list is steady at this point, we allow the player
                  // to hold the pen against the screen to change the current level,
                  // but only to a level which exists!

                  if (touched_level < level_category->level_sets[*selected_level_set]->no_levels) {
                     (*selected_level) = touched_level;
                  }
               }
            } else if (KeyDown(KEY_L)
                    || KeyDown(KEY_R)) {
               // On the steady state, the shoulder buttons fade out to play level:
               // Change the state to fade out, and set the result.
               level_select_screen_state = LEVEL_SELECT_SCREEN_STATE_FADE_OUT;
               level_select_screen_result = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL;
               // Because we're transitioning to play level, we don't need to
               // change anything about the sub screen.
            } else if (KeyDown(KEY_SELECT)
                    || KeyDown(KEY_START) ) {
               // On the steady state, the Start and Select buttons fade out to main menu:
               // Change the state to fade out, and set the result.
               level_select_screen_state    = LEVEL_SELECT_SCREEN_STATE_FADE_OUT;
               level_select_screen_result   = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_TO_MAIN_MENU;
               // Because we're transitioning to main menu, we need to
               // tell the sub screen to fade out, and fade out everything.
               level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_END_FADE_OUT;
               UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);
            } else if (KeyDown(KEY_UP)
                    || KeyDown(KEY_X) ) {
               // On the steady state, the UP and X buttons will attempt
               // to move the level list upwards:

               // Move the selected level cursor upwards:
               (*selected_level)--;

               // Only allow the move up if the level isn't at the top of the list already:
               if (*selected_level < 0) {
                  (*selected_level)++;
               } else {
                  // Inform the 'on top' variable that a new level has been selected.
                  level_shown_on_top = (*selected_level);
                  // This will most likely trigger a top screen switch.
               }
            } else if (KeyDown(KEY_DOWN)
                    || KeyDown(KEY_B)   ) {
               // On the steady state, the DOWN and B buttons will attempt
               // to move the level list upwards:

               // Move the selected level cursor downwards:
               (*selected_level)++;

               // Only allow the move down if the level isn't at the bottom of the list already:
               if (*selected_level >= level_category->level_sets[*selected_level_set]->no_levels) {
                  (*selected_level)--;
               } else {
                  // Inform the 'on top' variable that a new level has been selected.
                  level_shown_on_top = (*selected_level);
                  // This will most likely trigger a top screen switch.
               }
            } else if (KeyDown(KEY_LEFT)
                    || KeyDown(KEY_Y)   ) {
               // On the steady state, the LEFT and Y buttons will attempt
               // to change the selected level set to the previous one
               // (But only if the level set isn't already at the first)
               if ((*selected_level_set) != 0) {
                  // Decrement the level set
                  (*selected_level_set)--;

                  // Update the level won array to reflect the level set change.
                  free(*level_won_array);
                  (*level_won_array) = LemmingsDSSavedProgress_LoadProgressFromLevelSetFile(level_category, level_category->level_sets[(*selected_level_set)]);

                  // Change the next background to a different random palette.
                  do {
                     background_palette_index_next = 8*rng_rand(4);
                  } while (background_palette_index_next == background_palette_index_current);

                  DEBUG_SECTION {
                     char output_text[128];
                     siprintf(output_text, "Decrementing level set to set #%d.\r\n", *selected_level_set);
                     DebugAppend(output_text);
                     DebugWrite();
                  };

                  // Move the selected level so that it retains its value, but
                  // clamped to the extremes of the switched to level set.
                  (*selected_level) = min((*selected_level), level_category->level_sets[(*selected_level_set)]->no_levels - 1);

                  // Update top screen.
                  level_shown_on_top     = (*selected_level);

                  // Position the list so that the currently selected item is the second item, if possible.
                  if (*selected_level > 0) {
                     level_list_start = *selected_level - 1;
                  }
                  if (level_list_start > (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1))) {
                     level_list_start = (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1));
                  }
                  if (level_list_start < 0) {
                     level_list_start = 0;
                  }

                  // Invalidate the 'old' variables.
                  old_level_shown_on_top = 99999;
                  old_selected_level     = 99999;

                  // Tell the level list so shift the difficulty to the left.
                  level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_LEFT;
                  // Reset the level list frame.
                  level_select_level_list_frame = 0;

                  // Set the blend registers in preparation for the coming fade.
                  // We need to fade out the level list text (background two)
                  // against the background texture and the backdrop. (backgrounds three and backdrop).
                  BLEND_CR = BLEND_ALPHA | BLEND_SRC_BG2
                                         | BLEND_DST_BG3
                                         | BLEND_DST_BACKDROP;
               }
            } else if (KeyDown(KEY_RIGHT)
                    || KeyDown(KEY_A)    ) {
               // On the steady state, the RIGHT and A buttons will attempt
               // to change the selected level set to the next one
               // (But only if the level set isn't already at the last)
               if (*selected_level_set != (level_category->no_level_sets - 1)) {
                  // Increment the level set.
                  (*selected_level_set)++;

                  // Update the level won array to reflect the level set change.
                  free(*level_won_array);
                  (*level_won_array) = LemmingsDSSavedProgress_LoadProgressFromLevelSetFile(level_category, level_category->level_sets[(*selected_level_set)]);

                  // Change the next background to a different random palette.
                  do {
                     background_palette_index_next = 8*rng_rand(4);
                  } while (background_palette_index_next == background_palette_index_current);

                  DEBUG_SECTION {
                     char output_text[128];
                     siprintf(output_text, "Incrementing level set to set #%d.\r\n", *selected_level_set);
                     DebugAppend(output_text);
                     DebugWrite();
                  };

                  // Move the selected level so that it retains its value, but
                  // clamped to the extremes of the switched to level set.
                  (*selected_level) = min((*selected_level), level_category->level_sets[(*selected_level_set)]->no_levels - 1);
                                             
                  // Update top screen.
                  level_shown_on_top     = (*selected_level);

                  // Position the list so that the currently selected item is the second item, if possible.
                  if (*selected_level > 0) {
                     level_list_start = *selected_level - 1;
                  }
                  if (level_list_start > (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1))) {
                     level_list_start = (level_category->level_sets[*selected_level_set]->no_levels - (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 1));
                  }
                  if (level_list_start < 0) {
                     level_list_start = 0;
                  }

                  // Invalidate the 'old' variables.
                  old_level_shown_on_top = 99999;
                  old_selected_level     = 99999;

                  // Tell the level list so shift the difficulty to the right.
                  level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_RIGHT;
                  // Reset the level list frame.
                  level_select_level_list_frame = 0;
                                                                    
                  // Set the blend registers in preparation for the coming fade.
                  // We need to fade out the level list text (background two)
                  // against the background texture and the backdrop. (backgrounds three and backdrop).
                  BLEND_CR = BLEND_ALPHA | BLEND_SRC_BG2
                                         | BLEND_DST_BG3
                                         | BLEND_DST_BACKDROP;
               }
            }
         }
      }

      // If the selected level has changed:
      if ((*selected_level) != old_selected_level) {
         // Erase the text on the top of the main screen on background one.
         WriteText(                               global_gamephrase_blank_line,
                                                         SCREEN_BASE_BLOCK(12),
                                                                             0,
                                                                             1,
                                                                             0,
             UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the 'Level' caption on the main screen.
         WriteText(                 global_gamephrase_levelselectmainscreen[0],
                                                         SCREEN_BASE_BLOCK(12),
                                                                             1,
                                                                             1,
                                                                            11,
             UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the level number into level_number_text as a null-terminated string.
         char level_number_text[3];
         siprintf(          level_number_text, "% 3d", (*selected_level) + 1);

         // Write the level number to the main screen after the 'Level:' caption.
         WriteText(                                          level_number_text,
                                                         SCREEN_BASE_BLOCK(12),
                                                                             7,
                                                                             1,
                                                                            11,
             UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // Write the relevant difficulty string after the 'Rating' caption on the main screen.
         WriteText(level_category->level_sets[*selected_level_set]->level_set_name,
                                                         SCREEN_BASE_BLOCK(12),
                                                                            14,
                                                                             1,
                                                                            15,
             UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);

         // If the level list is in the steady state:
         if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_STEADY) {
            // If you've selected the top entry in the list, and it's not zero.
            if (((*selected_level) != 0               )
             && ((*selected_level) == level_list_start)) {
               // Move the level list up one:
               level_list_start--;
               // Inform the level list that it should shift all of the levels down.
               level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_DOWN;
               level_select_level_list_frame = 0;
            } else
            // If you've selected the bottom entry in the list, and it's not the last entry.
            if ((!((*selected_level) >= (level_category->level_sets[*selected_level_set]->no_levels - 1)                  ))
             && (  (*selected_level) == (level_list_start + (UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_NO_LEVEL_TEXT_ENTRIES - 2)))) {
               // Inform the level list that it should shift all of the levels down.
               level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_UP;
               level_select_level_list_frame = 0;
            }
         }

         // Rewrite the level list, if it's not in the middle of shifting difficulty.
         if ((level_select_level_list_state != LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_LEFT )
          && (level_select_level_list_state != LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_RIGHT)) {
            UpdateMainScreenAsLevelSelect_WriteLevelList(level_category->level_sets[*selected_level_set], level_list_start, (*selected_level), (*level_won_array));
         }

         // Mark the currently selected level as the 'old selected level'.
         old_selected_level = (*selected_level);
      }

      // If the level shown on top has changed:
      if (level_shown_on_top != old_level_shown_on_top) {
         // Inform the sub screen that it should fade out, everything.
         level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_OUT;
         UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);

         // Mark the current level on top as the 'old level on top'.
         old_level_shown_on_top = level_shown_on_top;
      }

      // If the level list is currently being shifted down:
      if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_DOWN) {
         // Advance the level list frame (range 0-3)
         level_select_level_list_frame++;

         // Calculate the new vertical position of the level list background (background two)
         BG2_Y0 = -40 + ((4 - level_select_level_list_frame) * 4);

         // When the frame hits four:
         if (level_select_level_list_frame == 4) {
            // Place the level list at its standard position and set state to steady.
            BG2_Y0 = -40;
            level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_STEADY;
         }
      } else
      // If the level list is currently being shifted up:
      if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_LEVELS_UP) {
         // Advance the level list frame (range 0-3)
         level_select_level_list_frame++;

         // Calculate the new vertical position of the level list background (background two)
         BG2_Y0 = -40 + ((level_select_level_list_frame) * 4);

         // When the frame hits four:
         if (level_select_level_list_frame == 4) {
            // Place the level list at its standard position and set state to steady.
            BG2_Y0 = -40;
            level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_STEADY;

            // Increment the level list start (because moving up means that the top line
            // 'disappears')
            level_list_start++;
            // Redraw the level list.
            UpdateMainScreenAsLevelSelect_WriteLevelList(level_category->level_sets[*selected_level_set], level_list_start, (*selected_level), (*level_won_array));
         }
      } else
      // If the level list is currently being difficulty shifted left:
      if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_LEFT) {
         // Advance the difficulty shift frame (range 0-15)
         level_select_level_list_frame++;

         // This variable will store the blend value calculated from the current level list frame:
         u16 blend_value = 0;

         // Frames 0 - 7 are fading out the old background:
         if (level_select_level_list_frame < 8) {
            // Fade from 0 to 14.
            blend_value = level_select_level_list_frame << 1;
         } else
         // Frame 8 is the frame where the level list will change
         // from the old difficulty to the new
         if (level_select_level_list_frame == 8) {
            // Fully fade:
            blend_value = 16;
            // Rewrite the level list using the new difficulty.
            UpdateMainScreenAsLevelSelect_WriteLevelList(level_category->level_sets[*selected_level_set], level_list_start, (*selected_level), (*level_won_array));
         } else
         // Frames 9 - 15 are fading in the new background.
         if (level_select_level_list_frame < 16) {
            // Fade from 16 to 2
            blend_value = 16 - ((level_select_level_list_frame - 8) << 1);
         } else
         // Frame 16 will set the state back to normal, and disable the fade.
         if (level_select_level_list_frame == 16) {
            // Don't blend:
            blend_value = 0;
            // Disable all blends
            BLEND_CR = 0;
            // Set state to steady,
            level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_STEADY;
            // Set the current background index to the 'next' value.
            background_palette_index_current = background_palette_index_next;
         }

         // Based on the current fade frame, calculate a background palette which is
         // (level_select_level_list_frame/16) between the old and new palettes.
         for (int pe = 0; pe < 8; pe++) {
            BG_PALETTE[pe] = ColourThatsXoutofYbetweenAandB(menugfx_background_texturePal[pe + background_palette_index_current],
                                                            menugfx_background_texturePal[pe + background_palette_index_next],
                                                            level_select_level_list_frame,
                                                            16);
         }

         // Set the blend value based on blend_value.
         BLEND_AB = ((blend_value) << 8) | (16 - blend_value);
      } else
      // If the level list is currently being difficulty shifted right:
      if (level_select_level_list_state == LEVEL_SELECT_LEVEL_LIST_STATE_SHIFT_DIFFICULTY_RIGHT) {
         // Advance the difficulty shift frame (range 0-15)
         level_select_level_list_frame++;

         // This variable will store the blend value calculated from the current level list frame:
         u16 blend_value = 0;

         // Frames 0 - 7 are fading out the old background:
         if (level_select_level_list_frame < 8) {
            // Fade from 0 to 14.
            blend_value = level_select_level_list_frame << 1;
         } else
         // Frame 8 is the frame where the level list will change
         // from the old difficulty to the new
         if (level_select_level_list_frame == 8) {
            // Fully fade:
            blend_value = 16;
            // Rewrite the level list using the new difficulty.
            UpdateMainScreenAsLevelSelect_WriteLevelList(level_category->level_sets[*selected_level_set], level_list_start, (*selected_level), (*level_won_array));
         } else
         // Frames 9 - 15 are fading in the new background.
         if (level_select_level_list_frame < 16) {
            // Fade from 16 to 2
            blend_value = 16 - ((level_select_level_list_frame - 8) << 1);
         } else
         // Frame 16 will set the state back to normal, and disable the fade.
         if (level_select_level_list_frame == 16) {
            // Don't blend:
            blend_value = 0;
            // Disable all blends
            BLEND_CR = 0;
            // Set state to steady,
            level_select_level_list_state = LEVEL_SELECT_LEVEL_LIST_STATE_STEADY;
            // Set the current background index to the 'next' value.
            background_palette_index_current = background_palette_index_next;
         }

         // Based on the current fade frame, calculate a background palette which is
         // (level_select_level_list_frame/16) between the old and new palettes.
         for (int pe = 0; pe < 8; pe++) {
            BG_PALETTE[pe] = ColourThatsXoutofYbetweenAandB(menugfx_background_texturePal[pe + background_palette_index_current],
                                                            menugfx_background_texturePal[pe + background_palette_index_next],
                                                            level_select_level_list_frame,
                                                            16);
         }

         // Set the blend value based on blend_value.
         BLEND_AB = ((blend_value) << 8) | (16 - blend_value);
      }

      // Handle the subscreen fade in state:
      if (level_select_subscreen_state == LEVEL_SELECT_SUBSCREEN_STATE_START_FADE_IN) {
         // When the frame hits zero: set the state to steady and disable all blends.
         if (level_select_subscreen_frame == 0) {
            UpdateSubScreenAsLevelInfo_SetFadeValue(0);
            level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_STEADY;
            UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);
         } else {
            // Decrement the frame and blend the subscreen by this value >> 1.
            level_select_subscreen_frame--;
            UpdateSubScreenAsLevelInfo_SetFadeValue(level_select_subscreen_frame >> 1);
         }
      } else
      // Handle the subscreen fade out state:
      if (level_select_subscreen_state == LEVEL_SELECT_SUBSCREEN_STATE_END_FADE_OUT) {
         // When the frame hits 32: set the state to steady and fully blend.
         if (level_select_subscreen_frame == 32) {
            level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_STEADY;
            UpdateSubScreenAsLevelInfo_SetFadeValue(16);
         } else {
            // Increment the frame and blend the subscreen by this value >> 1
            level_select_subscreen_frame++;
            UpdateSubScreenAsLevelInfo_SetFadeValue(level_select_subscreen_frame >> 1);
         }
      } else
      // Handle the subscreen trans fade out state:
      if (level_select_subscreen_state == LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_OUT) {
         // When the frame hits 8:
         if (level_select_subscreen_frame == 8) {
            // Set the state to fade in
            level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_IN;

            VitalStatistics_ExtractLemmingsLevelVitalStatisticsAndMapPreviewFromLevelFilename(level_vital_statistics, level_category->level_sets[*selected_level_set]->levels[*selected_level]->location);

            UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData();
            UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(level_preview_palette);

            UpdateSubScreenAsLevelInfo_WriteLevelInfo(level_vital_statistics, ((*level_won_array)[(*selected_level)] != 0));
         } else {
            // Increment the frame and blend the subscreen by this value.
            level_select_subscreen_frame++;
            UpdateSubScreenAsLevelInfo_SetFadeValue(level_select_subscreen_frame << 1);
         }
      } else
      // Handle the subscreen trans fade in state:
      if (level_select_subscreen_state == LEVEL_SELECT_SUBSCREEN_STATE_TRANS_FADE_IN) {
         // When the frame hits zero: set the state to steady
         if (level_select_subscreen_frame == 0) {
            level_select_subscreen_state = LEVEL_SELECT_SUBSCREEN_STATE_STEADY;
         } else {
            // Decrement the frame and blend the subscreen by this value.
            level_select_subscreen_frame--;
            UpdateSubScreenAsLevelInfo_SetFadeValue(level_select_subscreen_frame << 1);
         }
      }

      // Increment the level select controls text frame.
      level_select_controls_text_frame++;

      // After the controls text steady period:
      if (level_select_controls_text_frame > LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH) {
         // For the first sixteen frames after the steady period, move the background one upwards. (up to 15)
         if (level_select_controls_text_frame < (LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH + 16)) {
            BG1_Y0 = level_select_controls_text_frame - LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH;
         } else
         // When the frame hits 16 frames after the steady period.
         if (level_select_controls_text_frame == (LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH + 16)) {
            // Advance the current controls text line and wrap it around the maximum value.
            level_select_controls_text_current_line = Modulo(++level_select_controls_text_current_line, GLOBAL_GAMEPHRASE_LEVELSELECTCONTROLSTEXT_LINES);

            // Move background one away from view.
            BG1_Y0 = 16;

            // Depending on the number of letters in the line of controls text to be rendered
            // move the background one to the left by four to center the text.
         	if (strlen(global_gamephrase_levelselectcontrolstext[level_select_controls_text_current_line]) & 1) {
         	   // Move background one to the left by 4.
               BG1_X0 = 4;
            } else {
         	   // Move background one back to normal position.
               BG1_X0 = 0;
            }

            // Erase the text at the controls text position.
            WriteText(                                                         global_gamephrase_blank_line,
                                                                                      SCREEN_BASE_BLOCK(13),
                                                                                                          0,
                                                                                                         21,
                                                                                                          0,
                                           UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);
            // Write the current line of the controls text.
            WriteTextCen(global_gamephrase_levelselectcontrolstext[level_select_controls_text_current_line],
                                                                                      SCREEN_BASE_BLOCK(13),
                                                                                                         21,
                                                                                                         10,
                                           UPDATE_MAIN_SCREEN_AS_LEVEL_SELECT_LEMMINGS_FONT_TILE_DATA_INDEX);
         } else
         // For the 16-31 frames after the steady period:
         if (level_select_controls_text_frame < (LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH + 32)) {
            // Move the text up from underneath: (values -16 to -1)
            BG1_Y0 = (level_select_controls_text_frame - (LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH + 16)) - 16;
         }
         // For the 32nd frame after the steady period.
         if (level_select_controls_text_frame == (LEVEL_SELECT_CONTROLS_TEXT_STEADY_LENGTH + 32)) {
            // Place the text at its normal position.
            BG1_Y0 = 0;

            // Reset frame counter (putting the text back into its steady position)
            level_select_controls_text_frame = 0;
         }
      }

      // Handle the level select screen main state 'fade in':
      if (level_select_screen_state == LEVEL_SELECT_SCREEN_STATE_FADE_IN) {
         // Until the fade frame reaches zero, decrement it.
         if (main_screen_fade_frame == 0) {
            // When it hits zero set the state to steady.
            level_select_screen_state = LEVEL_SELECT_SCREEN_STATE_STEADY;
         } else {
            main_screen_fade_frame--;
         }
      // Handle the level select screen main state 'fade out':
      } else if (level_select_screen_state == LEVEL_SELECT_SCREEN_STATE_FADE_OUT) {
         // On the first frame, set the blend registers to blend everything on the main screen.
         if (main_screen_fade_frame == 0) {
            BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                                        | BLEND_SRC_BG1
                                        | BLEND_SRC_BG2
                                        | BLEND_SRC_BG3
                                        | BLEND_SRC_SPRITE
                                        | BLEND_SRC_BACKDROP;
         }
         // When the fade frame reaches 32, inform the main loop that it should terminate:
         if (main_screen_fade_frame == 32) {
            level_select_screen_exit = 1;
         } else {
            // Until the fade frame reaches 32, increment it.
            main_screen_fade_frame++;
         }
      }

      // If the main state is anything other than steady:
      if (level_select_screen_state != LEVEL_SELECT_SCREEN_STATE_STEADY) {
         // Use the main screen fade frame to blend the screen to and from black.
         BLEND_Y = main_screen_fade_frame >> 1;
      }

      // Manage background music: fadeout when fading to a level info screen.
      if ((level_select_screen_state  == LEVEL_SELECT_SCREEN_STATE_FADE_OUT                )
       && (level_select_screen_result == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL)) {
         // Only set the volume on every fourth frame, otherwise, weird clickety pops.
         if ((main_screen_fade_frame & 3) == 0) {
            Cond_CommandMRD_SetPlayerVolume((32 - main_screen_fade_frame) << 1);
         }
      }

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }

   // Reset all sprites and VRAM information on the main screen.
   initSprites(sprite_shadow_m, 128, 0);
   updateOAM(OAM    , sprite_shadow_m);
   BlankVRAMPagesAB();

   // Reset the main screen blend registers.
   BLEND_CR = 0;
   BLEND_Y = 0;

   // Reset the main screen window definitions and registers.
   WIN0X = 0;
   WIN0Y = 0;
   WIN1X = 0;
   WIN1Y = 0;
   DISPLAY_CR &= ~DISPLAY_WIN0_ON;
   DISPLAY_CR &= ~DISPLAY_WIN1_ON;
   DISPLAY_CR &= ~DISPLAY_SPR_WIN_ON;

   // Wait, and low power until VBlank.
   swiWaitForVBlank();

   // Return the result.
   return level_select_screen_result;
}

// These constants define the possible values of the 'past_shown' parameter to the GoLevelInfoGetReady function
#define PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_LEVEL_SELECT  0 // We're heading into the get ready screen from the level select.
                                                                            // (We've chosen a level and now we're gonna play it.)
#define PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_STATUS_SCREEN 1 // We're heading into the get ready screen from the status screen.
                                                                            // (We're either playing a new level after a successful
                                                                            //  level completion, or we're replaying an older level.
                                                                            //  This is determined by the next_level_is_new parameter.)                                                             //

// These are the possible return values from the GoLevelInfoGetReady function:
#define PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_PLAY_LEVEL      0 // Let's play the level!
#define PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_TO_LEVEL_SELECT 1 // I'm having second thoughts... back to level select.

// This will display and run the Get Ready screen that appears before every level.
int PlayLemmingsDS_GoLevelInfoGetReady(const LEMMINGS_LEVEL_VITAL_STATISTICS *level_vital_statistics, int past_shown, bool next_level_is_new) {
   // Based on the last shown parameter, set the sub screen fade objects to different things:
   // If the past shown is the status screen:
   if (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_STATUS_SCREEN) {
      // If this level is new, then:
      if (next_level_is_new) {
         // We need to redisplay the level info and regenerate the map.
         // Let's fade everything.
         UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);
      } else {
         // The level info remains valid, but the map must be regenerated.
         UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_MAP);

         // We need to regenerate the map! Refresh the sub screen by updating
         // the minimap using the preview data, and copying the map's palette.
         UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData();
         UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(current_level_runtime_stats.level_palette);
      }
   } else if (past_shown == PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_LEVEL_SELECT) {
      // If we're fading from the level select, everything should be valid.
      // We don't have to fade anything.
      UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_NOTHING);
   }

   // Set the blend value register for the sub screen to 16 (fully blend to black)
   UpdateSubScreenAsLevelInfo_SetFadeValue(16);

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);

   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);
                             
   // Main screen, turn on all the backgrounds.
   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Unused.
                          | DISPLAY_BG1_ACTIVE   // Alternate text backdrop. (For centering)
                          | DISPLAY_BG2_ACTIVE   // Level info.
                          | DISPLAY_BG3_ACTIVE); // Background texture.

   // Fade everything on the main screen to fully black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Reset everything on the main screen and wait for a single frame.
   BlankVRAMPagesAB();
   ResetBackgroundTranslationRegistersMain();
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
	BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06007000) Priority 3
	BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06006800) Priority 3
	BG1_X0 = 4;
	// Move background one to the left by 4.


	BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   }

	// Copy the eight palette entries for the blue texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe];
   }

   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the main screen during the get ready sequence.
#define GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX 243

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX))[d]
                     = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   // Write the level name to the main screen using red text.
   WriteText(           (const char *)level_vital_statistics->level_name,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       1,
                                                                       1,
                                                                      11,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'number of lemmings' caption.
   WriteText(                    global_gamephrase_levelinfosubscreen[0],
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6,
                                                                       6,
                                                                      10,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the number of lemmings into number_of_lemmings_text as a null-terminated string.
   char number_of_lemmings_text[4];
   siprintf(number_of_lemmings_text, "%d", level_vital_statistics->lemmings);

   // Write the number of lemmings to the main screen after the number of lemmings caption.
   WriteText(                                    number_of_lemmings_text,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6 + 19,
                                                                       6,
                                                                      10,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Work out the number of lemmings to be saved, as a percentage.
   int to_be_saved_number = IntDivR(level_vital_statistics->to_be_saved * 100,
                                    level_vital_statistics->lemmings);
   // We need to get the actual value of the number to display.

   // Write the percentage into to_be_saved_text as a null-terminated string.
   char to_be_saved_text[4];
   siprintf(to_be_saved_text, "%d", to_be_saved_number);

   // Write the percentage to the main screen.
   WriteText(                                           to_be_saved_text,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6,
                                                                       8,
                                                                      12,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'to be saved' caption to the sub screen after the percentage.
   WriteText(                    global_gamephrase_levelinfosubscreen[1],
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6 + strlen(to_be_saved_text),
                                                                       8,
                                                                      12,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'release rate' caption to the sub screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[2],
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6,
                                                                      10,
                                                                      13,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the release rate into release_rate_text as a null-terminated string.
   char release_rate_text[4];
   siprintf(release_rate_text, "%d", level_vital_statistics->release_rate);


   // Write the release rate to the main screen after the release rate caption.
   WriteText(                                          release_rate_text,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6 + 13,
                                                                      10,
                                                                      13,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'time' caption to the main screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[(level_vital_statistics->time_in_minutes == 1) ? 4 : 3],
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6,
                                                                      12,
                                                                      14,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the time in minutes into time_in_minutes_text as a null-terminated string.
   char time_in_minutes_text[4];
   siprintf(time_in_minutes_text, "%d", level_vital_statistics->time_in_minutes);

   // Write the time to the main screen after the time caption.
   WriteText(                                       time_in_minutes_text,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6 + 8,
                                                                      12,
                                                                      14,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the rating caption to the main screen.
   WriteText(                    global_gamephrase_levelinfosubscreen[5],
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6,
                                                                      14,
                                                                      15,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the rating description caption to the main screen.
   WriteText(                 level_vital_statistics->rating_description,
                                                   SCREEN_BASE_BLOCK(14),
                                                                       6 + 8,
                                                                      14,
                                                                      15,
             GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'Tap screen to continue' prompt followed by the
   // 'Press button for level select' prompt to the main screen
   WriteTextMegaCen(                  global_gamephrase_resultscreen_prompt[0],
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            19,
                                                                            10,
                  GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
   WriteTextMegaCen(                  global_gamephrase_resultscreen_prompt[2],
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            21,
                                                                            10,
                  GET_READY_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // This controls the fade register values for the main screen
   int fade_frame = 32;

   // These are the possible states for the title screen loop:
#define GET_READY_SCREEN_STATE_FADE_IN  0 // The get ready screen is fading in from black.
#define GET_READY_SCREEN_STATE_STEADY   1 // The get ready screen is steady and responding to input.
#define GET_READY_SCREEN_STATE_FADE_OUT 2 // The get ready screen is fading to black.
   int get_ready_screen_state       = GET_READY_SCREEN_STATE_FADE_IN;

   // This holds the value which will be returned by this function when it ends.
   int get_ready_screen_result      = 0;
   // These values are defined above the function.

   // The get ready loop will continue until this value is set to true.
   int get_ready_screen_exit        = 0;

   while (!get_ready_screen_exit) {
      // Update joy and joyp keycodes.
      ScanKeypad();

      // Read new touchscreen pen coordinates if available.
		ScanPenCoords();

		// ------------------------------------------------------------

      // This code block handles user interaction:
      // You can only act when the state is steady.
      if (get_ready_screen_state == GET_READY_SCREEN_STATE_STEADY) {
         // If the player has tapped on the screen:
         if (KeyDown(KEY_TOUCH)) {
            // Set the result to play level, and set the main state to fade out
            get_ready_screen_state  = GET_READY_SCREEN_STATE_FADE_OUT;
            get_ready_screen_result = PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_PLAY_LEVEL;
         }
         // If the player has pressed any button except the shoulder buttons:
         else if (KeyDown(KEY_A)
               || KeyDown(KEY_B)
               || KeyDown(KEY_X)
               || KeyDown(KEY_Y)
               || KeyDown(KEY_LEFT)
               || KeyDown(KEY_RIGHT)
               || KeyDown(KEY_UP)
               || KeyDown(KEY_DOWN)
               || KeyDown(KEY_SELECT)
               || KeyDown(KEY_START)) {
            // Set the result to 'to level select', and set the main state to fade out
            get_ready_screen_state  = GET_READY_SCREEN_STATE_FADE_OUT;
            get_ready_screen_result = PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_TO_LEVEL_SELECT;
         }
      }


      // This deals with the 'fade in' state:
      if (get_ready_screen_state == GET_READY_SCREEN_STATE_FADE_IN) {
         // Keep decrementing the fade_frame until it reaches zero.
         // At which point switch the state to steady.
         if (fade_frame == 0) {
            get_ready_screen_state = GET_READY_SCREEN_STATE_STEADY;
         } else {
            fade_frame--;
            // Set the sub screen blend value register to fade_frame >> 1
            UpdateSubScreenAsLevelInfo_SetFadeValue(fade_frame >> 1);
         }
      }
      // This deals with the 'fade out' state:
      else if (get_ready_screen_state == GET_READY_SCREEN_STATE_FADE_OUT) {
         // Keep incrementing the fade_frame until it reaches 32.
         // At which point tell the loop to exit.
         if (fade_frame == 32) {
            get_ready_screen_exit = 1;
         } else {
            fade_frame++;
         }

         // If we're fading back to the level select, we need to fade out the jukebox:
         if (get_ready_screen_result == PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_TO_LEVEL_SELECT) {
            // Only set the volume on every fourth frame, otherwise, weird clickety pops.
            if ((fade_frame & 3) == 0) {
               Cond_CommandMRD_SetPlayerVolume((32 - fade_frame) << 1);
            }
         }
      }

      // When not in a steady state, use the fade_frame variable to
      // fade the screen to and from black.
      if (get_ready_screen_state != GET_READY_SCREEN_STATE_STEADY) {
         BLEND_Y = fade_frame >> 1;
      }

      // Automatically switch the ingame music tracks using the jukebox.
      Music_HandleIngameSongJukeboxPlayback();

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }

   // Blank all tiles, tegels, etc. on main screen.
   BlankVRAMPagesAB();

   // Set the blend registers to no blend, no value.
   BLEND_CR = 0;
   BLEND_Y = 0;

   // Wait, and low power until VBlank.
   swiWaitForVBlank();

   // Return the result.
   return get_ready_screen_result;
}

// These are the possible return values from the GoStatusScreen function:
#define PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_REPLAY_LEVEL    0 // We didn't win the level, lets try it again.
#define PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_NEXT_LEVEL      1 // We won the level, onto the next.
#define PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_TO_LEVEL_SELECT 2 // Back to level select.
                                                                                                                   // Last level only has return to level select as its option.
int PlayLemmingsDS_GoStatusScreen(const LEMMINGS_LEVEL_VITAL_STATISTICS *level_vital_statistics, const LEVEL_STATUS_STRUCT *level_result, int last_level) {
   // Blank the in-out time strip and the map.
   UpdateSubScreenAsLevelInfo_BlankInOutTime();
   UpdateSubScreenAsLevelInfo_BlankMap();

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);
                    
   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);

   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Unused.
                          | DISPLAY_BG1_ACTIVE   // Alternate text backdrop. (For centering)
                          | DISPLAY_BG2_ACTIVE   // Level info.
                          | DISPLAY_BG3_ACTIVE); // Background texture.

   // Fade everything on the main screen to fully black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Reset everything on the main screen and wait for a single frame.
   BlankVRAMPagesAB();
   ResetBackgroundTranslationRegistersMain();
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
	BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06007000) Priority 3
	BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06006800) Priority 3
	BG1_X0 = 4;
	// Move background one to the left by 4.


	BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   }

	// Copy the eight palette entries for the green texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe];
   }
            
   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the main screen during the status screen sequence.
#define STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX 243

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * 243))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         // Plus one because the first tile has to be transparent.
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   // The caption at the top of the main screen will be
   // different depending on whether the timer expired.
   if (level_result->status_flags & LEVEL_STATUS_FLAG_TIME_UP) {
      // Write the 'Your time is up' caption at the top of the main screen.
      WriteTextMegaCen(                   global_gamephrase_resultscreen_title[1],
                                                            SCREEN_BASE_BLOCK(14),
                                                            SCREEN_BASE_BLOCK(13),
                                                                                1,
                                                                               14,
                      STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
   } else {
      // Write the 'All lemmings accounted for' caption at the top of the main screen.
      WriteTextMegaCen(                   global_gamephrase_resultscreen_title[0],
                                                            SCREEN_BASE_BLOCK(14),
                                                            SCREEN_BASE_BLOCK(13),
                                                                                1,
                                                                               14,
                      STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
   }

   // Write the 'You needed:' caption on the main screen.
   WriteText(               global_gamephrase_resultscreen_youneeded[0],
                                                  SCREEN_BASE_BLOCK(13),
                                                                      9,
                                                                      5,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Work out the number of lemmings to be saved, as a percentage.
   int you_needed_number = IntDivR(level_vital_statistics->to_be_saved * 100,
                                   level_vital_statistics->lemmings);
   // We need to get the actual value of the number to display.

   // Write the percentage into you_needed_text as a null-terminated string.
   char you_needed_text[4];
   siprintf(you_needed_text, "%d", you_needed_number);

   // Write the percentage to the main screen after the 'You needed:' caption.
   WriteText(                                           you_needed_text,
                                                  SCREEN_BASE_BLOCK(13),
                                                                     21,
                                                                      5,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the percentage sign to the main screen after the percentage.
   WriteText(                                                       "%",
                                                  SCREEN_BASE_BLOCK(13),
                                                                     21 + strlen(you_needed_text),
                                                                      5,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
   // There's probably a very good reason why this isn't part of the siprintf call.

   // Write the 'You rescued:' caption on the main screen.
   WriteText(               global_gamephrase_resultscreen_youneeded[1],
                                                  SCREEN_BASE_BLOCK(13),
                                                                      9,
                                                                      7,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Work out the number of lemmings to saved, as a percentage.
   int you_rescued_number = IntDivR(level_result->lemmings_saved * 100,
                                    level_vital_statistics->lemmings);
   // We need to get the actual value of the number to display.

   // Write the percentage into you_rescued_text as a null-terminated string.
   char you_rescued_text[4];
   siprintf(you_rescued_text, "%d", you_rescued_number);

   // Write the percentage to the main screen after the 'You rescued:' caption.
   WriteText(                                         you_rescued_text,
                                                  SCREEN_BASE_BLOCK(13),
                                                                     21,
                                                                      7,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the percentage sign to the main screen after the percentage.
   WriteText(                                                       "%",
                                                  SCREEN_BASE_BLOCK(13),
                                                                     21 + strlen(you_rescued_text),
                                                                      7,
                                                                     15,
           STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // These constants refer to the various performance commentary strings
   // that are shown on the status screen after a level has finished.
#define STATUS_SCREEN_COMMENTARY_0_NONE          0 // You didn't get any lemmings.
#define STATUS_SCREEN_COMMENTARY_1_A_COUPLE      1 // You got slightly more than zero.
#define STATUS_SCREEN_COMMENTARY_2_A_LOT         2 // You got a lot more than zero.
#define STATUS_SCREEN_COMMENTARY_3_ALMOST_GOT_IT 3 // You got slightly less than the right amount.
#define STATUS_SCREEN_COMMENTARY_4_EXACTLY       4 // You got exactly the right amount.
#define STATUS_SCREEN_COMMENTARY_5_A_FEW_MORE    5 // You got slightly more than the right amount.
#define STATUS_SCREEN_COMMENTARY_6_A_LOT_MORE    6 // You got a lot more than the right amount.
#define STATUS_SCREEN_COMMENTARY_7_ALL_OF_THEM   7 // You got every lemming.

   // This will store the commentary value
   int   status_screen_commentary;

   if (level_result->lemmings_saved == 0) {
      // You got NO LEMMINGS.
      status_screen_commentary = STATUS_SCREEN_COMMENTARY_0_NONE;
   } else if (level_result->lemmings_saved == level_vital_statistics->to_be_saved) {
      // You got EXACTLY THE RIGHT AMOUNT.
      status_screen_commentary = STATUS_SCREEN_COMMENTARY_4_EXACTLY;
   } else if (level_result->lemmings_saved == level_vital_statistics->lemmings) {
      // You got ALL OF THE LEMMINGS.
      status_screen_commentary = STATUS_SCREEN_COMMENTARY_7_ALL_OF_THEM;
   } else if (level_result->lemmings_saved < level_vital_statistics->to_be_saved) {
      // You got less than the amount you needed...

      if (you_rescued_number >= (you_needed_number - 10)) {
         // You got WITHIN TEN LEMMINGS OF THE TARGET
         status_screen_commentary = STATUS_SCREEN_COMMENTARY_3_ALMOST_GOT_IT;
      } else if (you_rescued_number >= (you_needed_number - 30)) {
         // You got WITHIN THIRTY LEMMINGS OF THE TARGET
         status_screen_commentary = STATUS_SCREEN_COMMENTARY_2_A_LOT;
      } else {
         // You DIDN'T GET WITHIN THIRTY LEMMINGS OF THE TARGET
         status_screen_commentary = STATUS_SCREEN_COMMENTARY_1_A_COUPLE;
      }
   } else {
      if (you_rescued_number <= (you_needed_number + 10)) {
         // You got WITHIN TEN MORE THAN YOU NEEDED
         status_screen_commentary = STATUS_SCREEN_COMMENTARY_5_A_FEW_MORE;
      } else {
         // You got MORE THAN TEN MORE THAN YOU NEEDED.
         status_screen_commentary = STATUS_SCREEN_COMMENTARY_6_A_LOT_MORE;
      }
   }

   // Did you win?
   bool  level_won = ((level_result->status_flags & LEVEL_STATUS_FLAG_LEVEL_WON) != 0);

   // Write the appropriate two lines of the commentary to the main screen
   // using the special centering function.
   WriteTextMegaCen(              global_gamephrase_resultscreen_commentary[2 * status_screen_commentary],
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            11,
                                                                            11,
                  STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
   WriteTextMegaCen(              global_gamephrase_resultscreen_commentary[2 * status_screen_commentary + 1],
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            13,
                                                                            11,
                  STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Depending on whether or not you won the level, you'll get a different
   // top line prompt at the bottom of the screen.
   const char *top_prompt_string;

   if (level_won &&   last_level ) {
      // You've just won the last level.. there's no further level to win.
      top_prompt_string = global_gamephrase_nullstring;
      // We might put an ending sequence thing here.
   } else
   if (level_won && (!last_level)) {
      // You won, but it wasn't the last level.
      // Invite the player to continue playing by tapping screen for next level.
      top_prompt_string = global_gamephrase_resultscreen_prompt[0];
   } else
   if (!level_won) {
      // You didn't win.
      // Invite the player to continue playing the current level by tapping screen.
      top_prompt_string = global_gamephrase_resultscreen_prompt[1];
   }

   // Write the two prompt strings at the bottom of the screen.
   // The first will vary based on the above criteria.
   WriteTextMegaCen(                                         top_prompt_string,
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            19,
                                                                            10,
                  STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // Write the 'Press any button for level select' prompt to the bottom of the main screen.
   WriteTextMegaCen(                  global_gamephrase_resultscreen_prompt[2],
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                            21,
                                                                            10,
                  STATUS_SCREEN_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

   // This controls the fade register values for the main screen
   int fade_frame = 32;

   // These are the possible states for the status screen loop:
#define STATUS_SCREEN_STATE_FADE_IN  0 // The status screen is fading in from black.
#define STATUS_SCREEN_STATE_STEADY   1 // The status screen is steady and responding to input.
#define STATUS_SCREEN_STATE_FADE_OUT 2 // The status screen is fading to black.
   int status_screen_state       = STATUS_SCREEN_STATE_FADE_IN;

   // This holds the value which will be returned by this function when it ends.
   int status_screen_result      = 0;
   // These values are defined above the function.

   // The status screen loop will continue until this value is set to true.
   int status_screen_exit        = 0;

   while (!status_screen_exit) {
      // Update joy and joyp keycodes.
      ScanKeypad();

      // Read new touchscreen pen coordinates if available.
		ScanPenCoords();

		// ------------------------------------------------------------

      // This code block handles user interaction:
      // You can only act when the state is steady.
      if (status_screen_state == STATUS_SCREEN_STATE_STEADY) {
         // If the player has tapped on the screen, and is able to do so:
         if (KeyDown(KEY_TOUCH) && (!(level_won && last_level))) {
            // Set the state to fade to black.
            status_screen_state = STATUS_SCREEN_STATE_FADE_OUT;

            // If the player has won the level, set the result to go to the next level.
            if (level_won) {
               status_screen_result = PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_NEXT_LEVEL;
            } else {
               // Else, set the result to repeat the current level.
               status_screen_result = PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_REPLAY_LEVEL;
            }

            // Tell the subscreen to fade everything, but set the blend register to zero blend value.
            UpdateSubScreenAsLevelInfo_SetFadeObjects(UPDATE_SUB_SCREEN_AS_LEVEL_INFO_SET_FADE_OBJECTS_OBJECT_EVERYTHING);
            UpdateSubScreenAsLevelInfo_SetFadeValue(0);
         }
         // If the player has pressed any button except the shoulder buttons:
         else if (KeyDown(KEY_A)
               || KeyDown(KEY_B)
               || KeyDown(KEY_X)
               || KeyDown(KEY_Y)
               || KeyDown(KEY_LEFT)
               || KeyDown(KEY_RIGHT)
               || KeyDown(KEY_UP)
               || KeyDown(KEY_DOWN)
               || KeyDown(KEY_SELECT)
               || KeyDown(KEY_START)) {
            // Set the result to 'to level select', and set the main state to fade out
            status_screen_state  = STATUS_SCREEN_STATE_FADE_OUT;
            status_screen_result = PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_TO_LEVEL_SELECT;
         }
      }

      // This deals with the 'fade in' state:
      if (status_screen_state == STATUS_SCREEN_STATE_FADE_IN) {
         // Keep decrementing the fade_frame until it reaches zero.
         // At which point switch the state to steady.
         if (fade_frame == 0) {
            status_screen_state = STATUS_SCREEN_STATE_STEADY;
         } else {
            fade_frame--;
         }
      }
      // This deals with the 'fade out' state:
      else if (status_screen_state == STATUS_SCREEN_STATE_FADE_OUT) {
         // Keep incrementing the fade_frame until it reaches 32.
         // At which point tell the loop to exit.
         if (fade_frame == 32) {
            status_screen_exit = 1;
         } else {
            fade_frame++;
            // If we're transitioning to the next level, fade out the sub screen by:
            if (status_screen_result == PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_NEXT_LEVEL) {
               // Setting the sub screen blend value register to fade_frame >> 1
               UpdateSubScreenAsLevelInfo_SetFadeValue(fade_frame >> 1);
            }
         }

         // If we're fading back to the level select, we need to fade out the jukebox:
         if (status_screen_result == PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_TO_LEVEL_SELECT) {
            // Only set the volume on every fourth frame, otherwise, weird clickety pops.
            if ((fade_frame & 3) == 0) {
               Cond_CommandMRD_SetPlayerVolume((32 - fade_frame) << 1);
            }
         }
      }

      // When not in a steady state, use the fade_frame variable to
      // fade the screen to and from black.
      if (status_screen_state != STATUS_SCREEN_STATE_STEADY) {
         BLEND_Y = fade_frame >> 1;
      }

      // Automatically switch the ingame music tracks using the jukebox.
      Music_HandleIngameSongJukeboxPlayback();

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }

   // Blank all tiles, tegels, etc. on main screen.
   BlankVRAMPagesAB();

   // Set the blend registers to no blend, no value.
   BLEND_CR = 0;
   BLEND_Y = 0;

   // Wait, and low power until VBlank.
   swiWaitForVBlank();

   // Return the result.
   return status_screen_result;
}

// This halts execution and displays an unexitable error screen.
// Title string is displayed at the top of the screen, and error_string_*
// will be displayed underneath.
void LemmingsDS_UnrecoverableError(const char *title_string, const char *error_string_1,
                                                             const char *error_string_2,
                                                             const char *error_string_3,
                                                             const char *error_string_4,
                                                             const char *error_string_5,
                                                             const char *error_string_6,
                                                             const char *error_string_7,
                                                             const char *error_string_8,
                                                             const char *error_string_9) {
   const char *error_array[10] = {error_string_1,
                                  error_string_2,
                                  error_string_3,
                                  error_string_4,
                                  error_string_5,
                                  error_string_6,
                                  error_string_7,
                                  error_string_8,
                                  error_string_9,
                                  NULL};

   // Count the number of error strings.
   int no_error_strings = 0;
   while (error_array[no_error_strings] != NULL) {
      no_error_strings++;
   }

   // Blank the top screen
   videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); // Single background
   BlankAllSubPalettes();
   
   // Set up the main screen for tiled text display:

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);
                    
   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);

   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Unused.
                          | DISPLAY_BG1_ACTIVE   // Alternate text backdrop. (For centering)
                          | DISPLAY_BG2_ACTIVE   // Level info.
                          | DISPLAY_BG3_ACTIVE); // Background texture.

   // Fade everything on the main screen to fully black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Reset everything on the main screen and wait for a single frame.
   BlankVRAMPagesAB();
   ResetBackgroundTranslationRegistersMain();
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
	BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06007000) Priority 3
	BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06006800) Priority 3
	BG1_X0 = 4;
	// Move background one to the left by 4.


	BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   }

	// Copy the eight palette entries for the green texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe];
   }
            
   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the main screen during the unrecoverable error screen sequence.
#define UNRECOVERABLE_ERROR_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX 243

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * 243))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         // Plus one because the first tile has to be transparent.
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   WriteTextMegaCen(                                              title_string,
                                                         SCREEN_BASE_BLOCK(14),
                                                         SCREEN_BASE_BLOCK(13),
                                                                             1,
                                                                            14,
                   UNRECOVERABLE_ERROR_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);
                   
   int error_string_write_y = 13 - no_error_strings;

   // Write the error strings:
   for (int error_string = 0; error_string < no_error_strings; error_string++) {
      WriteTextMegaCen(                                 error_array[error_string],
                                                            SCREEN_BASE_BLOCK(14),
                                                            SCREEN_BASE_BLOCK(13),
                                                             error_string_write_y,
                                                                               10,
                      UNRECOVERABLE_ERROR_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

      error_string_write_y += 2;
   }

   // This controls the fade register values for the main screen
   int fade_frame = 32;

   // These are the possible states for the unrecoverable error screen loop:
#define UNRECOVERABLE_ERROR_STATE_FADE_IN  0 // The unrecoverable error screen is fading in from black.
#define UNRECOVERABLE_ERROR_STATE_STEADY   1 // The unrecoverable error screen is steady and responding to input.
//#define UNRECOVERABLE_ERROR_STATE_FADE_OUT 2 // The unrecoverable error screen cannot fade to black!
   int unrecoverable_error_screen_state       = UNRECOVERABLE_ERROR_STATE_FADE_IN;

   while (1) {
      // This deals with the 'fade in' state:
      if (unrecoverable_error_screen_state == UNRECOVERABLE_ERROR_STATE_FADE_IN) {
         // Keep decrementing the fade_frame until it reaches zero.
         // At which point switch the state to steady.
         if (fade_frame == 0) {
            unrecoverable_error_screen_state = UNRECOVERABLE_ERROR_STATE_STEADY;
         } else {
            fade_frame--;
         }
      }

      // When not in a steady state, use the fade_frame variable to
      // fade the screen to and from black.
      if (unrecoverable_error_screen_state != UNRECOVERABLE_ERROR_STATE_STEADY) {
         BLEND_Y = fade_frame >> 1;
      }

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }
}

void LemmingsDS_UnrecoverableError(unsigned int error, const char *parameter) {
   switch (error) {
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_UNSPECIFIED_ERROR) : {
         LemmingsDS_UnrecoverableError("Error",

                                       "Unspecified error");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_MISSING_CONFIG_FILE) : {
         LemmingsDS_UnrecoverableError("Error loading config file",

                                       "The Lemmings DS config file",
                                       "could not be found",
                                       " ",
                                       "Please check that the file",
                                       "`lemmingsds_config.txt` exists",
                                       "in the root of your flash media",
                                       "and is not damaged, then reboot");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_CONFIG_FILE) : {
         LemmingsDS_UnrecoverableError("Error loading config file",

                                       "The config file was loaded",
                                       "correctly, but there was an",
                                       "error retrieving its data",
                                       " ",
                                       "Please check that the file",
                                       "lemmingsds_config.txt exists",
                                       "in the root of your flash media",
                                       "and is not damaged, then reboot");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_DIRECTORIES) : {
         LemmingsDS_UnrecoverableError("Error setting up Lemmings DS",

                                       "The standard Lemmings DS",
                                       "directory structure could",
                                       "not be verified.",
                                       " ",
                                       "Please check that the standard",
                                       "Lemmings DS directories are",
                                       "present on your flash media,",
                                       "then reboot");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_RETRIEVE_VITAL_STATISTICS) : {
         LemmingsDS_UnrecoverableError("Error loading level info",

                                       "The level information for the",
                                       "level with filename ending in",
                                       " ",
                                       parameter,
                                       " ",
                                       "may be damaged",
                                       " ",
                                       "Please verify that the file is",
                                       "not damaged, then reboot");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_CHAR_PTR_ARRAY) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "music category char ptr array");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_MUSIC_FILENAME_STRING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "music filename string");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_XM_SONG) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "loading music module");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_CATEGORY_LOCATION_STRING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level category location string");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE_PTR_ARRAY) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level set handle ptr array");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_HANDLE) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level set handle instance");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_SET_NAME_STRING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level set name string");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE_PTR_ARRAY) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level handle ptr array");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_HANDLE) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level handle instance");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOCATION_STRING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level location string");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_NAME_STRING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level name string");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LOADING_GRAPHICAL_OBJECT_FILE) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "loading graphical object file");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_INGAME_STATUS_STRUCT) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "ingame status structure");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOADING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level file loading");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_TEXTURE_ARCHIVE_LOADING) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "texture archive loading");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_DATA_ARRAY) : {
         LemmingsDS_UnrecoverableError("Memory request error",

                                       "Unable to continue due",
                                       "to lack of memory",
                                       " ",

                                       "Could not allocate memory for",
                                       "level data array");
      } break;
      case (LEMMINGS_DS_UNRECOVERABLE_ERROR_THERE_ARENT_ANY_LEVELS) : {
         LemmingsDS_UnrecoverableError("Level detection error",

                                       "Lemmings DS appears to be",
                                       "installed correctly...",
                                       " ",
                                       "However, there doesn't appear",
                                       "to be any valid levels in the",
                                       "level directories!"
                                       " ",
                                       "Check that the levels you're",
                                       "using are in the correct place",
                                       "and are the correct version.");
      } break;
   }
}

// This will fade on the specified message centered on the bottom screen
// using the specified colours
// Use LemmingsDS_FadeOutMessage to fade the message out.
void LemmingsDS_FadeInMessage(int string_1_colour, const char *string_1,
                              int string_2_colour, const char *string_2,
                              int string_3_colour, const char *string_3,
                              int string_4_colour, const char *string_4,
                              int string_5_colour, const char *string_5,
                              int string_6_colour, const char *string_6,
                              int string_7_colour, const char *string_7,
                              int string_8_colour, const char *string_8,
                              int string_9_colour, const char *string_9) {
   const char *string_array[10] = {string_1,
                                   string_2,
                                   string_3,
                                   string_4,
                                   string_5,
                                   string_6,
                                   string_7,
                                   string_8,
                                   string_9,
                                   NULL};

   const int string_colour_array[9] = {string_1_colour,
                                       string_2_colour,
                                       string_3_colour,
                                       string_4_colour,
                                       string_5_colour,
                                       string_6_colour,
                                       string_7_colour,
                                       string_8_colour,
                                       string_9_colour};

   // Count the number of error strings.
   int no_strings = 0;
   while (string_array[no_strings] != NULL) {
      no_strings++;
   }

   // Blank the top screen
   videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); // Single background
   BlankAllSubPalettes();
   
   // Set up the main screen for tiled text display:

   // VRAM A is the tile data and tegels.
   // VRAM B is the sprite data.
   // VRAM C is the sub tile data and tegels.
   // VRAM D is the sub sprite data.
   vramSetMainBanks(VRAM_A_MAIN_BG    ,
                    VRAM_B_MAIN_SPRITE,
                    VRAM_C_SUB_BG     ,
                    VRAM_D_SUB_SPRITE);
                    
   // Map banks F and G as LCDC for now
   vramSetBankF(VRAM_F_LCD);
   vramSetBankG(VRAM_G_LCD);

   videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE   // Unused.
                          | DISPLAY_BG1_ACTIVE   // Alternate text backdrop. (For centering)
                          | DISPLAY_BG2_ACTIVE   // Level info.
                          | DISPLAY_BG3_ACTIVE); // Background texture.

   // Fade everything on the main screen to fully black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;
   BLEND_Y = 16;

   // The MAIN screen is now the touch screen.
   lcdMainOnBottom();

   // Reset everything on the main screen and wait for a single frame.
   BlankVRAMPagesAB();
   ResetBackgroundTranslationRegistersMain();
   swiWaitForVBlank();

   // First, we set up the background control registers so they know what to do.
	BG3_CR = BG_COLOR_16 | (15 << 8) | 3;
	// The three background is a 16 colour background using the tegel data from
	// base block 15. (that's 0x06007800) Priority 3
	BG2_CR = BG_COLOR_16 | (14 << 8) | 3;
	// The two background is a 16 colour background using the tegel data from
	// base block 14. (that's 0x06007000) Priority 3
	BG1_CR = BG_COLOR_16 | (13 << 8) | 3;
	// The one background is a 16 colour background using the tegel data from
	// base block 13. (that's 0x06006800) Priority 3
	BG1_X0 = 4;
	// Move background one to the left by 4.


	BG0_CR = BG_COLOR_16 | (12 << 8) | 3;
	// The zero background is a 16 colour background using the tegel data from
	// base block 12. (that's 0x06006000) Priority 3

	// Now we're going to copy the tile data into the VRAM bank so it can be displayed
	for (int d = 0; d < (menugfx_background_textureTilesLen >> 1); d++) {
	   // This refers to the tile number 1. (not zero. that has to be left blank!)
      (BG_GFX+(S_TILE_SIZE_u16 *   1))[d] = menugfx_background_textureTiles[d];
   }

	// Copy the eight palette entries for the green texture to the BG palette.
   for (int pe = 0; pe < 8; pe++) {
      BG_PALETTE[pe] = menugfx_background_texturePal[pe];
   }
            
   // This define holds the tile index of the EXCLAMATION MARK character
   // used on the main screen during the unrecoverable error screen sequence.
#define FADE_ON_MESSAGE_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX 243

	// Now, copy the lemmings font into the VRAM bank.
	for (int d = 0; d < (menugfx_lemmings_fontTilesLen >> 1); d++) {
      (BG_GFX+(S_TILE_SIZE_u16 * 243))[d] = menugfx_lemmings_fontTiles[d];
   }

	// Copy the lemmings font palette entries to the BG palette.
   for (int pe = 0; pe < 88; pe++) {
      BG_PALETTE[160+pe] = menugfx_lemmings_fontPal[pe];
   }

   // Set the background transparent colour to black. (Otherwise we'd have some
   // manic pink showing through from whatever was loaded into the BG_PALETTE last)
   BG_PALETTE[0] = 0;

   // Fill in the tegel entries for the three background.
   // It's a 20x12 tile 16 colour image, filling the main screen.
   for (int tegely = 0; tegely < 24; tegely++) {
      for (int tegelx = 0; tegelx < 32; tegelx++) {
         // Plus one because the first tile has to be transparent.
         (*((vu16*)(SCREEN_BASE_BLOCK(15)) + tegelx + tegely * 32)) = Modulo(tegelx, 20) + Modulo(tegely, 12) * 20 + 1;
         // Plus one because the first tile has to be transparent.
         // These tiles are using 16-colour palette line 0.
      }
   } // That's the background texture.

   int string_write_y = 12 - no_strings;

   // Write the error strings:
   for (int write_string = 0; write_string < no_strings; write_string++) {
      WriteTextMegaCen(                                string_array[write_string],
                                                            SCREEN_BASE_BLOCK(14),
                                                            SCREEN_BASE_BLOCK(13),
                                                                   string_write_y,
                                                string_colour_array[write_string],
                      FADE_ON_MESSAGE_MENU_LEMMINGS_FONT_MAIN_SCREEN_TILE_DATA_INDEX);

      string_write_y += 2;
   }

   // This controls the fade register values for the main screen
   int fade_frame = 32;

   // Keep fading in until the fade_frame hits zero (the fade is complete)
   while (fade_frame > 0) {
		// ------------------------------------------------------------

      // Keep decrementing the fade_frame until it reaches zero.
      fade_frame--;

      // Use the fade_frame variable to fade the screen from black.
      BLEND_Y = fade_frame >> 1;

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }
}

// This function fades out a message placed by LemmingsDS_FadeInMessage
void LemmingsDS_FadeOutMessage() {
   // This controls the fade register values for the main screen
   int fade_frame = 0;

   // Keep fading out until the fade_frame hits 32 (the fade is complete)
   while (fade_frame < 32) {
		// ------------------------------------------------------------

      // Keep incrementing the fade_frame until it reaches 32.
      fade_frame++;

      // Use the fade_frame variable to fade the screen to black.
      BLEND_Y = fade_frame >> 1;

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }
}

// This function fades both screens from white to black
void LemmingsDS_FadeFromWhiteToBlack() {
   // Blank the screens:
   videoSetMode(   MODE_0_2D | DISPLAY_BG0_ACTIVE); // Single background
   videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); // Single background
   BlankAllPalettesToWhite();

   // Fade everything on the screens against black.
   BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;

   SUB_BLEND_CR =
              BLEND_FADE_BLACK | BLEND_SRC_BG0
                               | BLEND_SRC_BG1
                               | BLEND_SRC_BG2
                               | BLEND_SRC_BG3
                               | BLEND_SRC_SPRITE
                               | BLEND_SRC_BACKDROP;

   // This controls the fade register values for the main screen
   int fade_frame = 0;

   // Keep fading out until the fade_frame hits 32 (the fade is complete)
   while (fade_frame < 32) {
		// ------------------------------------------------------------

      // Keep incrementing the fade_frame until it reaches 32.
      fade_frame++;

      // Use the fade_frame variable to fade the screens to black.
          BLEND_Y = fade_frame >> 1;
      SUB_BLEND_Y = fade_frame >> 1;

      // Wait, and low power until VBlank.
      swiWaitForVBlank();
   }
}

void PlayLemmingsDS_PlayLemmingsDSUsingLevelCategory(LEMMINGS_DS_LEVEL_CATEGORY_HANDLE *level_category) {
   // These bools control the flow of the game modules from one to the next
   bool return_to_level_select = false;  // Are we going to level select?
   bool lets_play_next_level   = false;  // Are we going to the next level?

   // These hold the results of the various modules.
   int level_select_result, get_ready_result, status_screen_result;

   // These hold the number of the levels past shown.
   int level_select_past_shown;
   int level_info_get_ready_past_shown;
   
   // Is the next level new to the subscreen, or the prototype
   bool next_level_is_new_to_subscreen = true;
   bool next_level_is_new_to_prototype = true;

   int current_level_set = 0; // Let's go!
   int current_level     = 0;
   
   // This will point to an array of u8s determining whether each level
   // within the currently selected level set are won or not.
   u8 *level_won_array = LemmingsDSSavedProgress_LoadProgressFromLevelSetFile(level_category, level_category->level_sets[current_level_set]);

   // Try to find the next level that has not yet been won.
   // Go through each level set in turn if necessary.
   while (1) {
      if (level_won_array[current_level] == 0) break;
      current_level++;

      // We've searched through all of this level set's levels?
      // Increment it.
      if (current_level == level_category->level_sets[current_level_set]->no_levels) {
         current_level = 0;
         current_level_set++;

         // When incrementing the level set, we need to get a whole new progress array.
         free(level_won_array);
         
         // First, check if we've checked all of the level sets.
         // If we have, then set the level set to zero and break!
         if (current_level_set == level_category->no_level_sets) {
            current_level_set = 0;
         }

         level_won_array = LemmingsDSSavedProgress_LoadProgressFromLevelSetFile(level_category, level_category->level_sets[current_level_set]);

         if (current_level_set == 0) break;
      }
   }

   // This holds the result of playing a single level.
   LEVEL_STATUS_STRUCT level_result;
   // Blank it.
   memset(&level_result, 0, sizeof(LEVEL_STATUS_STRUCT));

   // This is a pointer to a vital statistics struct for the currently selected level.
   LEMMINGS_LEVEL_VITAL_STATISTICS current_vital_statistics;
   // Blank it.
   memset(&current_vital_statistics, 0, sizeof(LEMMINGS_LEVEL_VITAL_STATISTICS));

   // This is a pointer to the level file loaded into memory.
   // This memory should only exist just before a level is to be played.
   LEMMINGS_LEVEL_LDS_FILE_V7 *loaded_level_file;

   // This is a pointer to the terrain archive file loaded into memory.
   // This memory should also only exist just before a level is to be played.
   LEMMINGS_TEXTURE_ARCHIVE_HEADER *loaded_texture_archive_file;

   // Set up the sub screen to show the level info, map and in-out timer strip.
   SetUpSubScreenAsLevelInfo();

   // We last saw the title screen.
   level_select_past_shown = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_TITLE_SCREEN;

   do {
      DebugAppend("Going into PlayLemmingsDS_GoLevelSelect().\r\n");
      DebugWrite();

      // Let's pick a level from this category:
      level_select_result = PlayLemmingsDS_GoLevelSelect(level_category, &current_vital_statistics, &current_level_set, &current_level, &level_won_array, level_select_past_shown);

      DebugAppend("Exiting PlayLemmingsDS_GoLevelSelect().\r\n");
      DebugWrite();

      // We last saw the level select.
      level_info_get_ready_past_shown = PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_LEVEL_SELECT;
      // The next level isn't new to the subscreen, it was shown during the level select sequence.
      next_level_is_new_to_subscreen = false;
      // But you can bet your butt that it hasn't been rendered properly yet.
      next_level_is_new_to_prototype = true;

      // Are we going to play the level?
      if (level_select_result == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_PLAY_LEVEL) {
         // We're going to play the level.
         lets_play_next_level = true;

         // Start up the ingame level music jukebox
         Music_KickIngameSongJukeboxPlayback(level_category->level_sets[current_level_set]->music_category, -1);
      } else if (level_select_result == PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_RESULT_TO_MAIN_MENU) {
         // We're not going to play the level, and we're not returning to level select.
         lets_play_next_level   = false;
         return_to_level_select = false;
      }

      // Keep playing levels based on the current_level variable.
      while (lets_play_next_level) {
         DebugAppend("Restarting lets_play_next_level loop.\r\n");
         DebugWrite();

         // If the next level is new to the prototype, we have to load it and place
         // its vital statistics in the statistics struct.
         // We also have to grab the graphical objects found within:
         if (next_level_is_new_to_prototype) {
            DebugAppend("Next level is new to prototype: loading, rendering and acquiring.\r\n");
            DebugWrite();

            const char *level_location = level_category->level_sets[current_level_set]->levels[current_level]->location;

            FILE *level_file = fopen(level_location, "rb");

            // We can assume that, because the level was signified as valid
            // (as it must have been to be entered into the level category system)
            // that it exists and works.

            // Otherwise Lemmings DS will crash, and you'll look like a wally.
            // (Here's a hint, don't use mangled levels!)

            // Load the filesize from the level
            u32 level_filesize;
            fread(&level_filesize, 4, 1, level_file);

            // Rewind the file
            rewind(level_file);
            
            DEBUG_SECTION {
               char output_text[4096];
               siprintf(output_text, "Reading %d bytes from level file:\r\n", level_filesize);
               DebugAppend(output_text);
               DebugWrite();
            }

            // Allocate memory for the level file
            loaded_level_file = (LEMMINGS_LEVEL_LDS_FILE_V7 *)malloc(level_filesize);
            DEBUG_SECTION {
               if (loaded_level_file == NULL) {
                  DebugAppend("Failed to allocate memory for level loading.\r\n");
                  DebugWrite();

                  // Crash
                  LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_LOADING);
               }
            }

            // Load the file into memory
            fread(loaded_level_file, level_filesize, 1, level_file);

            // Close the level file after level has been loaded.
            fclose(level_file);
            
            DebugAppend("Level loading complete, loading texture archive...\r\n");
            DebugWrite();

            // Construct the texture archive full location string for the loaded level:
            char texture_archive_full_location[256];
            memset(texture_archive_full_location, 0, 256);

            // First, test for the custom texture archive...
            const char *last_slash_in_level_path = strfindlast("/", level_location);
            strncpy(texture_archive_full_location, level_location, 1 + (int)(((int)(last_slash_in_level_path)) - ((int)(level_location))));
            strcat(texture_archive_full_location, "custom_texture_archives/");
            strcat(texture_archive_full_location, loaded_level_file->texture_archive_using);
            strcat(texture_archive_full_location, ".LTA");

            // Attempt to load the custom texture archive.
            FILE *texture_archive_file = fopen(texture_archive_full_location, "rb");

            if (texture_archive_file != NULL) {
               DebugAppend("Constructed custom texture archive path '");
               DebugAppend(texture_archive_full_location);
               DebugAppend("', loading:.\r\n");
               DebugWrite();
            } else {                               
               // Reset errno
               errno = 0;

               DebugAppend("Custom texture archive not accessible, using standard:\r\n");
               DebugWrite();

               memset(texture_archive_full_location, 0, 256);

               strcat(texture_archive_full_location, lemmings_ds_root_dir);
               strcat(texture_archive_full_location, LEMMINGS_DS_DIRECTORY_STANDARD_TEXTURE_ARCHIVES);
               strcat(texture_archive_full_location, loaded_level_file->texture_archive_using);
               strcat(texture_archive_full_location, ".LTA"); 

               DebugAppend("Constructed '");
               DebugAppend(texture_archive_full_location);
               DebugAppend("', loading:.\r\n");
               DebugWrite();   

               // Load the texture archive.
               texture_archive_file = fopen(texture_archive_full_location, "rb");
            }
            
            // Load the filesize from the texture archive
            u32 texture_archive_filesize;
            fread(&texture_archive_filesize, 4, 1, texture_archive_file);

            // Rewind the file
            rewind(texture_archive_file);

            // Allocate memory for the texture archive file
            loaded_texture_archive_file = (LEMMINGS_TEXTURE_ARCHIVE_HEADER *)malloc(texture_archive_filesize);
            DEBUG_SECTION {
               if (loaded_texture_archive_file == NULL) {
                  DebugAppend("Failed to allocate memory for texture archive loading.\r\n");
                  DebugWrite();

                  // Crash
                  LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_TEXTURE_ARCHIVE_LOADING);
               }
            }

            // Read the texture archive into memory
            fread(loaded_texture_archive_file, texture_archive_filesize, 1, texture_archive_file);

            fclose(texture_archive_file);

            DEBUG_SECTION {                                  
               DebugAppend("\r\n");
               DebugAppend("About to render a level!\r\n");
               DebugAppend("------------------------\r\n");
               DebugAppend("Debugging statistics:\r\n");
               DebugWrite();

               char output_text[4096];
               siprintf(output_text, "Level ##'s: Set %d -> No. %d of %d. (At %s)\r\n"
                                     "Level name: %s\r\n"
                                     "Level size: %d\r\n", current_level_set,
                                                           current_level,
                                                           level_category->level_sets[current_level_set]->no_levels,
                                                           level_category->level_sets[current_level_set]->levels[current_level]->location,
                                                           current_vital_statistics.level_name,
                                                           loaded_level_file->lemmings_level_file_size);
               DebugAppend(output_text);
               DebugWrite();

               siprintf(output_text, "Texture archive loaded: %s\r\n"
                                     "Texture archive size: %d\r\n"
                                     "Texture archive contents: %d\r\n", texture_archive_full_location,
                                                                         loaded_texture_archive_file->texture_archive_file_size,
                                                                         loaded_texture_archive_file->no_texture_16s + loaded_texture_archive_file->no_texture_256s);

               DebugAppend(output_text);
               DebugWrite();
               
               siprintf(output_text, "Entrances, exits, traps, hazards, uninteractives, waters: %d, %d, %d, %d, %d, %d.\r\n",
                                                     loaded_level_file->runtime_stats.no_entrances,
                                                     loaded_level_file->runtime_stats.no_exits,
                                                     loaded_level_file->runtime_stats.no_traps,
                                                     loaded_level_file->runtime_stats.no_hazards,
                                                     loaded_level_file->runtime_stats.no_uninteractives,
                                                     loaded_level_file->runtime_stats.no_waters);

               DebugAppend(output_text);
               DebugWrite();
            }

            // Render the level into the prototype
            RenderLevel(level_data_prototype, loaded_level_file, loaded_texture_archive_file);

            // Copy the runtime stats from the loaded level into the runtime struct
            memcpy(&current_level_runtime_stats, &(loaded_level_file->runtime_stats), sizeof(LEMMINGS_LEVEL_RUNTIME_STATS_V7));

            // We need to set the alpha bit on the colours read from the file:
            for (int pe = 0; pe < 16; pe++) {
               current_level_runtime_stats. entrance_palette[pe] |= 0x8000;
               current_level_runtime_stats.     exit_palette[pe] |= 0x8000;
               for (int e = 0; e < NO_TRAP_GENUSES; e++) {
                  current_level_runtime_stats.trap_genus_palettes[e][pe] |= 0x8000;
               }
               for (int e = 0; e < NO_HAZARD_GENUSES; e++) {
                  current_level_runtime_stats.hazard_genus_palettes[e][pe] |= 0x8000;
               }
               for (int e = 0; e < NO_UNINTERACTIVE_GENUSES; e++) {
                  current_level_runtime_stats.uninteractive_genus_palettes[e][pe] |= 0x8000;
               }
               current_level_runtime_stats.    water_palette[pe] |= 0x8000;
            }

            DEBUG_SECTION {
               char output_text[4096];

               // Let's dump the current values of the entrance pallete contained within the
               // runtime stats struct to the debug file.
               DebugAppend("\r\nDumping the entrance palette (current runtime stats).\r\n");
               siprintf(output_text, "%04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X.\r\n", current_level_runtime_stats.entrance_palette[0x0],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x1],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x2],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x3],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x4],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x5],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x6],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x7],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x8],
                                                                                                                             current_level_runtime_stats.entrance_palette[0x9],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xa],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xb],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xc],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xd],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xe],
                                                                                                                             current_level_runtime_stats.entrance_palette[0xf]);

               DebugAppend(output_text);

               DebugAppend("Dumping the entrance palette (sprite pointer version).\r\n");
               siprintf(output_text, "%04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X.\r\n", active_sprite_entrance->palette[0x0],
                                                                                                                             active_sprite_entrance->palette[0x1],
                                                                                                                             active_sprite_entrance->palette[0x2],
                                                                                                                             active_sprite_entrance->palette[0x3],
                                                                                                                             active_sprite_entrance->palette[0x4],
                                                                                                                             active_sprite_entrance->palette[0x5],
                                                                                                                             active_sprite_entrance->palette[0x6],
                                                                                                                             active_sprite_entrance->palette[0x7],
                                                                                                                             active_sprite_entrance->palette[0x8],
                                                                                                                             active_sprite_entrance->palette[0x9],
                                                                                                                             active_sprite_entrance->palette[0xa],
                                                                                                                             active_sprite_entrance->palette[0xb],
                                                                                                                             active_sprite_entrance->palette[0xc],
                                                                                                                             active_sprite_entrance->palette[0xd],
                                                                                                                             active_sprite_entrance->palette[0xe],
                                                                                                                             active_sprite_entrance->palette[0xf]);
               DebugAppend(output_text);

               DebugAppend("Dumping the entrance palette (loaded level version).\r\n");
               siprintf(output_text, "%04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X %04X-%04X-%04X-%04X.\r\n", ((loaded_level_file->runtime_stats).entrance_palette)[0x0],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x1],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x2],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x3],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x4],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x5],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x6],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x7],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x8],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0x9],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xa],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xb],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xc],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xd],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xe],
                                                                                                                             ((loaded_level_file->runtime_stats).entrance_palette)[0xf]);
               DebugAppend(output_text);
               DebugAppend("Do they match?\r\n\r\n");

               DebugWrite();
            }

            // If the next level is to new to the subscreen, we have to
            // render it's vital statistics on the top screen.
            if (next_level_is_new_to_subscreen) {
               DebugAppend("Next level is new to subscreen: updating it.\r\n");
               DebugWrite();

               // Blank the in-out time strip.
               UpdateSubScreenAsLevelInfo_BlankInOutTime();

               // Extract the vital statistics and map preview data from the loaded level
               VitalStatistics_ExtractLemmingsLevelVitalStatisticsAndMapPreviewFromLoadedLevel(&current_vital_statistics, loaded_level_file);

               // We need to regenerate the map! Refresh the sub screen by updating
               // the minimap using the preview data, and copying the map's palette.
               UpdateSubScreenAsLevelInfo_UpdateMapFromPreviewData();
               UpdateSubScreenAsLevelInfo_CopyPaletteToMapPalette(loaded_level_file->runtime_stats.level_palette);

               // Refresh the level info on the sub screen.
               UpdateSubScreenAsLevelInfo_WriteLevelInfo(&current_vital_statistics, (level_won_array[current_level] != 0));
               /* This last parameter is 'level has been won' */
            }                              
            
            // Acquiring the graphical objects required for this new level:
            DebugAppend("About to acquire graphical objects:\r\n");
            DebugWrite();
            
            // Acquire the entrance, exit, NO_TRAP_GENUSES traps, NO_HAZARD_GENUSES hazards,
            // NO_UNINTERACTIVE_GENUSES uninteractives, and the water. Set their palettes!
            GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_entrance,
                                                                    &active_sprite_entrance,
                                                                    "entrance",
                                                                    loaded_level_file->runtime_stats.entrance_genus_junction,
                                                                    level_category, level_category->level_sets[current_level_set],
                                                                    current_level_runtime_stats.entrance_palette);

            GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_exit,
                                                                    &active_sprite_exit,
                                                                    "exit",
                                                                    loaded_level_file->runtime_stats.exit_genus_junction,
                                                                    level_category, level_category->level_sets[current_level_set],
                                                                    current_level_runtime_stats.exit_palette);

            for (u32 trap_genus = 0; trap_genus < NO_TRAP_GENUSES; trap_genus++) {
               GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_trap[trap_genus],
                                                                       &active_sprite_trap[trap_genus],
                                                                       "trap",
                                                                       loaded_level_file->runtime_stats.trap_genus_junctions[trap_genus],
                                                                       level_category, level_category->level_sets[current_level_set],
                                                                       current_level_runtime_stats.trap_genus_palettes[trap_genus]);
            }

            for (u32 hazard_genus = 0; hazard_genus < NO_HAZARD_GENUSES; hazard_genus++) {
               GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_hazard[hazard_genus],
                                                                       &active_sprite_hazard[hazard_genus],
                                                                       "hazard",
                                                                       loaded_level_file->runtime_stats.hazard_genus_junctions[hazard_genus],
                                                                       level_category, level_category->level_sets[current_level_set],
                                                                       current_level_runtime_stats.hazard_genus_palettes[hazard_genus]);
            }

            for (u32 uninteractive_genus = 0; uninteractive_genus < NO_UNINTERACTIVE_GENUSES; uninteractive_genus++) {
               GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_uninteractive[uninteractive_genus],
                                                                       &active_sprite_uninteractive[uninteractive_genus],
                                                                       "uninteractive",
                                                                       loaded_level_file->runtime_stats.uninteractive_genus_junctions[uninteractive_genus],
                                                                       level_category, level_category->level_sets[current_level_set],
                                                                       current_level_runtime_stats.uninteractive_genus_palettes[uninteractive_genus]);
            }

            GraphicalObjects_AcquireGraphicalObjectAndCreateSprites(&active_graphical_object_water,
                                                                    &active_sprite_water,
                                                                    "water",
                                                                    loaded_level_file->runtime_stats.water_genus_junction,
                                                                    level_category, level_category->level_sets[current_level_set],
                                                                    current_level_runtime_stats.water_palette);

            // Free the level data and texture archive memory.
            free(loaded_texture_archive_file);
            free(loaded_level_file);

            next_level_is_new_to_prototype = false;
         }                   

         DebugAppend("Going into PlayLemmingsDS_GoLevelInfoGetReady().\r\n");
         DebugWrite();

         DEBUG_SECTION {
            // Dumping the values within level_won_array:
            DebugAppend("Dumping the values within level_won_array:\r\n");

            for (int level = 0; level < level_category->level_sets[current_level_set]->no_levels; level++) {
               DebugAppend((level_won_array[level] != 0) ? "1" : "0");
            }                                                     

            DebugAppend("\r\n");

            DebugWrite();
         }

         // The famous Lemmings show level info get ready screen.
         get_ready_result = PlayLemmingsDS_GoLevelInfoGetReady(&current_vital_statistics,
                                                               level_info_get_ready_past_shown,
                                                               next_level_is_new_to_subscreen);

         DebugAppend("Exited PlayLemmingsDS_GoLevelInfoGetReady().\r\n");
         DebugWrite();

         // Acknowledge that the subscreen is rendered fully now.
         next_level_is_new_to_subscreen = false;

         // We last saw the level info (get ready) screen.
         level_select_past_shown = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_LEVEL_INFO;

         // If the player chose to play the level... that's good.
         if (get_ready_result == PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_PLAY_LEVEL) {


         } else if (get_ready_result == PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_RESULT_TO_LEVEL_SELECT) {
            // If the player chose to return to the level select:
            // We're not going to play the next level:
            lets_play_next_level   = false;
            // And we're going back to the level select.
            return_to_level_select = true;

            // Disable the automatic ingame level music switching jukebox
            Music_RetireIngameSongJukeboxPlayback();

            // Start a random title screen / level select music at full (64) volume. (Infinite loops.)
            Music_SwitchSongTo(music_category_title_screen_songs.music_filenames[rng_rand(music_category_title_screen_songs.no_music_files)]);

            Cond_CommandMRD_SetPlayerVolume(64);
   
            // Break out of the 'lets_play_next_level' loop
            // and return to the "// Let's pick a level from this category:" comment
            break;
         }                    

         DebugAppend("Going into PlayLemmingsDS_SetUpGoLemmingsLevelGFXModeMainScreen().\r\n");
         DebugWrite();

         // Set up the main screen for Lemmings game action!
         PlayLemmingsDS_SetUpGoLemmingsLevelGFXModeMainScreen();

         DebugAppend("Exited PlayLemmingsDS_SetUpGoLemmingsLevelGFXModeMainScreen().\r\n");
         DebugWrite();

         // Allocate memory for the level we're playing.
         level_data = (u8 (*)[168])malloc(LEVEL_X_SIZE * LEVEL_Y_SIZE);
         DEBUG_SECTION {
            if (level_data == NULL) {
               DebugAppend("Failed to allocate memory for level_data.\r\n");
               DebugWrite();

               // Crash
               LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_COULDNT_ALLOCATE_MEMORY_FOR_LEVEL_DATA_ARRAY);
            }
         }

         // Keep playing levels, if the player keeps restarting the thing.
         do {
            memcpy(level_data, level_data_prototype, LEVEL_X_SIZE * LEVEL_Y_SIZE);

            DebugAppend("Going into PlayLemmingsDS_GoLemmingsLevel().\r\n");
            DebugWrite();

            PlayLemmingsDS_GoLemmingsLevel(&level_result,
                                           (level_result.status_flags & LEVEL_STATUS_FLAG_RESTART)
                                            ? PLAY_LEMMINGS_DS_GO_LEMMINGS_LEVEL_PAST_SHOWN_INGAME
                                            : PLAY_LEMMINGS_DS_GO_LEMMINGS_LEVEL_PAST_SHOWN_LEVEL_INFO);

            DebugAppend("Exited PlayLemmingsDS_GoLemmingsLevel().\r\n");
            DebugWrite();
         } while (level_result.status_flags & LEVEL_STATUS_FLAG_RESTART);

         // Free the memory for the level we just played.
         free(level_data);

         if (level_result.status_flags & LEVEL_STATUS_FLAG_LEVEL_WON) {
            // If they won, update the structure full of level statuses, and save the level set progress file
            level_won_array[current_level] = 1;
            LemmingsDSSavedProgress_SaveProgressToLevelSetFile(level_category, level_category->level_sets[current_level_set], level_won_array);
         }

         // After the level, there is always a status screen. Store its result.
         status_screen_result = PlayLemmingsDS_GoStatusScreen(&current_vital_statistics, &level_result, (current_level == (level_category->level_sets[current_level_set]->no_levels - 1)));
                                                                                                 // This expression is true if the player is on the last level.

         // We last saw the status screen.
         level_info_get_ready_past_shown = PLAY_LEMMINGS_DS_GO_LEVEL_INFO_GET_READY_PAST_SHOWN_STATUS_SCREEN;
   
         // If they chose to restart the level:
         if (status_screen_result == PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_REPLAY_LEVEL) {
            // We're not changing the current_level, so these are false.
            next_level_is_new_to_subscreen = false;
            next_level_is_new_to_prototype = false;
            // This way, the sub screen will remain visible between replays, and the prototype
            // level data will remain constant.

            // Continue the 'lets_play_next_level' loop
            // and return to the "// Keep playing levels based on the current_level variable." comment
            continue;
         } else {
            // We're either heading toward the next level, or going back to the main screen.
   
            if ((current_level == (level_category->level_sets[current_level_set]->no_levels - 1))
             && (level_result.status_flags & LEVEL_STATUS_FLAG_LEVEL_WON         )
             && (false                                                           )) {
               // This is the handler for when you've won all the levels...
               // There's nothing here at the moment though.
   
               // Let's not play another level.
               lets_play_next_level = false;
               // Let's go back to the level select.
               return_to_level_select = true;
            } else if (status_screen_result == PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_NEXT_LEVEL) {
               // The next level is not the same as the last.
               next_level_is_new_to_subscreen = true;
               next_level_is_new_to_prototype = true;
               // Advance level.
               current_level++;
               // Let's play another level!
               lets_play_next_level = true;
            } else if (status_screen_result == PLAY_LEMMINGS_DS_GO_STATUS_SCREEN_RESULT_TO_LEVEL_SELECT) {
               // Let's not play another level.
               lets_play_next_level = false;
               // Let's go back to the level select.
               return_to_level_select = true;
   
               // We last saw the level results screen.
               level_select_past_shown = PLAY_LEMMINGS_DS_GO_LEVEL_SELECT_PAST_SHOWN_STATUS_SCREEN;

               // Disable the automatic ingame level music switching jukebox
               Music_RetireIngameSongJukeboxPlayback();
   
               // Start a random title screen / level select music at full (64) volume. (Infinite loops.)
               Music_SwitchSongTo(music_category_title_screen_songs.music_filenames[rng_rand(music_category_title_screen_songs.no_music_files)]);
               
               Cond_CommandMRD_SetPlayerVolume(64);
            }

            // If we're playing a different level or returning to the level select,
            // we should destroy the graphical objects and associated sprites.
            if (lets_play_next_level || return_to_level_select) {
               free(active_graphical_object_entrance);
               free(active_graphical_object_exit);
               free(active_sprite_entrance);
               free(active_sprite_exit);
               
               for (u32 t = 0; t < NO_TRAP_GENUSES; t++) {
                  free(active_graphical_object_trap[t]);
                  free(active_sprite_trap[t]);
               }
               for (u32 h = 0; h < NO_HAZARD_GENUSES; h++) {
                  free(active_graphical_object_hazard[h]);
                  free(active_sprite_hazard[h]);
               }
               for (u32 u = 0; u < NO_UNINTERACTIVE_GENUSES; u++) {
                  free(active_graphical_object_uninteractive[u]);
                  free(active_sprite_uninteractive[u]);
               }

               free(active_graphical_object_water);
               free(active_sprite_water);
            }
         }
      } // lets_play_next_level loop
   } while (return_to_level_select);
}

// The main function. The main of mains. Mainly main.
// (For gawds sake, don't poke a fork in here. That's a good way to get a shock.)
int main() {
   // Turn everything on.
   powerON(POWER_ALL);    

   // Set up fatlib.
   fatInitDefault();
   
   // Start a new debugging session.
   NewDebug();
   DebugAppend("FAT enabled.\r\n");  
   DebugWrite();
   
   CommandInit();
   DebugAppend("Interprocess 'Command' structure init.\r\n");  
   DebugWrite();

   // Initialize and enable interrupts

	// A vblank interrupt is needed to use swiWaitForVBlank()
	// since the dispatcher handles the flags no other handling is required
	irqInit();
	irqSet(IRQ_VBLANK, irqVBlank);
	irqEnable(IRQ_VBLANK);

   DebugAppend("IRQ setup and enabled.\r\n");
   DebugWrite();                  

   // Fade from white to black
   LemmingsDS_FadeFromWhiteToBlack();

   // Begin the config file loading sequence
   int config_file_loading_success = LemmingsDSConfig_LoadConfigFromFile();

   // If file loading failed, show error screen.
   if (config_file_loading_success != LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_OK) {       
      // Execution stops here; show error.
      if (config_file_loading_success == LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_FILE_NOT_FOUND) {
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_MISSING_CONFIG_FILE);
      } else
      if (config_file_loading_success == LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_FILE) {
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_CONFIG_FILE);
      } else
      if (config_file_loading_success == LEMMINGS_DS_CONFIG_FILE_LOADING_STATUS_BAD_DIRECTORIES) {
         LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_DAMAGED_DIRECTORIES);
      }
   }
   
   // Save a reformatted, valid configuration back to the flash media.
   LemmingsDSConfig_SaveConfigToFile();

   DebugAppend("Gathering and sorting level sets for categories:\r\n");
   DebugWrite();

   // Fade on a message asking the player to wait for the... length
   // level loading sequence to complete
   LemmingsDS_FadeInMessage(14, "Please wait...",
                             0, "",
                            11, "Now loading levels!");

   // Attempt to identify all of the valid level sets
   // living within the Lemmings DS filesystem.
   LemmingsDSLevels_GatherAndSortAllLevelCategories();

   // Fade out the level loading message
   LemmingsDS_FadeOutMessage();
   
   // If there aren't any levels, crash out.
   if ((no_lemmings_ds_level_category_one_player_standard_levels == 0)
    && (no_lemmings_ds_level_category_one_player_custom_levels   == 0)) {
      LemmingsDS_UnrecoverableError(LEMMINGS_DS_UNRECOVERABLE_ERROR_THERE_ARENT_ANY_LEVELS);
   }

   // Set up the NitroTracker engine:
   Music_SetupMusicEngine();

   // Set up the music categories
   Music_SetUpXMMusicCategories();

   // --------------------------------------------------------------------------

   // Set up the structs for the lemming sprite GFX.
   SetUpPermanentLemmingSprites();

   // Start a random title screen / level select music at full (64) volume. (Infinite loops.)
   Music_SwitchSongTo(music_category_title_screen_songs.music_filenames[rng_rand(music_category_title_screen_songs.no_music_files)]);

   Cond_CommandMRD_SetPlayerVolume(64);

   int menu_result;

   // Keep looping through the modular logics.
   while (1) {
      // Fade on the title screen, and fade on the lemmings level info subscreen.
      menu_result = PlayLemmingsDS_GoTitleScreenMenu();

      // Did the player hit the 'one player standard' lemming.
      if (menu_result == PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_STANDARD) {
         PlayLemmingsDS_PlayLemmingsDSUsingLevelCategory(&lemmings_ds_level_category_one_player_standard);
      } else
      if (menu_result == PLAY_LEMMINGS_DS_GO_TITLE_SCREEN_MENU_RESULT_ONE_PLAYER_CUSTOM) {     
         PlayLemmingsDS_PlayLemmingsDSUsingLevelCategory(&lemmings_ds_level_category_one_player_custom);
      }
   }

   // The end.
   return 0;
}

// I hope you enjoyed this source code. ^_^
//           - Matt Carr (mattcarr@gmail.com) 18th November 2006

// Crikey! Another major release, eh? Let's hope that this time
// I've come up with a level format that doesn't... lack. :D
//           - Matt Carr (mattcarr@gmail.com) 13th June 2007
