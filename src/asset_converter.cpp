#define MAX_RESOURCES_TO_MONITOR 1024

#define CACHED_ASSETS_DIR_NAME "cache"
#define SAVED_CONVERT_TABLE_FILE_NAME "saved_convert_table.bin"

Public Global u32 assetsToReloadCount;
Public Global u64 assetsToReload[256];

// NOTE(JENH): Starts at 1 for the nul asset.
Private Global u32 gCurrentID;
Private Global u32 gGlobalFileTableCount;

typedef struct {
    u32 entryCount;
    Convert_Table_Entry table[KiB(4)];

    u32 stringTableSize;
    char stringTable[KiB(16)];

    u32 assetIDTableSize;
    u32 assetIDTable[KiB(2)];

    b8 checkingOfflineHasEnded;
    u32 offlineDeleteIndex; // TODO(JENH): Is this safe?.
    b8 checkingOfflineDeletesHasEnded;

    Dir_Stream dirStream;
} Converter_State;

Private Global Converter_State gConverterState;

Private void Converter_Monitor_Init(Memory_Arena* inArena, String inDirPath);

Private b8 Converter_Get_Offline_Change(String inDirPath, Dir_Change* outChange);

Private b8 Converter_Convert(Memory_Arena* inArena, String srcPath, u64 fileID, Resource_Format format);
Private Convert_Table_Entry* Converter_Find_Entry_By_File_Rel_Path(String inFileRelPath);
Private void Converter_Resolve_Depencencies(String inDirPath, String inFileName, u64 inAssetID);
Private void Convert_Create_Asset_File(u64 inAssetID, String inFileName, Asset_Type inType, u64 inWriteTime, u64 fileID);

