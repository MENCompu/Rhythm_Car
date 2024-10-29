#ifndef DSV_DATASTRUCTURES_H
#define DSV_DATASTRUCTURES_H

#define Array_Count(array) (sizeof(array) / sizeof((array)[0]))

#define Local_Array_Init(type, arrayName, ...) Local_Array_Init_(type, arrayName, __COUNTER__, __VA_ARGS__)
#define Local_Array_Init_(type, arrayName, counter, ...) Local_Array_Init__(type, arrayName, counter, __VA_ARGS__)
#define Local_Array_Init__(type, arrayName, counter, ...) \
    Array_##type arrayName; \
    type dummyBufferNameArray##counter[] = {__VA_ARGS__}; \
    arrayName.A = dummyBufferNameArray##counter; \
    arrayName.size = Array_Count(dummyBufferNameArray##counter);

#define Local_Array(type, name, capacity) Local_Array_(type, name, capacity, __COUNTER__)
#define Local_Array_(type, name, capacity, counter) Local_Array__(type, name, capacity, counter)
#define Local_Array__(type, name, inCapacity, counter) \
    type P(dummyBufferNameArray, counter)[inCapacity]; \
    P(Array_,type) name =  { \
        .capacity = inCapacity, \
        .count = 0, \
        .E = P(dummyBufferNameArray, counter), \
    }; \

#define LitToArray(arrayLit) { Array_Count(arrayLit), arrayLit }

#define Array_Create(array, inCapacity, arena) \
    do { \
        (array).capacity = (inCapacity); \
        (array).count = 0; \
        (array).E = Arena_Alloc_Mem(arena, sizeof((array).E[0]) * (array).capacity); \
    } while (0)

#define Array_Static_Create(array) \
    do { \
        (array).capacity = Array_Count((array).E); \
        (array).count = 0; \
    } while (0)

#define Array_Clear(array) (array)->count = 0;

#define DArray_Create(array, arena) Array_Create(array, 1, arena)

#define Array_Push_C(array, ...) &(array).E[(array).count]; \
    Assert( ( (array).count < (array).capacity ), "Pushing out of bounds"); \
    (array).E[(array).count++] = __VA_ARGS__;

#define Array_Push(array) &(array).E[(array).count++]; Assert( (array).count <= (array).capacity )
#define Array_Pop(array) &(array).E[--(array).count]

#define Array_Index_Back(array, index) (array).E[( (array).count - 1 ) - index]

#define Array_Copy_All(dst, src) \
    do { \
        Assert( (dst).capacity >= (src).count ); \
        (dst).count = (src).count; \
        Mem_Copy_Forward((dst).E, (src).E, sizeof((src).E[0]) * (src).count); \
    } while (0)

#define Array_Alloc_And_Copy(dst, src, arena) \
    do { \
        u32 count = (src).count; \
        void* old = (src).E; \
        Array_Create(*dst, (src).count, arena); \
        (dst)->count = (src).count; \
        Mem_Copy_Forward((dst)->E, old, sizeof((src).E[0]) * count); \
    } while (0)

#define Array_Push_Array(dst, src) \
    do { \
        Assert( ( (dst).count + (src).count ) <= (dst).capacity ); \
        Mem_Copy_Forward(&(dst).E[(dst).count], (src).E, sizeof((src).E[0]) * (src).count); \
        (dst).count += (src).count; \
    } while (0)

#define Array_Alloc_Array(array, subArray, inCapacity) \
    do { \
        (subArray).capacity = (inCapacity); \
        (subArray).count = 0; \
        (subArray).E = &(array).E[(array).count]; \
        (array).count += (inCapacity); \
    } while (0)


#define DArray_Push(arena, darray) \
    DArray_Expand(arena, (Array_void*)(darray), sizeof((darray)->E[0])); \
    ++(darray)->count

#define DArray_Push_C(arena, darray, ...) DArray_Expand(arena, (Array_void*)(darray), sizeof((darray)->E[0])); \
    (darray)->E[(darray)->count++] = __VA_ARGS__; \

#define Array_Equal(array1, array2) ( ( (array1).count == (array2).count ) && \
                                      ( Mem_Equal((array1).E, (array2).E, sizeof((array1).E[0]) * (array1).count) ) )

