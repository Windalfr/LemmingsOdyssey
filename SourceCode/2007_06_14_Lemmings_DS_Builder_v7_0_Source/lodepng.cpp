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

//The manual and changelog can be found in the header file "lodepng.h"

#include "lodepng.h"

#include <vector>
#include <string>
#include <fstream>
#include <climits>

#include <iostream>

namespace LodePNG
{

//////////////////////////////////////////////////////////////////////////////
// File IO                                                                  //
//////////////////////////////////////////////////////////////////////////////

void loadFile(std::vector<unsigned char>& buffer, const std::string& filename)
{
  std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

  //get filesize
  if(!file.seekg(0, std::ios::end).good()) return;
  size_t size = file.tellg();
  if(!file.seekg(0, std::ios::beg).good()) return;
  size -= file.tellg();

  //read contents of the file into the vector
  buffer.resize(size);
  file.read((char*)(&buffer[0]), size);
  file.close();
}

//write given buffer to the file, overwriting the file, it doesn't append to it.
void saveFile(const std::vector<unsigned char>& buffer, const std::string& filename)
{
  std::ofstream file(filename.c_str(), std::ios::out|std::ios::binary);
  file.write((char*)&buffer[0], buffer.size());
  file.close();
}

//////////////////////////////////////////////////////////////////////////////
// Private, Non-Class Functions And Data                                    //
//////////////////////////////////////////////////////////////////////////////

static const unsigned long lengthbase[29] //the base lenghts represented by codes 257-285
  = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const unsigned long lengthextra[29] //the extra bits used by codes 257-285 (added to base length)
  = {0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,   4,   5,   5,   5,   5,   0};
static const unsigned long distancebase[30] //the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)
  = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const unsigned long distanceextra[30] //the extra bits of backwards distances (added to base)
  = {0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,   8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13};
static const unsigned long clcl[19] //the order in which "code length alphabet code lengths" are stored, out of this the huffman tree of the dynamic huffman tree lengths is generated
  = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15}; 

//Paeth predicter, used by one of the PNG filter types
inline signed long paethPredictor(signed long a, signed long b, signed long c)
{
  signed long p = a + b - c;
  signed long pa = p > a ? p - a : a - p;
  signed long pb = p > b ? p - b : b - p;
  signed long pc = p > c ? p - c : c - p;
  
  if(pa <= pb && pa <= pc) return a;
  else if(pb <= pc) return b;
  return c;
}

//get the tree of a deflated block with fixed tree, as specified in the deflate specification
inline void generateFixedTreeLengths(unsigned long* fixedTreeLength)
{
  //288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused
  for(size_t i =   0; i <= 143; i++) fixedTreeLength[i] = 8;
  for(size_t i = 144; i <= 255; i++) fixedTreeLength[i] = 9;
  for(size_t i = 256; i <= 279; i++) fixedTreeLength[i] = 7;
  for(size_t i = 280; i <= 287; i++) fixedTreeLength[i] = 8;
}

inline void generateDistanceTreeLengths(unsigned long* distanceTreeLength)
{
  //there are 32 distance codes, but 30-31 are unused
  for(size_t i = 0; i < 32; i++) distanceTreeLength[i] = 5;
}

//generate Huffman code tree out of given code lengths
void genCodetree(unsigned long* tree, const unsigned long* bitlen, size_t numcodes, size_t maxbitlen)
{
  std::vector<unsigned long> blcount_v(17); unsigned long* blcount = &blcount_v[0];
  std::vector<unsigned long> nextcode_v(17); unsigned long* nextcode = &nextcode_v[0];
  //std::vector<unsigned long> temptree_v(numcodes); unsigned long* temptree = &temptree_v[0];
  
  //step 1: count number of instances of each code length
  for(size_t bits = 0; bits < maxbitlen; bits++) blcount[bits] = nextcode[bits] = 0; //initialize all to 0 so that unencountered counts will be 0
  for(size_t bits = 0; bits < numcodes; bits++) blcount[bitlen[bits]]++; //fill in the bit length counts
  
  //step 2: generate the nextcode values
  unsigned long code = 0;
  for(size_t bits = 1; bits <= maxbitlen; bits++)
  {
    code = (code + blcount[bits - 1]) << 1;
    nextcode[bits] = code;
  }
  
  //step 3: generate all the codes
  for(size_t n = 0; n < numcodes; n++) if(bitlen[n] != 0) tree[n] = nextcode[bitlen[n]]++;
}

//////////////////////////////////////////////////////////////////////////////
// Reading and writing single bits to byte stream                           //
//////////////////////////////////////////////////////////////////////////////

void addBitToStream(size_t& bitpointer, std::vector<unsigned char>& bitstream, unsigned long bit)
{
  if(bitpointer % 8 == 0) bitstream.push_back(0); //add a new byte at the end
  unsigned long shiftedbit = (bit << (bitpointer % 8)); //earlier bit of huffman code is in a lesser significant bit of an earlier byte
  bitstream.back() |= shiftedbit;
  bitpointer++;
}

void addBitsToStream(size_t& bitpointer, std::vector<unsigned char>& bitstream, unsigned long value, size_t nbits)
{
  for(size_t j = 0; j < nbits; j++)
  {
    unsigned long bit = (value >> j) & 1;
    addBitToStream(bitpointer, bitstream, bit);
  }
}

void addBitsToStreamReversed(size_t& bitpointer, std::vector<unsigned char>& bitstream, unsigned long value, size_t nbits)
{
  for(size_t j = 0; j < nbits; j++)
  {
    unsigned long bit = (value >> (nbits - 1 - j)) & 1;
    addBitToStream(bitpointer, bitstream, bit);
  }
}

unsigned long readBitFromStream(size_t& bitpointer, const unsigned char* bitstream)
{
  unsigned long bit = (bitstream[bitpointer >> 3] >> (bitpointer & 0x7)) & 1;
  bitpointer++;
  return bit;
}

