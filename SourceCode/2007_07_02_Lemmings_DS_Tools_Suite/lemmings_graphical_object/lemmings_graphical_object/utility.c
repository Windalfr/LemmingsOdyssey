// /************************************************************************\
// *                                                                        *
// *  Matt's utility routines                                               *  
// *      by Mathew Carr                                                    *
// *                                                                        *
// \************************************************************************/

#include "utility.h"
#include "types.h"

s32 Modulo(s32 src, s32 divisor) {
   // Keep taking away until we can't take away no mores, and then return the result.    
   while (src >= divisor) src -= divisor;
   return src;
}

u32 intdiv;
u32 intdiv_returned;

u32 IntDiv(u32 src, u32 divisor) {
   // Keep adding until we have an equal to the rounded down answer.      
   intdiv = intdiv_returned = 0;

   src -= Modulo(src, divisor);

   while (intdiv != src) {
      intdiv += divisor;
      intdiv_returned++;
   }

   //intdiv_returned = 0;

   //while ((src -= divisor) != 0) {
   //   intdiv_returned++;
   //}

   return intdiv_returned;
}

s32 IntDivS(s32 src, s32 divisor) {                               
   if (!(src < 0) && !(divisor < 0)) return  IntDiv( src, divisor);
   if ( (src < 0) && !(divisor < 0)) return -IntDiv(-src, divisor);
   if (!(src < 0) &&  (divisor < 0)) return -IntDiv( src,-divisor);
   if ( (src < 0) &&  (divisor < 0)) return  IntDiv(-src,-divisor);
}

u32 IntDivR(u32 src, u32 divisor) {
   // Keep adding until we have an equal to the rounded down answer.
   intdiv = intdiv_returned = 0;   

   if (divisor != 1) {
      if ((Modulo(src, divisor) * 2) >= divisor) {
         intdiv_returned++;
      }
   }

   src -= Modulo(src, divisor);

   while (intdiv != src) {
      intdiv += divisor;
      intdiv_returned++;
   }

   //intdiv_returned = 0;

   //while ((src -= divisor) != 0) {
   //   intdiv_returned++;
   //}
   
   return intdiv_returned;
}

void ParseIntIntoComponents(u32 input, u32 *tens, u32 *units) {
   (*tens)  = IntDiv(input, 10);
   
   (*units) = Modulo(input, 10);   
}

int Max(int a, int b) {
   return (a > b) ? a : b; 
}

int Min(int a, int b) {
   return (a < b) ? a : b; 
}

u32 RangeWrap(s32 n, u32 greatest) {
   while (n < 0        ) n += greatest;
   while (n >= greatest) n -= greatest;
   
   return n;
}

int FloatRound(float a) {
   return ((int)(a + 0.5f)); 
}

int PowerOfTwo(int n) {
   int r = 1;
   for (;;) {      
      if (!n) return r;
      r <<= 1;   
      --n;
   }
}

int ClosestPowerOfTwo(int n) {
   int p = 2;
   for (;;) {
      if (n < (PowerOfTwo(p - 1) * 3)) return PowerOfTwo(p);   
      ++p;
   } 
}

#ifdef ARM9
// Cearn's TONC random, again! Back for another season, so to speak!
u16 rng_rand(u16 rng) {
   return ((rand()&0x7fff)*rng)>>15;
}          
#endif

#ifndef ARM9
#ifndef ARM7
int NearestXtoY(int x, int y) {
   return x * FloatRound(((float)(y)) / ((float)(x)));
}
#endif
#endif

// This routine takes the number n to the nearest stride + offset
// The returned value will always be ((stride * X) + offset)
// n = 0x0015, stride = 0x0100, offset = 0x0000. Return: 0x0100
// n = 0x0115, stride = 0x0100, offset = 0x0000. Return: 0x0200
// n = 0x0095, stride = 0x0040, offset = 0x0000. Return: 0x00C0
// n = 0x0095, stride = 0x0100, offset = 0x0010. Return: 0x0110
// n = 0x0195, stride = 0x0100, offset = 0x0010. Return: 0x0210
int ToTheNearestStride(int n, int stride, int offset) {
   return (((IntDiv((n - offset + (stride - 1)),
                                        stride)) * stride) + offset);
}

int IntSqrt(int n) {
   for (int r = 0; r < 4096; r++) {
      if ((r*r) < n) {
         continue;
      }
      return r;
   }
}
