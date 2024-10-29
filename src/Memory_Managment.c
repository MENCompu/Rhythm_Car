Global Dynamic_Memory dyMem;

void Arena_Create(Memory_Arena* arena, void* base, u32 size) {
    arena->used = 0;
    arena->size = size;
    arena->base = base;
}

void Arena_Clear(Memory_Arena* arena) {
    arena->used = 0;
}

void* ArenaPush(Memory_Arena* arena, WordSize size) {
    Assert((arena->used + size) <= arena->size);
    void *ret = (byte*)arena->base + arena->used;
    arena->used += size;
    return ret;
}

Memory_Arena* AllocTempArena(u32 size) {
    Assert(dyMem.temp.tempArenas.size < TEMP_ARENA_COUNT);
    Temp_Arena *tempArena = &dyMem.temp.tempArenas.E[dyMem.temp.tempArenas.size++];

    tempArena->allocated = JENH_TRUE;

    Arena_Create(&tempArena->arena, ArenaPushMem(&dyMem.temp.arena, size), size);
    return &tempArena->arena;
}

void* AllocTempArray_(Memory_Arena** arena, u32 size) {
    (*arena) = AllocTempArena(size);

    void *ret = ArenaPushMem(*arena, size);
    return ret;
}

void FreeTempArena(Memory_Arena* arena) {
    Assert((byte*)dyMem.temp.tempArenas.E <= (byte*)arena);

    u32 arenaIndex = (u32)((byte*)arena - (byte*)dyMem.temp.tempArenas.E) / sizeof(Temp_Arena);

    Assert(arenaIndex < dyMem.temp.tempArenas.size);

    dyMem.temp.tempArenas.E[arenaIndex].allocated = JENH_FALSE;

    for (s32 i = (s32)dyMem.temp.tempArenas.size - 1; i >= 0; --i) {
        Temp_Arena *tempArena = &dyMem.temp.tempArenas.E[i];

        if (tempArena->allocated) { break; }

        --dyMem.temp.tempArenas.size;
        dyMem.temp.arena.used -= tempArena->arena.size;
        tempArena->arena.base = 0;
    }
}

void ClearTempMem() {
    //foreach (Temp_Arena, tempArena, dyMem.temp.tempArenas) {
    for (u32 i = 0; i < dyMem.temp.tempArenas.size; ++i) {
        Temp_Arena* tempArena = &dyMem.temp.tempArenas.E[i];

        tempArena->allocated = JENH_FALSE;
        tempArena->arena.base = 0;
    }

    dyMem.temp.tempArenas.size = 0;
    dyMem.temp.arena.used = 0;
}

void Mem_Init() {
#ifdef JENH_DEBUG
    void* baseMem = (void*)TiB((u64)2);
#else
    void* baseMem = 0;
#endif

    Heap_Mem_Init(baseMem, PERMA_ARENA_SIZE + TEMP_ARENA_SIZE);

    Arena_Create(&dyMem.perma.arena, gHeapMem, PERMA_ARENA_SIZE);
    Arena_Create(&dyMem.temp.arena, (byte*)gHeapMem + PERMA_ARENA_SIZE, TEMP_ARENA_SIZE);

    #define X(permaArena, size) \
        Arena_Create(&dyMem.perma.##permaArena, ArenaPushMem(&dyMem.perma.arena, size), size);
    LIST_PERMA_MEMORY_ARENAS
    #undef X
}
