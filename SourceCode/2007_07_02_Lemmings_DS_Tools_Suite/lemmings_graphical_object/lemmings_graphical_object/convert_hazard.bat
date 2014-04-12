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

REM                         A                     B   C   D   E   F   G   H   I   J   K    L   M   N   O               P

lemmings_graphical_object object_sources\o01_07_  2   8   8   0   0   16  0   0   1   7    30  14  0   HAZ_fire_coals  hazard_0.lgo
lemmings_graphical_object object_sources\o01_08_  2   10  10  0   4   16  0   0   5   5    66  18  0   HAZ_fire_shootR hazard_1.lgo
lemmings_graphical_object object_sources\o01_10_  2   10  10  0   4   16  0   0   13  5    74  18  0   HAZ_fire_shootL hazard_2.lgo
lemmings_graphical_object object_sources\o02_09_  2   16  16  0   2   16  15  16  -13 -11  14  2   0   HAZ_marble_fire hazard_3.lgo
lemmings_graphical_object object_sources\o07_09_  2   11  11  0   5   16  0   0   5   13   59  22  0   HAZ_snow_icegun hazard_4.lgo