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

lemmings_graphical_object object_sources\o00_06_  2   15  1   14  1   16  7   20   0  -1   1   0   1   T_soil_mantrap  trap_0.lgo
lemmings_graphical_object object_sources\o00_08_  2   17  1   16  1   16  6   36  -1  -1   2   0   1   T_soil_rocktrap trap_1.lgo
lemmings_graphical_object object_sources\o00_10_  2   12  1   11  1   16  14  38  -1  -3   2   1   1   T_soil_10tons   trap_2.lgo
lemmings_graphical_object object_sources\o02_08_  2   15  1   14  0   16  15  29  -2  -3   3   1   1   T_marble_squish trap_3.lgo
lemmings_graphical_object object_sources\o03_08_  2   37  1   36  1   16  3   37  -1  -5   2   2   1   T_pillar_string trap_4.lgo
lemmings_graphical_object object_sources\o03_09_  2   7   1   6   1   16  12  15  -5  -1   4   2   3   T_pillar_spikeR trap_5.lgo
lemmings_graphical_object object_sources\o03_10_  2   7   1   6   1   16  3   15  -4  -1   5   2   3   T_pillar_spikeL trap_6.lgo
lemmings_graphical_object object_sources\o04_07_  2   25  1   24  1   16  14  8   -1  -1   2   1   1   T_ice_slicer    trap_7.lgo
lemmings_graphical_object object_sources\o04_09_  2   8   1   7   1   16  7   39  -1  -1   2   1   1   T_ice_frazzle   trap_8.lgo
lemmings_graphical_object object_sources\o04_10_  2   16  1   15  1   16  15  9   -1  -2   2   1   1   T_ice_dis-grate trap_9.lgo
lemmings_graphical_object object_sources\o05_06_  2   18  1   17  1   16  6   33  -1  -2   2   1   1   T_brick_stomper trap_10.lgo
lemmings_graphical_object object_sources\o05_07_  2   20  1   19  1   16  10  29  -2  -2   3   4   1   T_brick_wheeler trap_11.lgo
lemmings_graphical_object object_sources\o06_06_  2   10  1   9   1   16  6   13  -1  -1   2   1   1   T_rock_tentacle trap_12.lgo
lemmings_graphical_object object_sources\o06_08_  2   4   1   3   1   16  2   20   0  -1   2   2   3   T_rock_chameatL trap_13.lgo
lemmings_graphical_object object_sources\o06_10_  2   4   1   3   1   16  61  20  -2  -1   0   2   3   T_rock_chameatR trap_14.lgo
lemmings_graphical_object object_sources\o07_08_  2   14  1   13  1   16  13  53  -1  -1   2   1   1   T_snow_icicle   trap_15.lgo
lemmings_graphical_object object_sources\o08_08_  2   14  1   13  1   16  39  33  -2  -2   2   2   3   T_bubble_zapper trap_16.lgo
lemmings_graphical_object object_sources\o08_09_  2   20  1   19  1   16  5   31  -1  -2   2   1   1   T_bubble_suckup trap_17.lgo
