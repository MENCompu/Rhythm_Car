#ifndef RC_FREE_LIST_H
#define RC_FREE_LIST_H

#define MAX_FREE_NODES 1024

typedef struct Free_List_Node {
    File_Ptr(struct Free_List_Node*, next);

    u64 size;
    u64 offset;
} Free_List_Node;

typedef struct {
    u64 size;
    Free_List_Node head;
    Free_List_Node nodes[MAX_FREE_NODES];
} Free_List;

Public void Free_List_Init(u64 inSize, Free_List* outFreeList);
Public u64 Free_List_Alloc(Free_List* inFreeList, u64 inSize);
Public void Free_List_Free(Free_List* inFreeList, u64 inOffset, u64 inSize);

#endif // RC_FREE_LIST_H
