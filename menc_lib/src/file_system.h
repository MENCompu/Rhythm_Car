#ifndef RC_FILE_SYSTEM_H
#define RC_FILE_SYSTEM_H

// TODO(JENH): File maping io (CreateFileMappingA, MapViewOfFile).

typedef HANDLE File_Handle;

#define MAX_FILE_NAME 256
#define MAX_FILE_PATH KiB(1)

typedef struct {
    u32 size;
    byte* data;
} File;

typedef struct {
    char name[MAX_FILE_NAME];
    Time writeTime;
} File_Info;

#define INVALID_FILE_HANDLE 0xffffffff

Public b8 File_Exists(CString dirPath);
Public File_Handle File_Create(CString path);
Public File_Handle File_Open(CString path);
Public b8 File_Handle_Is_Valid(File_Handle inHandle);
Public void File_Close(File_Handle handle);
Public b8 File_Delete(CString path);
Public u32 File_Get_Size(File_Handle retHandle);
Public u64 File_Get_Write_Time(CString inPath);
Public b8 File_Read(File_Handle handle, u32 bytesToRead, void* mem);
Public b8 File_Read_At(File_Handle handle, WordSize offset, u32 bytesToRead, void* mem);
Public void* File_Read_All(String path, Memory_Arena* arena, u32* size);
Public b8 File_Write(File_Handle handle, u32 bytesToWrite, void* mem);
Public b8 File_Write_At(File_Handle handle, WordSize offset, u32 bytesToWrite, void* mem);
Public void File_Clear(File_Handle handle);
Public void File_Size_Set(File_Handle handle, u32 size);
Public void File_Create_With_Contents(String path, void* data, u32 size);
Public b8 File_Replace_Contents(String path, void* data, u32 size);

// TODO(JENH): The funcionality of append with overlapped i/o seems to have an aling of 2 bytes. Better don't use it.
// Public b8 File_Append(File_Handle handle, u32 bytesToAppend, void* mem);

// Directories.

Public b8 Dir_Exists(CString dirPath);
Public b8 Dir_Create(CString dirPath);
Public void Dir_Recursive_Delete(String inDirPath);
Public b8  Dir_Get_All_Files_Write_Times(CString dirPath, File_Info* infos, u32 DEBUGCount);

typedef enum {
    FILE_TAG_NIL  = 0,
    FILE_TAG_FILE = 1,
    FILE_TAG_DIRECTORY = 2,
} File_Tag;

Public File_Tag File_Get_Tag(CString path);

#if 0
#define Fn_Sig_Dir_Callback(fnName) void fnName(String filePath, void* userData)
typedef Fn_Sig_Dir_Callback(Fn_Dir_Callback);

Public void Dir_Recursive_Iter(String inDirPath, Fn_Dir_Callback* callBack, void* userMem);
#endif

typedef struct {
    HANDLE handle;
    WIN32_FIND_DATA fileData;
} Dir_Stream;

Public b8 Dir_Stream_Open(String inDirPath, Dir_Stream* outDirStream);
Public b8 Dir_Stream_Read(Dir_Stream* inDirStream, String* outFileName);
Public void Dir_Stream_Close(Dir_Stream* inDirStream);

typedef enum {
    DCT_Rename,
    DCT_Crate,
    DCT_Delete,
    DCT_Write,
} Dir_Change_Type;

typedef struct {
    Dir_Change_Type type;

    union {
        String fileName;
        String oldFileName;
    };

    String newFileName;
} Dir_Change;

typedef struct {
    File_Handle dir;
    char buffer[1024];
} Dir_Monitor;

// TODO(JENH): For linux use the "inotify API" and for mac use "fsevents API".
Public void Dir_Monitor_Init(String inDirPath, Dir_Monitor* outMonitor);
Public b8 Dir_Monitor_Wait_Till_Change(Dir_Monitor* inMonitor, HANDLE reloadSemaphore, Dir_Change* outDirChange);
Public void Dir_Monitor_End(Dir_Monitor* inMonitor);

#endif // RC_FILE_SYSTEM_H
