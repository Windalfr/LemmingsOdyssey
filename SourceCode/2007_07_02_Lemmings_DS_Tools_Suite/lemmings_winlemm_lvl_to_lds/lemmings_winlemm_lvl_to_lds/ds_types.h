// Nintendo DS compatible types for loading and saving of data.

#ifndef __DS_TYPES_H__
#define __DS_TYPES_H__
  
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define BIT(n) (1 << (n))

typedef unsigned char           uint8;
typedef unsigned short int      uint16;
typedef unsigned int            uint32;
typedef unsigned long long int  uint64;

typedef signed char             int8;
typedef signed short int        int16;
typedef signed int              int32;
typedef signed long long int    int64;

typedef float                   float32;
typedef double                  float64;

typedef volatile uint8          vuint8;
typedef volatile uint16         vuint16;
typedef volatile uint32         vuint32;
typedef volatile uint64         vuint64;

typedef volatile int8           vint8;
typedef volatile int16          vint16;
typedef volatile int32          vint32;
typedef volatile int64          vint64;

typedef volatile float32        vfloat32;
typedef volatile float64        vfloat64;

typedef uint8                   byte;

typedef int64                   dfixed;

typedef volatile int32          vfixed;


typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned int            u32;
typedef unsigned long long int  u64;

typedef signed char             s8;
typedef signed short int        s16;
typedef signed int              s32;
typedef signed long long int    s64;

typedef volatile u8          vu8;
typedef volatile u16         vu16;
typedef volatile u32         vu32;
typedef volatile u64         vu64;

typedef volatile s8           vs8;
typedef volatile s16          vs16;
typedef volatile s32          vs32;
typedef volatile s64          vs64;

typedef int f32;             // 1.19.12 fixed point for matricies

#endif // __DS_TYPES_H__
