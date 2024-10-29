#pragma warning(push, 1)
#ifdef _WIN32
    #include "stdio.h"
    #include <windows.h>
    #include <shlwapi.h>
#endif
#pragma warning(pop)

//#define BUILD_32
#define BUILD_64

#include "types.h"
#include "Utils.h"
#include "Memory_Utils.h"
#include "String.h"
#include "file_system.h"
#include "logger.h"
#include "Math.h"
#include "file_format_utils.h"
#include "intrinsics.c"
#include "virtual_mem_allocator.h"

#define BASE_BIN_EXPORT
#include "base_bin.h"

#define MEM_EXPORT
#include "Memory_Managment.h"
#include "Memory_Managment.c"

#include "logger.c"
#include "file_system.c"

#include "free_list.h"
#include "free_list.c"

#include "parse.h"

Private Global HANDLE gReloadSemaphore;
Public b8 All_Threads_Are_Safe(void);
Public void Wake_Up_All_Threads(void);

#include "main_dll.h"
#include "main_dll.c"

Global void* gHeapMem;

Private Global u32 gThreadCount = 1;
Private Global volatile u32 gSafeThreadCount = 1; // This is 1 for the main thread that it's already safe.
Private Global Base_Thread_Info baseThreadPool[256];

#define NAME_MAIN_DLL "RC_Main_DLL.dll"
#define NAME_TEMP_MAIN_DLL "RC_Main_DLL_temp.dll"

Private void Get_DLL_Paths(String binDir, String *DLLPath, String *tempDLLPath) {
    CatStr(DLLPath, binDir, LitToStr(NAME_MAIN_DLL));
    DLLPath->str[DLLPath->size] = '\0';

    CatStr(tempDLLPath, binDir, LitToStr(NAME_TEMP_MAIN_DLL));
    tempDLLPath->str[tempDLLPath->size] = '\0';
}

#ifdef _WIN32
Export void PL_Entry_Point(int argc, char* argv[]) {
    Local_Str(binPath, MAX_PATH);
    binPath.size = (u32)GetModuleFileNameA(0, binPath.str, MAX_PATH);
    String binDir = Str_Skip_End(binPath, FindCharBackwards(binPath, '\\'));

    Local_Str(DLLPath, MAX_PATH);
    Local_Str(tempDLLPath, MAX_PATH);
    Get_DLL_Paths(binDir, &DLLPath, &tempDLLPath);

    InitMainDLL(DLLPath, tempDLLPath);

    gReloadSemaphore = CreateSemaphoreA(0, 0, MAX_S32, 0);
    Assert( gReloadSemaphore );

    // TODO(JENH): Fix all this mem and vulkan nonesense form the base layer.
    Mem_Init();

    while ( !stateHR.Should_Shutdown() ) {
        ReloadMainDLLIfNeeded();
        stateHR.MainLoop();
    }

    stateHR.Shutdown();
}
#else

Export void RC_Main(void) {
    main2(argc, argv);
}
#endif

// HEAP MEMORY.
void Heap_Mem_Init(void* inBaseAddress, u64 inSize) {
    Assert(inSize < MAX_U32);

    gHeapMem = VirtualAlloc(inBaseAddress, inSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    //gHeapMem = mmap(inBaseAddress, inSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
}
//

Private DWORD WINAPI Thread_Home(LPVOID inArgs);

void Thread_Create_(CString inFuncName, Fn_Thread_Function* inFunc, void* inArgs) {
    DWORD unused;
    u32 threadID = gThreadCount++;

    Base_Thread_Info* threadInfo = &baseThreadPool[threadID];
    threadInfo->hotReloading = JENH_FALSE; // NOTE(JENH): "hotReloading" is already false.
    threadInfo->ID = threadID;
    threadInfo->funcName = inFuncName;
    threadInfo->func = inFunc;
    threadInfo->args = inArgs;
    threadInfo->handle = CreateThread(0, 0, Thread_Home, threadInfo, 0, &unused);
}

Private DWORD WINAPI Thread_Home(LPVOID inArgs) {
    Base_Thread_Info* info = (Base_Thread_Info*)inArgs;

    while ( INFINITE_LOOP ) {
        info->func(info, gReloadSemaphore, info->args);

        Atomic_Inc_U32(&gSafeThreadCount);
        while ( gMainDLLIsLoading );
    }
}

Public void Wake_Up_All_Threads(void) {
    for (u32 i = 1; i < gThreadCount;  ++i) {
        Win32_Check( ReleaseSemaphore(gReloadSemaphore, 1, 0), >= 0 );
    }
}

Public b8 All_Threads_Are_Safe(void) {
    if ( gSafeThreadCount == gThreadCount ) {
        gSafeThreadCount = 1;
        return JENH_TRUE;
    }

    return JENH_FALSE;
}

Public void Base_Thread_Reload(void) {
    for (u32 i = 1; i < gThreadCount; ++i) {
        Base_Thread_Info* threadInfo = &baseThreadPool[i];
        DWORD hola = ResumeThread(threadInfo->handle);
        hola += 1;
    }
}
