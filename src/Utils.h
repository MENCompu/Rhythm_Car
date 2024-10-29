#ifndef UTILS_H
#define UTILS_H

#define S(inStringLiteral) (CString)inStringLiteral

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
#define CapBottom(cap, val) Max(cap, val)
#define Clip(bottom, val, top) (((val) < (bottom)) ? (bottom) : ((val) > (top)) ? (top) : (val))
#define Clamp(bottom, val, top) (((val) < (bottom)) ? (bottom) : ((val) > (top)) ? (top) : (val))

#define Bit_Pos(p) (1 << p)

#define Mc_Util_Paste(a, b) a##b
#define Mc_To_Str(a) #a

#define Const_if(c) _Pragma("warning(suppress: 4127)") if (c)

#define DEBUGGER_BREAKPOINT __debugbreak()
//#define DEBUGGER_BREAKPOINT (*(int *)0 = 0)


#if 1
#define Assert(expr)                                                           \
    do {                                                                       \
        Const_if (!(expr)) {                                                   \
            LogFatal(S("%s:%d (%s): Assert failed with the expresion \"%s\""), \
                     __FILE__, __LINE__, __func__, #expr);                     \
            DEBUGGER_BREAKPOINT;                                               \
        }                                                                      \
    } while(0)
#endif

#define Swap(var1, var2, type) \
    do {		               \
        type temp = *(var1);   \
		*(var1) = *(var2);     \
		*(var2) = temp;        \
    } while (0);

#define INVALID_PATH(str) Assert(0 && str)
#define NO_DEFAULT default: { INVALID_PATH(S("There should not be a default.")); } break;
#define NO_ELSE else { INVALID_PATH(S("There should not be an else.")); }

#define Field(structType, field) ((((structType *)0)->field))
#define FieldOffset(structType, field) ((WordSize)&(((structType *)0)->field))
#define FieldSize(structType, field) sizeof(Field(structType, field))
#define Field_Size(structType, field) sizeof(Field(structType, field))
#define Field_Array_Count(structType, field) (FieldSize(structType, field) / sizeof(((structType *)0)->field[0]))

#define foreach(type, name, array)                                          \
    for (type* name = (array).A; name < ((array).A + (array).size); ++name)

#define Foreach(inType, inName, inArray, inCount) \
    for (inType* inName = inArray; inName < &(inArray)[inCount]; ++inName)

// defer
#define defer(retVal) do { ret = (retVal); goto defer; } while(0)

#define INFINITE_LOOP 1

#define CACHE_LINE_SIZE 64
#define _cache_align __declspec(align(CACHE_LINE_SIZE))

#define _no_alias __restrict

#define JENH_SAFE
#define JENH_DEBUG
#define INVALID_THING 0xffffffff

#ifdef JENH_DEBUG
//#define Check(inThingToCkeck, inExpr) inThingToCkeck
#define Check(inThingToCkeck, inExpr) Assert( (inThingToCkeck) inExpr)
#else
#define Check(inThingToCkeck, inExpr) inThingToCkeck
#endif

#define ASIO_Check(inFuncCall) \
    do { \
        if ( !(inFuncCall) ) { \
            LogError(S("(ASIO %u) %s"), 0, "ERROR STRINGS NOT AVAILABLE"); \
            DEBUGGER_BREAKPOINT; \
        } \
    } while (0)


#endif //UTILS_H
