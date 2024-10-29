#ifndef RC_MEM_H
#define RC_MEM_H

#include "dll_api.h"
#ifdef MEM_EXPORT
    #define Mem_API Export
#else
    #define Mem_API Import
#endif

C_Symbols_Begin

Mem_API void* gHeapMem;

Mem_API void Heap_Mem_Init(void* baseAddress, u64 size);

C_Symbols_End

#endif // RC_MEM_H
