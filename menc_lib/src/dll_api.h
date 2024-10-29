#ifndef JENH_DLL_API_H
#define JENH_DLL_API_H

#ifdef __cplusplus
    #define C_Symbols_Begin extern "C" {
    #define C_Symbols_End   }
#else
    #define C_Symbols_Begin
    #define C_Symbols_End
#endif

#ifdef __cplusplus
    #define C_SYMBOL extern "C"
#else
    #define C_SYMBOL
#endif

#define Import C_SYMBOL __declspec(dllimport)
#define Export C_SYMBOL __declspec(dllexport)

#endif //JENH_DLL_API_H
