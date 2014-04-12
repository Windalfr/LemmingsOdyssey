//
// Lemmings DS Builder
//
// (c) May 2006, June 2006, July 2006, August 2006
//
// bmp_loader.h
//   Headers and structures necessary for loading and manipulating
//   'memory bmp's.
//
// By Mathew Carr. LDS format devised by Mathew Carr.
// mattcarr@gmail.com
//
                                         
#ifndef _BMP_LOADER_H_
#define _BMP_LOADER_H_
                                
#include <stdio.h>
#include "ds_types.h"
                    
typedef unsigned char  BYTE;    
typedef unsigned short WORD;  
typedef unsigned short UINT;  
typedef unsigned long  DWORD;
                       
typedef short INT;         
typedef long  LONG;            
typedef float FLOAT;

typedef struct tagBITMAPFILEHEADER {    /* bmfh */
    UINT    bfType      __attribute__ ((packed));
    DWORD   bfSize      __attribute__ ((packed));
    UINT    bfReserved1 __attribute__ ((packed));
    UINT    bfReserved2 __attribute__ ((packed));
    DWORD   bfOffBits   __attribute__ ((packed));
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {    /* bmih */
    DWORD   biSize          __attribute__ ((packed)); // This attribute thing gets rid of structure packing!
    LONG    biWidth         __attribute__ ((packed));
    LONG    biHeight        __attribute__ ((packed));
    WORD    biPlanes        __attribute__ ((packed));
    WORD    biBitCount      __attribute__ ((packed));
    DWORD   biCompression   __attribute__ ((packed));
    DWORD   biSizeImage     __attribute__ ((packed));
    LONG    biXPelsPerMeter __attribute__ ((packed));
    LONG    biYPelsPerMeter __attribute__ ((packed));
    DWORD   biClrUsed       __attribute__ ((packed));
    DWORD   biClrImportant  __attribute__ ((packed));
} BITMAPINFOHEADER;


typedef struct tagRGBQUAD {     /* rgbq */
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagMEMORY_BMP_FILE {
   BITMAPFILEHEADER fileheader;     
   BITMAPINFOHEADER infoheader;
   RGBQUAD          palette[1];     
} MEMORY_BMP_FILE;

typedef struct tagMEMORY_BMP {
   MEMORY_BMP_FILE *bmp_memory_begin; // This is where the BMP in memory starts.
                                      // It's a -new u8- array.     
                                       
   int memory_length; // The length of the memory chunk containing the BMP file data.
} MEMORY_BMP;

bool MemoryBMP_LoadBMP(MEMORY_BMP *memory_bmp, const char *filename);
void MemoryBMP_UnloadBMP(MEMORY_BMP *memory_bmp);
void MemoryBMP_Initialise(MEMORY_BMP *memory_bmp); 

static inline int MemoryBMP_GetNoColours(MEMORY_BMP *memory_bmp) {
   switch (memory_bmp->bmp_memory_begin->infoheader.biBitCount) {
       case ( 1) : { return        2; } break;   
       case ( 4) : { return       16; } break;   
       case ( 8) : { return      256; } break;   
       case (24) : { return 16777216; } break;   
   };   
}    

static inline int MemoryBMP_GetWidth(MEMORY_BMP *memory_bmp) {  
   return memory_bmp->bmp_memory_begin->infoheader.biWidth;   
}

static inline int MemoryBMP_GetHeight(MEMORY_BMP *memory_bmp) { 
   return memory_bmp->bmp_memory_begin->infoheader.biHeight;   
}

static inline int MemoryBMP_GetPixel(MEMORY_BMP *memory_bmp, int x, int y) {  
   u8 *pixel_source_cursor = ((u8 *)(memory_bmp->bmp_memory_begin))
                                + memory_bmp->bmp_memory_begin->fileheader.bfOffBits;   
                                
   int line_length; // the length in bytes of a line of raw data.
   int byte_location; // the offset into the bytepile where the pixel lives.
   
   u8 *pixel_source;
   
   if (MemoryBMP_GetNoColours(memory_bmp) ==  16) {
      line_length = (((MemoryBMP_GetWidth(memory_bmp) + 7)/8) * 4);
      
      byte_location = (line_length * (MemoryBMP_GetHeight(memory_bmp) - y - 1)) + (x /2);
      
      pixel_source = pixel_source_cursor + byte_location;
      
      if (x & 1) {
         return ((*pixel_source)     ) & 0x0F;
      } else {
         return ((*pixel_source) >> 4) & 0x0F;
      }
   } else
   if (MemoryBMP_GetNoColours(memory_bmp) == 256) {
      line_length = (((MemoryBMP_GetWidth(memory_bmp) + 3)/4) * 4);
      
      byte_location = (line_length * (MemoryBMP_GetHeight(memory_bmp) - y - 1)) + (x);   
      
      pixel_source = pixel_source_cursor + byte_location;
      
      return (*pixel_source);
   }
   return 5;
}                

static inline int MemoryBMP_GetNoPaletteTuples(MEMORY_BMP *memory_bmp) {
   if (memory_bmp->bmp_memory_begin->infoheader.biClrUsed == 0) {
      return MemoryBMP_GetNoColours(memory_bmp);
   } else {
      return memory_bmp->bmp_memory_begin->infoheader.biClrUsed;
   }   
}        

static inline RGBQUAD *MemoryBMP_GetPaletteEntryTuple(MEMORY_BMP *memory_bmp, int n) {
   return (&(memory_bmp->bmp_memory_begin->palette[n]));
}




#endif // _BMP_LOADER_H_

#if 0
/*
The BITMAPFILEHEADER structure contains information about the type, size, and
layout of a device-independent bitmap (DIB) file.

Member          Description

bfType          Specifies the type of file. This member must be BM. 
bfSize          Specifies the size of the file, in bytes. 
bfReserved1     Reserved; must be set to zero. 
bfReserved2     Reserved; must be set to zero.
bfOffBits       Specifies the byte offset from the BITMAPFILEHEADER structure
to the actual bitmap data in the file.

Comments

A BITMAPINFO or BITMAPCOREINFO structure immediately follows the
BITMAPFILEHEADER structure in the DIB file.

The BITMAPINFO structure fully defines the dimensions and color information
for a Windows 3.0 or later device-independent bitmap (DIB).

Member          Description

bmiHeader       Specifies a BITMAPINFOHEADER structure that contains
information about the dimensions and color format of a DIB.

bmiColors       Specifies an array of RGBQUAD structures that define the
colors in the bitmap.

Comments

A Windows 3.0 or later DIB consists of two distinct parts: a BITMAPINFO
structure, which describes the dimensions and colors of the bitmap, and an
array of bytes defining the pixels of the bitmap. The bits in the array are
packed together, but each scan line must be zero-padded to end on a LONG
boundary. Segment boundaries, however, can appear anywhere in the bitmap. The
origin of the bitmap is the lower-left corner.

The biBitCount member of the BITMAPINFOHEADER structure determines the number
of bits which define each pixel and the maximum number of colors in the
bitmap. This member may be set to any of the following values:

Value   Meaning

1       The bitmap is monochrome, and the bmciColors member must contain two
entries. Each bit in the bitmap array represents a pixel. If the bit is
clear, the pixel is displayed with the color of the first entry in the
bmciColors table. If the bit is set, the pixel has the color of the second
entry in the table.

4       The bitmap has a maximum of 16 colors, and the bmciColors member
contains 16 entries. Each pixel in the bitmap is represented by a four-bit
index into the color table.

For example, if the first byte in the bitmap is 0x1F, the byte represents two
pixels. The first pixel contains the color in the second table entry, and the
second pixel contains the color in the sixteenth table entry.

8       The bitmap has a maximum of 256 colors, and the bmciColors member
contains 256 entries. In this case, each byte in the array represents a
single pixel.

24      The bitmap has a maximum of 2^24 colors. The bmciColors member is
NULL, and each 3-byte sequence in the bitmap array represents the relative
intensities of red, green, and blue, respectively, of a pixel.

The biClrUsed member of the BITMAPINFOHEADER structure specifies the number
of color indexes in the color table actually used by the bitmap. If the
biClrUsed member is set to zero, the bitmap uses the maximum number of colors
corresponding to the value of the biBitCount member.

The colors in the bmiColors table should appear in order of importance.
Alternatively, for functions that use DIBs, the bmiColors member can be an
array of 16-bit unsigned integers that specify an index into the currently
realized logical palette instead of explicit RGB values. In this case, an
application using the bitmap must call DIB functions with the wUsage
parameter set to DIB_PAL_COLORS.

Note:   The bmiColors member should not contain palette indexes if the bitmap
is to be stored in a file or transferred to another application. Unless the
application uses the bitmap exclusively and under its complete control, the
bitmap color table should contain explicit RGB values.

The BITMAPINFOHEADER structure contains information about the dimensions and
color format of a Windows 3.0 or later device-independent bitmap (DIB).

Member          Description

biSize          Specifies the number of bytes required by the
BITMAPINFOHEADER structure.

biWidth         Specifies the width of the bitmap, in pixels. 
biHeightSpecifies the height of the bitmap, in pixels. 

biPlanesSpecifies the number of planes for the target device. This
member must be set to 1.

biBitCount      Specifies the number of bits per pixel. This value must be 1,
4, 8, or 24.

biCompression   Specifies the type of compression for a compressed bitmap. It
can be one of the following values:

Value           Meaning

BI_RGB          Specifies that the bitmap is not compressed. 

BI_RLE8         Specifies a run-length encoded format for bitmaps with 8 bits
per pixel. The compression format is a 2-byte format consisting of a count
byte followed by a byte containing a color index.  For more information, see
the following Comments section.

BI_RLE4         Specifies a run-length encoded format for bitmaps with 4 bits
per pixel. The compression format is a 2-byte format consisting of a count
byte followed by two word-length color indexes.  For more information, see
the following Comments section.

biSizeImage     Specifies the size, in bytes, of the image. It is valid to
set this member to zero if the bitmap is in the BI_RGB format.

biXPelsPerMeter Specifies the horizontal resolution, in pixels per meter, of
the target device for the bitmap. An application can use this value to select
a bitmap from a resource group that best matches the characteristics of the
current device.

biYPelsPerMeter Specifies the vertical resolution, in pixels per meter, of
the target device for the bitmap.

biClrUsed       Specifies the number of color indexes in the color table
actually used by the bitmap. If this value is zero, the bitmap uses the
maximum number of colors corresponding to the value of the biBitCount member.
For more information on the maximum sizes of the color table, see the
description of the BITMAPINFO structure earlier in this topic.

If the biClrUsed member is nonzero, it specifies the actual number of colors
that the graphics engine or device driver will access if the biBitCount
member is less than 24. If biBitCount is set to 24, biClrUsed specifies the
size of the reference color table used to optimize performance of Windows
color palettes.  If the bitmap is a packed bitmap (that is, a bitmap in which
the bitmap array immediately follows the BITMAPINFO header and which is
referenced by a single pointer), the biClrUsed member must be set to zero or
to the actual size of the color table.

biClrImportant  Specifies the number of color indexes that are considered
important for displaying the bitmap. If this value is zero, all colors are
important.

Comments

The BITMAPINFO structure combines the BITMAPINFOHEADER structure and a color
table to provide a complete definition of the dimensions and colors of a
Windows 3.0 or later DIB. For more information about specifying a Windows 3.0
DIB, see the description of the BITMAPINFO structure.

An application should use the information stored in the biSize member to
locate the color table in a BITMAPINFO structure as follows:

pColor = ((LPSTR) pBitmapInfo + (WORD) (pBitmapInfo->bmiHeader.biSize))

The RGBQUAD structure describes a color consisting of relative intensities of
red, green, and blue. The bmiColors member of the BITMAPINFO structure
consists of an array of RGBQUAD structures.

Member          Description

rgbBlue         Specifies the intensity of blue in the color. 
rgbGreenSpecifies the intensity of green in the color. 
rgbRed          Specifies the intensity of red in the color. 
rgbReserved     Not used; must be set to zero. 
*/
#endif
