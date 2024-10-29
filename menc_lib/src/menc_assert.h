#define Const_if(c) _Pragma("warning(suppress: 4127)") if (c)

#define DEBUGGER_BREAKPOINT __debugbreak()
//#define DEBUGGER_BREAKPOINT (*(int *)0 = 0)

#if _MSC_VER
#define _Assert(expr, __VA_ARGS__)                                          \
    do {                                                                    \
        Const_if ( !(expr) ) {                                              \
            LogFatal("%s:%d (%s): Assert failed with the expresion \"%s\"", \
                     __FILE__, __LINE__, __func__, #expr);                  \
            DEBUGGER_BREAKPOINT;                                            \
        }                                                                   \
    } while(0)
#else
    do {                                                                        \
        Const_if ( !(expr) ) {                                                  \
            LogFatal("%s:%d (%s): Assert failed with the expresion \"%s\": %s", \
                     __FILE__, __LINE__, __func__, #expr, msg);                 \
            DEBUGGER_BREAKPOINT;                                                \
        }                                                                       \
    } while(0)
#endif

// "frontend", default arguments get implemented here. the suffix is the number of arguments, in hexadecimal base
#define Assert1(...) _Assert(__VA_ARGS__, "")
#define Assert2(...) _Assert(__VA_ARGS__)
#define Assert(...)  vfn(Assert,__VA_ARGS__)

#define Dbg_If(expr) \
    do {                                                                       \
        if ( (expr) ) { DEBUGGER_BREAKPOINT; } \
    } while(0)

#define INVALID_PATH(str) Assert(0 && str)

#define NO_DEFAULT default: { INVALID_PATH("There should not be a default."); } break;
#define NO_ELSE else { INVALID_PATH("There should not be an else."); }

#define JENH_SAFE
#define JENH_DEBUG

#ifdef JENH_DEBUG
//#define Check(inThingToCkeck, inExpr) inThingToCkeck
#define Check(inThingToCkeck, inExpr) Assert( (inThingToCkeck) inExpr)
#else
#define Check(inThingToCkeck, inExpr) inThingToCkeck
#endif
