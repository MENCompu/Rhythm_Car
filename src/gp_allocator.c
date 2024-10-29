Public void GP_Allocator_Init(void* inMem, u32 inSize, GP_Allocator* outAlloc) {
    outAlloc->mem  = inMem;
    outAlloc->size = inSize;

    outAlloc->freeList.next = (GP_Allocator_Node*)inMem;

    GP_Allocator_Node* freeList = outAlloc->freeList.next;

    freeList->prev = &outAlloc->freeList;
    freeList->next = &outAlloc->freeListTop;
    freeList->size = inSize;
}

Public void* GP_Allocator_Alloc(GP_Allocator* inAlloc, u32 inSize) {
    u32 newNodeSize = Max(inSize, sizeof(GP_Allocator_Node));

    for (GP_Allocator_Node* node = inAlloc->freeList.next; node; node = node->next) {
        if ( newNodeSize == node->size ) {
            node->prev->next = node->next;
            node->next->next = node->prev;

            return (void*)node;
        } else if ( newNodeSize < node->size ) {
            GP_Allocator_Node* newNode = (GP_Allocator_Node*)((byte*)node + newNodeSize);
            newNode->size = node->size - newNodeSize;

            Assert( newNode->size <= sizeof(GP_Allocator_Node) );

            newNode->next = node->next;
            newNode->prev = node;

            newNode->prev->next = newNode;
            newNode->next->prev = newNode;

            return (void*)node;
        }
    }

    LogError("Failed to find a node that have enough space");
    return 0;
}

Public void GP_Allocator_Free(GP_Allocator* inAlloc, void* inMem, u32 inSize) {
    Assert( inSize >= sizeof(GP_Allocator_Node) );
    if ( inSize < sizeof(GP_Allocator_Node) ) { return; }

    Assert( inAlloc->mem <= inMem && inMem <= ((byte*)inAlloc->mem + inAlloc->size) );

    GP_Allocator_Node* node = inAlloc->freeList.next;
    for (; (byte*)inMem <= (byte*)node; node = node->next) {
        if ( !node ) {
            LogError("Failed to find free node");
            return;
        }
    }

    GP_Allocator_Node* newNode = (GP_Allocator_Node*)inMem;
    newNode->size = inSize;

    newNode->next = node->next;
    newNode->prev = node;

    newNode->prev->next = newNode;
    newNode->next->prev = newNode;

    if ( (byte*)newNode->next == ((byte*)newNode + newNode->size) ) {
        GP_Allocator_Node* nodeToDelete = newNode->next;
        newNode->size += nodeToDelete->size;

        nodeToDelete->next->prev = nodeToDelete->prev;
        nodeToDelete->prev->next = nodeToDelete->next;
    }

    if ( (byte*)newNode == ((byte*)newNode->prev + newNode->prev->size) ) {
        GP_Allocator_Node* nodeToDelete = newNode;

        newNode->prev->size += nodeToDelete->size;

        nodeToDelete->next->prev = nodeToDelete->prev;
        nodeToDelete->prev->next = nodeToDelete->next;
    }
}
