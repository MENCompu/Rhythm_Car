#ifndef RC_CONVERTER_MONITOR_H
#define RC_CONVERTER_MONITOR_H

typedef struct {
    //u64 parentAssetID;
    u64 writeTime;
    File_String srcFileRelPath;
    u64 assetID;
} Convert_Table_Entry;

#pragma pack(push, 1)
typedef struct {
    u32 entryCount;
    Convert_Table_Entry* table;
    u32 reserver1;

    u32 stringTableSize;
    char* stringTable;
    u32 reserver2;
} Convert_Table_Header_32;

typedef struct {
    u32 entryCount;
    Convert_Table_Entry* table;

    u32 stringTableSize;
    char* stringTable;
} Convert_Table_Header_64;
#pragma pack(pop)

typedef Convert_Table_Header_64 Convert_Table_Header;

Public Convert_Table_Entry* Converter_Find_Entry_By_File_Name(String inFileName);
Public Asset_Handle Converter_Get_Asset_Handle(String inFileName);

#endif // RC_CONVERTER_MONITOR_H
