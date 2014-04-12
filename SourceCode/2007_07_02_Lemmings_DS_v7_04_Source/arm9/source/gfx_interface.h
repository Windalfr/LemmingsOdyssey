//======================================================================
//
//	gfx_interface, 256x24@4, 
//	+ palette 16 entries, not compressed
//	+ 96 tiles not compressed
//	Total size: 32 + 3072 = 3104
//
//	Time-stamp: 2007-05-12, 23:01:25
//	Exported by Cearn's Usenti v1.7.4
//	(comments, kudos, flames to "daytshen@hotmail.com")
//
//======================================================================

#ifndef __GFX_INTERFACE__
#define __GFX_INTERFACE__

#define gfx_interfacePalLen 32
extern const unsigned short gfx_interfacePal[16];

#define gfx_interfaceTilesLen 3072
extern const unsigned short gfx_interfaceTiles[1536];

#endif // __GFX_INTERFACE__

