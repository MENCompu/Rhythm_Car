Public b8 File_Exists(CString path) {
    BOOL res = PathFileExistsA((char*)path);

    return (b8)res;
}

Public File_Handle File_Create(CString inPath) {
    // OPEN_ALWAYS
    File_Handle retHandle = (File_Handle)CreateFileA((char*)inPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                     0, CREATE_ALWAYS, 0, 0);
    return retHandle;
}

Public File_Handle File_Open(CString inPath) {
    // OPEN_ALWAYS
    File_Handle retHandle = (File_Handle)CreateFileA((char*)inPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                     0, OPEN_EXISTING, 0, 0);
    return retHandle;
}

Public void File_Close(File_Handle inHandle) {
    CloseHandle((HANDLE)inHandle);
}

Public b8 File_Delete(CString inPath) {
    if ( !DeleteFile((char*)inPath) ) {
        Win32_LogError("Failed to delete the file");
        return (b8)false;
    }

    return (b8)true;
}

Public b8 File_Handle_Is_Valid(File_Handle inHandle) {
    return (b8)( inHandle != INVALID_HANDLE_VALUE );
}

// TODO(JENH): Is worth supporting files bigger than 4 GiB?
Public u32 File_Get_Size(File_Handle inHandle) {
    HANDLE handleOS = (HANDLE)inHandle;

    u32 ret;

	LARGE_INTEGER size;
	if (!GetFileSizeEx(handleOS, &size)) {
        Win32_LogWarn("Couldn't get file size");
        return MAX_U32;
    }

    ret = (u32)size.QuadPart;

    return ret;
}

// TODO(JENH): Is worth supporting files bigger than 4 GiB?
Public b8 File_Read(File_Handle handle, u32 bytesToRead, void* mem) {
    HANDLE handleOS = (HANDLE)handle;

    DWORD bytesRead;
    if (!ReadFile(handleOS, mem, (DWORD)bytesToRead, &bytesRead, 0) || bytesRead != bytesToRead) {
        Win32_LogWarn("Failed to read the file at the amount requested");
        return false;
    }

    return true;
}

Public b8 File_Read_At(File_Handle handle, WordSize offset, u32 bytesToRead, void* mem) {
    HANDLE handleOS = (HANDLE)handle;

    u32 offset32 = Safe_Cast_WordSize_To_U32(offset);

    OVERLAPPED overlapped = {0};
    overlapped.Offset = offset32;

    DWORD bytesRead;
    if (!ReadFile(handleOS, mem, (DWORD)bytesToRead, &bytesRead, &overlapped) || bytesRead != bytesToRead) {
        Win32_LogWarn("Failed to read the file at the amount requested");
        return false;
    }

    return true;
}

Public void* File_Read_All(String path, Memory_Arena* arena, u32* size) {
    u32 dummy = 0;
    if ( !size ) { size = &dummy; }

    void* retData;

    File_Handle file = File_Open(path.E);

    *size = File_Get_Size(file);
    retData = Arena_Alloc_Mem(arena, *size);

    Win32_Check( File_Read(file, *size, retData), > 0 );

    File_Close(file);

    return retData;
}

// TODO(JENH): Is worth supporting files bigger than 4 GiB?
Public b8 File_Write(File_Handle handle, u32 bytesToWrite, void* mem) {
    HANDLE handleOS = (HANDLE)handle;

    DWORD bytesWritten;
    if (!WriteFile(handleOS, mem, bytesToWrite, &bytesWritten, 0) || bytesWritten != bytesToWrite) {
        Win32_LogWarn("Failed to write to the file by the amount of bytes requested");
        return false;
    }

    return true;
}

