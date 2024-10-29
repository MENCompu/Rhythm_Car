// Files

#ifdef _WIN32
#include <windows.h>

Public b8 File_Exists(CString path) {
    BOOL res = PathFileExistsA(path);

    return (b8)res;
}

Public File_Handle File_Create(CString inPath) {
    // OPEN_ALWAYS
    File_Handle retHandle = (File_Handle)CreateFileA(inPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
    return retHandle;
}

Public File_Handle File_Open(CString inPath) {
    // OPEN_ALWAYS
    File_Handle retHandle = (File_Handle)CreateFileA(inPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    return retHandle;
}

Public void File_Close(File_Handle inHandle) {
    CloseHandle((HANDLE)inHandle);
}

Public b8 File_Delete(CString inPath) {
    if ( !DeleteFile(inPath) ) {
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

Private u64 OS_Covert_FILETIME_To_U64(FILETIME inFileTime) {
    u64 retFileTime;

    ULARGE_INTEGER ularge;
    ularge.LowPart  = inFileTime.dwLowDateTime;
    ularge.HighPart = inFileTime.dwHighDateTime;

    retFileTime = (u64)ularge.QuadPart;

    return retFileTime;
}

Public u64 File_Get_Write_Time(CString inPath) {
    FILETIME fileTimeOS = {0};
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    if (!GetFileAttributesExA(inPath, GetFileExInfoStandard, &fileData)) {
        return 0;
    }

	fileTimeOS = fileData.ftLastWriteTime;

    return OS_Covert_FILETIME_To_U64(fileTimeOS);
}

// Directories

Public b8 Dir_Exists(CString path) {
    BOOL res = PathFileExistsA(path);

    return (b8)res;
}

Public b8 Dir_Create(CString path) {
    BOOL res = CreateDirectoryA(path, 0);

    return (b8)res;
}

Public File_Handle Dir_Open(CString path, b8 async) {
    s32 flags = ((async) ? FILE_FLAG_OVERLAPPED : 0);
    File_Handle retHandle = (File_Handle)CreateFileA(path, FILE_LIST_DIRECTORY|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                     0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED , 0);
    return retHandle;
}

Public void Dir_Recursive_Delete(String inDirPath) {
    Local_Str(dirWildcard, MAX_FILE_PATH);
    CatStr(&dirWildcard, inDirPath, LitToStr("*"));
    dirWildcard.str[dirWildcard.size] = '\0';

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(dirWildcard.str, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return;
    }

    Assert(CompCStrSize(data.cFileName, S(".") , ArrayCount(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, S(".."), ArrayCount("..")) == 0);

    while ( FindNextFileA(hFind, &data) ) {
        String fileName = Str(data.cFileName, CStrLen(data.cFileName));

        Local_Str(filePath, MAX_FILE_PATH);
        CatStr(&filePath, inDirPath, fileName);
        filePath.str[filePath.size] = '\0';

        File_Delete(filePath.str);
    }

    FindClose(hFind);

    Win32_Check( RemoveDirectoryA(inDirPath.str), > 0 );
}

Public u32 Dir_Recursive_File_Count(CString dirPath) {
    u32 retFileCount = 0;

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(dirPath, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return MAX_U32;
    }

    Assert(CompCStrSize(data.cFileName, S("."), ArrayCount(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, S(".."), ArrayCount("..")) == 0);

    while ( FindNextFileA(hFind, &data) ) {
        ++retFileCount;
    }

    FindClose(hFind);

    return retFileCount;
}

// Directory Stream

Public b8 Dir_Stream_Open(String inDirPath, Dir_Stream* outDirStream) {
    Local_Str(pathWithWildCard, 256);
    CatStr(&pathWithWildCard, inDirPath, LitToStr("*"));
    pathWithWildCard.str[pathWithWildCard.size] = '\0';

    outDirStream->handle = FindFirstFileA(pathWithWildCard.str, &outDirStream->fileData);

    if ( outDirStream->handle == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return false;
    }

    Assert(CompCStrSize(outDirStream->fileData.cFileName, S(".") , ArrayCount(".")) == 0);
    FindNextFileA(outDirStream->handle, &outDirStream->fileData);
    Assert(CompCStrSize(outDirStream->fileData.cFileName, S(".."), ArrayCount("..")) == 0);

    return true;
}

Public b8 Dir_Stream_Read(Dir_Stream* inDirStream, String* outFileName) {
    if ( FindNextFileA(inDirStream->handle, &inDirStream->fileData) ) {
        *outFileName = Str(inDirStream->fileData.cFileName, CStrLen(inDirStream->fileData.cFileName));
        return true;
    }

    return false;
}

Public void Dir_Stream_Close(Dir_Stream* inDirStream) {
    Win32_Check( FindClose(inDirStream->handle), > 0 );
}

Public b8 Dir_Get_All_Files_Write_Times(CString dirPath, File_Info* infos, u32 count) {
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFileA(dirPath, &data);

    if ( hFind == INVALID_HANDLE_VALUE ) {
        Win32_LogError("Failed to create a directory search handle");
        return false;
    }

    Assert(CompCStrSize(data.cFileName, S("."), ArrayCount(".")) == 0);
    FindNextFileA(hFind, &data);
    Assert(CompCStrSize(data.cFileName, S(".."), ArrayCount("..")) == 0);

    u32 filesCount = 0;

    for (u32 i = 0; i < count && FindNextFileA(hFind, &data); ++i) {
        File_Info* info = &infos[filesCount++];

        Mem_Copy_Forward(info->name, data.cFileName, MAX_FILE_NAME);
        info->writeTime = OS_Covert_FILETIME_To_U64(data.ftLastWriteTime);
    }

    FindClose(hFind);

    return true;
}

// NOTE(JENH): inDirPath Shuould no be deleted.
Public void Dir_Monitor_Init(String inDirPath, Dir_Monitor* outMonitor) {
    outMonitor->dir = Dir_Open(inDirPath.str, true);
}

Public b8 Dir_Monitor_Wait_Till_Change(Dir_Monitor* inMonitor, HANDLE reloadSemaphore, Dir_Change* outDirChange) {
    byte buffer[KiB(1)];
    DWORD bytesReturned;

    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEventA(0, false, false, 0);

    Win32_Check( ReadDirectoryChangesW(inMonitor->dir, &buffer, ArrayCount(buffer), true,
                                       FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_DIR_NAME,
                                       &bytesReturned, &overlapped, 0), > 0 );

    HANDLE handles[] = { reloadSemaphore, overlapped.hEvent };
    DWORD handleIndex = WaitForMultipleObjects(ArrayCount(handles), handles, false, INFINITE);

    if ( handleIndex == 0 ) { return false; }

    //Win32_Check( WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0 );

    FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;

    outDirChange->fileName.size = notifyInfo->FileNameLength / 2;
    outDirChange->fileName.str = inMonitor->buffer;
    Str_16_To_8((c16*)notifyInfo->FileName, outDirChange->fileName.size, outDirChange->fileName.str);

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

            outDirChange->newFileName.size = notifyInfoNewName->FileNameLength / 2;
            outDirChange->newFileName.str = (inMonitor->buffer + outDirChange->fileName.size);
            Str_16_To_8((c16*)notifyInfoNewName->FileName, outDirChange->newFileName.size, outDirChange->newFileName.str);
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

#endif // _WIN32
