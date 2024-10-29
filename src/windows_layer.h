#ifndef RA_WINDOWS_LAYER_H
#define RA_WINDOWS_LAYER_H

#include <DSound.h>

#define Fn_Job(name) void (name)(void *args)
typedef Fn_Job(*Fn_Ptr_Job);

typedef struct {
    Fn_Ptr_Job func;
    void *args;
} Job_Entry;

typedef struct {
    LPDIRECTSOUNDBUFFER secondaryBuffer;

    u32 samplesPerSecond;
    u32 bytesPerSample;
    u32 bufferSize;
    s32 wavePeriod;
    u32 safetyBytes;
    s16* samples;

    b8  isValid;
    u32 runningSampleIndex;
} Win32_Sound;

typedef struct {
    LONG windowMaximizedFlag;

    ATOM windowClassAtom;
    HWND window;
    HINSTANCE instance;

    HANDLE cursor;

    Win32_Sound sound;
} OS_State;

Intern void ConsoleWrite(u32 stdhandleToGet, String string, u8 color);

Public b8 OS_Init();
Public void OS_Cleanup();

Public void Time_OS_Init();
Public void Time_OS_Cleanup();

Public u32 OS_Get_Page_Size();
Public s32 OS_Get_Refresh_Rate();
Public s32x2 OS_Window_Get_Dims();

Public void Window_Normal_Size();
Public void Window_Fullsreen_Size();

// TODO(JENH): Temporary.
Public void Program_Close();

#endif //RA_WINDOWS_LAYER_H
