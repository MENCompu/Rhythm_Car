#ifndef JENH_LOGGER_H
#define JENH_LOGGER_H

#define BREAKPOINT_ERROR   1
#define BREAKPOINT_WARNING 0

#define LOG_BUFFER_CAPACITY KiB(32)

typedef enum {
    LT_Debug,
    LT_Info,
    LT_Warn,
    LT_Error,
    LT_Fatal,
    LOG_TYPE_COUNT
} Log_Type;

typedef struct {
    CString name;
    CString color;
} Log_Type_Info;

Intern_Global Log_Type_Info logTypeInfoTable[] = {
   //{"TRACE", "\x1b[94m"},
   {(CString)"DEBUG", (CString)"\x1b[36m"},
   {(CString)"INFO" , (CString)"\x1b[32m"},
   {(CString)"WARN" , (CString)"\x1b[33m"},
   {(CString)"ERROR", (CString)"\x1b[31m"},
   {(CString)"FATAL", (CString)"\x1b[35m"}
};

#define LogDebug(fmt, ...) Log(LT_Debug, (CString)fmt, __VA_ARGS__)
#define LogInfo(fmt, ...)  Log(LT_Info , (CString)fmt, __VA_ARGS__)
#define LogWarn(fmt, ...)  Log(LT_Warn , (CString)fmt, __VA_ARGS__)
#define LogError(fmt, ...) Log(LT_Error, (CString)fmt, __VA_ARGS__)
#define LogFatal(fmt, ...) Log(LT_Fatal, (CString)fmt, __VA_ARGS__)

Intern void Log(Log_Type type, CString fmt, ...);

#ifdef _WIN32
#define Win32_LogWarn(fmt, ...)  Win32_Log(LT_Warn,  (CString)fmt, __VA_ARGS__)
#define Win32_LogError(fmt, ...) Win32_Log(LT_Error, (CString)fmt, __VA_ARGS__)
#define Win32_LogFatal(fmt, ...) Win32_Log(LT_Fatal, (CString)fmt, __VA_ARGS__)

#define Win32_Check(inFuncCall, inCondicional) \
    do { \
        if ( !(inFuncCall inCondicional) ) { \
            DWORD lastError = GetLastError(); \
            char* buffer; \
            (void)FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM| \
                                 FORMAT_MESSAGE_IGNORE_INSERTS, 0, lastError, 0, (LPSTR)&buffer, 0, 0); \
            LogError("(windows %u) %s", lastError, buffer); \
            LocalFree(buffer); \
            DEBUGGER_BREAKPOINT; \
        } \
    } while (0)

#endif

#endif // JENH_LOGGER_H
