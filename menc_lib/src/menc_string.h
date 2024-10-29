#ifndef TE_STRING_H
#define TE_STRING_H

#include <types.h>

#define OS_PATH_COMPONENT_SEPARATOR_CHAR '\\'

#define Local_Str(strName, bufSize) Local_Str_(strName, bufSize, __LINE__)
#define Local_Str_(strName, bufSize, counter) Local_Str__(strName, bufSize, counter)
#define Local_Str__(strName, bufSize, counter) \
    u8 P(dummyBufferName, counter) [bufSize]; \
    String strName; \
    strName.count = 0; \
    strName.E = P(dummyBufferName, counter); \
    Mem_Zero(P(dummyBufferName, counter), bufSize)

#if 0
typedef struct {
    u32 length;
} LengthBaseString;

#define BACKWARDSTRING(address, length) ((u8*)(address) - (length))

typedef struct {
    u32 length;
    u8 str[0];
} Len_Prefix_String;
#endif

typedef struct {
    u32 count;
    u8* E;
} String;

Array(String);
Slice(String);

typedef u8* CString;
Array(CString);

#define S_Const(lit) { .E = (u8*)lit, .count = ( Array_Count(lit) - 1 ) }

#define SNul(strLit) (String){ .E = (u8*)strLit, .count = Array_Count(strLit) }
#define S(strLit) (String){ .E = (u8*)strLit, .count = ( Array_Count(strLit) - 1 ) }

#define CopyCStrBackwardToPointer(dest, src, end)         \
    CopyCStrBackward(dest, src, (u32)((src) - (end) + 1))

#define CopyCStrForward(dest, src, count) Mem_Copy_Forward(dest, src, count * sizeof((src)[0]))
#define AllocAndCopyCStrForward(arena, dest, src, count) \
    (dest) = ArenaPush(arena, sizeof((src)[0]) * count); \
    CopyCStrForward(dest, src, count)

#define CopyCStrBackward(dest, src, count) Mem_Copy_Backward(dest, src, count * sizeof((src)[0]))
#define AllocAndCopyCStrBackward(arena, dest, src, count) \
    (dest) = ArenaPush(arena, sizeof((src)[0]) * count);  \
    CopyCStrBackward(dest, src, count)

//#define CopyStrForward(dest, src) Mem_Copy_Forward((dest).str, (src).str, src.count * sizeof((src).str[0]))
#define AllocAndCopyStrForward(arena, dest, src)                                \
    do { \
        (dest).str = (u8*)ArenaPush(arena, sizeof((src).str[0]) * ((src).count + 1)); \
        CopyStrForward(dest, src); \
        (dest).count = (src).count; \
    } while (0)

#define CopyStrBackward(dest, src) Mem_Copy_Backward(dest.str, src.str, src.count * sizeof((src).str[0]))
#define AllocAndCopyStrBackward(arena, dest, src)                     \
    (dest).str = ArenaPush(arena, sizeof((src).str[0]) * (src).count); \
    CopyStrBackward(dest, src)

#define FindAnyDiffCharForward(mem, end, chars, timesToIgnore) \
    (u8*)FindAnyDiffByteForward(mem, end, chars.str, chars.count, timesToIgnore)

#define FindAnyDiffCharBackward(mem, end, chars, sizeChars, timesToIgnore) \
    (u8*)FindAnyDiffByteBackward(mem, end, chars, sizeChars, timesToIgnore)

#define FindAnyCharForwardTimes(mem, memSize, chars, timesToIgnore) \
    (u8*)FindAnyByteForwardTimes(mem, memSize, chars.str, chars.count, timesToIgnore)

#define FindAnyCharForward(mem, end, chars) FindAnyByteForward(mem, end, chars.str, chars.count)

#define FindCharForwardTimes(mem, end, charToFind, timesToIgnore) \
    (u8*)FindByteForwardTimes(mem, end, charToFind, timesToIgnore)

#define FindCharBackwardsTimes(mem, end, charToFind, timesToIgnore) \
    FindByteBackwardsTimes(mem, end, charToFind, timesToIgnore)

#define StrFindCharForward(s, c) FindByteForward((s).str, ((s).str + (s).count), c)

#define FindCharBackwards(string, charToFind) \
    FindByteBackwards((((string).str + (string).count) - 1), (string).count, charToFind)

#define CompCStrSize(str1, str2, count) Mem_Comp(str1, str2, count)

#define Str_From_Fmt(outString, fmt, ...) (outString)->count = (u32)sprintf((char*)(outString)->E, (char*)(fmt), __VA_ARGS__)

#endif //TE_STRING_H
