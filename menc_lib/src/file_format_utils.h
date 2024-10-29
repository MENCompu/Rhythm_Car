#ifndef JENH_FILE_FORMAT_UTILS_H
#define JENH_FILE_FORMAT_UTILS_H

#define Magic_2(str) ((((u16)(str)[0]) << 0) | (((u16)(str)[1]) << 8))
#define Magic_4(str) ((((u32)(str)[0]) << 0) | (((u32)(str)[1]) << 8) | (((u32)(str)[2]) << 16) | (((u32)(str)[3]) << 24))

#define C_Magic_2(str) ((((u16)(str)[0]) << 0) | (((u16)(str)[1]) << 8))
#define C_Magic_4(a, b, c, d) ((((u32)a) << 0) | (((u32)b) << 8) | (((u32)c) << 16) | (((u32)d) << 24))

#if defined(BUILD_64)
    #define Build_32_Padding(inVarName)
    #define Check_Padding(inVarName)
#elif defined(BUILD_32)
    #define Build_32_Padding(inVarName) u32 inVarName##_build_32_reserved; /* This doesn't work currently */
    #define Check_Padding(inVarName) (void)inVarName##_build_32_reserved
#else
    #error should define BUILD_32 or BUILD_64
#endif


#define File_Ptr(ptrType, inVarName) \
    union { \
        u64 offset; \
        struct { \
            ptrType inVarName; \
            Build_32_Padding(inVarName) \
        }; \
    }


#define File_Ptr_Do(name, srcBase, dstBase) \
    Check_Padding(name); \
    *(WordSize*)(name) = ( *(WordSize*)(name) >= (WordSize)srcBase ) ? ((*(WordSize*)(name) - (WordSize)srcBase) + (WordSize)dstBase) : 0

#define File_Ptr_Undo(name, srcBase, dstBase) \
    Check_Padding(name); \
    *(WordSize*)(name) = ( *(WordSize*)(name) >= (WordSize)srcBase ) ? ((*(WordSize*)(name) - (WordSize)srcBase) + (WordSize)dstBase) : 0



#define File_Ptr_Array_Do(name, srcBase, dstBase) \
    Check_Padding(name); \
    *(WordSize*)(name) = ( *(WordSize*)(name) >= (WordSize)srcBase ) ? ((*(WordSize*)(name) - (WordSize)srcBase) + (WordSize)dstBase) : 0

#define File_Ptr_Array_Undo(name, srcBase, dstBase) \
    Check_Padding(name); \
    *(WordSize*)(name) = ( *(WordSize*)(name) >= (WordSize)srcBase ) ? ((*(WordSize*)(name) - (WordSize)srcBase) + (WordSize)dstBase) : 0

#if 0
#define File_Ptr_Do(name, base) \
    Check_Padding(inVarName);
    (*name##_FP) = (u64)((byte*)(void*)*name - (byte*)(void*)base)

#define File_Ptr_Undo(name, base) \
    Check_Padding(inVarName);
    (*name) = (void*)(fileBase + (*name##_FP))
#endif

#endif // JENH_FILE_FORMAT_UTILS_H
