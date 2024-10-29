// returns the incremented value.
Public u32 Atomic_Inc_U32(volatile u32* inValuePtr) {
    Assert( ((WordSize)inValuePtr % 4) == 0 );
    return _InterlockedIncrement(inValuePtr);
}
