#ifndef DSV_DATASTRUCTURES_H
#define DSV_DATASTRUCTURES_H

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define Local_Array_Init(type, arrayName, ...) Local_Array_Init_(type, arrayName, __COUNTER__, __VA_ARGS__)
#define Local_Array_Init_(type, arrayName, counter, ...) Local_Array_Init__(type, arrayName, counter, __VA_ARGS__)
#define Local_Array_Init__(type, arrayName, counter, ...) \
    Array_##type arrayName; \
    type dummyBufferNameArray##counter[] = {__VA_ARGS__}; \
    arrayName.A = dummyBufferNameArray##counter; \
    arrayName.size = ArrayCount(dummyBufferNameArray##counter);

#define Local_Array(type, arrayName, arraySize) Local_Array_(type, arrayName, arraySize, __COUNTER__)
#define Local_Array_(type, arrayName, arraySize, counter) Local_Array__(type, arrayName, arraySize, counter)
#define Local_Array__(type, arrayName, arraySize, counter) \
    Array_##type arrayName = { 0 }; \
    type dummyBufferNameArray##counter[arraySize]; \
    arrayName.A = dummyBufferNameArray##counter

#define LitToArray(arrayLit) { ArrayCount(arrayLit), arrayLit }

#define Fix_Array_Push(array) &(array).A[(array).size++] \

#define Array_Push(inArray, inCount) &(inArray)[(*(inCount))++]; \
    Assert( (*(inCount)) <= ArrayCount(inArray) )

#define Array_Ptr(name, type) \
    typedef struct {          \
        u32 size;             \
        type *A;              \
    } name

#define NulArray_Ptr(name, type) typedef type *name;

#define Array(name, type) \
    typedef struct {          \
        u32 size;             \
        type *A;              \
    } name

#define Array_Static(name, sizeArray, type) \
    typedef struct {           \
        u32 size;              \
        type A[sizeArray];     \
    } name

#define ArrayAlloc(array, arraySize, arena)                             \
    (array).A = ArenaPush((arena), sizeof((array).A[0]) * (arraySize)); \
    (array).size = arraySize;

Array(Array_byte_Ptr, byte *);
Array(Array_byte, byte);
Array(Array_u64, u64);
Array(Array_u32, u32);
Array(Array_s64, s64);

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

#endif //DSV_DATASTRUCTURES_H