// TODO(JENH): First, this is windows specific. And second, abstract away all the "sleeping thread" to a system like the job one.
Export Fn_Prot_Thread_Function(Converter_Monitor_Main) {
    String inDirPath = Str((char*)inArgs, CStrLen((char*)inArgs));

    u32 memSize = MiB(256);
    void* converterMem = OS_Alloc_Mem(memSize);

    Memory_Arena arena;
    Arena_Create(&arena, converterMem, memSize);

    Converter_Monitor_Init(&arena, inDirPath);

    Arena_Clear(&arena);

    Dir_Monitor monitor;
    Dir_Monitor_Init(inDirPath, &monitor);

    while ( !gAssetSystemIsInitialize );

    // TODO(JENH): This should end at some point?.
    while ( INFINITE_LOOP ) {
        Dir_Change change;

        if ( !Converter_Get_Offline_Change(inDirPath, &change) ) {
            if ( !Dir_Monitor_Wait_Till_Change(&monitor, reloadSemaphore, &change) ) { return; }
        }

        String fileExtension = Str_End(change.fileName, Str_Find_Char_Backward(change.fileName, '.'));
        Resource_Format format = Asset_Get_Format_By_Extension(fileExtension);

        // Skip unknown formats.
        if (format == RF_Nul) { continue; }

        Local_Str(filePath, MAX_FILE_PATH);
        CatStr(&filePath, inDirPath, change.fileName);
        filePath.str[filePath.size] = '\0';

        switch ( change.type ) {
            case DCT_Rename: {
                Convert_Table_Entry* oldEntry = Converter_Find_Entry_By_File_Rel_Path(change.oldFileName);
                Convert_Table_Entry* newEntry = Converter_Find_Entry_By_File_Rel_Path(change.newFileName);

                Asset* oldAsset = &gAssetSystem.assets[oldEntry->assetID];

                char fileName[ASSET_FILE_NAME_SIZE];
                Asset_Get_File_Name(oldAsset->fileID, fileName);
                LogInfo("Renaming asset file: (name: \"%.*s\" | cache: \"%.*s\" | type: %s)", filePath.size,
                        filePath.str, ASSET_FILE_NAME_SIZE, fileName, Asset_Get_Type_Name(oldAsset->type));

                if ( newEntry ) {
                    newEntry->writeTime = oldEntry->writeTime;

                    Asset* newAsset = &gAssetSystem.assets[newEntry->assetID];
                    newAsset->fileID = oldAsset->fileID;

                    Asset_Reload(newEntry->assetID);
                } else {
                    // TODO(JENH): This should be done just for assets that don't have dependencies. (ie. Models).

                    oldEntry->srcFileRelPath.size = change.newFileName.size;

                    // TODO(JENH): Proper allocator and delete the older name in the string table.
                    oldEntry->srcFileRelPath.str = gConverterState.stringTable + gConverterState.stringTableSize;
                    gConverterState.stringTableSize += oldEntry->srcFileRelPath.size;

                    Mem_Copy_Forward(oldEntry->srcFileRelPath.str, change.newFileName.str, oldEntry->srcFileRelPath.size);
                    //
                }
            } break;

            // TODO(JENH): Should delete strings and asset IDs from its respectives tables.
            case DCT_Delete: {
                Convert_Table_Entry* entry = Converter_Find_Entry_By_File_Rel_Path(change.fileName);
                Asset* asset = &gAssetSystem.assets[entry->assetID];

                char fileName[ASSET_FILE_NAME_SIZE];
                Asset_Get_File_Name(asset->fileID, fileName);
                LogInfo("Destroying asset file: (name: \"%.*s\" | cache: \"%.*s\" | type: %s)", filePath.size,
                        filePath.str, ASSET_FILE_NAME_SIZE, fileName, Asset_Get_Type_Name(asset->type));

                Assert(asset->type == Asset_Get_Type_By_Reasource_Format(format));

                String cacheFileName = Str(fileName, ASSET_FILE_NAME_SIZE);

                Local_Str(cacheFilePath, MAX_FILE_PATH);
                CatStr(&cacheFilePath, LitToStr("..\\assets\\assets\\cache\\"), cacheFileName);
                CatStr(&cacheFilePath, cacheFilePath, Asset_Get_Extension_By_Asset_Type(asset->type));
                cacheFilePath.str[cacheFilePath.size] = '\0';

                Assert( File_Delete(cacheFilePath.str) );

                Asset_Reload(entry->assetID);
                // TODO(JENH): Delete the converter entry and its references of the assets that don't have any dependencies (ie. Models).
                //Asset_Reload(entry->assetID);
                //*entry = gConverterState.table[--gConverterState.entryCount];
            } break;

            case DCT_Crate: {
                u64 assetID;
                u64 fileID;

                Convert_Table_Entry* entry = Converter_Find_Entry_By_File_Rel_Path(change.fileName);

                if ( entry ) {
                    entry->writeTime = File_Get_Write_Time(filePath.str);

                    Asset* asset = &gAssetSystem.assets[entry->assetID];
                    fileID = asset->fileID;
                    assetID = entry->assetID;
                } else {
                    assetID = gGlobalFileTableCount++;
                    fileID = gCurrentID++;
                }

                char fileName[ASSET_FILE_NAME_SIZE];
                Asset_Get_File_Name(fileID, fileName);
                LogInfo("Creating asset file: (name: \"%.*s\" | cache: \"%.*s\" | type: %s)", filePath.size,
                        filePath.str, ASSET_FILE_NAME_SIZE, fileName, Asset_Get_Type_Name(Asset_Get_Type_By_Reasource_Format(format)));

                (void)Converter_Convert(&arena, filePath, fileID, format);

                if ( entry ) {
                    Asset_Reload(assetID);
                } else {
                    // TODO(JENH): This should be below "Converter_Convert", because the asset system shouldn't know about the
                    //             the new file until the convertion has finished.
                    Convert_Create_Asset_File(assetID, change.fileName, Asset_Get_Type_By_Reasource_Format(format),
                                              File_Get_Write_Time(filePath.str), fileID);
                }
            } break;

            case DCT_Write: {
                Convert_Table_Entry* entry = Converter_Find_Entry_By_File_Rel_Path(change.fileName);

                entry->writeTime = File_Get_Write_Time(change.fileName.str);

                // TODO(JENH): Private.
                Asset* asset = &gAssetSystem.assets[entry->assetID];

                char fileName[ASSET_FILE_NAME_SIZE];
                Asset_Get_File_Name(asset->fileID, fileName);
                LogInfo("Updating asset file: (name: \"%.*s\" | cache: \"%.*s\" | type: %s)", filePath.size,
                        filePath.str, ASSET_FILE_NAME_SIZE, fileName, Asset_Get_Type_Name(asset->type));

                Assert( asset->type == Asset_Get_Type_By_Reasource_Format(format) || asset->type == AT_Nul );

                (void)Converter_Convert(&arena, filePath, asset->fileID, format);

                // TODO(JENH): This should release the previous used memory and problems with syncronisation.
                Asset_Reload(entry->assetID);
                //
            } break;
        }

        Arena_Clear(&arena);

        // Copy convert table, 'couse of string offsets being relative.
        Convert_Table_Entry* fileTable = ArenaPushArray(&dyMem.temp.arena, Convert_Table_Entry, gConverterState.entryCount - 1);
        Mem_Copy_Forward(fileTable, gConverterState.table, (gConverterState.entryCount - 1) * sizeof(Convert_Table_Entry));

        for (u32 i = 1; i < gConverterState.entryCount; ++i) {
            Convert_Table_Entry* entry = &fileTable[i];

            entry->srcFileRelPath.strOffset = (WordSize)entry->srcFileRelPath.str - (WordSize)gConverterState.stringTable;
        }

        Convert_Table_Header newHeader;
        newHeader.entryCount = gConverterState.entryCount - 1;
        newHeader.table = (Convert_Table_Entry*)(sizeof(Convert_Table_Header));
        newHeader.stringTableSize = gConverterState.stringTableSize;
        newHeader.stringTable = (char*)((byte*)newHeader.table + (newHeader.entryCount * sizeof(Convert_Table_Entry)));

        Local_Str(convertTableFilePath, MAX_FILE_PATH);
        CatStr(&convertTableFilePath, inDirPath, LitToStr(SAVED_CONVERT_TABLE_FILE_NAME));
        convertTableFilePath.str[convertTableFilePath.size] = '\0';

        File_Handle convertTableFile = File_Create(convertTableFilePath.str);
        File_Write_At(convertTableFile, 0, sizeof(Convert_Table_Header), &newHeader);
        File_Write_At(convertTableFile, (WordSize)newHeader.table, newHeader.entryCount * sizeof(Convert_Table_Entry), fileTable);
        File_Write_At(convertTableFile, (WordSize)newHeader.stringTable, gConverterState.stringTableSize, gConverterState.stringTable);
        File_Close(convertTableFile);

        Arena_Clear(&arena);
    }

    Dir_Monitor_End(&monitor);
}

