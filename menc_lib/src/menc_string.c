Public String Str_Alloc(Memory_Arena* arena, u32 count) {
    return (String){
        .count = 0,
        .E = Arena_Alloc_Array(arena, u8, count),
    };
}

Public u32 Str_Find_Any_Char_Backward(String string, String chars) {
    return FindAnyByteBackward((string.E + string.count - 1), string.E, chars.E, chars.count, 0);
}

Public u32 Str_Find_Diff_Char_Forward(String str, u8 character) {
    return FindAnyDiffByteForward(str.E, str.count, &character, 1, 0);
}

Public u32 Str_Find_Any_Diff_Char_Forward(String str, String chars) {
    return FindAnyDiffByteForward(str.E, str.count, chars.E, chars.count, 0);
}

Public u32 Str_Find_Char_Forward(String str, u8 inChar) {
    return FindByteForward(str.E, &str.E[str.count] - 1, (byte)inChar);
}

Public void Str_Copy(String* dst, String src) {
    Mem_Copy_Forward(dst->E, src.E, src.count * sizeof(src.E[0]));
    dst->count = src.count;
}

Public void Str_Null_Terminate(String* str) {
    str->E[str->count] = '\0';
}

Public String Str_Alloc_And_Copy(Memory_Arena* arena, String src, b8 shouldPutNullTerminator) {
    u32 count = src.count;
    if ( shouldPutNullTerminator ) { ++count; }

    String ret = Str_Alloc(arena, count);
    Str_Copy(&ret, src);

    if ( shouldPutNullTerminator ) { Str_Null_Terminate(&ret); }

    return ret;
}

Public u32 Str_Find_Char_Backward_Times(String string, u8 charToFind, u32 timesToIgnore) {
    return FindCharBackwardsTimes(string.E + string.count - 1, string.count, (byte)charToFind, timesToIgnore);
}

Public u32 Str_Find_Char_Backward(String string, u8 charToFind) {
    return FindByteBackwards(string.E + string.count - 1, string.count, (byte)charToFind);
}

Public b8 CStr_Size_Equal(CString str1, CString str2, u32 count) {
    return ( Mem_Comp(str1, str2, count) == 0 );
}

Public String Str_Begin(String string, u32 count) {
    String ret = (String){ .E = string.E, .count = CapTop(string.count, count) };
    return ret;
}

Public String Str_End(String string, u32 count) {
    u32 cappedSize = CapTop(string.count, count);
    u32 CharsSkip = string.count - cappedSize;
    String ret = (String){ .E = (string.E + CharsSkip), .count = cappedSize };
    return ret;
}

Public String Str_Skip_Begin(String string, u32 skip) {
    u32 cappedSkip = CapTop(string.count, skip);
    String ret = (String){ .E = (string.E  + cappedSkip), .count = (string.count - cappedSkip) };
    return ret;
}

Public String Str_Skip_End(String string, u32 skip) {
    u32 cappedSkip = CapTop(string.count, skip);
    String ret = (String){ .E = string.E, .count = (string.count - cappedSkip) };
    return ret;
}

Public void* Check_Bounds(void* min, void* x, void* max) {
    Assert( Is_In_Bounds(min, x, max) );
    return x;
}

Public String Str_Begin_Ptr(String string, u8* ptr) {
    Array_Assert_Bounds(string, ptr);
    return (String){ .E = ptr, .count = (u32)( &string.E[string.count] - ptr ) };
}

Public b8 Chars_Equal(u8* str1, u8* str2, u32 count) {
    return ( Mem_Comp(str1, str2, count) == 0 );
}

Public b8 Chars_Ptr_Equal(u8* ptr, String str) {
    return ( Mem_Comp(ptr, str.E, str.count) == 0 );
}

Public s32 CStr_Comp(CString str1, CString str2) {
    u8* scan1 = str1;
    u8* scan2 = str2;

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

Public u32 CStr_Len(CString str) {
    u8* scan = &str[0];

    while (*scan++ != '\0');

    // Decrement by 1 to compensate the last increment made to the scan by the "++" operator.
    u32 len = (u32)((scan - 1) - &str[0]);
    return len;
}

Public String CStr_To_Str(CString str) {
    return (String){ .E = str, .count = CStr_Len(str) };
}

Public s32 Str_Comp(String str1, String str2) {
    if (str1.count != str2.count) {
        return (str1.count > str2.count) ? 1 : -1;
    }

    return CompCStrSize(str1.E, str2.E, str1.count);
}

Public b8 Str_Equal(String str1, String str2) {
    return Str_Comp(str1, str2) == 0;
}

Public b8 Str_Begin_Equal(String a, String b) {
    if ( a.count <= b.count ) { return false; }
    //if ( a.count <= b.count ) { Swap(&a, &b, String); }

    return ( Str_Equal(Str_Begin(a, b.count), b) );
}

Public b8 Str_End_Equal(String a, String b) {
    if ( a.count <= b.count ) { Swap(&a, &b, String); }

    return ( Str_Equal(Str_End(a, b.count), b) );
}

Public u32 Str_Find_Str_Forward(String src, String match) {
    for (String scan = src; ; scan = Str_Skip_Begin(scan, 1)) {
        if ( scan.count < match.count ) { return MAX_U32; }

        if ( Chars_Equal(scan.E, match.E, match.count) ) {
            return (u32)( scan.E - src.E );
        }
    }
}

Public String Str_Consume(String src, String match, b8 shouldAdvanceMatch) {
    u32 index = Str_Find_Str_Forward(src, match);

    if ( index == MAX_U32 ) { return (String){ 0 }; }

    if ( shouldAdvanceMatch ) {
        return Str_Skip_Begin(src, index + match.count );
    } else {
        return Str_Skip_Begin(src, index);
    }
}

Public void CatStr(String* strOut, String str1, String str2) {
    CopyCStrForward(strOut->E, str1.E, str1.count);
    CopyCStrForward(strOut->E + str1.count, str2.E, str2.count);
    strOut->count = str1.count + str2.count;
}

Public String Str_Get_File_Name_In_Path(String path) {
    u32 nameSize = Str_Find_Char_Backward(path, OS_PATH_COMPONENT_SEPARATOR_CHAR);
    String retName = (nameSize != MAX_U32) ? Str_End(path, nameSize) : path;
    return retName;
}

Public void Str_16_To_8(u16* inStr16, u32 inSize, u8* outStr8) {
    for (u32 i = 0; i < inSize; ++i) {
        outStr8[i] = (u8)inStr16[i];
    }
}

Public void Str_Push(String* dst, String src) {
    Array_Copy(&dst->E[dst->count], src.E, src.count);
    dst->count += src.count;
}

Public void Str_Push_Char(String* dst, u8 ch) {
    dst->E[dst->count] = ch;
    ++dst->count;
}

Public b8 Array_String_Exists(Array_String strs, String str) {
    foreach (String, iter, strs) {
        if ( Str_Equal(*iter, str) ) {
            return true;
        }
    }

    return false;
}
