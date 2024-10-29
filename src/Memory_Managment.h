#ifndef RC_ARENA_ALLOCATOR_H
#define RC_ARENA_ALLOCATOR_H

#define UNDETERMINED_SIZE 0
#define TEMP_ARENA_COUNT  1024

typedef struct {
    WordSize size;
    WordSize used;
    void* base;
} Memory_Arena;

typedef struct {
    b8 allocated;
    Memory_Arena arena;
} Temp_Arena;

typedef struct {
    u32 size;
    Temp_Arena E[TEMP_ARENA_COUNT];
} Array_Temp_Arena;

typedef struct {
    Array_Temp_Arena tempArenas;
    Memory_Arena arena;
} Temp_Mem;

#define PERMA_ARENA_SIZE MiB(512)
#define TEMP_ARENA_SIZE  MiB(512)

#define LIST_PERMA_MEMORY_ARENAS \
    X(dwarf    , MiB(16)) \
    X(strings  , MiB(16)) \
    X(bindings , MiB(16)) \
    X(assets   , MiB(256)) \

typedef struct {
    struct {
        #define X(permaArena, size) Memory_Arena permaArena;
        LIST_PERMA_MEMORY_ARENAS
        #undef X
    };
    Memory_Arena arena;
} Perma_Mem;

typedef struct {
    Temp_Mem  temp;
    Perma_Mem perma;
} Dynamic_Memory;

#include "dll_api.h"

#ifdef MEM_EXPORT
    #define MEM_API Export
#else
    #define MEM_API Import
#endif

MEM_API Global Dynamic_Memory dyMem;

#define ArenaPushType(arena, type) (type*)ArenaPush(arena, sizeof(type))
#define ArenaPushMem(arena, size) (void*)ArenaPush(arena, sizeof(byte) * (size))
#define ArenaPushArray(arena, type, size) (type*)ArenaPush(arena, sizeof(type) * (size))

MEM_API void Mem_Init();
MEM_API void Arena_Create(Memory_Arena* arena, void* base, u32 size);
MEM_API void Arena_Clear(Memory_Arena* arena);
MEM_API void* ArenaPush(Memory_Arena* arena, WordSize size);
MEM_API Memory_Arena *AllocTempArena(u32 size);

#define AllocTempArray(arenaPtr, type, count) (type*)AllocTempArray_(arenaPtr, sizeof(type) * count)
MEM_API void* AllocTempArray_(Memory_Arena** arena, u32 size);

MEM_API void FreeTempArena(Memory_Arena* arena);
MEM_API void ClearTempMem();

#endif // RC_ARENA_ALLOCATOR_H