Public Asset_Handle Converter_Get_Asset_Handle(String inFileName) {
    Convert_Table_Entry* entry = Converter_Find_Entry_By_File_Rel_Path(inFileName);

    if ( entry ) {
        return Asset_Get_Handle(entry->assetID);
    } else {
        String fileExtension = Str_End(inFileName, Str_Find_Char_Backward(inFileName, '.'));
        Asset_Type type = Asset_Get_Type_By_Reasource_Format(Asset_Get_Format_By_Extension(fileExtension));

        u64 assetID = gGlobalFileTableCount++;
        Convert_Create_Asset_File(assetID, inFileName, type, 0, gCurrentID++);

        return Asset_Get_Handle(assetID);
    }
}

Public Convert_Table_Entry* Converter_Find_Entry_By_File_Name(String inFileName) {
    return Converter_Find_Entry_By_File_Rel_Path(inFileName);
}

Private void Convert_Create_Asset_File(u64 inAssetID, String inFileName, Asset_Type inType, u64 inWriteTime, u64 inFileID) {
    Convert_Table_Entry* entry = Array_Push(gConverterState.table, &gConverterState.entryCount);

    entry->writeTime = inWriteTime;
    entry->srcFileRelPath.size = inFileName.size;

    // TODO(JENH): Proper allocator.
    entry->srcFileRelPath.str = gConverterState.stringTable + gConverterState.stringTableSize;
    gConverterState.stringTableSize += entry->srcFileRelPath.size;

    Mem_Copy_Forward(entry->srcFileRelPath.str, inFileName.str, entry->srcFileRelPath.size);
    //

    // TODO(JENH): Private.
    entry->assetID = inAssetID;

    Asset* asset = &gAssetSystem.assets[entry->assetID];
    asset->type = inType;
    asset->state = AS_Unloaded;
    asset->dataID = INVALID_THING;
    asset->fileID = inFileID;

    RCAT_Asset fileAsset;
    fileAsset.defaultID = 0;
    fileAsset.type = asset->type;
    fileAsset.fileID = asset->fileID;

    // TODO(JENH): Private.
    // TODO(JENH: Get rid of Asset_Get_Table_File().

    // Update the asset Count in the asset table file.
    File_Write_At(Asset_Get_Table_File(), FieldOffset(RCAT_Header, assetCount),
                  FieldSize(RCAT_Header, assetCount), &(++gAssetSystem.assetCount));

    // Add the new entry.
    File_Write_At(Asset_Get_Table_File(), FieldOffset(RCAT_Header, assets) + (entry->assetID * sizeof(RCAT_Asset)),
                  sizeof(RCAT_Asset), &fileAsset);
}

