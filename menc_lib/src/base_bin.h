#ifndef JENH_BASE_BIN_H
#define JENH_BASE_BIN_H

#include "dll_api.h"

C_Symbols_Begin

#ifdef BASE_BIN_EXPORT
    #define Base_API Export
#else
    #define Base_API Import
#endif

// HEAP MEMORY.
Base_API Global void* gHeapMem;
Base_API void Heap_Mem_Init(void* inBaseAddress, u64 inSize);
//

// Threads.
struct Base_Thread_Info;

#define Fn_Prot_Thread_Function(name) void (name)(struct Base_Thread_Info* inThreadInfo, HANDLE reloadSemaphore, void* inArgs)
typedef Fn_Prot_Thread_Function(Fn_Thread_Function);

typedef struct Base_Thread_Info {
    u32 ID;
    HANDLE handle;
    b8 hotReloading;

    CString funcName;
    Fn_Thread_Function* func;
    void* args;
} Base_Thread_Info;

#define Thread_Create(inThreadFunc, inArgs) Thread_Create_(S(#inThreadFunc), inThreadFunc, inArgs)
Base_API void Thread_Create_(CString inFuncName, Fn_Thread_Function* inFunc, void* inArgs);
//

C_Symbols_End

#endif // JENH_BASE_BIN_H
