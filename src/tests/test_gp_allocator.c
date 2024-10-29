Private void Test_GP_Overflow(void) {
    Free_List freeList;

    u32 freeListSize = MiB(1);
    Free_List_Init(freeListSize, &freeList);

    u32 expectedOffset = 0;

    u32 blocCount = 64;
    u32 blockSize = freeListSize / blocCount;
    for (u32 i = 0; i < blocCount; ++i) {
        u64 offset = Free_List_Alloc(&freeList, blockSize);

        Test_Check( expectedOffset == offset );
        expectedOffset += blockSize;
    }

    u64 offset = Free_List_Alloc(&freeList, blockSize);
    Test_Check( offset == MAX_U32 );
}

typedef struct {
    b8 shouldAlloc;
    union {
        u32 size;
        u32 id;
    };
    u64 offset;
} Test_Alloc_And_Free_Data;

Private void Mark_Alloced_Region(void* inBase, u64 inOffset, u64 inSize) {
    void* mem = (byte*)inBase + inOffset;
    Mem_Fill_With_Byte(mem, (u32)inSize, 0xcc);
}

Private void Mark_Freed_Region(void* inBase, u64 inOffset, u64 inSize) {
    void* mem = (byte*)inBase + inOffset;
    Mem_Fill_With_Byte(mem, (u32)inSize, 0xff);
}

Private b8 Is_Region_Already_Allocated(void* inBase, u64 inOffset, u64 inSize) {
    static byte howMemShouldLookLike[MiB(2)];
    Mem_Fill_With_Byte(howMemShouldLookLike, (u32)inSize, 0xcc);

    void* mem = (byte*)inBase + inOffset;
    return Mem_Equal(howMemShouldLookLike, mem, (u32)inSize);
}

Private b8 Is_Region_Already_Freed(void* inBase, u64 inOffset, u64 inSize) {
    static byte howMemShouldLookLike[MiB(2)];
    Mem_Fill_With_Byte(howMemShouldLookLike, (u32)inSize, 0xff);

    void* mem = (byte*)inBase + inOffset;
    return Mem_Equal(howMemShouldLookLike, mem, (u32)inSize);
}

Private void Test_GP_Alloc_And_Free(void) {
    Test_Alloc_And_Free_Data dataTable[] = {
        { 1, 75, 0 }, { 1, 507, 0 }, { 0, 1, 0 }, { 1, 74, 0 },
        { 0, 0, 0 }, { 1, 92, 0 }, { 1, 30, 0 }, { 1, 50, 0 },
        { 0, 7, 0 }, { 1, 50, 0 }, { 0, 6, 0 }, { 0, 5, 0 },
        { 1, 800, 0 }, { 0, 12, 0 }, { 0, 3, 0 }, { 1, 35, 0 },
        { 0, 15, 0 }, { 0, 9, 0 },
    };

    Free_List freeList;

    u32 freeListSize = MiB(1);
    static byte mem[MiB(1)];

    Free_List_Init(freeListSize, &freeList);

    for (u32 i = 0; i < ArrayCount(dataTable); ++i) {
        Test_Alloc_And_Free_Data* data = &dataTable[i];

        if ( data->shouldAlloc ) {
            data->offset = Free_List_Alloc(&freeList, data->size);

            Test_Check( !Is_Region_Already_Allocated(mem, data->offset, data->size) );
            Mark_Alloced_Region(mem, data->offset, data->size);
        } else {
            Test_Alloc_And_Free_Data* allocedData = &dataTable[data->id];

            Test_Check( !Is_Region_Already_Freed(mem, allocedData->offset, allocedData->size) );

            Free_List_Free(&freeList, allocedData->offset, allocedData->size);

            Mark_Freed_Region(mem, allocedData->offset, allocedData->size);
        }
    }
}