Private Convert_Table_Entry* Converter_Find_Entry_By_File_Rel_Path(String inFileRelPath) {
    Convert_Table_Entry* retEntry = 0;

    // TODO(JENH): This is just to asset that there is only one fileRelPath in the table that matches the one requested.
    //             That there are no repeated paths in the table in short.
    b8 found = JENH_FALSE;

    for (u32 i = 1; i < gConverterState.entryCount; ++i) {
        Convert_Table_Entry* entry = &gConverterState.table[i];

        String entryFileRelPath = Str(entry->srcFileRelPath.str, entry->srcFileRelPath.size);

        if (Str_Equal(entryFileRelPath, inFileRelPath)) {
            Assert( !found );

            retEntry = entry;
            found = JENH_TRUE;
        }
    }

    return retEntry;
}

Private b8 Converter_Convert(Memory_Arena* inArena, String srcPath, u64 inFileID, Resource_Format format) {
    File_Handle srcFile = File_Open(srcPath.str);

    void* dstFileMem = 0;
    u32 dstFileSize = 0;

    u32 srcFileSize;

#if 0
    // NOTE(JENH): While recreating a file its size can be zero.
    u32 srcFileSize = File_Get_Size(srcFile);
    if ( srcFileSize == MAX_U32 ) {
        LogWarn("Failed to query size of resource file");
        return JENH_FALSE;
    }
#else
    do {
        srcFileSize = File_Get_Size(srcFile);

        if (srcFileSize == MAX_U32) {
            LogWarn("Failed to query size of resource file");
            //return JENH_FALSE;
        }
    } while (srcFileSize == 0);
#endif

    void* srcFileMem = ArenaPushMem(inArena, srcFileSize);

    if (!File_Read(srcFile, srcFileSize, srcFileMem)) {
        LogWarn("Failed to open resource file");
        return JENH_FALSE;
    }

    File_Close(srcFile);

    Local_Str(dstName, ASSET_FILE_NAME_SIZE);
    Asset_Get_File_Name(inFileID, dstName.str);
    dstName.size = ASSET_FILE_NAME_SIZE;

    Local_Str(dstFilePath, KiB(2));
    CatStr(&dstFilePath, LitToStr("..\\assets\\assets\\cache\\"), dstName);
    CatStr(&dstFilePath, dstFilePath, Asset_Get_Extension_By_Asset_Type(Asset_Get_Type_By_Reasource_Format(format)));
    dstFilePath.str[dstFilePath.size] = '\0';

    // TODO(JENH): This should not use the same fileID of the original file becouse overwrites it. Just create a temporary file and at the
    //             end replace it with the original file.
    File_Handle dstFile = File_Create(dstFilePath.str);

    Memory_Arena dstFileArena;
    Arena_Create(&dstFileArena, (byte*)inArena->base + inArena->used, (u32)((inArena->size - inArena->used)));

    u32 headerSize = 0;

#if 0
    if ( Str_Equal(srcPath, LitToStr("..\\assets\\assets\\vase_plant.tga")) ) {
        DEBUG_Breakpoint;
    }
#endif

    switch (format) {
        case RF_BMP: {
            RCAF_Texture tex;

            dstFileMem = Image_Converter_BMP_To_RCAF(srcFileMem, &dstFileArena, &tex);
            dstFileSize = (u32)dstFileArena.used;

            File_Write_At(dstFile, 0, sizeof(RCAF_Texture), &tex);
            File_Write_At(dstFile, sizeof(RCAF_Texture), dstFileSize, dstFileMem);
        } break;

        case RF_PNG: {
            RCAF_Texture tex;

            if ( !Image_Converter_PNG_To_RCAF(srcFileMem, srcFileSize, dstFile, &tex) ) {
                LogWarn("Failed to load texture \"%s\": %s", srcPath.str, stbi_failure_reason());
            }
        } break;

        case RF_TGA: {
            RCAF_Texture tex;

            dstFileMem = Image_Converter_TGA_To_RCAF(srcFileMem, srcFileSize, &dstFileArena, &tex);
            dstFileSize = (u32)dstFileArena.used;

            File_Write_At(dstFile, 0, sizeof(RCAF_Texture), &tex);
            File_Write_At(dstFile, sizeof(RCAF_Texture), dstFileSize, dstFileMem);
        } break;

        case RF_WAV: {
        } break;

        case RF_OBJ: {
            String srcDirPath = Str_Skip_End(srcPath, Str_Find_Char_Backward(srcPath, OS_PATH_COMPONENT_SEPARATOR_CHAR));

            (void)Model_Converter_OBJ_To_RCAF(srcFileMem, srcFileSize, dstFile, srcDirPath, &dstFileArena);
        } break;

        case RF_Fnt: {
            (void)Font_Converter_Fnt_To_RCF(srcFileMem, srcFileSize, dstFile, &dstFileArena);
        } break;

        case RF_Nul:
        NO_DEFAULT
    }

    File_Close(dstFile);

    return JENH_TRUE;
}

