#ifndef JENH_VIRTUAL_MEM_ALLOCATOR_H
#define JENH_VIRTUAL_MEM_ALLOCATOR_H

#define OS_Alloc_Type(inType) (inType*)OS_Alloc_Mem(sizeof(inType))
#define OS_Alloc_Array(inType, inCount) (inType*)OS_Alloc_Mem(inCount * sizeof(inType))
Public void* OS_Alloc_Mem(WordSize inSize) {
    void* retMem = VirtualAlloc(0, inSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Win32_Check( retMem, != 0 );
    return retMem;
}

Public void OS_Free_Mem(void* inMem) {
    Win32_Check( VirtualFree(inMem, 0, MEM_RELEASE), > 0 );
}

#endif // JENH_VIRTUAL_MEM_ALLOCATOR_H
