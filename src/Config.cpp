#define CONFIG_FILE_NAME S("config.jcgf")

#define CONFIG_DEF_ENTRY_COUNT 1024

typedef struct {
    u32 count;
    Config_Def E[CONFIG_ID_MAX];
} Array_Config_Def;

typedef struct {
    u32 count;
    Config_Serialize_Data E[CONFIG_ID_MAX];
} Array_Config_Serialize_Data;

typedef struct {
    u32 filePtrOffsetCount;
    File_Ptr_Offset filePtrOffsets[KiB(32)];
    Array_Config_Def defTable;
    Array_Config_Serialize_Data serDataTable;
} Config_State;

Private Global Config_State gConfigState;

Private void Config_File_Ptr_Offsets_Create(Array_File_Ptr_Offset* outFilePtrOffsets, ...) {
    *outFilePtrOffsets = { 0 };

    va_list vaStream;
    va_start(vaStream, outFilePtrOffsets);

    u32 savedCount = gConfigState.filePtrOffsetCount;
    outFilePtrOffsets->E = &gConfigState.filePtrOffsets[savedCount];

    for (u16 offset = va_arg(vaStream, u16); offset != MAX_U16; offset = va_arg(vaStream, u16)) {
        File_Ptr_Offset* filePtrOffset = Array_Push(gConfigState.filePtrOffsets, &gConfigState.filePtrOffsetCount);

        filePtrOffset->offset = offset;
        filePtrOffset->ID = va_arg(vaStream, Config_ID);
    }

    outFilePtrOffsets->count = (u16)(gConfigState.filePtrOffsetCount - savedCount);

    va_end(vaStream);
}

Public void Config_Init() {
    gConfigState.filePtrOffsetCount = 0;

    Config_Def* def;
    Config_Serialize_Data* serData;

    #define X(configID, inType, inElemSize, location, countLocation, ...) \
        def = Array_Push(gConfigState.defTable.E, &gConfigState.defTable.count); \
        def->ID = configID; \
        def->type = inType; \
        def->elemSize = inElemSize; \
        Config_File_Ptr_Offsets_Create(&def->filePtrOffsets, __VA_ARGS__, MAX_U16); \
        \
        serData = Array_Push(gConfigState.serDataTable.E, &gConfigState.serDataTable.count); \
        serData->dataLoc = location; \
        serData->countVarLoc = countLocation;
    CONFIG_DEF_LIST
    #undef X
}

Private Config_ID Config_Find_Array_By_Loc(void* inPtr) {
    for (u32 ID = 0; ID < gConfigState.serDataTable.count; ++ID) {
        Config_Serialize_Data* serData = &gConfigState.serDataTable.E[ID];
        Config_Def* def = &gConfigState.defTable.E[ID];

        if ( (byte*)serData->arrayLoc <= (byte*)inPtr &&
             (byte*)inPtr <= ((byte*)serData->arrayLoc + ((*(u32*)serData->countVarLoc - 1) * def->elemSize))) {
            return (Config_ID)ID;
        }
    }

    //INVALID_PATH("");
    return (Config_ID)INVALID_THING;
}

Private Config_ID Config_Find_Array_By_Offset(void* inOffset, Config_Def* inConfigDefs, u32 inConfigDefCount) {
    u64 offset = (WordSize)inOffset;

    for (u32 ID = 0; ID < inConfigDefCount; ++ID) {
        Config_Def* configDef = &inConfigDefs[ID];

        if ( configDef->arrayOffset <= offset &&
             offset <= (configDef->arrayOffset + (configDef->count - 1) * configDef->elemSize)) {
            return (Config_ID)ID;
        }
    }

    //INVALID_PATH("");
    return (Config_ID)INVALID_THING;
}

Public void Config_Load() {
    File_Handle file = File_Open(CONFIG_FILE_NAME);

    Config_Header header;
    if ( !File_Read_At(file, 0, sizeof(Config_Header), &header) ) { INVALID_PATH(""); return; }

    Assert( header.magic == Magic_4("jcfg")   );
    Assert( header.version == CURRENT_VERSION );
    Assert( header.endiannes == LITTLE_ENDIAN );

    Config_Def configDefs[4096];
    Check( File_Read_At(file, FieldOffset(Config_Header, defs), header.defCount * sizeof(Config_Def), configDefs), == JENH_TRUE );

    File_Ptr_Offset filePtrOffsets[4096];
    Check( File_Read_At(file, header.filePtrOffsetsOffset, header.filePtrOffsetCount * sizeof(File_Ptr_Offset), filePtrOffsets), == JENH_TRUE );

    Foreach (Config_Def, configDef, configDefs, header.defCount) {
        File_Ptr_Array_Undo(&configDef->filePtrOffsets.E, header.filePtrOffsetsOffset, filePtrOffsets);
    }

    Foreach (Config_Def, configDef, configDefs, header.defCount) {
        // TODO(JENH): Better handling of serialize data for multiple version file formats.
        Config_Serialize_Data* serData = &gConfigState.serDataTable.E[configDef->ID];

        switch ( configDef->type ) {
            case CPET_Var: {
            } break;

            case CPET_Array: {
                // NOTE(JENH): This is arbitrary. Find a better way of handle it (avoiding this uneccesary check if possible).
                if ( (WordSize)serData->countVarLoc > MiB(128) ) {
                    *(u32*)serData->countVarLoc = configDef->count;
                }

                Check( File_Read_At(file, configDef->arrayOffset, configDef->count * configDef->elemSize, serData->arrayLoc), == JENH_TRUE);

                for (u32 i = 0; i < configDef->count; ++i) {
                    byte* elemBase = (byte*)serData->arrayLoc + (configDef->elemSize * i);

                    Foreach (File_Ptr_Offset, filePtrOffset, configDef->filePtrOffsets.E, configDef->filePtrOffsets.count) {
                        void** ptr = (void**)(elemBase + filePtrOffset->offset);

                        Config_ID IDToOffset = Config_Find_Array_By_Offset(*ptr, configDefs, header.defCount);
                        if ( IDToOffset != (Config_ID)INVALID_THING ) {
                            Config_Def* defToOffset = &configDefs[IDToOffset];
                            Config_Serialize_Data* serDataToOffset = &gConfigState.serDataTable.E[IDToOffset];

                            File_Ptr_Array_Undo(ptr, defToOffset->arrayOffset, serDataToOffset->arrayLoc);
                        } else {
                            *ptr = 0;
                        }
                    }
                }
            } break;

            NO_DEFAULT
        }
    }
}

