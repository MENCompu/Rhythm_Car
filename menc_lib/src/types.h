#ifndef DSV_DATATYPES_H
#define DSV_DATATYPES_H

#define Global
#define Function

#define Local_Persistant static
#define Intern_Global static
#define Intern static
#define Extern extern

#define Public  Intern
#define Private Intern

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef enum {
    JENH_Result_Success,
    JENH_Result_Failure,
} JENH_Result;

#if 0
typedef union {
    struct {
        u64 l64;
        u64 h64;
    };
    u32 l32;
    u16 l16;
    u8  l8;
    u8  B[16];
} u128;
#endif

#define WordSize size_t

#define byte uint8_t

typedef float  f32;
typedef double f64;

#ifdef false
    #undef false
#endif
#ifdef true
    #undef true
#endif

#ifndef __cplusplus
typedef enum b8 {
    false,
    true,
} b8;
#else
typedef u8 b8;
#endif

//typedef s32 b32;

#define c8  char
#define c16 char16_t
#define c32 char32_t

#define MAX_U8  (u8 )(0xff)
#define MAX_U16 (u16)(0xffff)
#define MAX_U32 (u32)(0xffffffff)
#define MAX_U64 (u64)(0xffffffffffffffff)

#define MAX_S8  (s8 )(0x7f)
#define MAX_S16 (s16)(0x7fff)
#define MAX_S32 (s32)(0x7fffffff)
#define MAX_S64 (s64)(0x7fffffffffffffff)

#define MIN_S16 (s16)(-32768)

#define MIN_F32 (f32)1.175494351e-38F
#define MAX_F32 (f32)3.402823466e+38F

#define MAX_WORDSIZE _Pragma("warning(suppress: 4310)") (WordSize)(0xffffffffffffffff)

#define BYTE_SIZE 8

#endif //DSV_DATATYPES_H
