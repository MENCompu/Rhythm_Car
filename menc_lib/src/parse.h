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

#pragma pack(push, 1)
typedef struct {
    u32 count;

#if 0
    u64 offset;
    u8* E;
#else
    File_Ptr(u8*, E);
#endif
} File_String;
#pragma pack(pop)

typedef struct {
    String data;

    u8* scan;
    u8* end;
} String_Scan;

Intern String_Scan StrScan(u8* scan, u8* end) {
    String_Scan ret;
    ret.scan = scan;
    ret.end  = end;
    return ret;
}

Intern String_Scan Str_Scan_From_Str(String string) {
    String_Scan ret;
    ret.scan = string.E;
    ret.end  = string.E + string.count;
    return ret;
}

#define FindCharForward(mem, end, u8ToFind) FindByteForward(mem, end, charToFind)

#define Str_Parse_Fmt(src, fmt, ...) (b8)sscanf((char*)(src).E, fmt, __VA_ARGS__)

// TODO(JENH): This is could lead to race conditions.
Private Global u32 dummyArg;

#define Str_Parse_And_Consume_Fmt(src, fmt, ...) Str_Parse_And_Consume_Fmt_Impl(src, fmt, __VA_ARGS__, &dummyArg)
Public String Str_Parse_And_Consume_Fmt_Impl(String src, String fmt, ...) {
    Assert( fmt.count <= KiB(4) );
    Local_Str(fmtNew, KiB(4));
    Str_Copy(&fmtNew, fmt);
    Str_Push(&fmtNew, S("%n\0"));

    va_list args;
    va_start(args, fmt);
    (void)vsscanf((char*)src.E, (char*)fmtNew.E, args);
    va_end(args);

    return Str_Skip_Begin(src, dummyArg);
}

Intern void StrScanAdvance(String_Scan* strScan, u32 charCount) {
    if (strScan->scan >= strScan->end) {
        strScan->data.E  = 0;
        strScan->data.count = 0;
        return;
    }

    u32 cappedCharCount = CapTop((u32)(strScan->end - strScan->scan), charCount);
    strScan->data.E = strScan->scan;
    strScan->data.count = cappedCharCount;
    strScan->scan += cappedCharCount + 1;
}

Intern b8 StrScanEOS(String_Scan strScan) {
    b8 ret = (strScan.scan >= strScan.end);
    return ret;
}

Intern inline u8 Digit_To_Char(u8 inDigit) {
    Assert( 0 <= inDigit && inDigit <= 9 );
    return (u8)(inDigit + 48);
}

Intern inline u8 Char_To_Digit(u8 digitChar) {
    return (u8)(digitChar - 48);
}

Intern inline u8 Hex_Digit_To_Char(u8 value) {
    Assert( value <= 0xf );
    return ( value <= 9 ) ? (u8)(value + 48) : (u8)(value + 97 - 10);
}

Intern inline u8 Char_To_Hex_Digit(u8 digitChar) {
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

Public b8 Str_Is_Integer(String str) {
    foreach (u8, c, str) {
        if ( *c < '0' || *c > '9' ) {
            return false;
        }
    }

    return true;
}

#if 1
Public String U32_To_Str(u32 value) {
    String out = Str_Alloc(&gTempArena, KiB(4));

    out.count = Get_Digit_Count(value);
    u32 index = out.count - 1;

    do {
        u8 digit = value % 10;
        out.E[index--] = Digit_To_Char(digit);
        value /= 10;
    } while ( value != 0 );

    return out;
}
#else
Public void U32_To_Str(u32 inValue, String* outString) {
    outString->count = Get_Digit_Count(inValue);
    u32 index = outString->count - 1;

    do {
        u8 digit = inValue % 10;
        outString->E[index--] = Digit_To_Char(digit);
        inValue /= 10;
    } while ( inValue != 0 );
}
#endif

Intern s64 Str_To_S64(String string) {
    s32 sing = 1;
    s64 ret = 0;

    u8 *endStr = string.E + string.count;
    u8 *scan = string.E;

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

    u8 *endStr = string.E + string.count;
    u8 *scan = string.E;

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

#if 0
Public void StrScan_Consume_White_Space(String_Scan* strScan) {
    strScan->scan += Str_Find_Any_Diff_Char_Forward(strScan->scan, strScan->end, S(" \t\r\n"));
}
#endif

Public void* Abs_To_Rel_Ptr(void* base, void* ptr) {
    return (void*)((byte*)ptr - (byte*)base);
}

Public void* Rel_To_Abs_Ptr(void* base, void* ptr) {
    return (void*)((byte*)base + (WordSize)ptr);
}

/* This should be for 64 bits? */
Public String Str_From_File_Str(File_String inFileString) {
    return (String){ .E = inFileString.E, .count = inFileString.count };
}

Public File_String File_String_From_Str(String str) {
    return (File_String){ .count = str.count, .E = str.E, };
}

#endif //JENH_PARSE_H
