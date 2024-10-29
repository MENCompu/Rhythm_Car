Private void Free_List_Release_Node(Free_List_Node* ioNode);
Private Free_List_Node* Free_List_Get_Node(Free_List* inFreeList);

Public void Free_List_Init(u64 inSize, Free_List* outFreeList) {
    Assert(inSize <= MAX_U32);

    outFreeList->size = inSize;

    Free_List_Node* firstNode = &outFreeList->nodes[0];
    firstNode->next = 0;
    firstNode->size = inSize;
    firstNode->offset = 0;

    for (u32 i = 1; i < ArrayCount(outFreeList->nodes); ++i) {
        Free_List_Node* node = &outFreeList->nodes[i];
        node->offset = MAX_U32;
    }

    outFreeList->head.next = firstNode;
    outFreeList->head.size = MAX_U32;
    outFreeList->head.offset = MAX_U32;
}

Public u64 Free_List_Alloc(Free_List* inFreeList, u64 inSize) {
    Assert(inSize <= MAX_U32);

    Free_List_Node* prev = (Free_List_Node*)&inFreeList->head;

    for (Free_List_Node* node = prev->next; node; prev = node, node = node->next) {
        if (node->size == inSize) {
            u64 offset = node->offset;

            prev->next = node->next;
            Free_List_Release_Node(node);

            return offset;
        } else if (node->size > inSize) {
            u64 offset = node->offset;

            node->offset += inSize;
            node->size -= inSize;

            return offset;
        }
    }

    LogError("Theres no more free list nodes");
    return MAX_U32;
}

Public void Free_List_Free(Free_List* inFreeList, u64 inOffset, u64 inSize) {
    Assert(inSize <= MAX_U32 && inOffset <= MAX_U32);

    Free_List_Node* prev = (Free_List_Node*)&inFreeList->head;

    for (Free_List_Node* node = prev->next; node; prev = node, node = node->next) {
        if (node->offset > inOffset) {
            Free_List_Node* newNode = Free_List_Get_Node(inFreeList);
            newNode->size = inSize;
            newNode->offset = inOffset;

            prev->next = newNode;
            newNode->next = node;

            if (newNode->offset + newNode->size == node->offset) {
                newNode->size += node->size;
                newNode->next = node->next;
                Free_List_Release_Node(node);
            }

            if (prev->offset + prev->size == newNode->offset) {
                prev->size += newNode->size;
                prev->next = newNode->next;
                Free_List_Release_Node(newNode);
            }

            return;
        }
    }

    LogWarn("wata fu");
}

Private void Free_List_Release_Node(Free_List_Node* ioNode) {
    ioNode->next = 0;
    ioNode->size = MAX_U32;
    ioNode->offset = MAX_U32;
}

Private Free_List_Node* Free_List_Get_Node(Free_List* inFreeList) {
    for (u32 i = 0; i < ArrayCount(inFreeList->nodes); ++i) {
        Free_List_Node* node = &inFreeList->nodes[i];
        if (node->offset == MAX_U32) {
            return node;
        }
    }

    LogError("All nodes in the free list are in use");
    return 0;
}

