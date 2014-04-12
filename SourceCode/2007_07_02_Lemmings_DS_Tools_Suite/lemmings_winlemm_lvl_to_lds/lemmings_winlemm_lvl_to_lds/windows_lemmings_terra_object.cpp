//
// Lemmings DS - Windows Lemmings terra objects.
//
// (c) December 2006
//
// windows_lemmings_terra_object.cpp
//   Functions for handling memory representations of windows lemmings terra objects.
//
// By Mathew Carr.
// mattcarr@gmail.com
//

#include "windows_lemmings_terra_object.h"

// These hold the names of the different style directories.
extern const char windows_lemmings_terra_object_theme_names[][NO_WINDOWS_LEMMING_TERRA_OBJECT_STYLES] = {
                                     "Soil",    
                                     "Fire",    
                                     "Marble",    
                                     "Pillar",    
                                     "Ice",    
                                     "Brick",    
                                     "Rock",    
                                     "Snow",    
                                     "Bubble",    
                                     "Snow",    
};

// These hold the numbers of terra objects for each style.
extern const int windows_lemmings_terra_object_count_per_style[NO_WINDOWS_LEMMING_TERRA_OBJECT_STYLES] = {
                                     60,
                                     64,
                                     60,
                                     64,
                                     37,
                                     60,
                                     64,
                                     37,
                                     64,
                                     37,
};
