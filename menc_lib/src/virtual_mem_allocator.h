#ifndef JENH_VIRTUAL_MEM_ALLOCATOR_H
#define JENH_VIRTUAL_MEM_ALLOCATOR_H

#define OS_Alloc_Type(inType) (inType*)OS_Alloc_Mem(sizeof(inType))
#define OS_Alloc_Array(inType, inCount) (inType*)OS_Alloc_Mem(inCount * sizeof(inType))
Public void* OS_Alloc_Mem(WordSize inSize) {
    void* retMem;
    Win32_Check( (retMem = VirtualAlloc(0, inSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)), != 0 );
    return retMem;
}

Public void* OS_Alloc_Mem_At(WordSize inSize, void* inAddress) {
    void* retMem;
    Win32_Check( (retMem = VirtualAlloc(inAddress, inSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)), != 0 );
    return retMem;
}

Public void OS_Free_Mem(void* inMem) {
    Win32_Check( VirtualFree(inMem, 0, MEM_RELEASE), > 0 );
}

#endif // JENH_VIRTUAL_MEM_ALLOCATOR_H
