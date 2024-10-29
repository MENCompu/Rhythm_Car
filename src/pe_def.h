#ifndef DSV_PEDEF_H
#define DSV_PEDEF_H

#define SYMBOL_SIZE 18

typedef struct JENH_GUID {
    u64 thing1;
    u64 thing2;
} JENH_GUID;

typedef struct {
    u16 e_magic;    // Magic number
    u16 e_cblp;     // Bytes on last page of file
    u16 e_cp;       // Pages in file
    u16 e_crlc;     // Relocations
    u16 e_cparhdr;  // Size of header in paragraphs
    u16 e_minalloc; // Minimum extra paragraphs needed
    u16 e_maxalloc; // Maximum extra paragraphs needed
    u16 e_ss;       // Initial (relative) SS value
    u16 e_sp;       // Initial SP value
    u16 e_csum;     // Checksum
    u16 e_ip;       // Initial IP value
    u16 e_cs;       // Initial (relative) CS value
    u16 e_lfarlc;   // File address of relocation table
    u16 e_ovno;     // Overlay number
    u16 e_res[4];   // Reserved words
    u16 e_oemid;    // OEM identifier (for e_oeminfo)
    u16 e_oeminfo;  // OEM information; e_oemid specific
    u16 e_res2[10]; // Reserved words
    u32 e_lfanew;   // File address of new exe header
} DOS_Header;

typedef struct {
    u16 Machine;
    u16 NumberOfSections;
    u32 TimeDateStamp;
    u32 PointerToSymbolTable;
    u32 NumberOfSymbols;
    u16 SizeOfOptionalHeader;
    u16 Characteristics;
} File_Header;

#define DIRECTORY_ENTRIES_COUNT 16

typedef struct {
    u32 VirtualAddress;
    u32 Size;
} Data_Directory;

#define PE_WIN_32  0x10B
#define PE_WIN_64  0x20B
#define PE_WIN_ROM 0x107

typedef enum {
    OHF_32,
    OHF_64,
    OHF_ROM,
} Optional_Header_Format;

typedef struct {
    // Standard fields.
    u16  Magic;
    byte MajorLinkerVersion;
    byte MinorLinkerVersion;
    u32  SizeOfCode;
    u32  SizeOfInitializedData;
    u32  SizeOfUninitializedData;
    u32  AddressOfEntryPoint;
    u32  BaseOfCode;

    // NT additional fields.
    u32  BaseOfData;
    u32  ImageBase;
    u32  SectionAlignment;
    u32  FileAlignment;
    u16  MajorOperatingSystemVersion;
    u16  MinorOperatingSystemVersion;
    u16  MajorImageVersion;
    u16  MinorImageVersion;
    u16  MajorSubsystemVersion;
    u16  MinorSubsystemVersion;
    u32  Win32VersionValue;
    u32  SizeOfImage;
    u32  SizeOfHeaders;
    u32  CheckSum;
    u16  Subsystem;
    u16  DllCharacteristics;
    u32  SizeOfStackReserve;
    u32  SizeOfStackCommit;
    u32  SizeOfHeapReserve;
    u32  SizeOfHeapCommit;
    u32  LoaderFlags;
    u32  NumberOfRvaAndSizes;
    Data_Directory DataDirectory[DIRECTORY_ENTRIES_COUNT];
} Optional_Header32;

typedef enum {
    DD_Export         = 0,
    DD_Import         = 1,
    DD_Resource       = 2,
    DD_Exception      = 3,
    DD_Security       = 4,
    DD_Basereloc      = 5,
    DD_Debug          = 6,
    DD_Copyright      = 7,
    DD_Architecture   = 7,
    DD_Globalptr      = 8,
    DD_Tls            = 9,
    DD_Load_config    = 10,
    DD_Bound_import   = 11,
    DD_Iat            = 12,
    DD_Delay_import   = 13,
    DD_Com_descriptor = 14,
} Data_Directory_ID;

typedef struct Debug_Directory {
  u32 Characteristics;
  u32 TimeDateStamp;
  u16 MajorVersion;
  u16 MinorVersion;
  u32 Type;
  u32 SizeOfData;
  u32 AddressOfRawData;
  u32 PointerToRawData;
} Debug_Directory;

typedef struct {
    // Standard fields.
    u16  Magic;
    byte MajorLinkerVersion;
    byte MinorLinkerVersion;
    u32  SizeOfCode;
    u32  SizeOfInitializedData;
    u32  SizeOfUninitializedData;
    u32  AddressOfEntryPoint;
    u32  BaseOfCode;

    // NT additional fields.
    u64  ImageBase;
    u32  SectionAlignment;
    u32  FileAlignment;
    u16  MajorOperatingSystemVersion;
    u16  MinorOperatingSystemVersion;
    u16  MajorImageVersion;
    u16  MinorImageVersion;
    u16  MajorSubsystemVersion;
    u16  MinorSubsystemVersion;
    u32  Win32VersionValue;
    u32  SizeOfImage;
    u32  SizeOfHeaders;
    u32  CheckSum;
    u16  Subsystem;
    u16  DllCharacteristics;
    u64  SizeOfStackReserve;
    u64  SizeOfStackCommit;
    u64  SizeOfHeapReserve;
    u64  SizeOfHeapCommit;
    u32  LoaderFlags;
    u32  NumberOfRvaAndSizes;
    Data_Directory DataDirectory[DIRECTORY_ENTRIES_COUNT];
} Optional_Header64;

typedef struct {
    u16 Signature;
    File_Header FileHeader;
    union {
        byte firstByteOptionalHeader;
        u16 OptionalHeaderMagic; // PE_WIN_FORMAT
        Optional_Header32 OptionalHeader32;
        Optional_Header64 OptionalHeader64;
    };
} NT_Headers;

#define SHORT_NAME_SIZE 8

typedef struct {
    char Name[SHORT_NAME_SIZE];
    union {
        u32 PhysicalAddress;
        u32 VirtualSize;
    } Misc;
    u32 VirtualAddress;
    u32 SizeOfRawData;
    u32 PointerToRawData;
    u32 PointerToRelocations;
    u32 PointerToLinenumbers;
    u16 NumberOfRelocations;
    u16 NumberOfLinenumbers;
    u32 Characteristics;
} Section_Header;

typedef struct {
    byte* baseAddrImage;
    Optional_Header_Format format;
    DOS_Header* DOSHeader;
    NT_Headers* NTHeaders;

    u32 sectionHeaderCount;
    Section_Header* sectionHeaders;

    char* strTable;
} PE_Image;

#define PE_CODEVIEW_PDB20_MAGIC 0x3031424e
#define PDB_CV70_MAGIC Magic_4("RSDS") //0x53445352

#pragma pack(push, 1)
typedef struct CodeView_20_Header {
  u32 magic;
  u32 offset;
  u32 time;
  u32 age;
  char fileName[0];
} CodeView_20_Header;

typedef struct CodeView_70_Header {
  u32 magic;
  JENH_GUID guid;
  u32 age;
  char fileName[0];
} CodeView_70_Header;
#pragma pack(pop)

#define PE_RELOC_OFFSET_MASK 0x0fff
#define PE_RELOC_TYPE_MASK   0xf000

typedef struct PE_Base_Reloc_Block {
    u32 address;
    u32 size;
} PE_Base_Reloc_Block;

#endif //DSV_PEDEF_H