#define DArray_Push_Front_C(arena, darray, ...) \
    (void)DArray_Push((arena), (darray)); \
    if ( (darray)->capacity != 1 ) { \
        Mem_Shift_Right_Array((darray)->E, 0, (darray)->count - 2, 1); \
    } \
    (darray)->E[0] = __VA_ARGS__

#define Array_Cast(dstType, srcType, src) \
    (P(Array_, dstType)){ \
        .capacity = ( ( (src).capacity * sizeof(srcType) / sizeof(dstType) ) ), \
        .count = ( ( (src).count * sizeof(srcType) / sizeof(dstType) ) ), \
        .E = (dstType*)(src).E, \
    }


#define Array(typeName) \
    typedef struct P(Array_, typeName) { \
        u32 capacity; \
        u32 count;    \
        typeName* E;  \
    } P(Array_, typeName)

#define Array_Ref(typeName) \
    typedef struct P2(Array_, typeName, _P) { \
        u32 capacity; \
        u32 count;    \
        typeName** E; \
    } P2(Array_, typeName, _P)

#define Array_Static(typeName, inCapacity) \
    typedef struct P(Array_Static_, typeName) {  \
        u32 capacity;           \
        u32 count;              \
        typeName E[inCapacity]; \
    } P(Array_Static_, typeName)

Array(void);
Array_Ref(void);

Array(s8);
Array(s16);
Array(s32);
Array(s64);

Array(u8);
Array(u16);
Array(u32);
Array(u64);

Array(b8);

Array(char);
#if 0
Array(byte);
Array_Ref(byte);
#endif

#define Slice(typeName) \
    typedef struct P(Slice_, typeName) { \
        u32 count;   \
        typeName* E; \
    } P(Slice_, typeName)

#define Slice_Ref(typeName) \
    typedef struct P2(Slice_, typeName, _P) { \
        u32 count;   \
        typeName** E; \
    } P2(Slice_, typeName, _P)

Slice_Ref(void);

Slice(s8);
Slice(s16);
Slice(s32);
Slice(s64);

Slice(u8);
Slice(u16);
Slice(u32);
Slice(u64);

Slice(b8);

Slice(char);
//Slice(byte);

#define Slice_Create(array, inCount, arena) \
    do { \
        (array).count = inCount; \
        (array).E = Arena_Alloc_Mem(arena, sizeof((array).E[0]) * (array).count); \
    } while (0)

#define Array_From_Array_Static(typeName, arrayStatic) \
    (P(Array_, typeName)){ .capacity = (arrayStatic).capacity, .count = (arrayStatic).count, .E = (arrayStatic).E }

#define Array_From_Raw_Array(typeName, rawArray) \
    (P(Array_, typeName)){ .capacity = Array_Count(rawArray), .count = Array_Count(rawArray), .E = rawArray }

#define Slice_From_Array(typeName, array) \
    (P(Slice_, typeName)){ .count = (array).count, .E = (array).E }

#define Slice_From_Raw_Array(typeName, rawArray) \
    (P(Slice_, typeName)){ .count = Array_Count(rawArray), .E = rawArray }

#define Array_From_Slice(typeName, slice) \
    (P(Array_, typeName)){ .capacity = (slice).count, .count = (slice).count, .E = (slice).E }

#define Array_Assert_Bounds(array, ptr) \
    Check_Bounds((array).E, ptr, &(array).E[(array).count - 1])

#define Array_Is_In_Bounds_Ptr(array, ptr) \
    Is_In_Bounds((array).E, ptr, ( (u8*)&(array).E[(array).count] ) - 1)

#define Slice_E(array, index) \
    Is_In_Bounds((void*)(array).E, &(array).E[index], ( (byte*)&(array).E[(array).count] ) - 1)

#define Array_To_Slice(type, array) \
    (P(Slice_, type)){ .E = (array).E, .count = (array).count }