Public b8 File_Write_At(File_Handle handle, WordSize offset, u32 bytesToWrite, void* mem) {
    HANDLE handleOS = (HANDLE)handle;

    u32 offset32 = Safe_Cast_WordSize_To_U32(offset);

    OVERLAPPED overlapped = {0};
    overlapped.Offset = offset32;

    DWORD bytesWritten;
    if (!WriteFile(handleOS, mem, bytesToWrite, &bytesWritten, &overlapped) || bytesWritten != bytesToWrite) {
        Win32_LogWarn("Failed to write to the file by the amount of bytes requested");
        return false;
    }

    return true;
}

Public void File_Size_Set(File_Handle handle, u32 size) {
    Win32_Check( SetFilePointer(handle, size, 0, FILE_BEGIN), != INVALID_SET_FILE_POINTER );
    Win32_Check( SetEndOfFile(handle), > 0 );
}

Public void File_Clear(File_Handle handle) {
    File_Size_Set(handle, 0);
}

Public void File_Create_With_Contents(String path, void* data, u32 size) {
    File_Handle file = File_Create(path.E);

    Win32_Check( File_Write(file, size, data) , > 0);

    File_Close(file);
}

Public b8 File_Replace_Contents(String path, void* data, u32 size) {
    Local_Str(tempFilePath, MAX_FILE_PATH);
    Str_From_Fmt(&tempFilePath, "%s.temp", path.E);

    File_Handle tempFile = File_Create(tempFilePath.E);
    Win32_Check( File_Write(tempFile, size, data), > 0 );
    File_Close(tempFile);

    Win32_Check( ReplaceFileA((char*)path.E, (char*)tempFilePath.E, 0, 0, 0, 0), > 0 );

    return true;
}

Public void File_Fill_With_Contents_Safe(String path, void* data, u32 size) {
    if ( File_Exists(path.E) ) {
        File_Replace_Contents(path, data, size);
    } else {
        File_Create_With_Contents(path, data, size);
    }
}

Public b8 File_Append(File_Handle handle, u32 bytesToAppend, void* mem) {
    HANDLE handleOS = (HANDLE)handle;

    OVERLAPPED overlapped = {0};
    overlapped.Offset = 0xffffffff;
    overlapped.OffsetHigh = 0xffffffff;

    DWORD bytesWritten;
    if (!WriteFile(handleOS, mem, bytesToAppend, &bytesWritten, &overlapped) || bytesWritten != bytesToAppend) {
        Win32_LogWarn("Failed to write to the file by the amount of bytes requested");
        return false;
    }

    return true;
}

Public Time File_Get_Write_Time(CString inPath) {
    FILETIME fileTimeOS = {0};
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    if ( !GetFileAttributesExA((char*)inPath, GetFileExInfoStandard, &fileData) ) {
        return 0;
    }

	fileTimeOS = fileData.ftLastWriteTime;

    return OS_FILETIME_To_Time(fileTimeOS);
}

// Directories

Public b8 Dir_Exists(CString path) {
    BOOL res = PathFileExistsA((char*)path);

    return (b8)res;
}

Public b8 Dir_Create(CString path) {
    BOOL res = CreateDirectoryA((char*)path, 0);

    return (b8)res;
}

Public File_Handle Dir_Open(CString path, b8 async) {
    //s32 flags = ((async) ? FILE_FLAG_OVERLAPPED : 0);
    File_Handle retHandle = (File_Handle)CreateFileA((char*)path, FILE_LIST_DIRECTORY|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                     0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED , 0);
    return retHandle;
}

Public File_Tag File_Get_Tag(CString path) {
    DWORD attrib;
    Win32_Check( ( attrib = GetFileAttributesA((char*)path) ) , != INVALID_FILE_ATTRIBUTES);

    return ( attrib & FILE_ATTRIBUTE_DIRECTORY ) ? FILE_TAG_DIRECTORY : FILE_TAG_FILE;
}

