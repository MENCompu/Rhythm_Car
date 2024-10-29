#include "pe_def.h"
#include "pe.c"

typedef struct {
    String  path;
    String  tempPath;
    HMODULE handle;

    PE_Image PE;
    PE_Image savedPE;

    FILETIME lastTimeWritten;

    u32 globalDataSize;
    void* globalData;

    u32 globalDataSize2;
    void* globalData2;

    void* prevBaseAddress;

    Fn_Init1* Init1;
    Fn_Init2* Init2;
    Fn_MainLoop* MainLoop;
    Fn_Shutdown* Shutdown;
    Fn_Should_Shutdown* Should_Shutdown;
} Hot_Reloading_State;

Private Global Hot_Reloading_State stateHR;
Public Global volatile b8 gMainDLLIsLoading;

#ifdef _WIN32

// TODO(JENH): Windows specific.
Public void* RH_Get_DLL_Base_Address() {
    return (void*)stateHR.handle;
}

Intern inline FILETIME WIN32_GetFileWriteTime(CString fileName) {
    FILETIME result = {0};
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    if ( GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileData) ) {
	    result = fileData.ftLastWriteTime;
    }

    return result;
}

Intern void LoadMainDLL() {
    CopyFile(stateHR.path.str, stateHR.tempPath.str, FALSE);

    stateHR.lastTimeWritten = WIN32_GetFileWriteTime(stateHR.path.str);

    HMODULE handle = LoadLibraryA(stateHR.tempPath.str);
    if ( !handle ) {
        LogError("Failed to load the main DLL");
    }

    stateHR.handle = handle;

    stateHR.Init1 = (Fn_Init1*)(void*)GetProcAddress(handle, "Init1");
    stateHR.Init2 = (Fn_Init2*)(void*)GetProcAddress(handle, "Init2");

    stateHR.MainLoop = (Fn_MainLoop*)(void*)GetProcAddress(handle, "MainLoop");
    stateHR.Shutdown = (Fn_Shutdown*)(void*)GetProcAddress(handle, "Shutdown");
    stateHR.Should_Shutdown = (Fn_Should_Shutdown*)(void*)GetProcAddress(handle, "Should_Shutdown");

    if ( !stateHR.Init1 || !stateHR.Init2 ||!stateHR.MainLoop || !stateHR.Shutdown || !stateHR.Should_Shutdown ) {
        LogError("Failed to link with the main DLL");
    }
}

Public void HR_Save_Global_State() {
    void* oldData = PE_Get_Global_Data_Section(&stateHR.PE, &stateHR.globalDataSize);
    stateHR.globalData = OS_Alloc_Mem(stateHR.globalDataSize);
    Mem_Copy_Forward(stateHR.globalData, oldData, stateHR.globalDataSize);

#if 0
    void* oldData2 = PE_Image_Get_Section_Data(&stateHR.PE, ".bss", &stateHR.globalDataSize2);
    stateHR.globalData = OS_Alloc_Mem(stateHR.globalDataSize2);
    Mem_Copy_Forward(stateHR.globalData2, oldData2, stateHR.globalDataSize2);
#endif
}

Public void HR_Apply_Global_State() {
    (void)PE_Image_Parse(RH_Get_DLL_Base_Address(), &stateHR.PE);

    Assert( stateHR.PE.baseAddrImage == stateHR.prevBaseAddress );

    u32 newDataSize;
    void* newData = PE_Get_Global_Data_Section(&stateHR.PE, &newDataSize);
    Mem_Copy_Forward(newData, stateHR.globalData, stateHR.globalDataSize);

#if 0
    newData = PE_Image_Get_Section_Data(&stateHR.PE, ".bss", &newDataSize);
    Mem_Copy_Forward(newData, stateHR.globalData2, stateHR.globalDataSize2);
#endif
}

Intern void UnloadMainDLL() {
    if ( stateHR.handle ) {
	    FreeLibrary(stateHR.handle);
        stateHR.handle = 0;
    }
}

Private Global u32 gCounter; // TODO(JENH): This shouldn't be necessary if we handle pointer to global variables.

Public void ReloadMainDLLIfNeeded() {
    FILETIME updateWriteTime = WIN32_GetFileWriteTime(stateHR.path.str);

    if ( CompareFileTime(&updateWriteTime, &stateHR.lastTimeWritten) && (gCounter++ == 60) ) {
        gMainDLLIsLoading = JENH_TRUE;
        Wake_Up_All_Threads();
        while ( !All_Threads_Are_Safe() );

        HR_Save_Global_State();
        UnloadMainDLL();
        LoadMainDLL();
        HR_Apply_Global_State();

        gMainDLLIsLoading = JENH_FALSE;
        gCounter = 0;
    }
}

Intern inline void InitMainDLL(String pathDLL, String tempPathDLL) {
    stateHR.path = pathDLL;
    stateHR.tempPath = tempPathDLL;
    LoadMainDLL();

    (void)PE_Image_Parse(RH_Get_DLL_Base_Address(), &stateHR.PE);
    stateHR.prevBaseAddress = stateHR.PE.baseAddrImage;
    PE_Image_Print(&stateHR.PE);
}

#endif
