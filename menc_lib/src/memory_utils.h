#ifndef DSV_MEMORY_UTILS_H
#define DSV_MEMORY_UTILS_H

#define Mem_Shift_Right(mem, size, shifts) Mem_Copy_Backward(((byte*)(mem) + (shifts)), mem, size)
#define Mem_Shift_Right_Ptr(mem, ptr, shifts) \
    do { \
        Assert((byte*)(ptr) <= (byte*)(mem)); \
        Mem_Shift_Right(mem, (u32)((byte*)(mem) - (byte*)(ptr) + 1), shifts); \
    } while (0)

Public void Mem_Copy_Backward(void* dest, void* src, u32 size) {
    byte* scanSrc  = (byte*)src;
    byte* scanDest = (byte*)dest;

    while ( size-- ) { *scanDest-- = *scanSrc--; }
}

#define Mem_Shift_Right_Array(inArray, inStartIndex, inEndIndex, inShiftCount) \
    do { \
        Assert( (s32)(inStartIndex) <= (s32)(inEndIndex) ); \
        Mem_Shift_Right((u8*)(&(inArray)[(inEndIndex)+1]) - 1, ((inEndIndex) - (inStartIndex) + 1) * sizeof((inArray)[0]), (inShiftCount) * sizeof((inArray)[0])); \
    } while ( 0 )

#define Mem_Shift_Left(mem, size, shifts) Mem_Copy_Forward((byte*)(mem) - (shifts), mem, size)
#define Mem_Shift_Left_Ptr(mem, ptr, shifts) \
    do { \
        Assert((byte*)(ptr) >= (byte*)(mem)); \
        Mem_Shift_Left(mem, (u32)((byte*)(mem) - (byte*)(ptr) + 1), shifts); \
    } while (0)

#define Array_Copy(dst, src, count) Mem_Copy_Forward((dst), (src), (count) * sizeof((src)[0]))
#define Mem_Copy_Type(dst, src) Mem_Copy_Forward(dst, src, sizeof(src))
Public void Mem_Copy_Forward(void* dest, void* src, u32 size) {
    byte* scanSrc  = (byte*)src;
    byte* scanDest = (byte*)dest;

    while (size--) { *scanDest++ = *scanSrc++; }
}

#define Mem_Shift_Left_Array(inArray, inStartIndex, inEndIndex, inShiftCount) \
    do { \
        Assert( (inStartIndex) <= (inEndIndex) && (Safe_Cast_U32_To_S32(inStartIndex) - Safe_Cast_U32_To_S32(inShiftCount)) >= 0 ); \
        Mem_Shift_Left(&(inArray)[inStartIndex], ((inEndIndex) - (inStartIndex) + 1) * sizeof((inArray)[0]), (inShiftCount) * sizeof((inArray)[0])); \
    } while ( 0 )

Public u32 FindAnyDiffByteForward(void* mem, u32 size, void* bytesToFind, u32 sizeBytes, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    byte* bytes = (byte*)bytesToFind;
    u32 lastFindIndex = MAX_U32;

    for (u32 index = 0; index < size; ++index) {
        for (u32 i = 0; i < sizeBytes; ++i) {
            if (*scan == bytes[i]) { goto skip_byte; }
        }

        if (timesLeft-- == 0) { return index; }
        lastFindIndex = index;

skip_byte:
        ++scan;
    }

    return lastFindIndex;
}

Public void* FindAnyDiffByteBackward(void* mem, void* end, void* bytesToFind, u32 sizeBytes, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    byte* lastByteFind = 0;
    byte* bytes = (byte*)bytesToFind;

    while (scan >= (byte*)end) {
        for (u32 i = 0; i < sizeBytes; ++i) {
            if (*scan == bytes[i]) { goto skip_byte; }
        }

        if (timesLeft-- == 0) { return scan; }
        lastByteFind = scan;

skip_byte:
        --scan;
    }

    return lastByteFind;
}

