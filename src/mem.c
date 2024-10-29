#include <stdio.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif

#include "DataTypes.h"
#include "Utils.h"
#include "String.h"

#include "logger.h"

#define MEM_EXPORT
#include "mem.h"

#ifdef _WIN32
#define Win32_LogError(fmt, ...) Win32_Log(LT_Error, fmt, __VA_ARGS__)

#define ConsoleWriteError(string, color)  ConsoleWrite(STD_ERROR_HANDLE , string, color)
#define ConsoleWriteOutput(string, color) ConsoleWrite(STD_OUTPUT_HANDLE, string, color)
Intern void ConsoleWrite(u32 stdhandleToGet, String string, u8 color) {
    HANDLE std = GetStdHandle(stdhandleToGet);
    OutputDebugStringA(string.str);

    //Local_Persistant u8 colors[] = {1, 2, 6, 4, 64};
    //(void)SetConsoleTextAttribute(std, colors[color]);

    //DWORD bytesWritten;
    //Win32_Check(WriteConsoleA(GetStdHandle(stdhandleToGet), string.str, string.size, &bytesWritten, 0));
}

Private void Win32_Log(Log_Type logType, CString fmt, ...) {
    DWORD lastError = GetLastError();

    Local_Str(logBuffer, LOG_BUFFER_CAPACITY);
    va_list fmtArgs;
    va_start(fmtArgs, fmt);
    logBuffer.size = vsnprintf(logBuffer.str, LOG_BUFFER_CAPACITY, fmt, fmtArgs);
    va_end(fmtArgs);

    char *buffer;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
                                 FORMAT_MESSAGE_IGNORE_INSERTS, 0, lastError, 0, (LPSTR)&buffer, 0, 0);

    buffer[size - 3] = '\0';

    Log(logType, "%s | (windows %u) %s", logBuffer.str, lastError, buffer);
    LocalFree(buffer);
}
#else
#endif

#include "logger.c"

Mem_API void* gHeapMem;

Mem_API void Heap_Mem_Init(void* inBaseAddress, u64 inSize) {
    Assert(inSize < MAX_U32);

#ifdef _WIN32
    gHeapMem = VirtualAlloc(inBaseAddress, inSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!gHeapMem) {
        Win32_LogError("Failed to allocate the heap memory for the program");
    }
#else
    gHeapMem = mmap(inBaseAddress, inSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
#endif
}
