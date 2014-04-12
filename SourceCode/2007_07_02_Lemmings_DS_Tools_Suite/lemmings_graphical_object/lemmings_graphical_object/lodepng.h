/*
LodePNG version 20061228

Copyright (c) 2005-2006 Lode Vandevenne
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Lode Vandevenne nor the names of his contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LODEPNG_H
#define LODEPNG_H

#include <vector>
#include <string>

namespace LodePNG
{

  void loadFile(std::vector<unsigned char>& buffer, const std::string& filename);
  void saveFile(const std::vector<unsigned char>& buffer, const std::string& filename);

  class Decoder
  {
    public:

    //decoding functions
    void decode32(std::vector<unsigned char>& out, std::vector<unsigned char>& in);
    void decodeGeneric(std::vector<unsigned char>& out, std::vector<unsigned char>& in);
    
    //error checking after decoding
    bool hasError() const;
    int getError() const;
    
    //get image size after decoding
    size_t getWidth() const;
    size_t getHeight() const;
    
    struct Info
    {
      //Decoder error value
      int error; //the error value of the decode attempt
      
      //dimensions of the image
      size_t w; //width of the image in pixels
      size_t h; //height of the image in pixels
      
      //pixel color info
      unsigned long bitDepth;      //bits per sample
      unsigned long bpp;           //bits per pixel
      unsigned long colorChannels; //amount of channels
      
      //palette info
      unsigned long paletteSize; //size of the palette (palette.size() / 4)
      std::vector<unsigned char> palette; //palette in RGBARGBA... order
      
      //transparent color key info
      bool          colorKey; //is a transparent color key given?
      unsigned char keyR;     //red/greyscale component of color key
      unsigned char keyG;     //green component of color key
      unsigned char keyB;     //blue component of color key
      
      //PNG specific information (general)
      unsigned long colorType;     //color type of the original PNG file
      unsigned long compressionMethod; //compression method of the original file
      unsigned long filterMethod;      //filter method of the original file
      unsigned long interlaceMethod;   //interlace method of the original file
      
      //PNG specific information (bKGD chunk)
      bool          backgroundColor; //is a suggested background color given?
      unsigned char backgroundR;     //red component of sugg. background color
      unsigned char backgroundG;     //green component of sugg. background color
      unsigned char backgroundB;     //blue component of sugg. background color
      
      //constructor
      Info();
    };
    
    //get other image info after decoding
    const Info& getInfo() const;
    
    private:
    
    Info info;
    
    unsigned long codelengthcode[19]; //the lengths of the tree used to decode the lengths of the dynamic tree
    
    //Huffman trees
    unsigned long codetree[287][2]; //the code tree for Huffman codes
    unsigned long codetreeD[31][2]; //the code tree for distance codes
    unsigned long codelengthcodetree[18][2]; //the code tree for code length codes
    
    unsigned long huffmanDecodeSymbol(unsigned char* in, size_t& bp, unsigned long* codetree, size_t numcodes, size_t inlength);
    void getTreeInflateFixed(unsigned long* tree, unsigned long* treeD);
    void getTreeInflateDynamic(unsigned long* tree, unsigned long* treeD, unsigned char* in, size_t& bp, size_t inlength);
    void inflateHuffmanBlock(unsigned char* out, unsigned char* in, size_t& bp, size_t& pos, size_t outlength, size_t inlength, unsigned long btype) ;
    void inflate(unsigned char* out, unsigned char* in, size_t inlength, size_t outlength);
    void decodeZlib(unsigned char* out, unsigned char* in, size_t outlength, size_t inlength);
    void readPngHeader(unsigned char* in, size_t inlength);
    void unFilterScanline(unsigned char* recon, unsigned char* scanline, unsigned char* precon, bool top, size_t bytewidth, unsigned long filterType, size_t length);
    void adam7Pass(unsigned char* out, unsigned char* in, unsigned char* scanlinen, unsigned char* scanlineo, size_t w, size_t bytewidth, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp);
    void resetParameters();
  };
  
  class Crc
  {
    private:
    unsigned long crc_table[256]; //Table of CRCs of all 8-bit messages.
    bool crc_table_computed; //Flag: has the table been computed? Initially false.
    
    //Make the table for a fast CRC.
    void make_crc_table(void);
    
    public:
    Crc();
    unsigned long update_crc(unsigned char *buf, unsigned long crc, size_t len);
    unsigned long crc(unsigned char *buf, size_t len);
  };
  
  class Adler32
  {
    private:
    const static int BASE = 65521; //largest prime smaller than 65536
    unsigned long update_adler32(unsigned long adler, const unsigned char *buf, size_t len) const;
    
    public:
    unsigned long adler32(const unsigned char *buf, size_t len) const;
  };
  
  class Encoder //unfinished
  {
    public:
    
    struct Settings
    {
      int btype; //currently only 0 "no compression" and 1 "fixed huffman" are supported
      bool useLZ77; //if false, no LZ77 is used. No LZ77 results in a larger file and is slower so you want this boolean to be true.
      int window_size; //the maximum is 32768. Smaller window sizes give faster LZ77 encoding but worse compression
      
      Settings();
    };

    void encode(std::vector<unsigned char>& out, const std::vector<unsigned char>& image, size_t w, size_t h);
    
    void setSettings(const Settings& settings); //to set the settings of the encoder, declare an Encoder::Settings, set its parameters, and then feed it to this function
    const Settings& getSettings() const;
    
    private:
    Settings settings;
    Crc crc;
    Adler32 adler32;
    
    unsigned long codetree[287][2]; //the code tree for Huffman codes

    void deflateNoCompression(const std::vector<unsigned char>& data, std::vector<unsigned char>& out);
    void deflateFixed(const std::vector<unsigned char>& data, std::vector<unsigned char>& out);
    void deflate(const std::vector<unsigned char>& data, std::vector<unsigned char>& out);
    void encodeZlib(const std::vector<unsigned char>& data, std::vector<unsigned char>& out);
    void addChunk(std::string chunkName, unsigned char* data, size_t length, std::vector<unsigned char>& out);
    void writeSignature(std::vector<unsigned char>& out);
    void writeChunk_tEXt(std::string keyword, std::string textstring, std::vector<unsigned char>& out);
    void writeChunk_IHDR(int w, int h, int bitDepth, int colorType, std::vector<unsigned char>& out);
    void writeChunk_IDAT(const std::vector<unsigned char>& data, std::vector<unsigned char>& out);
    void writeChunk_IEND(std::vector<unsigned char>& out);
    void dontFilter(const std::vector<unsigned char>& image, size_t w, size_t h, std::vector<unsigned char>& out);
    void filterScanline(const unsigned char* scanline, const unsigned char* prevline, unsigned char* out, bool top, size_t length, size_t bytewidth, int filterType);
    void filter(const std::vector<unsigned char>& image, size_t w, size_t h, std::vector<unsigned char>& out); //will filter scanline per scanline and add filter type in front
    void encodeNonCompressedRGBA(std::vector<unsigned char>& out, const std::vector<unsigned char>& image, size_t w, size_t h);
  };

} //end of namespace LodePNG

#endif


/*
LodePNG Documentation
---------------------

0. table of contents
--------------------

  1. about
   1.1. supported features
   1.2. features not supported
  2. classes
  3. decoder
   3.1. usage
   3.2. info values
   3.3. error values
  4. encoder
   4.1. usage
   4.2. error values
  5. file IO
   5.1. loadFile
   5.2. saveFile
  6. compiling
  7. examples
   7.1. decoder example
   7.2. SDL example
   7.3. encoder example
  8. changes
  9. contact information

1. about
--------

PNG is a file format to store raster images losslessly with good compression,
supporting different color types. It's patent-free. Because the compression is
lossless and it supports an alpha channel, PNG is a very good file format
to store textures and tilesets for computer games.

LodePNG is a PNG codec according to the Portable Network Graphics (PNG)
Specification (Second Edition) - W3C Recommendation 10 November 2003. The
specification can be found on line at http://www.w3.org/TR/2003/REC-PNG-20031110

The most recent version of LodePNG can currently be found at
http://student.kuleuven.be/~m0216922/pngloader/
However, this location is temporary and won't exist anymore in 2007.

LodePNG exists out of the source code files lodepng.cpp and lodepng.h

LodePNG is simple but only supports the basic requirements. In the tradeoff
between functionality and simplicity, LodePNG is on the side of simplicity.
To achieve this simplicity, the following design choices were made: There
are no dependencies on any external library whatsoever. To decode PNGs,
there's a Decoder class that can convert any PNG file data into an RGBA image
buffer with a single function call. To encode PNGs, there's an Encoder class
that can convert image data into PNG file data with a single function call.
To read and write files, a simple saveFile and loadFile function were added.

This all makes LodePNG suitable for loading textures in games, raytracers,
intros, or for loading images into programs that require them only for simple
usage. It's less suitable for full fledged image editors, loading PNGs over
network (since this decoder requires all the image data to be available before
the decoding can begin), life-critical systems, ... Even though it contains
a conformant decoder and encoder, it's still not a conformant editor,
because unknown chunks are simply discarded.

Note that while the decoder is in a mature state, the encoder is only
experimental and at the moment doesn't even use compression.

Because LodePNG is BSD licensed, it's allowed to use these source files in your
project, as long as you include the above copyright message and conditions
somewhere in the release of your program. You're not required to dynamically
link to it and the software doesn't have to be open source.

1.1. supported features
-----------------------

The following features are supported by the decoder:

*) conformant decoding of PNGs with any color type, bit depth and interlace mode
*) converting the result to 32-bit RGBA pixel data
*) loading the image from harddisk or decoding it from a buffer.
*) support for translucent PNG's, including translucent palettes and color key
*) zlib decompression (inflate)
*) 64 bit color (untested due to lack of such images)
*) the following chunks are interpreted by the decoder:
    IHDR (header information)
    PLTE (color palette)
    IDAT (pixel data)
    IEND (the final chunk)
    tRNS (transparency for palettized images)
    bKGD (suggested background color)
*) conformant encoding of PNGs, with LZ77 compression, but currently only with
    fixed huffman trees (no dynamic ones), and only 32-bit RGBA images.


1.2. features not supported
---------------------------

The following features are _not_ supported:

*) editing a PNG image (unless you use the decoder, then edit it, then use the
    encoder, but ignored chunks will then be gone from the original image)
*) encoding PNGs of another color type than 32-bit RGBA
*) CRC checks of chunks: they are ignored by the decoder for speed,
    but it's correctly generated by the encoder
*) ADLER32 checksum of zlib data: it's ignored by the decode,
    but it's correctly generated by the encoder
*) partial loading. All data must be available and is processed in one call.
*) The following optional chunk types are not interpreted by the decoder:
    cHRM (device independent color info)
    gAMA (device independent color info)
    iCCP (device independent color info)
    sBIT (original number of significant bits)
    sRGB (device independent color info)
    tEXt (textual information)
    zTXt (compressed textual information)
    iTXt (international textual information)
    hIST (palette histogram)
    pHYs (physical pixel dimensions)
    sPLT (suggested reduced palette)
    tIME (last image modification time)

2. classes
----------

The following classes currently exist in LodePNG:

*) Decoder: this is used to decode PNG images
*) Decoder::Info: this contains properties of a decoded image
*) Encoder: this is used to encode PNG images
*) Crc: this is used to generate CRC checks of chunks
*) Adler32: this is used to generate ADLER32 checksums of zlib data

There are also a few free functions:

*) loadFile: used to load a file into the std::vector<unsigned char> format used by LodePNG
*) saveFile: used to save a file from the std::vector<unsigned char> format used by LodePNG

3. decoder
----------

3.1. usage
----------

The basic steps of decoding a PNG file with LodePNG are as follows:

1) Load the PNG image file from harddisk and store the bytes of the file in an
 std::vector<unsigned char> with the same size as the file (you can use the
 LodePNG::loadFile function to load a file in such a vector)
2) Declare another std::vector<unsigned char> that will contain the pixel data.
3) Declare a LodePNG::Decoder object and call the decode32 function of it with
 the above vectors as parameters
4) Check for errors with the hasError() function of the Decoder.
5) If no error occurred, the raw pixel data is now stored in the output vector.
 The data can be used for example to create OpenGL textures and can be accessed
 as buffer with &out[0]. Use getWidth() and getHeight() of the Decoder to
 get the size of the texture.

A very basic usage example, and a second example showing how to load a PNG
image and show it in an SDL window, are demonstrated in two examples in a
section below.

To decode a PNG file, you can use one of the two decode functions of Decoder.
The function decode32 will store the pixel data in 32 bit RGBA format, no
matter what color format was used in the PNG file. The function decodeGeneric
will store the pixel data in the same color format as in the file. A more
detailed description of both functions, as well as the optional loadFile
function, is given below.

3.1.1. decode32
---------------

This function converts the given compressed PNG data into uncompressed 32-bit
RGBA pixel data, row by row. The pixel data is always in 32-bit RGBA, no matter
what color type was used in the PNG image. This makes it rather easy to
create textures or plot pixels from any kind of PNG file.

SIGNATURE:
void LodePNG::Decoder::decode32(std::vector<unsigned char>& out, std::vector<unsigned char>& in);

PARAMETERS:

in: This is used to give the original file data.
 Provide a vector filled with the contents of a PNG file. The contents of
 this buffer will be modified during the decoding process, so it cannot be 
 reused. If you need the file contents a second time, make a copy before
 decoding the PNG or reload the file.

out: This is used to return the pixel data.
 Provide an empty vector. The vector will be resized and the uncompressed 
 pixel data will be stored in the buffer. The pixel data will be of the RGBA
 format with 32 bits (4 bytes) per pixel. Pixels are stored row by row. The
 first four bytes of the data contain the four bytes of the pixel of the top 
 left of the image. The next four bytes contain the second pixel, and so on.
 The first byte of a pixel is the R component, the second the G component, the
 third the B component and the fourth the A component. The size of the out
 buffer will be the number of pixels in the image times four bytes.

ERROR CHECKING:
It sets hasError() to false if everything went ok, or true if an error
happened. If an error happened, the data stored in out and info output will
have no useful meaning. See a section below for the meaning of the error codes.
Note: error values are overwritten when a decoding function is called again.

GETTING OTHER INFORMATION:
The width and height of the image in pixels can be gotten as unsigned longs by
calling getWidth() and getHeight() of the Decoder.
Other information about the image can be gotten with LodePNG::getInfo(), see
the info values section below for more about this. This information usually
isn't needed when using decode32, since it converts the data to RGBA and the
original amount of color channels or bit depth then isn't relevant anymore.
Note: info values are overwritten when a decoding function is called again.


3.1.2. decodeGeneric
--------------------

This function converts the given compressed PNG data into uncompressed pixel
data. The original color type is preserved, which can make it more difficult
to interpret the pixel data since you have to take into account what color 
type it is.

For normal usage of LodePNG, this function is *not* recommended, but in special
cases where you know beforehand what color type the PNG image has it can provide
an increase in efficiency. Use decode32 instead if you want any type of PNG 
to be easily displayed! The decode32 function first calls decodeGeneric
and then does the job of converting all possible color types to RGBA.

SIGNATURE:
void LodePNG::Decoder::decodeGeneric(std::vector<unsigned char>& out, std::vector<unsigned char>& in);

PARAMETERS:

in: This is used to give the original file data.
 Provide a vector filled with the contents of a PNG file. The contents of 
 this buffer will be modified during the decoding process, so it cannot be 
 reused. If you need the file contents a second time, make a copy before
 decoding the PNG or reload the file.

out: This is used to return the pixel data.
 Provide an empty vector. The vector will be resized and the uncompressed 
 pixel data will be stored in the buffer. The image data will be of the same 
 color type as the PNG image. Pixels are always stored row by row. The first
 byte of the data contains the first byte of the pixel of the top left of
 the image. 1-bit images or other formats with less than one byte per pixel
 will be stored bit per bit. In that case, scanlines don't necessarily start 
 at the boundary of a  byte. If RGB or RGBA is used, color components per pixel
 are successive bits/bytes in the order R, G, B, A.

ERROR CHECKING:
It sets hasError() to false if everything went ok, or true if an error
happened. If an error happened, the data stored in out and info output will
have no useful meaning. See a section below for the meaning of the error codes.
Note: error values are overwritten when a decoding function is called again.

GETTING OTHER INFORMATION:
The width and height of the image in pixels can be gotten as unsigned longs by
calling getWidth() and getHeight() of the Decoder.
If you use decodeGeneric instead of decode32, other information about the
image, such as the amount of color channels, bit depth and maybe the palette is
required. This information can be gotten with LodePNG::getInfo(), see the info
values section below for more about this.
Note: info values are overwritten when a decoding function is called again.

3.2. info values
----------------

Information about the PNG image can be retrieved in the form of a struct of
type LodePNG::Info. This information can be gotten by calling the getInfo()
function of the Decoder.

The info struct is filled with information about the last image this Decoder
decoded. Usually, the only two values ever needed are the width and height of
the image. These values can also be gotten directly with the functions
getWidth() and getHeight() of the Decoder and are returned as unsigned longs.

The Info struct is overwritten everytime a new image is decoded by the same
Decoder. It contains all information set by the last decode, including the
error value, but not the input and output vectors as those are supplied as
parameters by reference to the decode functions.

Here's a list of each of the values stored in the Info struct:

*) w: width of the image in pixels
*) h: height of the image in pixels
*) bitDepth: bit depth: for indexed-color images, the number of bits per palette index. For other images, the number of bits per sample in the image.
*) colorType: The original color type: 0 = greyscale, 2 = RGB, 3 = indexed color (palette), 4 = greyscale with alpha, 6 = RGBA
*) bpp: bits per pixel
*) colorChannels: The amount of color channels, for example RGBA has 4 color channels
*) compressionMethod: The compression method used in the file, always 0
*) filterMethod: The filter method used in the file, always 0
*) interlaceMethod: The interlace method used in the original file, 0 = no interlace, 1 = Adam7 interlace
*) colorKey: If a color key is given. 0 if there's no color key, 1 if there's a color key. A color key is a color that will be drawn as invisible, and can only appear if the PNG image has no alpha channel.
*) keyR: the red component of the color key, or the color key value if the image is greyscale.
*) keyG: the green component of the color key
*) keyB: the blue component of the color key
*) backgroundColor: if suggested background color is given: 0 if none is given, 1 if one is given.
*) backgroundR: the red component of the suggested background color, or the value of the background color for greyscale or indexed color images.
*) backgroundG: the green component of the suggested background color
*) backgroundB: the blue component of the suggested background color
*) paletteSize: the size of the palette. 0 if there's no palette, a value from 2 to 256 if there's a palette.
*) palette: This is an std::vector<unsigned char> containing the palette values, in the order RGBARGBA... The size of the vector is info.paletteSize * 4

*) palette[0]: R component of palette color 0
*) palette[1]: G component of palette color 0 
*) palette[2]: B component of palette color 0
*) palette[3]: A component of palette color 0
...
*) palette[paletteSize - 4]: R component of last palette color
*) palette[paletteSize - 3]: G component of last palette color
*) palette[paletteSize - 2]: B component of last palette color
*) palette[paletteSize - 1]: A component of last palette color

*) error: the error value, more about this in the section "errors".

3.3. error values
-----------------

After using decodeGeneric or decode32, please always check the hasError()
function of the Decoder to see if everything went ok. Only if it's false,
it went ok and the output vector and info will contain meaningful data.

If something went wrong, hasError() will be true and you can use getError()
to get a value indicating the error. Below is a table that gives the meaning of
the different values.

The error values are set after using decodeGeneric or decode32. They are reset
when using those functions again to decode another PNG file.

*) 0: no error, everything went ok
*) 1: there is no PNG loaded yet, so it makes no sense trying to read any information values
*) 10: while huffman decoding: end of input memory reached without endcode
*) 11: while huffman decoding: error in code tree made it jump outside of tree
*) 13: problem while processing dynamic deflate block
*) 14: problem while processing dynamic deflate block
*) 15: problem while processing dynamic deflate block
*) 16: unexisting code while processing dynamic deflate block
*) 17: while inflating: end of out buffer memory reached
*) 18: while inflating: invalid distance code
*) 19: while inflating: end of out buffer memory reached
*) 20: invalid deflate block BTYPE encountered
*) 21: NLEN is not ones complement of LEN in a deflate block
*) 22: while inflating: end of out buffer memory reached.
   This can happen if the inflated deflate data is longer than the amount of bytes required to fill up
   all the pixels of the image, given the color depth and image dimensions. Something that doesn't
   happen in a normal, well encoded, PNG image.
*) 23: while inflating: end of in buffer memory reached
*) 24: invalid FCHECK in zlib header
*) 25: invalid compression method in zlib header
*) 26: FDICT encountered in zlib header while it's not used for PNG
*) 27: PNG file is smaller than a PNG header
*) 28: incorrect PNG signature (the first 8 bytes of the PNG file)
*) 29: first chunk is not the header chunk
*) 30: chunk length too large, chunk broken off at end of file
*) 31: illegal PNG color type
*) 32: illegal PNG compression method
*) 33: illegal PNG filter method
*) 34: illegal PNG interlace method
*) 35: chunk length of a chunk is too large or the chunk too small
*) 36: illegal PNG filter type encountered
*) 37: illegal bit depth for this color type given
*) 38: the palette is too big (more than 256 colors)
*) 39: more palette alpha values given in tRNS, than there are colors in the palette
*) 40: tRNS chunk has wrong size for greyscale image
*) 41: tRNS chunk has wrong size for RGB image
*) 42: tRNS chunk appeared while it was not allowed for this color type
*) 43: bKGD chunk has wrong size for palette image
*) 44: bKGD chunk has wrong size for greyscale image
*) 45: bKGD chunk has wrong size for RGB image
*) 46: value encountered in indexed image is larger than the palette size (bitdepth == 8)
*) 47: value encountered in indexed image is larger than the palette size (bitdepth < 8)
*) 48: the input data is empty. Maybe a PNG file you tried to load doesn't exist or is in the wrong path.
*) 49: jumped past memory while generating dynamic huffman tree
*) 50: jumped past memory while generating dynamic huffman tree
*) 51: jumped past memory while inflating huffman block
*) 52: jumped past memory while inflating
*) 53: size of zlib data too small to contain header
*) 54: CHAR_BITS is smaller than 8 on this platform, an 8-bit datatype is needed to store the file and pixel data
*) 55: jumped past tree while generating huffman tree, this could be when the code
       lengths are not of an optimal_ tree, which causes there to be more nodes than
       the arrays used can support (e.g. if there are 19 codes a tree has 18 nodes,
       but if all 19 code lengths are 7, there will be much more nodes), this may be a
       BUG that has to be fixed though I haven't actually encountered PNGs causing
       this problem outside the experimentation environment
*) 56: As of LodePNG version 20061209, the input and output parameter of the decode functions got swapped!
       Try to swap the input and output parameter in the decode function call. (first out, then in)
       If they are already correct and you get this error, then see description of error 48.

4. encoder
----------

I'm working on this encoder. It already produces much smaller files than a while ago, because LZ77 encoding is now implemented.

Next on the planning is: dynamic huffman trees.

The encoder currently produces a valid PNG, with a size much smaller than an uncompressed image, but up to 5 times as big
as one encoded by libPNG.

For large images, it's very slow at the moment. But, compiling with optimization (-O3) makes it 12 times faster than without on
my system.

4.1. usage
----------

Please see the encoder example in a section below.

To encode data, use the "encode" function. It takes as parameters:

*) std::vector<unsigned char>& out: this will contain the PNG data after encoding
*) const std::vector<unsigned char>& image: this must contain 4 * w * h values,
    corresponding to the RGBA pixel data of a raw image. The image must be in the
    vector row by row, and the color channels in the order RGBA.
*) size_t w, size_t h: the width and height of the image you provide, in pixels.

To change the settings of the encoder, declare an Encoder::Settings, set the
parameters to your liking, and give it to the encoder using setSettings,
before using encode.

The settings affect the deflate algorithm and are as follows:

*) btype: The type of compression: 
    0 = no compression (results in large, uncompressed image files)
    1 = fixed Huffman
    dynamic Huffman is currently not yet supported.
*) useLZ77: whether or not to use the LZ77 algorithm. If you don't use the LZ77
    algorithm, then only "literal values" are used and no length/distance
    codes. Not using LZ77 makes the algorithm slower and results in a file
    that is larger than the uncompressed one. Normally you want useLZ77 to be
    true, to get compression and faster encoding.
*) window_size: the window size to be used by the LZ77 encoder. Smaller window
    size results in faster encoding but less compression, larger window size
    results in slower encoding but better compression. The maximul value
    for window_size allowed by the PNG specification, is 32768.
    
Warning: like most compression algorithms, this compression reduces redundancy
in the data. As a result, the effects of data corruption on an encoded
image will be more severe than on for example an uncompressed text file, that
will still be readable apart from the corrupted bytes.

4.2. error values
-----------------

No errors are created by the encoder yet. This, because it should be able
to encode any raw image into a PNG without complaining.

Beware though that currently no error is created yet when trying to compress
with an illegal window size that is larger than 32768.

Also beware that no error is created yet when the width and height you give
to the encoder are incorrect compared to the length of the vector you give.


5. file IO
----------

5.1. loadFile
-------------

This function was added for convenience. It can read a file with given filename
and store it in the given std::vector<unsigned char> buffer. This vector can
then be given as input to the decodeGeneric or decode32 function.

It's not required to use this function, you can also obtain the file data in
an std::vector<unsigned char> in any other way. If you use special file
formats from which PNG file data has to be extracted, loadFile won't be useful.

This function can read any file, be it a PNG file or another type of file, but
the buffer can only successfully be decoded by LodePNG if it's a PNG file.

SIGNATURE:
void LodePNG::loadFile(std::vector<unsigned char>& buffer, const std::string filename);

PARAMETERS:

filename: The filename or full path of the file you want to read.

buffer: Provide an empty vector. It will be resized to the size of the file
 and then be filled with the bytes read from the file.

ERROR CHECKING:
No errors are handled by this function, if the file doesn't exist the buffer
vector will be empty after using the function, and when used on a PNG Decoder
it will most likely generate an error number 48 when trying to decode this
empty buffer.

5.2. saveFile
-------------

This function was added for convenience. It can save the given
std::vector<unsigned char> buffer in a file with given filename. This can be
used to write the result of the encoder into a file.

Warning: If the file already exists, it'll be overwritten and whatever was in it will
be lost.

SIGNATURE:
void saveFile(std::vector<unsigned char>& buffer, const std::string& filename);

PARAMETERS:

filename: The filename or full path of the file you want to write.

buffer: The buffer containing the data to be written in the file.

ERROR CHECKING:
No errors are handled by this function.

6. compiling
------------

LodePNG is designed for standard C++. Some old compilers that do not correctly
support standard C++ may not be able to compile LodePNG correctly. Please use
a compiler that supports the current standard.

No libraries other than the standard C++ library are needed to compile LodePNG.
Add lodepng.cpp and lodepng.h to the collection of source code files of your
project, #include the lodepng.h header where needed, and your program can
decode PNG files.

Letting the compiler use optimization makes this PNG decoder significantly
faster than without optimization, and the encoder even more. In g++, use
the -O3 option. Most IDEs provide options for enabling optimization. A test
of decoding a 1729x1307 PNG photo resulted in 1.6 seconds without -O3, 0.66
seconds with -O3 on an AthlonXP 1700+.

7. examples
-----------

These examples demonstrate LodePNG and can be used as test. Place the example
code in a new .cpp file, and compile together with lodepng.cpp and lodepng.h,
with optimization.

The first example decodes the image and displays the size in the console.
The second example will decode the image and show the image in an SDL window,
provided this library (Simple DirectMedia Layer) is available on the system.

Usage: ./a.out FILENAME


7.1. console example
--------------------

////////////////////////////////////////////////////////////////////////////////
// LodePNG console example
// This example decodes a PNG file and displays its info in the console
// g++ *.cpp -O3
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include "lodepng.h"

int main(int argc, char *argv[])
{
  LodePNG::Decoder decoder; //declare the decoder
  std::vector<unsigned char> buffer; //this will contain the file
  std::vector<unsigned char> image; //this will contain the pixel data
  
  std::string filename = argc > 1 ? argv[1] : "test.png";
  LodePNG::loadFile(buffer, filename); //load the image file with given filename
  
  decoder.decode32(image, buffer); //decode the PNG
  
  //if there's an error, display it, otherwise display width and height of the image
  if(decoder.hasError()) std::cout << "error: " << decoder.getError() << std::endl;
  else std::cout << "\n" <<
    "w: " << decoder.getInfo().w << "\n" <<
    "h: " << decoder.getInfo().h << "\n" <<
    "bitDepth: " << decoder.getInfo().bitDepth << "\n" <<
    "bpp: " << decoder.getInfo().bpp << "\n" <<
    "colorChannels: " << decoder.getInfo().colorChannels << "\n" <<
    "paletteSize: " << decoder.getInfo().paletteSize << "\n" <<
    "colorType: " << decoder.getInfo().colorType << "\n" <<
    "compressionMethod: " << decoder.getInfo().compressionMethod << "\n" <<
    "filterMethod: " << decoder.getInfo().filterMethod << "\n" <<
    "interlaceMethod: " << decoder.getInfo().interlaceMethod << "\n";
  
  //the raw pixel data is now stored in the image vector
  
  return 0;
}
////////////////////////////////////////////////////////////////////////////////


7.2. SDL example
----------------

////////////////////////////////////////////////////////////////////////////////
// LodePNG SDL example
// This example displays a PNG with a checkerboard pattern to show tranparency
// It requires the SDL library to compile and run
// g++ *.cpp -lSDL -O3
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <SDL/SDL.h> //requires SDL
#include "lodepng.h"

int main(int argc, char *argv[])
{
  LodePNG::Decoder decoder; //declare the decoder
  std::vector<unsigned char> buffer; //this will contain the file
  std::vector<unsigned char> image; //this will contain the pixel data
  
  std::string filename = argc > 1 ? argv[1] : "test.png";
  LodePNG::loadFile(buffer, filename); //load the image file with given filename
  
  decoder.decode32(image, buffer); //decode the png
  
  //get width and height
  unsigned long w = decoder.getWidth(), h = decoder.getHeight();

  //stop if there is an error
  if(decoder.hasError())
  {
    std::cout << "error: " << decoder.getError() << std::endl;
    return -1;
  }
  
  //avoid too large window size by downscaling large image
  unsigned long jump = 1;
  if(w / 1024 >= jump) jump = w / 1024 + 1;
  if(h / 1024 >= jump) jump = h / 1024 + 1;
  
  //init SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
  SDL_Surface* scr = SDL_SetVideoMode(w / jump, h / jump, 32, SDL_HWSURFACE);
  if(!scr) return -1;
  SDL_WM_SetCaption(filename.c_str(), NULL); //set window caption
  
  //plot the pixels of the PNG file
  for(unsigned long y = 0; y + jump - 1 < h; y += jump)
  for(unsigned long x = 0; x + jump - 1 < w; x += jump)
  {
    //get RGBA components
    Uint32 r = image[4 * y * w + 4 * x + 0]; //red
    Uint32 g = image[4 * y * w + 4 * x + 1]; //green
    Uint32 b = image[4 * y * w + 4 * x + 2]; //blue
    Uint32 a = image[4 * y * w + 4 * x + 3]; //alpha
    
    //make translucency visible by placing checkerboard pattern behind image
    int checkerColor = 191 + 64 * (((x / 16) % 2) == ((y / 16) % 2));
    r = (a * r + (255 - a) * checkerColor) / 255;
    g = (a * g + (255 - a) * checkerColor) / 255;
    b = (a * b + (255 - a) * checkerColor) / 255;
    
    //give the color value to the pixel of the screenbuffer
    Uint32* bufp;
    bufp = (Uint32 *)scr->pixels + (y * scr->pitch / 4) / jump + (x / jump);
    *bufp = 65536 * r + 256 * g + b;
  }
  
  //pause until you press escape and meanwhile redraw screen
  SDL_Event event;
  int done = 0;
  while(done == 0)
  {
    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT) done = 1;
      if(SDL_GetKeyState(NULL)[SDLK_ESCAPE]) done = 1;
    }
    SDL_UpdateRect(scr, 0, 0, 0, 0); //redraw screen
    SDL_Delay(5); //pause 5 ms so it consumes less processing power
  }
  
  SDL_Quit();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////

7.3. encoder example
--------------------

////////////////////////////////////////////////////////////////////////////////
// LodePNG Encoder example
// This example encodes a PNG containing a XOR texture
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include "lodepng.h"

int main(int argc, char *argv[])
{
  //check if user gave a filename
  if(argc <= 1)
  {
    std::cout << "please provide a filename to save to\n";
    return 0;
  }
  
  //generate some image
  const int sizex = 512;
  const int sizey = 512;
  std::vector<unsigned char> image;
  image.resize(sizex * sizey * 4);
  for(int y = 0; y < sizey; y++)
  for(int x = 0; x < sizex; x++)
  {
    image[4 * sizex * y + 4 * x + 0] = 255 * !(x & y);
    image[4 * sizex * y + 4 * x + 1] = x ^ y;
    image[4 * sizex * y + 4 * x + 2] = x | y;
    image[4 * sizex * y + 4 * x + 3] = 255;
  }
  
  //encode as PNG
  LodePNG::Encoder encoder;
  std::vector<unsigned char> out;
  encoder.encode(out, image, sizex, sizey);

  //save
  LodePNG::saveFile(out, argv[1]);
}

////////////////////////////////////////////////////////////////////////////////

8. changes
----------

The version number of LodePNG is the date of the change given in the format
yyyymmdd. Multiple changes on the same date may have the same version number.
Minor changes aren't mentioned here but may still have a different version 
number.

Some changes aren't backwards compatible. Those are indicated with a (!)
symbol. Those changes may require you to change your code when upgrading
from an older version. They are usually for the better.

*) 28 dec 2006: Added "Settings" to the encoder.
*) 26 dec 2006: The encoder now does LZ77 encoding and produces much smaller files now.
    Removed some code duplication in the decoder. Fixed little bug in an example.
*) 09 dec 2006: (!) Placed output parameters of public functions as first parameter.
    Fixed a bug with 16-bit per color.
    The encoder is still experimental and a work in progress!
*) 15 okt 2006: Changed documentation structure
*) 09 okt 2006: Encoder class added. It encodes a valid PNG image from the
    given image buffer, however for now it's not compressed.
*) 01 okt 2006: (!) Info class definition put in Decoder class
*) 08 sep 2006: (!) Changed to interface with a Decoder class
*) 30 jul 2006: (!) Info , width and height are now retrieved in different
    way. Renamed decodePNG to decodePNGGeneric.
*) 29 jul 2006: (!) Changed the interface: image info is now returned as a
    struct of type LodePNG::Info, instead of a vector, which was a bit clumsy.
*) 28 jul 2006: Cleaned the code and added new error checks.
    Corrected terminology "deflate" into "inflate".
*) 23 jun 2006: Added SDL example in the documentation in the
    header, this example allows easy debugging by displaying the PNG and its
    transparency.
*) 22 jun 2006: (!) Changed way to obtain error value. Added
    loadFile function for convenience. Made decodePNG32 faster.
*) 21 jun 2006: (!) Changed type of info vector to unsigned long.
    Changed position of palette in info vector. Fixed an important bug that
    happened on PNGs with an uncompressed block.
*) 16 jun 2006: Internally changed int into unsigned long where
    needed, and performed some optimizations.
*) 07 jun 2006: (!) Renamed functions to decodePNG and placed them
    in LodePNG namespace. Changed the order of the parameters. Rewrote the
    documentation in the header. Renamed files to lodepng.cpp and lodepng.h
*) 22 apr 2006: Optimized and improved some code
*) 07 sep 2005: (!) Changed to std::vector interface
*) 12 aug 2005: Initial release


9. contact information
-----------------------

Feel free to contact me with suggestions, problems, comments, ... concerning 
LodePNG. If you encounter a PNG image that doesn't work properly with this
decoder, feel free to send it and I'll use it to find and fix the problem.

My address in hexadecimal ASCII code is
6c:6f:64:65:2e:76:61:6e:64:65:76:65:6e:6e:65:40:67:6d:61:69:6c:2e:63:6f:6d


Copyright (c) 2005-2006 Lode Vandevenne
*/