#define FindAnyByteForward(mem, end, bytesToFind, sizeBytes) FindAnyByteForwardTimes(mem, end, bytesToFind, sizeBytes, 0)
Public u32 FindAnyByteForwardTimes(void* mem, u32 count, void* bytesToFind, u32 byteCount, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    byte* bytes = (byte*)bytesToFind;
    u32 lastFindIndex = MAX_U32;

    while (count--) {
        for (u32 i = 0; i < byteCount; ++i) {
            if (*scan == bytes[i]) {
                if (timesLeft-- == 0) { return (u32)(scan - (byte*)mem); }
                lastFindIndex = (u32)(scan - (byte*)mem);
                break;
            }
        }

        ++scan;
    }

    return lastFindIndex;
}

Public u32 FindAnyByteBackward(void* mem, void* end, void* bytesToFind, u32 sizeBytes, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    byte* bytes = (byte*)bytesToFind;
    u32 lastFindIndex = MAX_U32;

    while (scan >= (byte*)end) {
        for (u32 i = 0; i < sizeBytes; ++i) {
            if (*scan == bytes[i]) {
                if (timesLeft-- == 0) { return (u32)(scan - (byte*)mem); }
                lastFindIndex = (u32)(scan - (byte*)mem);
                break;
            }
        }

        --scan;
    }

    return lastFindIndex;
}

#define FindByteForward(mem, end, byteToFind) FindByteForwardTimes(mem, end, byteToFind, 0)
Public u32 FindByteForwardTimes(void* mem, void* end, byte byteToFind, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    u32 lastFindIndex = MAX_U32;

    while (scan <= (byte*)end) {
    	if (*scan == byteToFind) {
            if (timesLeft-- == 0) { return (u32)(scan - (byte*)mem); }
            lastFindIndex = (u32)(scan - (byte*)mem);
	    }

        ++scan;
    }

    return lastFindIndex;
}

#define FindByteBackwards(mem, size, byteToFind) FindByteBackwardsTimes(mem, size, byteToFind, 0)
Public u32 FindByteBackwardsTimes(void* mem, u32 size, byte byteToFind, u32 timesToIgnore) {
    u32 timesLeft = timesToIgnore;

    byte* scan = (byte*)mem;
    u32 lastByteFind = MAX_U32;

    for (u32 index = 0; index < size; ++index) {
    	if (*scan == byteToFind) {
            if (timesLeft-- == 0) { return index; }
            lastByteFind = index;
	    }

        --scan;
    }

    return lastByteFind;
}

#define Mem_Zero(mem, size) Mem_Fill_With_Byte(mem, size, 0)
#define Mem_Zero_Type(type) Mem_Zero(type, sizeof(*(type)))
#define Mem_Zero_Array(arr) Mem_Zero(arr, sizeof(arr))
Public void Mem_Fill_With_Byte(void* mem, u32 size, byte value) {
    byte* scan = (byte*)mem;

    while (size--) { *scan++ = value; }
}

#define Mem_Comp_Type(mem1, mem2) CompMem(mem1, mem2, sizeof(*(mem1)))
Public s32 Mem_Comp(void* mem1, void* mem2, u32 size) {
    byte* scan1 = (byte*)mem1;
    byte* scan2 = (byte*)mem2;

    while (size--) {
        if (*scan1 != *scan2) {
            return (*scan1 < *scan2) ? 1 : -1;
        }

        scan1++;
        scan2++;
    }

    return 0;
}

#define Mem_Type_Equal(inMem1, inMem2) Mem_Equal(inMem1, inMem2, sizeof(*inMem1))

Public b8 Mem_Equal(void* inMem1, void* inMem2, u32 inSize) {
    return ( Mem_Comp(inMem1, inMem2, inSize) == 0 );
}

#define Mem_Equal_Zero_Array(array) Mem_Equal_Zero(array, sizeof(array))
#define Mem_Equal_Zero_Type(type) Mem_Equal_Zero(type, sizeof(*(type)))
Public b8 Mem_Equal_Zero(void* mem, u32 size) {
    byte* scan = (byte*)mem;

    for (u32 index = 0; index < size; ++index) {
        if (scan[index] != 0) {
            return false;
        }
    }

    return true;
}

#endif //DSV_MEMORY_UTILS_H
