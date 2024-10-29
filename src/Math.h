#ifndef TE_MATH_H
#define TE_MATH_H

#define JENH_PI (3.14159265359f)

#define PowerBase2(x) (1 << (x))

inline s32 TruncateI64ToS32(s64 value) {
    s32 result = value & 0xFFFFFFFF;
    return result;
}

inline s32 TruncateF32ToS32(f32 value) {
    s32 result = (s32)(value);
    return result;
}

inline u32 TruncateF32ToU32(f32 value) {
    u32 result = (u32)(value);
    return result;
}

inline s32 RoundF32ToS32(f32 value) {
    s32 result = (s32)(value + 0.5f);
    return result;
}

inline u32 RoundF32ToU32(f32 value) {
    u32 result = (u32)(value + 0.5f);
    return result;
}

inline s32 FloorF32ToS32(f32 value) {
    s32 result = (s32)(value);
    return result;
}

inline s32 CeilF32ToS32(f32 value) {
    s32 result = TruncateF32ToS32(value);
    result++;
    return result;
}

inline u32 CeilF32ToU32(f32 value) {
    u32 result = TruncateF32ToU32(value);
    result++;
    return result;
}

#include <math.h>
inline f32 Cos32(f32 value) {
    f32 ret = cosf(value);
    return ret;
}

inline f32 Sin32(f32 value) {
    f32 ret = sinf(value);
    return ret;
}

inline f64 Cos64(f64 value) {
    f64 ret = cos(value);
    return ret;
}

inline f64 Sin64(f64 value) {
    f64 ret = sin(value);
    return ret;
}

inline f32 Tan32(f32 value) {
    f32 ret = tanf(value);
    return ret;
}

inline f32 Sq32(f32 value) {
    f32 ret = value * value;
    return ret;
}

inline f32 Cb32(f32 value) {
    f32 ret = value * value * value;
    return ret;
}

inline f32 Atan2(f32 inA, f32 inB) {
    return atan2f(inA, inB);
}

inline f32 Sqrt32(f32 value) {
    f32 ret = sqrtf(value);
    return ret;
}

Public inline f32 Abs(f32 inValue) {
    return (f32)fabs((f64)inValue);
}

Public inline u32 Abs(s32 inValue) {
    return (u32)abs(inValue);
}

Public inline f32 F32_Abs(f32 inValue) {
    return (f32)fabs((f64)inValue);
}

inline b8 Is_Pos(s16 inValue) {
    return ( inValue >= 0 );
}

inline s32 Sign(s32 inValue) {
    if ( inValue == 0 ) { return 0; }
    return inValue / (s32)Abs(inValue);
}

inline f32 Sign(f32 inValue) {
    return inValue / F32_Abs(inValue);
}

#define Radians(inGrades) ((inGrades) * (JENH_PI / 180.0f))

Public inline u32 Safe_Cast_WordSize_To_U32(WordSize value) {
    Assert( value <= MAX_U32 );
    u32 ret = (u32)(value & (MAX_U32 - 1));
    return ret;
}

Public inline s32 Safe_Cast_U32_To_S32(u32 inValue) {
    Assert( inValue <= MAX_S32 );
    s32 retValue = (s32)inValue;
    return retValue;
}

#define GetIntegerDigits(value) Log10S32((value))

inline s32 Log10S32(s32 value) {
    s32 result = 0;

    while (value >= 10) {
        value /= 10;
	    ++result;
    }

    return result;
}

#endif //TE_MATH_H
