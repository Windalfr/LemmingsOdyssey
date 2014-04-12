// **************************************************************************
// *                                                                        *
// *  Matt's utility routines                                               *
// *      by Mathew Carr                                                    *
// *                                                                        *
// **************************************************************************
                   
#ifndef _UTILITY_H_
#define _UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif
               
#ifdef ARM9
#include <nds.h>
#endif

#include "types.h"

#ifndef ARM9
#define RGB15(r,g,b) (((b)<<10)|((g)<<5)|(r))
#endif

#define RGB15A(r, g, b) ((RGB15(r, g, b)) | 0x8000)
#define RBG15A(r, b, g) ((RGB15(r, g, b)) | 0x8000)
#define GRB15A(g, r, b) ((RGB15(r, g, b)) | 0x8000)
#define GBR15A(g, b, r) ((RGB15(r, g, b)) | 0x8000)
#define BRG15A(b, r, g) ((RGB15(r, g, b)) | 0x8000)
#define BGR15A(b, g, r) ((RGB15(r, g, b)) | 0x8000)

s32 Modulo(s32 src, s32 divisor);
u32 IntDiv(u32 src, u32 divisor);   
s32 IntDivS(s32 src, s32 divisor);
u32 IntDivR(u32 src, u32 divisor);
void ParseIntIntoComponents(u32 input, u32 *tens, u32 *units);

int Max(int a, int b);
int Min(int a, int b);

#define Abs(x) ((x >= 0) ? (x) : (0-x))

u32 RangeWrap(s32 n, u32 greatest);

u16 rng_rand(u16 rng);

int FloatRound(float a);
#define RandomInt(x) rng_rand(x)

#define bits(src, bits) ((src & bits) == bits)
            
int ClosestPowerOfTwo(int n);
                  
#ifndef ARM9
#ifndef ARM7
int NearestXtoY(int x, int y);  
#endif
#endif

int ToTheNearestStride(int n, int stride, int offset);

int IntSqrt(int n);

// This function dereferences a pointer to a 32 bit value. Yeah, for some reason
// the DS doesn't like to dereference pointers that aren't on a word boundary.
static inline u32 Dereference32Bit(const u32 *value) {
   return (((const u8*)(value))[3] << 24)
        | (((const u8*)(value))[2] << 16)
        | (((const u8*)(value))[1] <<  8)
        | (((const u8*)(value))[0] <<  0);
}

#ifdef __cplusplus
}
#endif
#endif


