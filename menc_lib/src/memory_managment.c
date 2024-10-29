//Global Dynamic_Memory dyMem;

void Arena_Create(Memory_Arena* arena, void* E, u32 capacity) {
    arena->count = 0;
    arena->capacity = capacity;
    arena->E = E;
}

void Arena_Clear(Memory_Arena* arena) {
    arena->count = 0;
}

void Arena_Zero(Memory_Arena* arena) {
    Mem_Zero(arena->E, arena->count);
    arena->count = 0;
}

Public Memory_Arena Arena_Create_From_Arena(Memory_Arena* parent, u32 capacity) {
    Memory_Arena ret;
    Arena_Create(&ret, Arena_Alloc_Mem(parent, capacity), capacity);
    return ret;
}

void* Arena_Alloc(Memory_Arena* arena, u32 size) {
    Assert((arena->count + size) <= arena->capacity);
    void *ret = (byte*)arena->E + arena->count;
    arena->count += size;
    return ret;
}

void* Arena_Alloc_And_Copy(Memory_Arena* arena, u32 size, void* data) {
    void* pushedData = Arena_Alloc(arena, size);
    Mem_Copy_Forward(pushedData, data, (u32)size);
    return pushedData;
}

Public void Arena_Push_Arena(Memory_Arena* arena, Memory_Arena src) {
    void* mem = Arena_Alloc_Mem(arena, src.count);
    Mem_Copy_Forward(mem, src.E, src.count);
}

void Mem_Init(void) {
#ifdef JENH_DEBUG
    void* EMem = (void*)TiB((u64)2);
#else
    void* EMem = 0;
#endif

    dyMem.mem = OS_Alloc_Mem_At(PERMA_ARENA_SIZE + TEMP_ARENA_SIZE, EMem);
    Arena_Create(&dyMem.arena, dyMem.mem, PERMA_ARENA_SIZE + TEMP_ARENA_SIZE);

    //Arena_Create(&dyMem.perma.arena, dyMem.mem, PERMA_ARENA_SIZE);
    //Arena_Create(&dyMem.temp.arena, (byte*)dyMem.mem + PERMA_ARENA_SIZE, TEMP_ARENA_SIZE);
}
