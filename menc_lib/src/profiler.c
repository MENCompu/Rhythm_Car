#define PROFILE_ENTRY_COUNT 4096

typedef struct {
    u32 count;
    Profile_Entry E[PROFILE_ENTRY_COUNT];
} Array_Profile_Entry;

typedef struct {
    Array_Profile_Entry table;
} Profile_State;

Private Global Profile_State gProfileState;

Public void Profile_Init() {
    gProfileState.table.count = 1;
}

Public void Profile_Block_Begin_(u32 inID, CString inFile, CString inFunction, u32 inLine, u64* outTimeStamp) {
    Profile_Entry* entry = &gProfileState.table.E[inID];

    entry->file = inFile;
    entry->function = inFunction;
    entry->lineNum = inLine;

    *outTimeStamp = Time_CPU_Counter();
}

Public void Profile_Block_End(u32 inID, u64 inTimeStamp) {
    Profile_Entry* entry = &gProfileState.table.E[inID];

    entry->timeStampCPU += Time_CPU_Counter() - inTimeStamp;
}

Public void Profile_Render() {
#if 1
    Foreach(Profile_Entry, entry, gProfileState.table.E, gProfileState.table.count) {
        LogInfo("fl: %s, fn: %s, ln: %u, timeStamp: %u",
                entry->file, entry->function, entry->lineNum, entry->timeStampCPU);
        entry->timeStampCPU = 0;
    }
#endif
}