#define Flags_Add(value, flags) ((value) |= (flags))
#define Flags_Del(value, flags) ((value) &= ~(flags))
#define Flags_Switch(value, flags) ((value) ^= (flags))
#define Flags_Has_All(value, flags) (((value) & (flags)) == (flags))
#define Flags_Has_Any(value, flags) (((value) & (flags)) != 0)
#define Flags_Is_Empty(value) ((value) == 0)

#define ToBits(bytes) ((bytes) * 8)

#define Bit_Flags(name, bitsCount)                          \
    typedef struct {                                        \
        u32 W[((bitsCount) - 1) / ToBits(sizeof(u32)) + 1]; \
    } name

#define Bit_Flags_Get_Word(bitFlags, bit)      ((bitFlags)->W[bit / ToBits(sizeof((bitFlags)->W[0]))])
#define Bit_Flags_Get_Word_Mask(bitFlags, bit) (1 << (bit % ToBits(sizeof((bitFlags)->W[0]))))
#define Bit_Flags_Set(bitFlags, bit)           (Bit_Flags_Get_Word(bitFlags, bit) |=  Bit_Flags_Get_Word_Mask(bitFlags, bit))
#define Bit_Flags_Unset(bitFlags, bit)         (Bit_Flags_Get_Word(bitFlags, bit) &= ~Bit_Flags_Get_Word_Mask(bitFlags, bit))
#define Bit_Flags_Switch(bitFlags, bit)        (Bit_Flags_Get_Word(bitFlags, bit) ^=  Bit_Flags_Get_Word_Mask(bitFlags, bit))
#define Bit_Flags_Is_Set(bitFlags, bit)      ((Bit_Flags_Get_Word(bitFlags, bit) & Bit_Flags_Get_Word_Mask(bitFlags, bit)) != 0)
#define Bit_Flags_Is_Empty(bitFlags)           (Mem_Comp_Zero((bitFlags), sizeof((bitFlags)->W)))

#define Stack(name, type, capacity, funcPopName, funcPushName, funcTopIndexName) \
    typedef struct { \
        u32 size; \
        type S[capacity]; \
    } name; \
\
    Inter type *funcPopName(name *stack) { \
        if (stack->size == 0) { return 0; } \
        return &stack->S[--stack->size]; \
    } \
\
    Inter type *funcPushName(name *stack) { \
        if (stack->size == capacity) { return 0; } \
        return &stack->S[stack->size++]; \
    } \
\
    Inter type *funcTopIndexName(name *stack, u32 index) { \
        if ((stack->size - 1 - index) >= 0) { return 0; } \
        return &stack->S[(stack->size - 1) - index]; \
    } \

#define File_Slice(typeName) \
    _Pragma("pack(push, 1)") \
    typedef struct { \
        u32 count; \
        File_Ptr(typeName*, E); \
    } P(File_Slice_, typeName); \
    _Pragma("pack(pop)")

#define Ring_Buffer(typeName) \
    typedef struct P(Ring_Buffer_, typeName) { \
        u32 capacity; \
        u32 begin;    \
        u32 end;      \
        typeName* E;  \
    } P(Ring_Buffer_, typeName)

Ring_Buffer(void);

#define Ring_Buffer_Create(arena, ringBuffer, inCapacity) \
    do { \
        (ringBuffer)->capacity = (inCapacity); \
        (ringBuffer)->E = Arena_Alloc_Mem(arena, sizeof((ringBuffer)->E[0]) * (ringBuffer)->capacity); \
    } while (0)

#define Ring_Buffer_Push(ringBuffer, elem) Ring_Buffer_Push_Impl((Ring_Buffer_void*)ringBuffer, (void*)&(elem), sizeof((ringBuffer)->E[0]))
#define Ring_Buffer_Pop(ringBuffer, outElem) Ring_Buffer_Pop_Impl((Ring_Buffer_void*)ringBuffer, outElem, sizeof((ringBuffer)->E[0]))

//#include <meta_data_structures.h>

#endif //DSV_DATASTRUCTURES_H
