#ifndef RC_GP_ALLOCATOR_H
#define RC_GP_ALLOCATOR_H

typedef struct GP_Allocator_Node {
    GP_Allocator_Node* prev;
    GP_Allocator_Node* next;

    u32 size;
} GP_Allocator_Node;

typedef struct {
    u32 size;
    void* mem;

    GP_Allocator_Node freeList;

    GP_Allocator_Node freeListTop;
} GP_Allocator;

#endif // RC_GP_ALLOCATOR_
