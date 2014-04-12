@echo off
REM A - SOURCE_DIRECTORY      A path which the program can lop 00.png onto to find
REM                           your images.
REM B - GRAPHICAL_OBJECT_TYPE [ 0          exit | 1      entrance | 2         trap ]
REM                           [ 3        hazard | 4 uninteractive | 5        water ]
REM C - NO_IMAGES             How many images are there in total?
REM D - NO_PRIMARY_IMAGES     (TRAPS) How many images in the primary animation?
REM E - NO_SECONDARY_IMAGES   (TRAPS) How many images in the secondary animation?
REM F - REPRESENTING_FRAME    What frame should be used as preview in LDS Builder?
REM G - TRANSPARENT_COLOUR    Which is the transparent colour in the source images?
REM H - HANDLE_X              Identify the active coordinate within the object.
REM I - HANDLE_Y              Identify the active coordinate within the object.
REM J - ACTIVE_ZONE_X1        Identify the active zone for exits, traps and hazards.
REM K - ACTIVE_ZONE_Y1        Identify the active zone for exits, traps, hazards and water.
REM L - ACTIVE_ZONE_X2        Identify the active zone for exits, traps and hazards.
REM M - ACTIVE_ZONE_Y2        Identify the active zone for exits, traps, hazards and water.
REM N - FLAGS                 Special flags describing the behaviour of the graphical object
REM O - DESCRIPTIVE_NAME      15 character description for the object. (NO SPACES)
REM P - OUTPUT_FILE_NAME      Specify a .LGO output file name.

REM                         A                     B   C   D   E   F   G   H   I   J   K    L   M   N    O               P

lemmings_graphical_object object_sources\o00_05_  5   8   8   0   0   16  0   4   0   0    0   11  0    WTR_soil_water  water_0.lgo
lemmings_graphical_object object_sources\o01_05_  5   8   8   0   0   16  0   9   0   0    0   10  0    WTR_fire_lava   water_1.lgo
lemmings_graphical_object object_sources\o02_05_  5   8   8   0   0   16  0   8   0   0    0   16  0    WTR_marble_acid water_2.lgo
lemmings_graphical_object object_sources\o03_05_  5   8   8   0   0   16  0   5   0   0    0   10  0    WTR_pillar_watr water_3.lgo
lemmings_graphical_object object_sources\o04_06_  5   8   8   0   0   16  0   4   0   0    0   11  0    WTR_ice_icewatr water_4.lgo
lemmings_graphical_object object_sources\o05_05_  5   9   9   0   0   16  0   13  0   0    0   10  0    WTR_brckcustard water_5.lgo
lemmings_graphical_object object_sources\o06_05_  5   4   4   0   0   16  0   14  0   0    0   9   0    WTR_rock_hairs  water_6.lgo
lemmings_graphical_object object_sources\o07_05_  5   8   8   0   0   16  0   4   0   0    0   13  0    WTR_snow_water  water_7.lgo
lemmings_graphical_object object_sources\o08_05_  5   6   6   0   0   16  0   14  0   0    0   16  0    WTR_bubble_bubs water_8.lgo
