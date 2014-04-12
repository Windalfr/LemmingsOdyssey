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

lemmings_graphical_object object_sources\o00_02_  4   14  14  0   8   16  0   0   0   0    0   0   0   UN_green_flag   uninteractive_0.lgo
lemmings_graphical_object object_sources\o00_09_  4   14  14  0   8   16  0   0   0   0    0   0   0   UN_blue_flag    uninteractive_1.lgo
lemmings_graphical_object object_sources\o00_07_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_soil_EXdec   uninteractive_2.lgo
lemmings_graphical_object object_sources\o01_06_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_fire_EXdec   uninteractive_3.lgo
lemmings_graphical_object object_sources\o02_06_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_marble_EXdec uninteractive_4.lgo
lemmings_graphical_object object_sources\o03_06_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_pillar_EXdec uninteractive_5.lgo
lemmings_graphical_object object_sources\o04_08_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_ice_EXdec    uninteractive_6.lgo
lemmings_graphical_object object_sources\o05_09_  4   4   4   0   0   16  0   0   0   0    0   0   0   UN_brick_EXdec  uninteractive_7.lgo
lemmings_graphical_object object_sources\o06_07_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_cham_body1   uninteractive_8.lgo
lemmings_graphical_object object_sources\o06_09_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_cham_body2   uninteractive_9.lgo
lemmings_graphical_object object_sources\o07_06_  4   5   5   0   0   16  0   0   0   0    0   0   0   UN_red_flag     uninteractive_10.lgo
lemmings_graphical_object object_sources\o08_06_  4   7   7   0   0   16  0   0   0   0    0   0   0   UN_bubble_EXdec uninteractive_11.lgo
lemmings_graphical_object object_sources\o08_10_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_bubblezapgun uninteractive_12.lgo
lemmings_graphical_object object_sources\o09_02_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_hols_ENTdec  uninteractive_13.lgo
lemmings_graphical_object object_sources\o09_03_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_hols_EXdec   uninteractive_14.lgo
lemmings_graphical_object object_sources\o09_04_  4   16  16  0   0   16  0   0   0   0    0   0   0   UN_snow_bounce  uninteractive_15.lgo
lemmings_graphical_object object_sources\o09_05_  4   6   6   0   0   16  0   0   0   0    0   0   0   UN_festivelight uninteractive_16.lgo
lemmings_graphical_object object_sources\o09_06_  4   4   4   0   0   16  0   0   0   0    0   0   0   UN_yule_roast   uninteractive_17.lgo
lemmings_graphical_object object_sources\o09_07_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_fireplacetop uninteractive_18.lgo
lemmings_graphical_object object_sources\o09_08_  4   1   1   0   0   16  0   0   0   0    0   0   0   UN_snowmans_box uninteractive_19.lgo
lemmings_graphical_object object_sources\o09_09_  4   14  14  0   0   16  0   0   0   0    0   0   0   UN_springy_snow uninteractive_20.lgo
