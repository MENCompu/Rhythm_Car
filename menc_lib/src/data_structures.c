Public void* Poly_(u32* dataTag, u32 tag, void* retVar) {
    Assert( *dataTag == tag );
    //return ( dataTag + 1 );
    return retVar;
}

Public void* DArray_Expand(Memory_Arena* arena, Array_void* array, u32 elemSize) {
    if ( array->count == array->capacity ) {
        Array_void copy = *array;

        array->capacity = ( array->capacity != 0 ) ? array->capacity * 2 : 1;
        array->E = Arena_Alloc_Mem(arena, elemSize * array->capacity);
        Mem_Copy_Forward(array->E, copy.E, elemSize * array->count);

        Mem_Zero(copy.E, elemSize * copy.capacity );
    }

    return (byte*)array->E + ( elemSize * array->count );
}

// Ring Buffer.
Public b8 Ring_Buffer_Push_Impl(Ring_Buffer_void* ringBuffer, void* elem, u32 elemSize) {
    if ( ringBuffer->begin == ringBuffer->end ) {
        return false;
    }

    void* data = (byte*)ringBuffer->E + ( elemSize * ringBuffer->end++ );
    Mem_Copy_Forward(data, elem, elemSize);

    if ( ringBuffer->begin == ringBuffer->capacity ) {
        ringBuffer->begin = 0;
    }

    return true;
}

Public b8 Ring_Buffer_Pop_Impl(Ring_Buffer_void* ringBuffer, void* outElem, u32 elemSize) {
    if ( ringBuffer->begin == ringBuffer->end ) {
        return false;
    }

    void* data = (byte*)ringBuffer->E + ( elemSize * ringBuffer->begin++ );
    Mem_Copy_Forward(outElem, data, elemSize);

    if ( ringBuffer->begin == ringBuffer->capacity ) {
        ringBuffer->begin = 0;
    }

    return true;
}
