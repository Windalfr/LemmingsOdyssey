// Miscellaneous graphical stuff that really shouldn't be in the main source file.
//   by Mathew Carr

#include "lemgfx.h"

// Why isn't this in the GBFS?

// Simple: it's only 1056 bytes long. :D

const u16 lemgfx_palette[16]          = { 0x8000 | 31775, 0x8000 |  30092, 0x8000 |    928, 0x8000 |    990, 0x8000 |  32767, 0x8000 |  16912, 0x8000 |   4254, 0x8000 |  27482,
                                          0x8000 | 22197, 0x8000 |  20083, 0x8000 |  25040, 0x8000 |      0, 0x8000 |      0, 0x8000 |      0, 0x8000 |      0, 0x8000 |      0};


const u16 lemgfx_palette_selected[16] = { 0x8000 | 31775, 0x8000 |    319, 0x8000 |   1023, 0x8000 |  32192, 0x8000 |  32767, 0x8000 |  16912, 0x8000 |  23848, 0x8000 |  25368,
                                          0x8000 |     0, 0x8000 | 0xffff, 0x8000 | 0xffff, 0x8000 | 0xffff, 0x8000 | 0xffff, 0x8000 |      0, 0x8000 |      0, 0x8000 |      0};