unsigned long readBitsFromStream(size_t& bitpointer, const unsigned char* bitstream, int nbits)
{
  unsigned long pot = 1; //power of two
  unsigned long result = 0;
  for(int i = 0; i < nbits; i++)
  {
    unsigned long bit = readBitFromStream(bitpointer, bitstream);
    result += bit * pot;
    pot *= 2;
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////////
// Decoder                                                                  //
//////////////////////////////////////////////////////////////////////////////

int make2DTree(unsigned long* tree2d, const unsigned long* tree1d, const unsigned long* bitlen, size_t numcodes)
{
  //convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as many columns as codes - 1
  //a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what their 0 and 1 option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen
  for(size_t n = 0;  n < (numcodes - 1); n++) tree2d[2 * n + 0] = tree2d[2 * n + 1] = 32767; //32767 here means the tree2d isn't filled there yet
  size_t nodefilled = 0; //up to which node it is filled
  size_t treepos = 0; //position in the tree (1 of the numcodes columns)
  
  for(size_t n = 0; n < numcodes; n++) //the codes
  for(size_t i = 0; i < bitlen[n]; i++) //the bits for this code
  {
    unsigned char bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;
    if(treepos > numcodes - 2) { return 55; } //error 55: see description in header
    if(tree2d[2 * treepos + bit] == 32767) //not yet filled in
    {
      if(i + 1 == bitlen[n]) //last bit
      {
        tree2d[2 * treepos + bit] = n; //put the current code in it
        treepos = 0;
      }
      else //put address of the next step in here, first that address has to be found of course (it's just nodefilled + 1)...
      {
        nodefilled++;
        tree2d[2 * treepos + bit] = nodefilled + numcodes; //addresses encoded with numcodes added to it
        treepos = nodefilled;
      }
    }
    else treepos = tree2d[2 * treepos + bit] - numcodes;
  }

  return 0;
}

//decode a single symbol from given list of bits with given code tree
unsigned long Decoder::huffmanDecodeSymbol(unsigned char* in, size_t& bp, unsigned long* codetree, size_t numcodes, size_t inlength)
{
  size_t treepos = 0, p_byte = bp >> 3, p_bit = bp & 0x7, byte;
  
  if(p_byte > inlength) { info.error = 10; return 0; } //error: end of input memory reached without endcode
  byte = in[p_byte];

  while(1)
  {
    if(p_bit == 8)
    {
      p_bit = 0;
      p_byte = bp >> 3; 
      if(p_byte > inlength) { info.error = 10; return 0; } //error: end of input memory reached without endcode
      byte = in[p_byte];
    }
    
    unsigned long bit = (byte >> p_bit) & 1;
    
    p_bit++;
    bp++;
    
    unsigned long ct = codetree[2 * treepos + bit];
    
    if(ct < numcodes) { return ct; }
    else treepos = ct - numcodes;
    
    if(treepos >= numcodes) { info.error = 11; return 0; } //error: you appeared outside the codetree
  }
}

//get the tree of a deflated block with fixed tree, as specified in the deflate specification
void Decoder::getTreeInflateFixed(unsigned long* tree, unsigned long* treeD)
{
  std::vector<unsigned long> temptree(288);
  std::vector<unsigned long> bitlen(288);
  generateFixedTreeLengths(&bitlen[0]);
  genCodetree(&temptree[0], &bitlen[0], 288, 16); //tree for symbol codes
  info.error = make2DTree(tree, &temptree[0], &bitlen[0], 288);
  
  std::vector<unsigned long> temptreeD(32);
  std::vector<unsigned long> bitlenD(32);
  generateDistanceTreeLengths(&bitlenD[0]);
  genCodetree(&temptreeD[0], &bitlenD[0], 32, 16); //tree for distance codes
  info.error = make2DTree(treeD, &temptreeD[0], &bitlenD[0], 32);
}

//get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
void Decoder::getTreeInflateDynamic(unsigned long* tree, unsigned long* treeD, unsigned char* in, size_t& bp, size_t inlength)
{
  size_t HLIT = 257; //number of literal/length codes. Unlike the spec, the value 257 is added to it here already
  std::vector<unsigned long> bitlen_v(288);
  unsigned long* bitlen = &bitlen_v[0];
  std::vector<unsigned long> bitlenD_v(32);
  unsigned long* bitlenD = &bitlenD_v[0];
  
  if(bp >> 3 >= inlength - 2) { info.error = 49; return; } //the bit pointer is or will go past the memory
  
  HLIT += readBitsFromStream(bp, in, 5);
  
  size_t HDIST = 1; //number of distance codes. Unlike the spec, the value 1 is added to it here already

  HDIST += readBitsFromStream(bp, in, 5);


  size_t HCLEN = 4; //number of code length codes. Unlike the spec, the value 4 is added to it here already
  
  HCLEN += readBitsFromStream(bp, in, 4);
  
  //read the code length codes out of 3 * (amount of code length codes) bits
  for(size_t i = 0; i < 19; i++)
  {
    codelengthcode[clcl[i]] = 0;
    if(i < HCLEN) //if not, it must stay 0
    {
      codelengthcode[clcl[i]] += readBitsFromStream(bp, in, 3);
    }
  }
  
  std::vector<unsigned long> temptreeDD(19);
  genCodetree(&temptreeDD[0], codelengthcode, 19, 7); //tree for distance codes
  info.error = make2DTree(codelengthcodetree[0], &temptreeDD[0], codelengthcode, 19);
  if(hasError()) return;
  
  //now we can use this tree to read the lengths for the tree that this function will return
  size_t i = 0;
  while(i < HLIT + HDIST) //i is the current symbol we're reading in the part that contains the code lengths of lit/len codes and dist codes
  {
    unsigned long code = huffmanDecodeSymbol(in, bp, codelengthcodetree[0], 19, inlength);
    if(hasError()) return;
    
    if(code <= 15) //a length code
    {
      if(i < HLIT) bitlen[i] = code;
      else bitlenD[i - HLIT] = code;
      i++;
    }
    else if(code == 16) //repeat previous
    {
      size_t replength = 3; //read in the 2 bits that indicate repeat length (3-6)
      if(bp >> 3 >= inlength) { info.error = 50; return; } //error, bit pointer jumps past memory
      
      replength += readBitsFromStream(bp, in, 2);
      
      unsigned long value; //set value to the previous code
      if((i - 1) < HLIT) value = bitlen[i - 1];
      else value = bitlenD[i - HLIT - 1];
      //repeat this value in the next lengths
      for(size_t n = 0; n < replength; n++)
      {
        if(i >= HLIT + HDIST) { info.error = 13; return; } //error: i is larger than the amount of codes
        if(i < HLIT) bitlen[i] = value;
        else bitlenD[i - HLIT] = value;
        i++;
      }
    }
    else if(code == 17) //repeat "0" 3-10 times
    {
      size_t replength = 3; //read in the bits that indicate repeat length
      if(bp >> 3 >= inlength) { info.error = 50; return; } //error, bit pointer jumps past memory

      replength += readBitsFromStream(bp, in, 3);
      
      //repeat this value in the next lengths
      for(size_t n = 0; n < replength; n++)
      {
        if(i >= HLIT + HDIST) { info.error = 14; return; } //error: i is larger than the amount of codes
        if(i < HLIT) bitlen[i] = 0;
        else bitlenD[i - HLIT] = 0;
        i++;
      }
    }
    else if(code == 18) //repeat "0" 11-138 times
    {
      size_t replength = 11; //read in the bits that indicate repeat length

      if(bp >> 3 >= inlength) { info.error = 50; return; } //error, bit pointer jumps past memory

      replength += readBitsFromStream(bp, in, 7);
      
      //repeat this value in the next lengths
      for(size_t n = 0; n < replength; n++)
      {
        if(i >= HLIT + HDIST) { info.error = 15; return; } //error: i is larger than the amount of codes
        if(i < HLIT) bitlen[i] = 0;
        else bitlenD[i - HLIT] = 0;
        i++;
      }
    }
    else { info.error = 16; return; } //error: somehow an unexisting code appeared. This can never happen.
  }
  
  //the other bitlen en bitlenD values must be 0, or a wrong tree will be generated
  for(size_t i = HLIT; i < 288; i++) bitlen[i] = 0;
  for(size_t i = HDIST; i < 32; i++) bitlenD[i] = 0;
  
  //now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
  std::vector<unsigned long> temptree(288);
  genCodetree(&temptree[0], bitlen, 288, 16); //tree for distance codes
  info.error = make2DTree(tree, &temptree[0], bitlen, 288);
  if(hasError()) return;
  
  std::vector<unsigned long> temptreeD(32);
  genCodetree(&temptreeD[0], bitlenD, 32, 16); //tree for distance codes
  info.error = make2DTree(treeD, &temptreeD[0], bitlenD, 32);
  if(hasError()) return;
}

//inflate a block with dynamic of fixed Huffman tree
void Decoder::inflateHuffmanBlock(unsigned char* out, unsigned char* in, size_t& bp, size_t& pos, size_t outlength, size_t inlength, unsigned long btype) 
{
  if(btype == 1) getTreeInflateFixed(codetree[0], codetreeD[0]);
  if(btype == 2) getTreeInflateDynamic(codetree[0], codetreeD[0], in, bp, inlength);
  
  if(hasError()) return;
  
  bool endreached = false;
  while(!endreached)
  {
    unsigned long code = huffmanDecodeSymbol(in, bp, codetree[0], 288, inlength);
    if(hasError()) return; //some error happened in the above function
    if(code == 256) //end code
    {
      endreached = true;
    }
    else if(code <= 255) //literal symbol
    {
      if(pos >= outlength) { info.error = 17; return; } //error: end of out buffer memory reached
      out[pos] = (unsigned char)(code);
      pos++;
    }
    else if(code >= 257 && code <= 285) //length code
    {
      //part 1: get length base
      size_t length = lengthbase[code - 257];
      
      //part 2: get extra bits and add the value of that to length
      size_t numextrabits = lengthextra[code - 257];
      if(bp >> 3 >= inlength) { info.error = 51; return; } //error, bit pointer will jump past memory
      length += readBitsFromStream(bp, in, numextrabits);
      
      //part 3: get distance code
      unsigned long codeD = huffmanDecodeSymbol(in, bp, codetreeD[0], 32, inlength);
      if(hasError()) return;
      if(codeD > 29) { info.error = 18; return; } //error: invalid distance code (30-31 are never used)
      unsigned long distance = distancebase[codeD];
      
      //part 4: get extra bits from distance
      unsigned long numextrabitsD = distanceextra[codeD];

      if(bp >> 3 >= inlength) { info.error = 51; return; } //error, bit pointer will jump past memory
      distance += readBitsFromStream(bp, in, numextrabitsD);
      
      //part 5: fill in all the out[n] values based on the length and dist
      size_t start = pos;
      size_t backward = start - distance;
      for(size_t forward = 0; forward < length; forward++)
      {
        if(pos >= outlength) { info.error = 19; return; } //error: end of out buffer memory reached
        out[pos] = out[backward];
        pos++;
        backward++;
        if(backward >= start) backward = start - distance;
      }
    }
  }
}

//inflate the deflated data (cfr. deflate spec)
void Decoder::inflate(unsigned char* out, unsigned char* in, size_t inlength, size_t outlength)
{
  size_t bp = 0; //bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)
  unsigned long BFINAL = 0;
  size_t pos = 0; //byte position in the out buffer
  
  while(!BFINAL)
  {
    if(bp >> 3 >= inlength) { info.error = 52; return; } //error, bit pointer will jump past memory
    BFINAL = readBitFromStream(bp, in);
    unsigned long BTYPE = 1 * readBitFromStream(bp, in) + 2 * readBitFromStream(bp, in);

    if(BTYPE == 3) { info.error = 20; return; } //error: invalid BTYPE
    else if(BTYPE == 0) //no compression
    {
      //go to first boundary of byte
      while((bp & 0x7) != 0) bp++;
      
      //read LEN (2 bytes) and NLEN (2 bytes)
      if(bp >> 3 >= inlength - 4) { info.error = 52; return; } //error, bit pointer will jump past memory
      unsigned long LEN = in[bp >> 3] + 256 * in[(bp >> 3) + 1]; bp += 16;
      unsigned long NLEN = in[bp >> 3] + 256 * in[(bp >> 3) + 1]; bp += 16;
      
      //check if 16-bit NLEN is really the one's complement of LEN
      if(LEN + NLEN != 65535) { info.error = 21; return; } //error: NLEN is not one's complement of LEN

      //read the literal data: LEN bytes are now stored in the out buffer
      for(unsigned long n = 0; n < LEN; n++)
      {
        if(pos >= outlength) { info.error = 22; return; } //error: reading outside of out buffer
        if((bp >> 3) > inlength) { info.error = 23; return; } //error: reading outside of in buffer
        out[pos] = in[bp >> 3]; bp += 8;
        pos++;
      }
    }
    else //compression, BTYPE 01 or 10
    {
      inflateHuffmanBlock(out, in, bp, pos, outlength, inlength, BTYPE);
      if(hasError()) return;
    }
  }
}

//decompress the zlib data of the PNG
void Decoder::decodeZlib(unsigned char* out, unsigned char* in, size_t outlength, size_t inlength)
{
  if(inlength < 2) { info.error = 53; return; } //error, size of zlib data too small to contain header
  //read information from zlib header
  if((in[0] * 256 + in[1]) % 31 != 0) { info.error = 24; return; } //error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way

  unsigned long CM = in[0] & 15;
  unsigned long CINFO = (in[0] >> 4) & 15;
  //unsigned long FCHECK = in[1] & 31;
  unsigned long FDICT = (in[1] >> 5) & 1;
  //unsigned long FLEVEL = (in[1] >> 6) & 3;
  //unsigned long ADLER32 = 16777216 * in[inlength - 4] + 65536 * in[inlength - 3] + 256 * in[inlength - 2] + in[inlength - 1];
  
  if(CM != 8 || CINFO > 7) { info.error = 25; return; } //error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
  if(FDICT != 0) { info.error = 26; return; } //error: the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
  
  inflate(out, &in[2], inlength - 2, outlength);
  if(hasError()) return;
}

//read the information from the header and store it in the Info
void Decoder::readPngHeader(unsigned char* in, size_t inlength)
{
  if(inlength < 29) { info.error = 27; return; } //error: the data length is smaller than the length of the header

  if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) { info.error = 28; return; } //error: the first 8 bytes are not the correct PNG signature
  if(in[12] != 73 || in[13] != 72 || in[14] != 68 || in[15] != 82) { info.error = 29; return; } //error: it doesn't start with a IHDR chunk!
  
  //read the values given in the header
  info.w = 256 * 256 * 256 * in[16] + 256 * 256 * in[17] + 256 * in[18] + in[19];
  info.h = 256 * 256 * 256 * in[20] + 256 * 256 * in[21] + 256 * in[22] + in[23];
  info.bitDepth = in[24];
  info.colorType = in[25];
  info.compressionMethod = in[26];
  info.filterMethod = in[27];
  info.interlaceMethod = in[28];
  //The 4 CRC bytes are ignored
  
  if(info.compressionMethod != 0) { info.error = 32; return; } //error: only compression method 0 is allowed in the specification
  if(info.filterMethod != 0)      { info.error = 33; return; } //error: only filter method 0 is allowed in the specification
  if(info.interlaceMethod > 1)    { info.error = 34; return; } //error: only interlace methods 0 and 1 exist in the specification
  
  //check if bit depth is valid for this color type, if not give error 37 "illegal bit depth for this color type given"
  unsigned long& bd = info.bitDepth; //longer variable name makes code below more easily readable
  switch(info.colorType)
  {
    case 0: if(bd != 1 && bd != 2 && bd != 4 && bd != 8 && bd != 16) { info.error = 37; return; } break;
    case 2: if(                                 bd != 8 && bd != 16) { info.error = 37; return; } break;
    case 3: if(bd != 1 && bd != 2 && bd != 4 && bd != 8            ) { info.error = 37; return; } break;
    case 4: if(                                 bd != 8 && bd != 16) { info.error = 37; return; } break;
    case 6: if(                                 bd != 8 && bd != 16) { info.error = 37; return; } break;
    default: { info.error = 31; return; } break; //error: invalid color type
  }
}

//filter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1)
//precon is the previous filtered scanline, recon the result, scanline the current one
void Decoder::unFilterScanline(unsigned char* recon, unsigned char* scanline, unsigned char* precon, bool top, size_t bytewidth, unsigned long filterType, size_t length)
{
  switch(filterType)
  {
    case 0:
      if(top) for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
      else for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
      break;
    case 1:
      if(top)
      {
        for(size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
      }
      else
      {
        for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
        for(size_t i = bytewidth; i < length   ; i++) recon[i] = scanline[i] + recon[i - bytewidth];
      }
      break;
    case 2: 
      if(top) for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
      else for(size_t i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
      break;
    case 3:
      if(top)
      {
        for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
      }
      else
      {
        for(size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
        for(size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
      }
      break;
    case 4:
      if(top)
      {
        for(size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0));
      }
      else
      {
        for(size_t i = 0; i < bytewidth; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(0, precon[i], 0));
        for(size_t i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
      }
      break;
  default: info.error = 36; return; //error: unexisting filter type given
  }
}

//filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
void Decoder::adam7Pass(unsigned char* out, unsigned char* in, unsigned char* scanlinen, unsigned char* scanlineo, size_t w, size_t bytewidth, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
{
  for(unsigned long s = 0; s < passh; s++)
  {
    size_t linelength = 1 + ((bpp * passw + 7) >> 3);
    size_t linestart = s * linelength; //position where we read the filterType: at the start of the scanline
    unsigned long filterType = in[linestart];
    
    unFilterScanline(scanlinen, &in[linestart + 1], scanlineo, (s < 1), bytewidth, filterType, (w * bpp + 7) >> 3);
    if(hasError()) return;
    
    //put the filtered pixels in the output image
    if(bpp >= 8)
    {
      for(size_t i = 0; i < passw; i++)
      for(size_t b = 0; b < bytewidth; b++) //b = current byte of this pixel
      {
        out[bytewidth * w * (passtop + spacey * s) + bytewidth * (passleft + spacex * i) + b] = scanlinen[bytewidth * i + b];
      }
    }
    else
    {
      for(size_t i = 0; i < passw; i++)
      {
        size_t outbitp = bpp * w * (passtop + spacey * s) + bpp * (passleft + spacex * i);
        for(size_t b = 0; b < bpp; b++) //b = current bit of this pixel
        {
          size_t obp = outbitp + b;
          size_t obitpos = 7 - (obp & 0x7); //where bitpos 0 refers to the LSB, bitpot 7 to the MSB of a byte
          size_t bp = i * bpp + b;
          size_t bitpos = 7 - (bp & 0x7); //where bitpos 0 refers to the LSB, bitpot 7 to the MSB of a byte
          unsigned long bit = (scanlinen[bp >> 3] >> bitpos) & 1;
          out[obp >> 3] = (unsigned char)((out[obp >> 3] & ~(1 << obitpos)) | (bit << obitpos));
        }
      }
    }

    //swap the two buffer pointers "scanline old" and "scanline new"
    unsigned char* temp = scanlinen;
    scanlinen = scanlineo;
    scanlineo = temp;
  }
}

void Decoder::resetParameters()
{
  info.error = 0; //initially no error happened yet
  info.paletteSize = info.backgroundColor = info.colorKey = 0; //initialize info variables that aren't necessarily set later on
}

//read a PNG, the result will be in the same color type as the PNG
void Decoder::decodeGeneric(std::vector<unsigned char>& out, std::vector<unsigned char>& in)
{
  if(CHAR_BIT < 8) { info.error = 54; return; }
  if(in.size() == 0 && out.size() > 0) { info.error = 56; return; } //the given input data is empty but the output data not
  if(in.size() == 0) { info.error = 48; return; } //the given data is empty
  
  resetParameters(); //when decoding a new PNG image, make sure all parameters created after previous decoding are reset
  
  unsigned char* in_ = &in[0]; //use a regular pointer to the std::vector for faster code if compiled without optimization

  readPngHeader(&in[0], in.size());
  if(hasError()) return;

  size_t pos = 33; //first byte of the first chunk after the header
  size_t dataPos = 0; //at this position in the in buffer the new data will be stored.
  size_t dataStart = 0;
  
  bool IEND = 0;
  while(!IEND) //loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
  {
    //get chunk length
    if(pos + 8 >= in.size()) { info.error = 30; return; } //error: size of the in buffer too small to contain next chunk
    size_t chunkLength = 256 * 256 * 256 * in_[pos] + 256 * 256 * in_[pos + 1] + 256 * in_[pos + 2] + in_[pos + 3]; pos += 4;
    if(pos + chunkLength >= in.size()) { info.error = 35; return; } //error: size of the in buffer too small to contain next chunk

    //IDAT chunk, containing compressed image data
    if(in_[pos + 0] == 'I' && in_[pos + 1] == 'D' && in_[pos + 2] == 'A' && in_[pos + 3] == 'T')
    {
      pos += 4;
      if(dataPos == 0) { dataStart = pos; dataPos = dataStart + chunkLength; pos += chunkLength; } //possible efficiency increase by not copying data from first chunk
      else for(size_t i = 0; i < chunkLength; i++) in_[dataPos++] = in_[pos++]; //multiple data chunks are added behind each other this way
    }
    //IEND chunk
    else if(in_[pos + 0] == 'I' && in_[pos + 1] == 'E' && in_[pos + 2] == 'N' && in_[pos + 3] == 'D')
    {
      IEND = 1;
    }
    //palette chunk (PLTE)
    else if(in_[pos + 0] == 'P' && in_[pos + 1] == 'L' && in_[pos + 2] == 'T' && in_[pos + 3] == 'E')
    {
      pos += 4; //go after the 4 letters
      info.paletteSize = chunkLength / 3;
      if(info.paletteSize > 256) { info.error = 38; return; } //error: palette too big
      info.palette.resize(4 * info.paletteSize);
      for(size_t i = 0; i < info.paletteSize; i++)
      {
        info.palette[i * 4 + 0] = in_[pos++]; //R
        info.palette[i * 4 + 1] = in_[pos++]; //G
        info.palette[i * 4 + 2] = in_[pos++]; //B
        info.palette[i * 4 + 3] = 255; //alpha
      }
    }
    //palette transparency chunk (tRNS)
    else if(in_[pos + 0] == 't' && in_[pos + 1] == 'R' && in_[pos + 2] == 'N' && in_[pos + 3] == 'S')
    {
      pos += 4; //go after the 4 letters
      if(info.colorType == 3)
      {
        if(chunkLength > info.paletteSize) { info.error = 39; return; } //error: more alpha values given than there are palette entries
        for(size_t i = 0; i < chunkLength; i++) info.palette[i * 4 + 3] = in_[pos++];
      }
      else if(info.colorType == 0)
      {
        if(chunkLength != 2) { info.error = 40; return; } //error: this chunk must be 2 bytes for greyscale image
        info.colorKey = 1;
        info.keyR = 256 * in_[pos] + in_[pos + 1]; pos += 2;
      }
      else if(info.colorType == 2)
      {
        if(chunkLength != 6) { info.error = 41; return; } //error: this chunk must be 6 bytes for RGB image
        info.colorKey = 1;
        info.keyR = 256 * in_[pos] + in_[pos + 1]; pos += 2;
        info.keyG = 256 * in_[pos] + in_[pos + 1]; pos += 2;
        info.keyB = 256 * in_[pos] + in_[pos + 1]; pos += 2;
      }
      else { info.error = 42; return; } //error: tRNS chunk not allowed for other color models
    }
    //background color chunk (bKGD)
    else if(in_[pos + 0] == 'b' && in_[pos + 1] == 'K' && in_[pos + 2] == 'G' && in_[pos + 3] == 'D')
    {
      pos += 4; //go after the 4 letters
      if(info.colorType == 3)
      {
        if(chunkLength != 1) { info.error = 43; return; } //error: this chunk must be 1 byte for indexed color image
        info.backgroundColor = 1;
        info.backgroundR = in_[pos++];
      }
      else if(info.colorType == 0 || info.colorType == 4)
      {
        if(chunkLength != 2) { info.error = 44; return; } //error: this chunk must be 2 bytes for greyscale image
        info.backgroundColor = 1;
        info.backgroundR = 256 * in_[pos] + in_[pos + 1]; pos += 2;
      }
      else if(info.colorType == 2 || info.colorType == 6)
      {
        if(chunkLength != 6) { info.error = 45; return; } //error: this chunk must be 6 bytes for greyscale image
        info.backgroundColor = 1;
        info.backgroundR = 256 * in_[pos] + in_[pos + 1]; pos += 2;
        info.backgroundG = 256 * in_[pos] + in_[pos + 1]; pos += 2;
        info.backgroundB = 256 * in_[pos] + in_[pos + 1]; pos += 2;
      }
    }
    else //it's not an implemented chunk type, so ignore it: skip over the data and the CRC
    {
      pos += 4; //go after the 4 letters
      pos += chunkLength; //skip uninterpreted data of unimplemented chunk
    }
    pos += 4; //skip CRC
  }

  switch(info.colorType)
  {
    case 0: info.bpp = info.bitDepth    ; info.colorChannels = 1; break; //grey
    case 2: info.bpp = info.bitDepth * 3; info.colorChannels = 3; break; //RGB
    case 3: info.bpp = info.bitDepth    ; info.colorChannels = 1; break; //palette
    case 4: info.bpp = info.bitDepth * 2; info.colorChannels = 2; break; //grey + alpha
    case 6: info.bpp = info.bitDepth * 4; info.colorChannels = 4; break; //RGBA
    default: break; //can never happen since colorType is always one of the above and error is already checked for that
  }

  size_t scanlength = ((info.w * (info.h * info.bpp + 7)) >> 3) + info.h * 8; //scanline buffer length is larger than final image size because up to h * 7 filter type codes can still be in it!
  std::vector<unsigned char> scanlines(scanlength); //now the out buffer will be filled
  
  //decompress the data
  decodeZlib(&scanlines[0], &in_[dataStart], scanlength, dataPos - dataStart); //dataPos - dataStart is size of the input data: size of the new data overwritten over the in buffer
  if(hasError()) return;
  
  //filter and interlace
  size_t bytewidth = (info.bpp + 7) >> 3; //bytewidth is used for filtering
  size_t outlength = (info.h * info.w * info.bpp + 7) >> 3;

  out.resize(outlength); //time to fill the out buffer
  unsigned char* out_ = &out[0]; //use a regular pointer to the std::vector for faster code if compiled without optimization

  if(info.interlaceMethod == 0)
  {
    std::vector<unsigned char> templine((info.w * info.bpp + 7) >> 3); //only used if bpp < 8
    size_t linestart = 0; //start of current scanline
    size_t obp = 0; //out bit pointer, only used if bpp < 8
    for(size_t s = 0; s < info.h; s++)
    {
      unsigned long filterType = scanlines[linestart];
      if(info.bpp >= 8) //byte per byte
      {
        unFilterScanline(&out_[linestart - s], &scanlines[linestart + 1], &out_[(s - 1) * info.w * bytewidth], (s < 1), bytewidth, filterType,  (info.w * info.bpp + 7) >> 3);
        if(hasError()) return;
      }
      else //less than 8 bits per pixel, so fill it up bit per bit
      {
        unFilterScanline(&templine[0], &scanlines[linestart + 1], &out_[(s - 1) * info.w * bytewidth], (s < 1), bytewidth, filterType,  (info.w * info.bpp + 7) >> 3);
        if(hasError()) return;

        for(size_t bp = 0; bp < info.w * info.bpp; bp++) //bp is here bit pointer in templine
        {
          size_t obitpos = 7 - (obp & 0x7);
          size_t bitpos = 7 - (bp & 0x7);
          unsigned long bit = (templine[bp >> 3] >> bitpos) & 1;
          out_[obp >> 3] = (unsigned char)((out_[obp >> 3] & ~(1 << obitpos)) | (bit << obitpos)); //set current bit
          obp++;
        }
      }
      linestart += 1 + ((info.w * info.bpp + 7) >> 3); //go to start of next scanline
    }
  }
  else //interlaceMethod is 1 (Adam7)
  {
    size_t passw[7], passh[7], passstart[7];
    
    passw[0] = (info.w + 7) / 8, passh[0] = (info.h + 7) / 8;
    passw[1] = (info.w + 3) / 8, passh[1] = (info.h + 7) / 8;
    passw[2] = (info.w + 3) / 4, passh[2] = (info.h + 3) / 8;
    passw[3] = (info.w + 1) / 4, passh[3] = (info.h + 3) / 4;
    passw[4] = (info.w + 1) / 2, passh[4] = (info.h + 1) / 4;
    passw[5] = (info.w + 0) / 2, passh[5] = (info.h + 1) / 2;
    passw[6] = (info.w + 0) / 1, passh[6] = (info.h + 0) / 2;
    
    passstart[0] = 0;
    passstart[1] =        passh[0] * ((passw[0] * info.bpp + 7) / 8) + passh[0];
    passstart[2] = passstart[1] + passh[1] * ((passw[1] * info.bpp + 7) / 8) + passh[1];
    passstart[3] = passstart[2] + passh[2] * ((passw[2] * info.bpp + 7) / 8) + passh[2];
    passstart[4] = passstart[3] + passh[3] * ((passw[3] * info.bpp + 7) / 8) + passh[3];
    passstart[5] = passstart[4] + passh[4] * ((passw[4] * info.bpp + 7) / 8) + passh[4];
    passstart[6] = passstart[5] + passh[5] * ((passw[5] * info.bpp + 7) / 8) + passh[5];
    
    std::vector<unsigned char> scanlineo(info.w * info.bpp); //"old" scanline
    std::vector<unsigned char> scanlinen(info.w * info.bpp); //"new" scanline
    
    adam7Pass(&out_[0], &scanlines[passstart[0]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 0, 0, 8, 8, passw[0], passh[0], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[1]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 4, 0, 8, 8, passw[1], passh[1], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[2]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 0, 4, 4, 8, passw[2], passh[2], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[3]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 2, 0, 4, 4, passw[3], passh[3], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[4]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 0, 2, 2, 4, passw[4], passh[4], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[5]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 1, 0, 2, 2, passw[5], passh[5], info.bpp);
    adam7Pass(&out_[0], &scanlines[passstart[6]], &scanlinen[0], &scanlineo[0], info.w, bytewidth, 0, 1, 1, 2, passw[6], passh[6], info.bpp);
  }
}

//read a PNG, the result will always be in 8-bit RGBA color, no matter what color type the PNG image had
void Decoder::decode32(std::vector<unsigned char>& out, std::vector<unsigned char>& in)
{
  if(in.size() == 0 && out.size() > 0) { info.error = 56; return; } //the given input data is empty but the output data not

  std::vector<unsigned char> buffer;

  decodeGeneric(buffer, in); //before we can convert to RGBA, decode the PNG in the format it is
  if(hasError()) return;

  size_t w = info.w; //width of the image
  size_t h = info.h; //height of the image

  out.resize(w * h * 4);
  
  unsigned char* buffer_ = &buffer[0]; //faster if compiled without optimization
  unsigned char* out_ = &out[0]; //faster if compiled without optimization

  switch(info.colorType)
  {
    case 0: //greyscale color
    for(size_t i = 0; i < w * h; i++)
    {
      out_[4 * i + 3] = 255;
      if(info.bitDepth == 8)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = buffer_[i];
        if(info.colorKey == 1)
        if(buffer_[i] == info.keyR) out_[4 * i + 3] = 0;
      }
      else if(info.bitDepth == 16)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = buffer_[2 * i];
        if(info.colorKey == 1)
        if(256U * buffer_[i] + buffer_[i + 1] == info.keyR) out_[4 * i + 3] = 0;
      }
      else
      {
        size_t bp = info.bitDepth * i;
        unsigned long value = 0;
        unsigned long pot = 1 << info.bitDepth; //power of two
        for(size_t j = 0; j < info.bitDepth; j++) 
        {
          pot /= 2;
          unsigned long bit = readBitFromStream(bp, buffer_);
          value += pot * bit;
        }
        if(info.colorKey == 1)
        if(value && ((1U << info.bitDepth) - 1U) == info.keyR && ((1U << info.bitDepth) - 1U)) out_[4 * i + 3] = 0;
        value = (value * 255) / ((1 << info.bitDepth) - 1); //scale value from 0 to 255
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = (unsigned char)(value);
      }
    }
    break;
    case 2: //RGB color
    for(size_t i = 0; i < w * h; i++)
    {
      out_[4 * i + 3] = 255;
      if(info.bitDepth == 8)
      {
        for(size_t c = 0; c < 3; c++) out_[4 * i + c] = buffer_[3 * i + c];
        if(info.colorKey == 1)
        if(buffer_[3 * i + 0] == info.keyR && buffer_[3 * i + 1] == info.keyG && buffer_[3 * i + 2] == info.keyB) out_[4 * i + 3] = 0;
      }
      else if(info.bitDepth == 16)
      {
        for(size_t c = 0; c < 3; c++) out_[4 * i + c] = buffer_[6 * i + 2 * c];
        if(info.colorKey == 1)
        if(256U * buffer_[6 * i + 0] + buffer_[6 * i + 1] == info.keyR
        && 256U * buffer_[6 * i + 2] + buffer_[6 * i + 3] == info.keyG
        && 256U * buffer_[6 * i + 4] + buffer_[6 * i + 5] == info.keyB) out_[4 * i + 3] = 0;
      }
    }
    break;
    case 3: //indexed color (palette)
    for(size_t i = 0; i < w * h; i++)
    {
      out_[4 * i + 3] = 255;
      if(info.bitDepth == 8)
      {
        unsigned long value = buffer_[i];
        if(value >= info.paletteSize) { info.error = 46; return; }
        for(size_t c = 0; c < 4; c++) out_[4 * i + c] = (unsigned char)(info.palette[c + 4 * value]); //get rgb colors from the palette, which is stored in info
      }
      else if(info.bitDepth < 8)
      {
        size_t bp = info.bitDepth * i;
        unsigned long value = 0;
        unsigned long pot = 1 << info.bitDepth; //power of two
        for(size_t j = 0; j < info.bitDepth; j++) 
        {
          pot /= 2;
          unsigned long bit = readBitFromStream(bp, buffer_);
          value += pot * bit;
        }
        if(value >= info.paletteSize) { info.error = 47; return; }
        for(size_t c = 0; c < 4; c++) out_[4 * i + c] = info.palette[c + 4 * value]; //get rgb colors from the palette, which is stored in info
      }
    }
    break;
    case 4: //greyscale with alpha
    for(size_t i = 0; i < w * h; i++)
    {
      out_[4 * i + 3] = 255;
      if(info.bitDepth == 8)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = buffer_[i * 2 + 0];
        out_[4 * i + 3] = buffer_[i * 2 + 1];
      }
      else if(info.bitDepth == 16)
      {
        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = buffer_[4 * i]; //most significant byte
        out_[4 * i + 3] = buffer_[4 * i + 2]; //most significant byte
      }
    }
    break;
    case 6: //RGB with alpha
    for(size_t i = 0; i < w * h; i++)
    {
      out_[4 * i + 3] = 255;
      if   (info.bitDepth ==  8) for(size_t c = 0; c < 4; c++) out_[4 * i + c] = buffer_[4 * i + c];
      else if(info.bitDepth == 16) for(size_t c = 0; c < 4; c++) out_[4 * i + c] = buffer_[8 * i + 2 * c];
    }
    break;
    default: break;
  }
}

bool Decoder::hasError() const { return info.error != 0; }
int Decoder::getError() const { return info.error; }

size_t Decoder::getWidth()  const { return info.w; }
size_t Decoder::getHeight() const { return info.h; }

const Decoder::Info& Decoder::getInfo() const  { return info; }

Decoder::Info::Info()
{
  error = 1; //start out with error 1, which means: no image decoded yet, this Info is just new
}

//////////////////////////////////////////////////////////////////////////////
// CRC                                                                      //
//////////////////////////////////////////////////////////////////////////////

Crc::Crc()
{
    crc_table_computed = false;
}

//Make the table for a fast CRC.
void Crc::make_crc_table(void)
{
  unsigned long c;
  int n, k;

  for (n = 0; n < 256; n++)
  {
    c = (unsigned long) n;
    for (k = 0; k < 8; k++) 
    {
      if (c & 1) c = 0xedb88320L ^ (c >> 1);
      else c = c >> 1;
    }
    crc_table[n] = c;
  }
  crc_table_computed = true;
}

/*Update a running CRC with the bytes buf[0..len-1]--the CRC should be 
initialized to all 1's, and the transmitted value is the 1's complement of the
final running CRC (see the crc() routine below).*/
unsigned long Crc::update_crc(unsigned char *buf, unsigned long crc, size_t len)
{
  unsigned long c = crc;
  size_t n;

  if (!crc_table_computed)
  make_crc_table();
  for (n = 0; n < len; n++)
  {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

//Return the CRC of the bytes buf[0..len-1].
unsigned long Crc::crc(unsigned char *buf, size_t len)
{
  return update_crc(buf, 0xffffffffL, len) ^ 0xffffffffL;
}

//////////////////////////////////////////////////////////////////////////////
// Adler32                                                                  //
//////////////////////////////////////////////////////////////////////////////

unsigned long Adler32::update_adler32(unsigned long adler, const unsigned char *buf, size_t len) const
{
  unsigned long s1 = adler & 0xffff;
  unsigned long s2 = (adler >> 16) & 0xffff;
  size_t n;

  for (n = 0; n < len; n++) 
  {
    s1 = (s1 + buf[n]) % BASE;
    s2 = (s2 + s1)     % BASE;
  }
  return (s2 << 16) + s1;
}

/* Return the adler32 of the bytes buf[0..len-1] */
unsigned long Adler32::adler32(const unsigned char *buf, size_t len) const
{
  return update_adler32(1L, buf, len);
}

//////////////////////////////////////////////////////////////////////////////
// Encoder                                                                  //
//////////////////////////////////////////////////////////////////////////////

void Encoder::encode(std::vector<unsigned char>& out, const std::vector<unsigned char>& image, size_t w, size_t h)
{
  encodeNonCompressedRGBA(out, image, w, h);
}

void Encoder::addChunk(std::string chunkName, unsigned char* data, size_t length, std::vector<unsigned char>& out)
{
  if(chunkName.size() != 4) return;
  
  //1: length
  out.push_back( (length / (256 * 256 * 256)) % 256 );
  out.push_back( (length / (      256 * 256)) % 256 );
  out.push_back( (length / (            256)) % 256 );
  out.push_back( (length / (              1)) % 256 );
  
  //2: chunk name (4 letters)
  out.push_back(chunkName[0]);
  out.push_back(chunkName[1]);
  out.push_back(chunkName[2]);
  out.push_back(chunkName[3]);
  
  //3: the data
  for(size_t i = 0; i < length; i++) out.push_back(data[i]);
  
  //4: CRC (of the chunkname characters and the data) 
  unsigned long CRC = crc.crc(&out[out.size() - length - 4], length + 4);

  out.push_back( (CRC / (256 * 256 * 256)) % 256 );
  out.push_back( (CRC / (      256 * 256)) % 256 );
  out.push_back( (CRC / (            256)) % 256 );
  out.push_back( (CRC / (              1)) % 256 );
}

void Encoder::writeSignature(std::vector<unsigned char>& out)
{
  //8 bytes PNG signature
  out.push_back(137);
  out.push_back(80);
  out.push_back(78);
  out.push_back(71);
  out.push_back(13);
  out.push_back(10);
  out.push_back(26);
  out.push_back(10);
}

void Encoder::writeChunk_IHDR(int w, int h, int bitDepth, int colorType, std::vector<unsigned char>& out)
{
  writeSignature(out);
  
  std::vector<unsigned char> header;
  
  //header
  header.push_back( (w / (256 * 256 * 256)) % 256 );
  header.push_back( (w / (      256 * 256)) % 256 );
  header.push_back( (w / (            256)) % 256 );
  header.push_back( (w / (              1)) % 256 );
  header.push_back( (h / (256 * 256 * 256)) % 256 );
  header.push_back( (h / (      256 * 256)) % 256 );
  header.push_back( (h / (            256)) % 256 );
  header.push_back( (h / (              1)) % 256 );
  header.push_back(bitDepth); //bit depth
  header.push_back(colorType); //color type
  header.push_back(0); //compression method
  header.push_back(0); //filter method
  header.push_back(0); //interlace method
  
  addChunk("IHDR", &header[0], header.size(), out);
}

void Encoder::writeChunk_tEXt(std::string keyword, std::string textstring, std::vector<unsigned char>& out)
{
  //add text chunk
  std::string text = keyword;
  text.push_back(0);
  text = text + textstring;
  addChunk("tEXt", (unsigned char*)&text[0], text.size(), out);
}

void Encoder::deflateNoCompression(const std::vector<unsigned char>& data, std::vector<unsigned char>& out)
{
  /*
  3) deflate data
  
  -1 bit BFINAL
  -2 bits BTYPE
  -(5) bits: spring naar begin volgende byte
  -2 bytes LEN
  -2 bytes NLEN
  -LEN bytes literal DATA
  */
  
  size_t numdeflateblocks = data.size() / 65536 + 1;
  size_t datapos = 0;
  for(size_t i = 0; i < numdeflateblocks; i++)
  {
    int BFINAL = (i == numdeflateblocks - 1);
    int BTYPE = 0;
    
    unsigned char firstbyte = 0 + BFINAL + ((BTYPE & 1) << 1) + ((BTYPE & 2) << 1);
    
    out.push_back(firstbyte);
    
    unsigned long LEN = 65535;
    if(data.size() - datapos < 65535) LEN = data.size() - datapos;
    unsigned long NLEN = 65535 - LEN;

    out.push_back(LEN % 256);
    out.push_back(LEN / 256);
    out.push_back(NLEN % 256);
    out.push_back(NLEN / 256);
    
    //uncompressed data
    for(size_t j = 0; j < 65535 && datapos < data.size(); j++)
    {
      out.push_back(data[datapos]);
      datapos++;
    }
  }
}

void addHuffmanSymbol(std::vector<unsigned char>& compressed, unsigned long val, size_t& bp, const std::vector<unsigned long>& codes, const std::vector<unsigned long> bitlen)
{
  size_t blen = bitlen[val];
  unsigned long code = codes[val];
  
  addBitsToStreamReversed(bp, compressed, code, blen);
}

void addLengthDistance(std::vector<int>& values, size_t length, size_t distance)
{
  /*
  values in encoded string:
  0-255: literal bytes
  256: end
  257-285: lengths (followed by distance code)
  286-287: invalid
  
  if value is a length, then there will be 3 additional values added, the 4 values together forming:
  (1) length code (257-285)
  (2) length extra bits
  (3) distance code (0-29)
  (4) distance extra bits
  */

  //the length
  int length_code = 28;
  for(size_t i = 1; i < 29; i++) //this could be changed to binary search
  {
    if(lengthbase[i] > length)
    {
      length_code = i - 1;
      break;
    }
  }
  int extra_length = length - lengthbase[length_code];
  values.push_back(length_code + 257); //(1)
  values.push_back(extra_length); //(2)
  
  //and now the distance
  int dist_code = 29;
  for(size_t i = 1; i < 30; i++) //this could be changed to binary search
  {
    if(distancebase[i] > distance)
    {
      dist_code = i - 1;
      break;
    }
  }
  int extra_distance = distance - distancebase[dist_code];
  values.push_back(dist_code); //(3)
  values.push_back(extra_distance); //(4)
}

void encodeLZ77(std::vector<int>& out, const unsigned char* in, int size, int window_size)
{
  //using pointer instead of vector for input makes it faster when NOT using optimization when compiling; no influence if optimization is used
  for(int pos = 0; pos < size; pos++)
  {
    int longest = 0;
    int longestoffset = 0;
    int max_offset = window_size;
    if(pos < max_offset) max_offset = pos; //calculating max offset here, instead of testing each time inside the for loop, makes it significantly faster
    for(int offset = 1; offset < max_offset; offset++) //search backwards through all possible distances (=offsets)
    {
      int backpos = pos - offset;
      
      if(in[backpos] == in[pos])
      {
        int length = 1;
        int backtest = backpos;
        int foretest = pos;
        while(true)
        {
          backtest++;
          foretest++;
          if(foretest >= size) break;
          
          if(backpos >= pos) backpos -= offset; //continue as if we work on the decoded bytes after pos by jumping back before pos

          if(in[backtest] == in[foretest])
          {
            length++;
            //length is maximum 258, and value "258" has 0 extra bits so can't be made longer. the encoded length may absolutely not be larger than 258.
            if(length >= 258) break;
          }
          else break;
        }
        if(length > longest)
        {
          longest = length; //the longest length
          longestoffset = offset; //the offset that is related to this longest length
          //you can also jump out of this for loop once a length of 258 is found (gives significant speed gain)
          if(length >= 258) break;
        }
      }
    } //end of testing offsets
    
    //in de output code steken
    if(longest < 3) //only lengths of 3 or higher are supported as length/distance pair
    {
      out.push_back(in[pos]);
    }
    else
    {
      addLengthDistance(out, longest, longestoffset);
      pos += (longest - 1);
    }
  } //end of the loop through each character of input
}

void Encoder::deflateFixed(const std::vector<unsigned char>& data, std::vector<unsigned char>& out)
{
  std::vector<unsigned long> codes(288);
  std::vector<unsigned long> bitlen(288);
  generateFixedTreeLengths(&bitlen[0]);
  genCodetree(&codes[0], &bitlen[0], 288, 16); //tree for literal values and length codes
  
  
  std::vector<unsigned long> codesD(32);
  std::vector<unsigned long> bitlenD(32);
  generateDistanceTreeLengths(&bitlenD[0]);
  genCodetree(&codesD[0], &bitlenD[0], 32, 16); //tree for distance codes

  bool BFINAL = 1; //make only one block... the first and final one
  size_t bp = 0; //the bit pointer
  
  addBitToStream(bp, out, BFINAL);
  addBitToStream(bp, out, 1); //first bit of BTYPE
  addBitToStream(bp, out, 0); //second bit of BTYPE
  
  if(settings.useLZ77) //LZ77 encoded
  {
    std::vector<int> lz77_encoded;
    
    encodeLZ77(lz77_encoded, &data[0], data.size(), settings.window_size);
    
    for(size_t i = 0; i < lz77_encoded.size(); i++)
    {
      int val = lz77_encoded[i];
      addHuffmanSymbol(out, val, bp, codes, bitlen);
      if(val > 256) //for a length code, 3 more things have to be added
      {
        int length_index = val - 257;
        int n_length_extra_bits = lengthextra[length_index];
        i++;
        int length_extra_bits = lz77_encoded[i];
        addBitsToStream(bp, out, length_extra_bits, n_length_extra_bits);
        
        i++;
        int distance_code = lz77_encoded[i];
        addHuffmanSymbol(out, distance_code, bp, codesD, bitlenD);
              
        int distance_index = distance_code;
        int n_distance_extra_bits = distanceextra[distance_index];
        i++;
        int distance_extra_bits = lz77_encoded[i];
        addBitsToStream(bp, out, distance_extra_bits, n_distance_extra_bits);
      }
    }
  }
  else //no LZ77, not really useful
  {
    for(size_t i = 0; i < data.size(); i++)
    {
      unsigned char val = data[i];
      addHuffmanSymbol(out, val, bp, codes, bitlen);
    }
  }
  
  addHuffmanSymbol(out, 256, bp, codes, bitlen); //"end" code
}

void Encoder::deflate(const std::vector<unsigned char>& data, std::vector<unsigned char>& out)
{
  if(settings.btype == 0) deflateNoCompression(data, out);
  else if(settings.btype == 1) deflateFixed(data, out);
}

void Encoder::encodeZlib(const std::vector<unsigned char>& data, std::vector<unsigned char>& out)
{
  /*
  2) zlib data
  
  -1 byte CMF (4 bit CM die 8 moet zijn, 4 bit CINFO, de window size (7), CM zijn de 4 LSBs)
  -1 byte FLG
  -deflate data
  -4 byte ADLER32 checksum van de uncompressed data
  */
  
  int CMF = 120; //0b01111000: CM 8, CINFO 7
  int FLEVEL = 0;
  int FDICT = 0;
  int CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
  int FCHECK = 31 - CMFFLG % 31;
  CMFFLG += FCHECK;
  //assert(CMFFLG % 31 == 0);
  out.push_back(CMFFLG / 256);
  out.push_back(CMFFLG % 256);
  
  std::vector<unsigned char> deflatedata;
  
  deflate(data, deflatedata);
  
  unsigned long ADLER32 = adler32.adler32(&data[0], data.size());

  for(size_t i = 0; i < deflatedata.size(); i++) out.push_back(deflatedata[i]);
  
  out.push_back( (ADLER32 / (256 * 256 * 256)) % 256 );
  out.push_back( (ADLER32 / (      256 * 256)) % 256 );
  out.push_back( (ADLER32 / (            256)) % 256 );
  out.push_back( (ADLER32 / (              1)) % 256 );
}


void Encoder::writeChunk_IDAT(const std::vector<unsigned char>& data, std::vector<unsigned char>& out)
{
  //IDAT
  std::vector<unsigned char> zlibdata;
  
  encodeZlib(data, zlibdata);
  
  addChunk("IDAT", &zlibdata[0], zlibdata.size(), out);
}

void Encoder::writeChunk_IEND(std::vector<unsigned char>& out)
{
  addChunk("IEND", 0, 0, out); 
}

void Encoder::dontFilter(const std::vector<unsigned char>& image, size_t w, size_t h, std::vector<unsigned char>& out)
{
  out.resize(image.size() + h);
  //generate the literal data out of given image vector. filterType has to be added per scanline.
  for(size_t y = 0; y < h; y++)
  {
    size_t begin = y * (w * 4 + 1);
    out[begin] = 0; //filterType 0 for this scanline
    for(size_t x = 0; x < w * 4; x++) out[begin + 1 + x] = image[y * w * 4 + x];
  }
}

void Encoder::filterScanline(const unsigned char* scanline, const unsigned char* prevline, unsigned char* out, bool top, size_t length, size_t bytewidth, int filterType)
{
  switch(filterType)
  {
    case 0:
      if(top) for(size_t i = 0; i < length; i++) out[i] = scanline[i];
      else for(size_t i = 0; i < length; i++) out[i] = scanline[i];
      break;
    case 1:
      if(top)
      {
        for(size_t i = 0; i < bytewidth; i++) out[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) out[i] = scanline[i] - scanline[i - bytewidth];
      }
      else
      {
        for(size_t i =         0; i < bytewidth; i++) out[i] = scanline[i];
        for(size_t i = bytewidth; i < length   ; i++) out[i] = scanline[i] - scanline[i - bytewidth];
      }
      break;
    case 2: 
      if(top) for(size_t i = 0; i < length; i++) out[i] = scanline[i];
      else for(size_t i = 0; i < length; i++) out[i] = scanline[i] - prevline[i];
      break;
    case 3:
      if(top)
      {
        for(size_t i = 0; i < length; i++) out[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) out[i] = scanline[i] - scanline[i - bytewidth] / 2;
      }
      else
      {
        for(size_t i = 0; i < bytewidth; i++) out[i] = scanline[i] - prevline[i] / 2;
        for(size_t i = bytewidth; i < length; i++) out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) / 2);
      }
      break;
    case 4:
      if(top)
      {
        for(size_t i = 0; i < bytewidth; i++) out[i] = scanline[i];
        for(size_t i = bytewidth; i < length; i++) out[i] = (unsigned char)(scanline[i] - paethPredictor(scanline[i - bytewidth], 0, 0));
      }
      else
      {
        for(size_t i = 0; i < bytewidth; i++) out[i] = (unsigned char)(scanline[i] - paethPredictor(0, prevline[i], 0));
        for(size_t i = bytewidth; i < length; i++) out[i] = (unsigned char)(scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
      }
      break;
  default: return; //unexisting filter type given
  }
}

void Encoder::filter(const std::vector<unsigned char>& image, size_t w, size_t h, std::vector<unsigned char>& out)
{
  //I use a heuristic described here: http://www.cs.toronto.edu/~cosmin/pngtech/optipng.html
  //it says: 
  // *  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e. use fixed filtering, with the filter None).
  // * (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is not smaller than 8, then use adaptive filtering as follows: independently for each row, apply all five filters and select the filter that produces the smallest sum of absolute values per row.
  //Here, the image is RGB with alpha and bit depth 8, so the one with smallest sum is used.
  
  out.resize(image.size() + h); //image size plus an extra byte per scanline
  
  size_t bytewidth = 4;
  size_t length = w * bytewidth; //NOTE: when using less than 8 bits per pixel, the length is NOT the width in pixels multiplied by the bytewidth, because bytewidth will then still is 1 ==> update this code when encoding such kind of images
  
  std::vector<unsigned char> attempt[5]; //five filtering attempts, one for each filter type
  for(size_t i = 0; i < 5; i++) attempt[i].resize(length);
  unsigned long sum[5];
  
  for(size_t y = 0; y < h; y++)
  {
    //try the 5 filter types
    for(size_t i = 0; i < 5; i++)
    {
      const unsigned char* prevline = (y == 0) ? 0 : &image[(y - 1) * length];
      filterScanline(&image[y * length], prevline, &attempt[i][0], y == 0, length, bytewidth, i);
      
      //calculate the sum of the result
      sum[i] = 0;
      for(size_t j = 0; j < 5; j++) sum[i] += attempt[i][j];
    }
    
    //find smallest sum
    unsigned long smallest = sum[0];
    size_t smallestType = 0;
    for(int i = 1; i < 5; i++)
    if(sum[i] < smallest)
    {
      smallestType = i;
      smallest = sum[i];
    }
    
    //now fill the out values
    out[y * (length + 1)] = smallestType; //the first byte of a scanline will be the filter type
    for(size_t i = 0; i < length; i++)
    out[y * (length + 1) + 1 + i] = attempt[smallestType][i];

  }
}

void Encoder::encodeNonCompressedRGBA(std::vector<unsigned char>& out, const std::vector<unsigned char>& image, size_t w, size_t h)
{
  std::vector<unsigned char> data;
  
  filter(image, w, h, data);
  
  writeChunk_IHDR(w, h, 8, 6, out);
  writeChunk_IDAT(data, out);
  writeChunk_tEXt("Comment", "Encoded with LodePNG", out);
  writeChunk_IEND(out);
}

void Encoder::setSettings(const Encoder::Settings& settings)
{
  this->settings = settings;
}

const Encoder::Settings& Encoder::getSettings() const
{
  return settings;
}

Encoder::Settings::Settings()
{
  window_size = 2048; //this is a good tradeoff between speed and compression ratio
  btype = 1;
  useLZ77 = true;
}

} //end of namespace LodePNG