Public void Config_Save() {
    Memory_Arena *arena = AllocTempArena(KiB(256));

    Config_Header* header = ArenaPushType(arena, Config_Header);
    header->magic = Magic_4("jcfg");
    header->version = CURRENT_VERSION;
    header->endiannes = LITTLE_ENDIAN;
    Mem_Zero_Array(header->padding);

    header->defCount = gConfigState.defTable.count;
    (void)ArenaPushArray(arena, Config_Def, header->defCount);
    Mem_Copy_Forward(header->defs, gConfigState.defTable.E, header->defCount * sizeof(Config_Def));

    header->filePtrOffsetCount = gConfigState.filePtrOffsetCount;
    void* filePtrOffsetsMem = ArenaPushArray(arena, File_Ptr_Offset, header->filePtrOffsetCount);
    Mem_Copy_Forward(filePtrOffsetsMem, gConfigState.filePtrOffsets, header->filePtrOffsetCount * sizeof(File_Ptr_Offset));
    header->filePtrOffsetsOffset = (u32)((byte*)filePtrOffsetsMem - (byte*)arena->base);

    Foreach (Config_Def, configDef, header->defs, header->defCount) {
        Config_Serialize_Data* serData = &gConfigState.serDataTable.E[configDef->ID];

        switch ( configDef->type ) {
            case CPET_Var: { } break;

            case CPET_Array: {
                // NOTE(JENH): This is arbitrary. Find a better way of handle it (avoiding this unnecessary check if possible).
                if ( (WordSize)serData->countVarLoc > MiB(128) ) {
                    configDef->count = *(u32*)serData->countVarLoc;
                } else {
                    configDef->count = (u32)(WordSize)serData->countVarLoc;
                }

                void* arrayMem = (byte*)ArenaPushMem(arena, configDef->count * configDef->elemSize);
                Mem_Copy_Forward(arrayMem, serData->arrayLoc, configDef->count * configDef->elemSize);

                configDef->arrayOffset = (u32)((byte*)arrayMem - (byte*)arena->base);
            } break;

            NO_DEFAULT
        }
    }

    Foreach (Config_Def, configDef, header->defs, header->defCount) {
        switch ( configDef->type ) {
            case CPET_Var: { } break;

            case CPET_Array: {
                for (u32 i = 0; i < configDef->count; ++i) {
                    byte* elemBase = ((byte*)arena->base + configDef->arrayOffset) + (configDef->elemSize * i);

                    Foreach (File_Ptr_Offset, filePtrOffset, configDef->filePtrOffsets.E, configDef->filePtrOffsets.count) {
                        void** ptr = (void**)(elemBase + filePtrOffset->offset);

                        Config_ID IDToOffset = Config_Find_Array_By_Loc(*ptr);
                        if ( IDToOffset != (Config_ID)INVALID_THING ) {
                            Config_Def* defToOffset = &header->defs[IDToOffset];
                            Config_Serialize_Data* serDataToOffset = &gConfigState.serDataTable.E[IDToOffset];

                            File_Ptr_Array_Do(ptr, serDataToOffset->arrayLoc, defToOffset->arrayOffset);
                        } else {
                            *ptr = 0;
                        }
                    }
                }
            } break;

            NO_DEFAULT
        }
    }

    Foreach (Config_Def, configDef, header->defs, header->defCount) {
        File_Ptr_Array_Do(&configDef->filePtrOffsets.E, gConfigState.filePtrOffsets, header->filePtrOffsetsOffset);
    }

    File_Handle file = File_Create(CONFIG_FILE_NAME);
    Check( File_Write(file, (u32)arena->used, arena->base), == JENH_TRUE );
    File_Close(file);

    FreeTempArena(arena);
}

#if 0
    size = sizeof(Field(Input_Mapping, inputToBindings));
    mem  = ArenaPushMem(arena, size);
    Mem_Copy_Forward(mem, gInputSystem.mapping.inputToBindings, size);

    size = sizeof(Field(Input_Mapping, bindingsPerAction));
    mem = ArenaPushMem(arena, size);
    Mem_Copy_Forward(mem, gInputSystem.mapping.bindingsPerAction, size);

    size = (u32)dyMem.perma.bindings.used;
    mem = ArenaPushMem(arena, size);
    Mem_Copy_Forward(mem, dyMem.perma.bindings.base, size);
#endif
