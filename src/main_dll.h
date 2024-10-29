#ifndef JENH_DLL_MAIN_H
#define JENH_DLL_MAIN_H

#include "dll_api.h"

C_Symbols_Begin

typedef void (Fn_Init1)(HWND* outWindow, HINSTANCE* outInstance, s32x2* inWinDims);
typedef void (Fn_Init2)(String binDir);
typedef void (Fn_MainLoop)();
typedef void (Fn_Shutdown)();
typedef b8   (Fn_Should_Shutdown)();

#ifdef MAIN_DLL_EXPORT
    #define Main_DLL_API Export

    Main_DLL_API void Init1(HWND* outWindow, HINSTANCE* outInstance, s32x2* inWinDims);
    Main_DLL_API void Init2(String binDir);
    Main_DLL_API void MainLoop();
    Main_DLL_API void Shutdown();
    Main_DLL_API b8   Should_Shutdown();
#endif

C_Symbols_End

#endif //JENH_DLL_MAIN_H