Private void Converter_Monitor_Init(Memory_Arena* inArena, String inDirPath) {
    Assert( Dir_Exists(inDirPath.str) );

    Local_Str(cachedAssetsDir, 256);
    CatStr(&cachedAssetsDir, inDirPath, LitToStr(CACHED_ASSETS_DIR_NAME"\\"));
    cachedAssetsDir.str[cachedAssetsDir.size] = '\0';

    if ( !Dir_Exists(cachedAssetsDir.str) ) {
        Assert( Dir_Create(cachedAssetsDir.str) );
    }

    Local_Str(savedConvertTableFilePath, MAX_FILE_PATH);
    CatStr(&savedConvertTableFilePath, inDirPath, LitToStr(SAVED_CONVERT_TABLE_FILE_NAME));
    savedConvertTableFilePath.str[savedConvertTableFilePath.size++] = '\0';

    // null asset.
    gConverterState.table[0] = { 0 };

    if ( File_Exists(savedConvertTableFilePath.str) && gAssetSystem.assetCount != 1 /* There are assets in the asset system. */ ) {
        File_Handle savedConvertTableFile;

        savedConvertTableFile = File_Open(savedConvertTableFilePath.str);

        u32 savedConvertFileSize = File_Get_Size(savedConvertTableFile);
        byte* savedConvertFileMem = (byte*)ArenaPushMem(inArena, savedConvertFileSize);

        Convert_Table_Header savedConvertHeader;
        Check( File_Read_At(savedConvertTableFile, 0, sizeof(Convert_Table_Header), &savedConvertHeader), == JENH_TRUE );

        Check( File_Read_At(savedConvertTableFile, (WordSize)savedConvertHeader.table,
                             savedConvertHeader.entryCount * sizeof(Convert_Table_Entry), &gConverterState.table[1]), == JENH_TRUE );

        Check( File_Read_At(savedConvertTableFile, (WordSize)savedConvertHeader.stringTable,
                             savedConvertHeader.stringTableSize, gConverterState.stringTable), == JENH_TRUE );

        File_Close(savedConvertTableFile);

        gConverterState.entryCount = savedConvertHeader.entryCount + 1;
        gConverterState.stringTableSize = savedConvertHeader.stringTableSize;

        for (u32 i = 1; i < gConverterState.entryCount; ++i) {
            Convert_Table_Entry* entry = &gConverterState.table[i];

            entry->srcFileRelPath.str = (char*)((byte*)gConverterState.stringTable + (WordSize)entry->srcFileRelPath.str);
        }
    } else {
        gConverterState.entryCount = 1;
        gConverterState.stringTableSize = 0;
    }

    gCurrentID = gAssetSystem.assetCount;
    gGlobalFileTableCount = gAssetSystem.assetCount;
    gConverterState.offlineDeleteIndex = 1;

#if 0
    Local_Str(cachePath, 256);
    CatStr(&cachePath, cachedAssetsDir, LitToStr("*"));

    u32 cachedDirFilesCount = Dir_Recursive_File_Count(cachePath.str);

    File_Info cachedDirFiles[1024];

    u32 cachedDirFilesCount = Dir_Get_All_Files_Write_Times(cachePath.str, cachedDirFiles, ArrayCount(cachedDirFiles));

    // TODO(JENH): Now that we are no longer using packed files revisit this.
    // See if files in the cached directory are present in the "file table". If not, delete them.
    for (u32 i = 0; i < cachedDirFilesCount; ++i) {
        File_Info* fileInfo = &cachedDirFiles[i];

        String fileName = Str(fileInfo->name, CStrLen(fileInfo->name));
        if (Str_Equal(fileName, LitToStr(SAVED_CONVERT_TABLE_FILE_NAME))) { continue; }

        fileName = Str_Skip_End(fileName, Str_Find_Char_Backward(fileName, '.') + 1);

        for (u32 j = 0; j < savedConvertHeader->entryCount; ++j) {
            File_Info* cachedFile = &savedConvertTable[j];

            String cachedName = Str(cachedFile->name, CStrLen(cachedFile->name));
            cachedName = Str_Skip_End(cachedName, Str_Find_Char_Backward(cachedName, '.') + 1);

            if (Str_Equal(cachedName, fileName)) {
                goto found_1;
            }
        }

        /* if not found */ {
            Local_Str(rscName, MAX_FILE_PATH);

            CatStr(&rscName, cachedAssetsDir, fileName);
            CatStr(&rscName, rscName, LitToStr(".rcr"));
            rscName.str[rscName.size] = '\0';

            File_Delete(rscName.str);
        }
        found_1:;
    }
#endif
}

