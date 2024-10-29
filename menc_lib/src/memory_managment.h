#ifndef RC_ARENA_ALLOCATOR_H
#define RC_ARENA_ALLOCATOR_H

#define UNDETERMINED_SIZE 0
#define TEMP_ARENA_COUNT  1024

#if 1
typedef struct {
    u32 capacity;
    u32 count;
    u8* E;
} Memory_Arena;
#else
typedef Array_u8 Memory_Arena;
#endif

#define PERMA_ARENA_SIZE MiB(512)
#define TEMP_ARENA_SIZE  MiB(512)

typedef struct {
    void* mem;
    Memory_Arena arena;

    //Temp_Mem  temp;
    //Perma_Mem perma;
} Dynamic_Memory;

Public Global Dynamic_Memory dyMem;

Public Global Memory_Arena gPermArena;
Public Global Memory_Arena gTempArena;

Public Global Memory_Arena gSymbolArena;
Public Global Memory_Arena gDataArena;

#define Arena_Alloc_Type(arena, type) (type*)Arena_Alloc((arena), sizeof(type))
#define Arena_Alloc_Mem(arena, size) (void*)Arena_Alloc((arena), (size))
#define Arena_Alloc_Array(arena, type, size) (type*)Arena_Alloc((arena), sizeof(type) * (size))

#define Arena_Alloc_Type_And_Copy(arena, type) Arena_Alloc_And_Copy(arena, sizeof(*(type)), type)
#define Arena_Alloc_Mem_And_Copy(arena, mem, size) Arena_Alloc_And_Copy(arena, sizeof(u8) * (size), mem)
#define Arena_Alloc_Array_And_Copy(arena, array, count) Arena_Alloc_And_Copy(arena, sizeof((array)[0]) * (count), array)

Public void Mem_Init(void);

Public void Arena_Create(Memory_Arena* arena, void* base, u32 capacity);
Public Memory_Arena Arena_Create_From_Arena(Memory_Arena* parent, u32 capacity);

Public void Arena_Clear(Memory_Arena* arena);
Public void* Arena_Alloc(Memory_Arena* arena, u32 size);
Public void Arena_Push_Arena(Memory_Arena* arena, Memory_Arena src);

//#define Arena_Is_In_Bounds_Ptr(arena, ptr) \
//    Is_In_Bounds((arena).E, ptr, ( (u8*)&(arena).E[(arena).count] ) - 1)

#endif // RC_ARENA_ALLOCATOR_H
