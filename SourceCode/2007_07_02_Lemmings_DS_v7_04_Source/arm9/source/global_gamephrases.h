// Useful phrases that shouldn't really be put into the main game code

#ifndef __GLOBAL_GAMEPHRASES_H__
#define __GLOBAL_GAMEPHRASES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

extern const char *global_gamephrase_nullstring;
extern const char *global_gamephrase_plural;

#define GLOBAL_GAMEPHRASE_TITLESCREENSUBTEXT_LINES 1
extern const char *global_gamephrase_titlescreensubtext[GLOBAL_GAMEPHRASE_TITLESCREENSUBTEXT_LINES];

#define GLOBAL_GAMEPHRASE_LEVELINFOSUBSCREEN_LINES 6
extern const char *global_gamephrase_levelinfosubscreen[GLOBAL_GAMEPHRASE_LEVELINFOSUBSCREEN_LINES];

#define GLOBAL_GAMEPHRASE_INOUTSUBSCREEN_LINES 3
extern const char *global_gamephrase_inoutsubscreen[GLOBAL_GAMEPHRASE_INOUTSUBSCREEN_LINES];

#define GLOBAL_GAMEPHRASE_LEVELSELECTMAINSCREEN_LINES 1
extern const char *global_gamephrase_levelselectmainscreen[GLOBAL_GAMEPHRASE_LEVELSELECTMAINSCREEN_LINES];
                                       
#define GLOBAL_GAMEPHRASE_LEVELSELECTCONTROLSTEXT_LINES 6
extern const char *global_gamephrase_levelselectcontrolstext[GLOBAL_GAMEPHRASE_LEVELSELECTCONTROLSTEXT_LINES];

#define GLOBAL_GAMEPHRASE_LEVELSELECTCUSTOMCONTROLSTEXT_LINES 5
extern const char *global_gamephrase_levelselectcustomcontrolstext[GLOBAL_GAMEPHRASE_LEVELSELECTCUSTOMCONTROLSTEXT_LINES];

#define GLOBAL_GAMEPHRASE_RESULTSCREEN_TITLE_LINES 2
extern const char *global_gamephrase_resultscreen_title[GLOBAL_GAMEPHRASE_RESULTSCREEN_TITLE_LINES];
       
#define GLOBAL_GAMEPHRASE_RESULTSCREEN_YOUNEEDED_LINES 2
extern const char *global_gamephrase_resultscreen_youneeded[GLOBAL_GAMEPHRASE_RESULTSCREEN_YOUNEEDED_LINES];

#define GLOBAL_GAMEPHRASE_RESULTSCREEN_COMMENTARY_LINES 16
extern const char *global_gamephrase_resultscreen_commentary[GLOBAL_GAMEPHRASE_RESULTSCREEN_COMMENTARY_LINES];

#define GLOBAL_GAMEPHRASE_RESULTSCREEN_PROMPT_LINES 3
extern const char *global_gamephrase_resultscreen_prompt[GLOBAL_GAMEPHRASE_RESULTSCREEN_PROMPT_LINES];

extern const char *global_gamephrase_blank_line;
                             
#ifdef __cplusplus
}
#endif

#endif // __GLOBAL_GAMEPHRASES_H__