Private b8 Converter_Is_Entry_Unresolved(Convert_Table_Entry* inEntry) {
    // NOTE(JENH): writeTime being equal to 0 means that this entry is an unresolve reference of other asset and must be created.
    return ( inEntry->writeTime == 0 );
}

Private b8 Converter_Get_Offline_Change(String inDirPath, Dir_Change* outChange) {
    if ( gConverterState.checkingOfflineHasEnded ) { return JENH_FALSE; }

    if ( !gConverterState.checkingOfflineDeletesHasEnded ) {
        // NOTE(JENH): This code traverse the entire table for each time that it finds a deleted file.
        while ( gConverterState.offlineDeleteIndex < gConverterState.entryCount ) {
            Convert_Table_Entry* entry = &gConverterState.table[gConverterState.offlineDeleteIndex];
            ++gConverterState.offlineDeleteIndex;

            String fileName = Str_From_File_Str(entry->srcFileRelPath);

            Local_Str(filePath, MAX_FILE_PATH);
            CatStr(&filePath, inDirPath, fileName);
            filePath.str[filePath.size] = '\0';

            // NOTE(JENH): writeTime being equal to 0 means that this entry is an unresolve reference of other asset and must be created.
            if ( !Converter_Is_Entry_Unresolved(entry) && !File_Exists(filePath.str) ) {
                outChange->type = DCT_Delete;
                outChange->fileName = fileName;
                return JENH_TRUE;
            }
        }

        gConverterState.checkingOfflineDeletesHasEnded = JENH_TRUE;
        Check( Dir_Stream_Open(inDirPath, &gConverterState.dirStream), == JENH_TRUE );
    }

    for (String fileName; Dir_Stream_Read(&gConverterState.dirStream, &fileName);) {
        Local_Str(filePath, MAX_FILE_PATH);
        CatStr(&filePath, inDirPath, fileName);
        filePath.str[filePath.size] = '\0';

        u64 writeTime = File_Get_Write_Time(filePath.str);

        Convert_Table_Entry* entry = Converter_Find_Entry_By_File_Rel_Path(fileName);

        // fileName doesn't exists in the convert table. So it's a new file and most be converted.
        if ( !entry || Converter_Is_Entry_Unresolved(entry) ) {
            outChange->type = DCT_Crate;
            outChange->fileName = fileName;
            return JENH_TRUE;
        }

        // fileName exists in the table, but it's out dated.
        if ( writeTime != entry->writeTime ) {
            outChange->type = DCT_Write;
            outChange->fileName = fileName;
            return JENH_TRUE;
        }

        // If file reach up to here, it means that it's up to date and should not be updated.
    }

    Dir_Stream_Close(&gConverterState.dirStream);
    gConverterState.checkingOfflineHasEnded = JENH_TRUE;

    // TODO(JENH): This should be better handled.
    converterFinish = JENH_TRUE;

    return JENH_FALSE;
}
