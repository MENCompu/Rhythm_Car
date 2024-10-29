#ifndef JENH_PARSE_H
#define JENH_PARSE_H

#define Consume_Type(ioPtr, inType) (inType*)Consume(ioPtr, sizeof(inType))
Public void* Consume(void** ioPtr, u32 inSize) {
    void* retPtr = *ioPtr;
    *(byte**)ioPtr += inSize;
    return retPtr;
}

#define Consume_Offset_Type(inBase, inType, ioOffset) (inType*)Consume_Offset(inBase, sizeof(inType), ioOffset)
Public void* Consume_Offset(void* inBase, u32 inSize, u32* ioOffset) {
    void* retPtr = (byte*)inBase + *ioOffset;
    *ioOffset += inSize;
    return retPtr;
}

#if 1
#pragma pack(push, 1)
typedef struct {
    u32 size;
    union {
        char* str;
        u32 strOffset;
    };
    u32 reserver1;
} File_String_32;

typedef struct {
    u32 size;
    union {
        char* str;
        u64 strOffset;
    };
} File_String_64;

typedef File_String_64 File_String;
#pragma pack(pop)
#endif

typedef struct {
    String data;

    char* scan;
    char* end;
} String_Scan;

Intern String_Scan StrScan(char *scan, char *end) {
    String_Scan ret;
    ret.scan = scan;
    ret.end  = end;
    return ret;
}

Intern String_Scan StrToStrScan(String string) {
    String_Scan ret;
    ret.scan = string.str;
    ret.end  = string.str + string.size;
    return ret;
}

#define FindCharForward(mem, end, charToFind) FindByteForward(mem, end, charToFind)

Intern void StrScanAdvance(String_Scan *strScan, u32 charCount) {
    if (strScan->scan >= strScan->end) {
        strScan->data.str  = 0;
        strScan->data.size = 0;
        return;
    }

    u32 cappedCharCount = CapTop((u32)(strScan->end - strScan->scan), charCount);
    strScan->data.str = strScan->scan;
    strScan->data.size = cappedCharCount;
    strScan->scan += cappedCharCount + 1;
}

Intern b8 StrScanEOS(String_Scan strScan) {
    b8 ret = (strScan.scan >= strScan.end);
    return ret;
}

Intern inline char Digit_To_Char(u8 inDigit) {
    Assert( 0 <= inDigit && inDigit <= 9 );
    return (char)(inDigit + 48);
}

Intern inline u8 Char_To_Digit(char digitChar) {
    return (u8)(digitChar - 48);
}

Intern inline char Hex_Digit_To_Char(u8 value) {
    Assert( value <= 0xf );
    return ( value <= 9 ) ? (char)(value + 48) : (char)(value + 97 - 10);
}

Intern inline u8 Char_To_Hex_Digit(char digitChar) {
    return (u8)(digitChar - 48);
}

Public u16 Get_Digit_Count(u64 inValue) {
    u16 digitsCount = 1;

    while ( inValue > 9 ) {
        inValue /= 10;
        ++digitsCount;
    }

    return digitsCount;
}

Public void U32_To_Str(u32 inValue, String* outString) {
    outString->size = Get_Digit_Count(inValue);
    u32 index = outString->size - 1;

    do {
        u8 digit = inValue % 10;
        outString->str[index--] = Digit_To_Char(digit);
        inValue /= 10;
    } while ( inValue != 0 );
}

Intern s32 Str_To_S32(String string) {
    s32 sing = 1;
    s32 ret = 0;

    char *endStr = string.str + string.size;
    char *scan = string.str;

    if (*scan == '-') {
        sing = -1;
        ++scan;
    }

    while (scan < endStr) {
        //Assert('0' <= *scan && *scan <= '9');
#if 1
        if (!('0' <= *scan && *scan <= '9')) {
            break;
        }
#endif
        ret = (ret * 10) + Char_To_Digit(*scan++);
    }

    ret *= sing;

    return ret;
}

Intern f32 StrToF32(String string) {
    f32 ret = 0.0f;
    f32 fracDiv = 1.0f;
    f32 sing  = 1.0f;

    char *endStr = string.str + string.size;
    char *scan = string.str;

    if (*scan == '-') {
        sing = -1.0f;
        ++scan;
    }

    while (*scan != '.') {
        Assert('0' <= *scan && *scan <= '9');
        ret = (ret * 10) + Char_To_Digit(*scan++);
    }

    ++scan;

    while (scan < endStr) {
        Assert('0' <= *scan && *scan <= '9');
        ret = (ret * 10.0f) + Char_To_Digit(*scan++);
        fracDiv *= 10.0f;
    }

    ret = sing * (ret / fracDiv);

    return ret;
}

Public void StrScan_Consume_White_Space(String_Scan* strScan) {
    strScan->scan += Str_Find_Any_Diff_Char_Forward(strScan->scan, strScan->end, LitToStr(" \t\r\n"));
}

Public void* Abs_To_Rel_Ptr(void* base, void* ptr) {
    return (void*)((byte*)ptr - (byte*)base);
}

Public void* Rel_To_Abs_Ptr(void* base, void* ptr) {
    return (void*)((byte*)base + (WordSize)ptr);
}

#if 1 /* This should be for 64 bits? */
Public String Str_From_File_Str(File_String inFileString) {
    return Str(inFileString.str, inFileString.size);
}
#else
#endif

#endif //JENH_PARSE_H
