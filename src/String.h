#ifndef TE_STRING_H
#define TE_STRING_H

#define OS_PATH_COMPONENT_SEPARATOR_CHAR '\\'

#define Local_Str(strName, bufSize) Local_Str_(strName, bufSize, __COUNTER__)
#define Local_Str_(strName, bufSize, counter) Local_Str__(strName, bufSize, counter)
#define Local_Str__(strName, bufSize, counter) \
    char dummyBufferName##counter [bufSize]; \
    String strName; \
    strName.str = dummyBufferName##counter

typedef struct {
    u32 length;
} LengthBaseString;

#define BACKWARDSTRING(address, length) ((char *)(address) - (length))

typedef struct {
    u32 length;
    char str[0];
} Len_Prefix_String;

typedef struct {
    union {
        u32 size;
    };
    char *str;
} String;

Array_Ptr(Array_String, String);

typedef char *CString;
Array_Ptr(Array_CString, CString);

#define LitToStrNul(strLit) Str((char *)strLit, ArrayCount(strLit))
#define LitToStr(strLit) Str((char *)strLit, ArrayCount(strLit) - 1)
Intern String Str(char *str, u32 size) {
    String ret;
    ret.size = size;
    ret.str = str;
    return ret;
}

#define CopyCStrBackwardToPointer(dest, src, end)         \
    CopyCStrBackward(dest, src, (u32)((src) - (end) + 1))

#define CopyCStrForward(dest, src, size) Mem_Copy_Forward(dest, src, size * sizeof((src)[0]))
#define AllocAndCopyCStrForward(arena, dest, src, size) \
    (dest) = ArenaPush(arena, sizeof((src)[0]) * size); \
    CopyCStrForward(dest, src, size)

#define CopyCStrBackward(dest, src, size) Mem_Copy_Backward(dest, src, size * sizeof((src)[0]))
#define AllocAndCopyCStrBackward(arena, dest, src, size) \
    (dest) = ArenaPush(arena, sizeof((src)[0]) * size);  \
    CopyCStrBackward(dest, src, size)

#define CopyStrForward(dest, src) Mem_Copy_Forward(dest.str, src.str, src.size * sizeof((src).str[0]))
#define AllocAndCopyStrForward(arena, dest, src)                                \
    do { \
        (dest).str = (char *)ArenaPush(arena, sizeof((src).str[0]) * ((src).size + 1)); \
        CopyStrForward(dest, src); \
        (dest).size = (src).size; \
    } while (0)

#define CopyStrBackward(dest, src) Mem_Copy_Backward(dest.str, src.str, src.size * sizeof((src).str[0]))
#define AllocAndCopyStrBackward(arena, dest, src)                     \
    (dest).str = ArenaPush(arena, sizeof((src).str[0]) * (src).size); \
    CopyStrBackward(dest, src)

Public u32 Str_Find_Any_Diff_Char_Forward(char* str, char* end, String chars) {
    return FindAnyDiffByteForward(str, (u32)(end - str), chars.str, chars.size, 0);
}

#define FindAnyDiffCharForward(mem, end, chars, timesToIgnore) \
    (char *)FindAnyDiffByteForward(mem, end, chars.str, chars.size, timesToIgnore)

#define FindAnyDiffCharBackward(mem, end, chars, sizeChars, timesToIgnore) \
    (char *)FindAnyDiffByteBackward(mem, end, chars, sizeChars, timesToIgnore)

#define FindAnyCharForwardTimes(mem, memSize, chars, timesToIgnore) \
    (char *)FindAnyByteForwardTimes(mem, memSize, chars.str, chars.size, timesToIgnore)

#define FindAnyCharForward(mem, end, chars) FindAnyByteForward(mem, end, chars.str, chars.size)

Public u32 Str_Find_Any_Char_Backward(String string, String chars) {
    return FindAnyByteBackward((string.str + string.size - 1), string.str, chars.str, chars.size, 0);
}

#define FindCharForwardTimes(mem, end, charToFind, timesToIgnore) \
    (char *)FindByteForwardTimes(mem, end, charToFind, timesToIgnore)

#define FindCharBackwardsTimes(mem, end, charToFind, timesToIgnore) \
    (char *)FindByteBackwardsTimes(mem, end, charToFind, timesToIgnore)