#if 0
Public void Dir_Recursive_Iter(String inDirPath, Fn_Dir_Callback* CallBack, void* userMem) {
    Local_Str(dirWildcard, MAX_FILE_PATH);
    CatStr(&dirWildcard, inDirPath, S("*"));
    dirWildcard.E[dirWildcard.count] = '\0';

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(dirWildcard.E, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return;
    }

    Assert(CompCStrSize(data.cFileName, "." , Array_Count(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, "..", Array_Count("..")) == 0);

    while ( FindNextFileA(hFind, &data) ) {
        String fileName = Str(data.cFileName, CStr_Len(data.cFileName));

        Local_Str(filePath, MAX_FILE_PATH);
        CatStr(&filePath, inDirPath, fileName);
        filePath.E[filePath.count] = '\0';

        CallBack(filePath, userMem);

        File_Tag fileTag = File_Get_Tag(filePath.E);

        if ( fileTag == FILE_TAG_DIRECTORY ) {
            CatStr(&filePath, inDirPath, S("\\"));
            filePath.E[filePath.count] = '\0';

            Dir_Recursive_Iter(filePath, CallBack, userMem);
        }
    }

    FindClose(hFind);
}
#endif

Public void Dir_Recursive_Delete(String inDirPath) {
    Local_Str(dirWildcard, MAX_FILE_PATH);
    CatStr(&dirWildcard, inDirPath, S("*"));
    dirWildcard.E[dirWildcard.count] = '\0';

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA((char*)dirWildcard.E, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return;
    }

    Assert(CompCStrSize(data.cFileName, "." , Array_Count(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, "..", Array_Count("..")) == 0);

    while ( FindNextFileA(hFind, &data) ) {
        String fileName = (String){ .E = (u8*)data.cFileName, .count = CStr_Len((u8*)data.cFileName) };

        Local_Str(filePath, MAX_FILE_PATH);
        CatStr(&filePath, inDirPath, fileName);
        filePath.E[filePath.count] = '\0';

        File_Delete(filePath.E);
    }

    FindClose(hFind);

    Win32_Check( RemoveDirectoryA((char*)inDirPath.E), > 0 );
}

Public u32 Dir_File_Count(String dirPath) {
    u32 retFileCount = 0;

    Local_Str(dirWildcard, MAX_FILE_PATH);
    CatStr(&dirWildcard, dirPath, S("\\*"));
    dirWildcard.E[dirWildcard.count] = '\0';

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA((char*)dirWildcard.E, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return MAX_U32;
    }

    Assert(CompCStrSize(data.cFileName, ".", Array_Count(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, "..", Array_Count("..")) == 0);

    while ( FindNextFileA(hFind, &data) ) {
        ++retFileCount;
    }

    FindClose(hFind);

    return retFileCount;
}

// Directory Stream

Public b8 Dir_Stream_Open(String inDirPath, Dir_Stream* outDirStream) {
    Local_Str(pathWithWildCard, 256);
    CatStr(&pathWithWildCard, inDirPath, S("\\*"));
    pathWithWildCard.E[pathWithWildCard.count] = '\0';

    outDirStream->handle = FindFirstFileA((char*)pathWithWildCard.E, &outDirStream->fileData);

    if ( outDirStream->handle == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return false;
    }

    Assert(CompCStrSize(outDirStream->fileData.cFileName, "." , Array_Count(".")) == 0);
    FindNextFileA(outDirStream->handle, &outDirStream->fileData);
    Assert(CompCStrSize(outDirStream->fileData.cFileName, "..", Array_Count("..")) == 0);

    return true;
}

Public b8 Dir_Stream_Read(Dir_Stream* inDirStream, String* outFileName) {
    if ( FindNextFileA(inDirStream->handle, &inDirStream->fileData) ) {
        *outFileName = CStr_To_Str((u8*)inDirStream->fileData.cFileName);
        return true;
    }

    return false;
}

Public void Dir_Stream_Close(Dir_Stream* inDirStream) {
    Win32_Check( FindClose(inDirStream->handle), > 0 );
}

Public b8 Dir_Get_All_Files_Write_Times(CString dirPath, File_Info* infos, u32 count) {
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFileA((char*)dirPath, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return false;
    }

    Assert(CompCStrSize(data.cFileName, ".", Array_Count(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, "..", Array_Count("..")) == 0);

    u32 filesCount = 0;

    for (u32 i = 0; i < count && FindNextFileA(hFind, &data); ++i) {
        File_Info* info = &infos[filesCount++];

        Mem_Copy_Forward(info->name, data.cFileName, MAX_FILE_NAME);
        info->writeTime = OS_FILETIME_To_Time(data.ftLastWriteTime);
    }

    FindClose(hFind);

    return true;
}

// NOTE(JENH): inDirPath Shuould no be deleted.
Public void Dir_Monitor_Init(String inDirPath, Dir_Monitor* outMonitor) {
    outMonitor->dir = Dir_Open(inDirPath.E, true);
}

Public b8 Dir_Monitor_Wait_Till_Change(Dir_Monitor* inMonitor, HANDLE reloadSemaphore, Dir_Change* outDirChange) {
    byte buffer[KiB(1)];
    DWORD bytesReturned;

    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEventA(0, false, false, 0);

    Win32_Check( ReadDirectoryChangesW(inMonitor->dir, &buffer, Array_Count(buffer), true,
                                       FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_DIR_NAME,
                                       &bytesReturned, &overlapped, 0), > 0 );

    HANDLE handles[] = { reloadSemaphore, overlapped.hEvent };
    DWORD handleIndex = WaitForMultipleObjects(Array_Count(handles), handles, false, INFINITE);

    if ( handleIndex == 0 ) { return false; }

    //Win32_Check( WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0 );

    FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;

    outDirChange->fileName.count = notifyInfo->FileNameLength / 2;
    outDirChange->fileName.E = (u8*)inMonitor->buffer;
    Str_16_To_8((u16*)notifyInfo->FileName, outDirChange->fileName.count, outDirChange->fileName.E);

    switch (notifyInfo->Action) {
        case FILE_ACTION_ADDED: {
            outDirChange->type = DCT_Crate;
        } break;

        case FILE_ACTION_REMOVED: {
            outDirChange->type = DCT_Delete;
        } break;

        case FILE_ACTION_MODIFIED: {
            outDirChange->type = DCT_Write;
        } break;

        case FILE_ACTION_RENAMED_OLD_NAME: {
            outDirChange->type = DCT_Rename;

            FILE_NOTIFY_INFORMATION* notifyInfoNewName = (FILE_NOTIFY_INFORMATION*)(buffer + notifyInfo->NextEntryOffset);
            Assert( notifyInfoNewName->Action == FILE_ACTION_RENAMED_NEW_NAME );

            outDirChange->newFileName.count = notifyInfoNewName->FileNameLength / 2;
            outDirChange->newFileName.E = (u8*)(inMonitor->buffer + outDirChange->fileName.count);
            Str_16_To_8((c16*)notifyInfoNewName->FileName, outDirChange->newFileName.count, outDirChange->newFileName.E);
        } break;

        case FILE_ACTION_RENAMED_NEW_NAME: {
            outDirChange->type = DCT_Rename;
        } break;
    }

    return true;
}

Public void Dir_Monitor_End(Dir_Monitor* inMonitor) {
    File_Close(inMonitor->dir);

#ifdef JENH_SAFE
    inMonitor->dir = INVALID_HANDLE_VALUE;
#endif
}

Public b8 Dir_Find_File(String dirPath, String fileName) {
    b8 ret;

    Dir_Stream dirStream;
    Dir_Stream_Open(dirPath, &dirStream);

    for (String dirFileName; Dir_Stream_Read(&dirStream, &dirFileName); ) {
        if ( Str_Equal(dirFileName, fileName) ) { defer(true);
        }
    }

    ret = false;

defer:
    Dir_Stream_Close(&dirStream);
    return ret;
}
