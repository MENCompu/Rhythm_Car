#define ConsoleWriteError(string, color)  ConsoleWrite(STD_ERROR_HANDLE , string, color)
#define ConsoleWriteOutput(string, color) ConsoleWrite(STD_OUTPUT_HANDLE, string, color)
Public void ConsoleWrite(u32 stdhandleToGet, String string, u8 color);

Private inline b8 LogTypeIsError(Log_Type type) {
    Assert( type < LOG_TYPE_COUNT );
    b8 isError = ( type >= LT_Error );
    return isError;
}

Public void Log(Log_Type type, CString fmt, ...) {
    Assert(type < LOG_TYPE_COUNT);
    Log_Type_Info typeInfo = logTypeInfoTable[type];

    Local_Str(logBuffer, LOG_BUFFER_CAPACITY);
    logBuffer.count = (u32)sprintf_s((char*)logBuffer.E, LOG_BUFFER_CAPACITY, "[%-5s]: ", typeInfo.name);

    va_list fmtArgs;
    va_start(fmtArgs, fmt);
    logBuffer.count += vsnprintf(((char*)logBuffer.E + logBuffer.count), LOG_BUFFER_CAPACITY, (char*)fmt, fmtArgs);
    va_end(fmtArgs);

    Assert( (logBuffer.count + 3) < LOG_BUFFER_CAPACITY );
    Str_Push(&logBuffer, S(".\n\0"));

    if ( LogTypeIsError(type) ) {
        ConsoleWriteError(logBuffer, (u8)type);
        if ( type == LT_Fatal ) {

            DEBUGGER_BREAKPOINT;
        }
    } else {
        ConsoleWriteOutput(logBuffer, (u8)type);
    }

#if (BREAKPOINT_ERROR == 1)
    if (type == LT_Error) {
        DEBUGGER_BREAKPOINT;
    }
#endif
#if (BREAKPOINT_WARNING == 1)
    if (type == LT_Warn) {
        DEBUGGER_BREAKPOINT;
    }
#endif
}

#ifdef _WIN32
Private void Win32_Log(Log_Type logType, CString fmt, ...) {
    DWORD lastError = GetLastError();

    Local_Str(logBuffer, LOG_BUFFER_CAPACITY);
    va_list fmtArgs;
    va_start(fmtArgs, fmt);
    logBuffer.count = (u32)vsnprintf((char*)logBuffer.E, LOG_BUFFER_CAPACITY, (char*)fmt, fmtArgs);
    va_end(fmtArgs);

    u8* buffer;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
                                 FORMAT_MESSAGE_IGNORE_INSERTS, 0, lastError, 0, (LPSTR)&buffer, 0, 0);

    // Errase end format of windows messages.
    buffer[size - 3] = '\0';

    Log(logType, (CString)"%s | (windows %u) %s", logBuffer.E, lastError, buffer);
    LocalFree(buffer);
}


Public void ConsoleWrite(u32 stdhandleToGet, String string, u8 color) {
    printf("%s", string.E);
    OutputDebugStringA((char*)string.E);

    //HANDLE std = GetStdHandle(stdhandleToGet);
    //Local_Persistant u8 colors[] = {1, 2, 6, 4, 64};
    //(void)SetConsoleTextAttribute(std, colors[color]);

    //DWORD bytesWritten;
    //Win32_Check(WriteConsoleA(GetStdHandle(stdhandleToGet), string.E, string.size, &bytesWritten, 0));
}
#endif
