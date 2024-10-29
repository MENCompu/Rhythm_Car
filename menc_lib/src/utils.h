#ifndef UTILS_H
#define UTILS_H

#define KB(x) (  (x) * 1000)
#define MB(x) (KB(x) * 1000)
#define GB(x) (MB(x) * 1000)
#define TB(x) (GB(x) * 1000)

#define KiB(x) (   (x) * 1024)
#define MiB(x) (KiB(x) * 1024)
#define GiB(x) (MiB(x) * 1024)
#define TiB(x) (GiB(x) * 1024)

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define CapTop(cap, val) Min(cap, val)
#define CapBtm(cap, val) Max(cap, val)
#define Clip(bottom, val, top) (((val) < (bottom)) ? (bottom) : ((val) > (top)) ? (top) : (val))
#define Clamp(bottom, val, top) (((val) < (bottom)) ? (bottom) : ((val) > (top)) ? (top) : (val))

#define Bit(p) (1 << p)

#define Swap(var1, var2, type) \
    do {		               \
        type temp = *(var1);   \
		*(var1) = *(var2);     \
		*(var2) = temp;        \
    } while (0);

//#define Fiel(structType, field) ((((structType *)0)->field))
#define FieldOffset(structType, field) ((WordSize)&(((structType *)0)->field))
#define FieldSize(structType, field) sizeof(Field(structType, field))
#define Field_Size(structType, field) sizeof(Field(structType, field))
#define Field_Array_Count(structType, field) (FieldSize(structType, field) / sizeof(((structType *)0)->field[0]))

#define Poly(type, data) ( (type*)Poly_((u32*)&(data)->tag, P(TAG_, type), &(data)->P(var,type)) )

#define foreach(type, name, slice)                                          \
    for (type* name = (slice).E; name < ((slice).E + (slice).count); ++name)

#define forrange(iterName, count)                        \
    for (u32 iterName = 0; iterName < count; ++iterName)

// defer
#define defer(retVal) do { ret = (retVal); goto defer; } while(0)

#define CACHE_LINE_SIZE 64
#define _cache_align __declspec(align(CACHE_LINE_SIZE))

#define _no_alias __restrict

#define INVALID_THING_64 (u64)( 0xffffffffffffffff )
#define INVALID_THING_32 (u32)( 0xffffffff )
#define INVALID_THING_P  (void*)( 0xffffffffffffffff )

#define P(a, b) a##b

#ifndef P2
    #define P2(a, b, c) a##b##c
#endif

#define Macro_To_Str(macro) #macro

Public b8 Toggle(b8* boolean) {
    *boolean = !(*boolean);
    return *boolean;
}

#define Loop for (;;)

Public b8 Is_In_Bounds(void* min, void* x, void* max) {
    return ( ( min <= x ) && ( x <= max ) );
}

#endif //UTILS_H
