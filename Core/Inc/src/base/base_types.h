#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include<stdlib.h>
#include<string.h>

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i08;
typedef int8_t i8;

typedef uint64_t u64; 
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u08;
typedef uint8_t u8;

typedef i64 b64;
typedef i32 b32;
typedef i16 b16;
typedef i08 b08;
typedef i8 b8;

typedef float r32;
typedef double r64;

#define internal static
#define local_persist static
#define global_variable static

#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)
#define Swap(T,a,b) do{T t__ = a; a = b; b = t__;}while(0)

#define Assert(exp) {if(!(exp)){while(1);}}
#define ArrayCount(arr)(sizeof(arr) / sizeof(arr[0]))
#define StaticAssert(exp, id) global_variable u8 Glue(id, __LINE__)[(exp)?1:-1]

#define Member(type, member) (((type *) 0)->member)
#define OffsetOf(type, member) (&Member(type, member)) 
#define InvalidCodePath Assert(!"Invalid code path")
#define InvalidDefaultCase default: {Invalid_Code_Path;} break;

#define Modf(X) (((X)<0.0f)?-(X):(X))
#define Max(A,B) ((A)>(B))?(A):(B)
#define Min(A,B) ((A)<(B))?(A):(B)
#define Clamp(A,X,B) (((X)<(A))?(A):((X)>(B))?(B):(X))
#define ClampBot(A,X) Max(A,X)
#define ClampTop(X,B) Min(X,B)

#define KB(val) (1024 * (val))
#define MB(val) (1024 * KB(val))
#define GB(val) (1024 * MB(val))

#define DeferIt(it) Glue(it, __LINE__)
#define DeferLoop(begin, end) for(int DeferIt(_i_) = ((begin), 0); !DeferIt(_i_); DeferIt(_i_) += 1, (end))
#define FOURCC(str) (((u32)(str[0]) << 0) | ((u32)(str[1]) << 8) | ((u32)(str[2]) << 16) | ((u32)(str[3]) << 24))

//
//~ rsb: Memory Operation Macros
//

#define MemoryCopy(dest, src, size) memmove((dest), (src), (size))
#define MemorySet(dest, byte, size) memset((dest), (byte), (size))

#define MemoryCopyStruct(d,s)       MemoryCopy((d), (s), sizeof(*(d)))
#define MemoryCopyArray(d,s)        MemoryCopy((d), (s), sizeof(d))

#define MemoryZero(s,z)             MemorySet((s), 0, (z))
#define MemoryZeroStruct(s)         MemoryZero((s), sizeof(*(s)))
#define MemoryZeroArray(a)          MemoryZero((a), sizeof((a)))

#define PI 3.14159265359f

#endif //TYPES_H