#define StrFindCharForward(s, c) FindByteForward((s).str, ((s).str + (s).size), c)
Public u32 Str_Find_Char_Forward(char* inStr, char* inEnd, char inChar) {
    return FindByteForward(inStr, inEnd, (byte)inChar);
}

#define FindCharBackwards(string, charToFind) \
    FindByteBackwards((((string).str + (string).size) - 1), (string).size, charToFind)

Public u32 Str_Find_Char_Backward(String string, char charToFind) {
    return FindByteBackwards(string.str + string.size - 1, string.size, (byte)charToFind);
}

#define CompCStrSize(str1, str2, size) Mem_Comp(str1, str2, size)

Public b8 CStr_Size_Equal(CString str1, CString str2, u32 size) {
    return ( Mem_Comp(str1, str2, size) == 0 );
}

Intern String Str_Begin(String string, u32 size) {
    String ret = Str(string.str, CapTop(string.size, size));
    return ret;
}

Intern String Str_End(String string, u32 size) {
    u32 cappedSize = CapTop(string.size, size);
    u32 CharsSkip = string.size - cappedSize;
    String ret = Str((string.str + CharsSkip), cappedSize);
    return ret;
}

Intern String Str_Skip_Begin(String string, u32 skip) {
    u32 cappedSkip = CapTop(string.size, skip);
    String ret = Str((string.str  + cappedSkip), (string.size - cappedSkip));
    return ret;
}

Intern String Str_Skip_End(String string, u32 skip) {
    u32 cappedSkip = CapTop(string.size, skip);
    String ret = Str(string.str, (string.size - cappedSkip));
    return ret;
}

Public b8 Chars_Equal(char* str1, char* str2, u32 size) {
    return ( Mem_Comp(str1, str2, size) == 0 );
}

Intern s32 CStr_Comp(CString str1, CString str2) {
    char *scan1 = str1;
    char *scan2 = str2;

    while (*scan1 != '\0') {
        if (*scan1 != *scan2) {
            return (*scan1 > *scan2) ? 1 : -1;
        }

        ++scan1;
        ++scan2;
    }

    return (*scan2 == '\0') ? 0 : -1;
}

Public b8 CStr_Equal(CString str1, CString str2) {
    return ( CStr_Comp(str1, str2) == 0 );
}

Intern u32 CStrLen(CString str) {
    char *scan = &str[0];

    while (*scan++ != '\0');

    // Decrement by 1 to compensate the last increment made to the scan by the "++" operator.
    u32 len = (u32)((scan - 1) - &str[0]);
    return len;
}

#if 0
Intern void S32ToCStr(CString str, s32 value, u32 digitsCount) {
    char *scan = str + digitsCount - 1;

    while (digitsCount--) {
        u8 mod = (u8)(value % 10);
        value /= 10;
        char digitChar = Digit_To_Char(mod);
        *scan-- = digitChar;
    }
}

Intern s32 StrToS32(String string) {
    s32 ret = 0;

    for (u32 i = 0; i < string.size; ++i) {
        ret = (10 * ret) + CharToDigit(string.str[i]);
    }

    return ret;
}
#endif

Intern s32 Str_Comp(String str1, String str2) {
    if (str1.size != str2.size) {
        return (str1.size > str2.size) ? 1 : -1;
    }

    return CompCStrSize(str1.str, str2.str, str1.size);
}

Public b8 Str_Equal(String str1, String str2) {
    return Str_Comp(str1, str2) == 0;
}

Intern void CatStr(String *strOut, String str1, String str2) {
    CopyCStrForward(strOut->str, str1.str, str1.size);
    CopyCStrForward(strOut->str + str1.size, str2.str, str2.size);
    strOut->size = str1.size + str2.size;
}

Public String Str_Get_File_Name_In_Path(String path) {
    u32 nameSize = Str_Find_Char_Backward(path, OS_PATH_COMPONENT_SEPARATOR_CHAR);
    String retName = (nameSize != MAX_U32) ? Str_End(path, nameSize) : path;
    return retName;
}

Public void Str_16_To_8(c16* inStr16, u32 inSize, c8* outStr8) {
    for (u32 i = 0; i < inSize; ++i) {
        outStr8[i] = (c8)inStr16[i];
    }
}
#endif //TE_STRING_H
