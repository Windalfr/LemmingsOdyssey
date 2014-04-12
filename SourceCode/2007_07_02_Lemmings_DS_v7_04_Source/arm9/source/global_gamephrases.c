// Useful phrases that shouldn't really be put into the main game code

#include "global_gamephrases.h"
#include <nds.h>

const char *global_gamephrase_nullstring = "";
const char *global_gamephrase_plural     = "s";

const char *global_gamephrase_titlescreensubtext[GLOBAL_GAMEPHRASE_TITLESCREENSUBTEXT_LINES] =
                                                     {"Lemmings DS 21/06/07",};

const char *global_gamephrase_levelinfosubscreen[GLOBAL_GAMEPHRASE_LEVELINFOSUBSCREEN_LINES] =
                                                     {"Number of Lemmings",
                                                      "% To Be Saved",
                                                      "Release Rate",    
                                                      "Time      Minutes",
                                                      "Time      Minute ",
                                                      "Rating",};

const char *global_gamephrase_inoutsubscreen[GLOBAL_GAMEPHRASE_INOUTSUBSCREEN_LINES] =
                                                     {"Out: ",
                                                      "In:    %",
                                                      "Time:  -",};

const char *global_gamephrase_levelselectmainscreen[GLOBAL_GAMEPHRASE_LEVELSELECTMAINSCREEN_LINES] =
                                                     {"Level",};

const char *global_gamephrase_levelselectcontrolstext[GLOBAL_GAMEPHRASE_LEVELSELECTCONTROLSTEXT_LINES] =
                                                     {"Use stylus to select a level",
                                                      "Tap any level twice to play",
                                                     //12345678901234567890123456789012
                                                      "Cycle levels with X-B or D pad",
                                                      "Cycle set with Y-A or D pad",
                                                      "Tap L or R button to play level",
                                                      "Press Start or Select to return",};

const char *global_gamephrase_resultscreen_title[GLOBAL_GAMEPHRASE_RESULTSCREEN_TITLE_LINES] =
                                                     {"All lemmings accounted for.",
                                                      "Your time is up!",};

const char *global_gamephrase_resultscreen_youneeded[GLOBAL_GAMEPHRASE_RESULTSCREEN_YOUNEEDED_LINES] =
                                                     {"You needed",
                                                      "You rescued",};
                                                      
const char *global_gamephrase_resultscreen_commentary[GLOBAL_GAMEPHRASE_RESULTSCREEN_COMMENTARY_LINES] =
                                                  //   {"ROCK BOTTOM! I hope for your sake",   The original Lemmings quotes dont fit!
                                                  //    "that you nuked that level!",};
                                                     {"You didn't save any lemmings!",    // none
                                                      "What happened?",                 
                                                      
                                                      "Have a break, clear your head,",   // a couple
                                                      "and try this level again.",
                                                      
                                                      "That's good, but you still",       // a lot
                                                      "need a lot more lemmings.",
                                                      
                                                      "So close! Take another shot",      // almost got it
                                                      "and get that last few %!",

                                                      "That's it! Nicely done!",          // exactly
                                                      "On to the next...",

                                                      "More than enough!",                // a few more
                                                      "Let's move on.",

                                                      "Nice work!",                       // a lot more
                                                      "That's loads of lemmings!",
                                                      
                                                      "You rescued all the lemmings!",    // all of them
                                                      "Excellent work!",};
                                                      
                                                                            
const char *global_gamephrase_resultscreen_prompt[GLOBAL_GAMEPHRASE_RESULTSCREEN_PROMPT_LINES] =
                                                     {"Tap screen to continue",
                                                      "Tap screen to retry",
                                                      "Press button for level select",};
//                                          12345678901234567890123456789012
const char *global_gamephrase_blank_line = "                                ";

