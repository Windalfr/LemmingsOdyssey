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

lemmings_graphical_object object_sources\o00_00_  0   1   1   0   0   16  25  16  -2  -6   3   0   0   EXIT_soil       exit_0.lgo
lemmings_graphical_object object_sources\o01_00_  0   1   1   0   0   16  21  27  -2  -7   3   0   0   EXIT_fire       exit_1.lgo
lemmings_graphical_object object_sources\o02_00_  0   1   1   0   0   16  25  15  -2  -5   3   0   0   EXIT_marble     exit_2.lgo
lemmings_graphical_object object_sources\o03_00_  0   1   1   0   0   16  25  16  -2  -7   3   0   0   EXIT_pillar     exit_3.lgo 
lemmings_graphical_object object_sources\o04_00_  0   1   1   0   0   16  21  14  -2  -5   3   0   0   EXIT_ice        exit_4.lgo
lemmings_graphical_object object_sources\o05_00_  0   1   1   0   0   16  18  34  -3  -6   3   0   0   EXIT_brick      exit_5.lgo
lemmings_graphical_object object_sources\o06_00_  0   2   2   0   0   16  16  31  -2  -6   3   0   0   EXIT_rock       exit_6.lgo
lemmings_graphical_object object_sources\o07_00_  0   1   1   0   0   16  20  34  -2  -5   3   0   0   EXIT_snow       exit_7.lgo
lemmings_graphical_object object_sources\o08_00_  0   1   1   0   0   16  24  31  -2  -6   3   0   0   EXIT_bubble     exit_8.lgo
lemmings_graphical_object object_sources\o09_00_  0   1   1   0   0   16  23  28  -3  -6   3   1   0   EXIT_hols       exit_9.lgo
