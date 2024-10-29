#ifndef PEP_PE_H
#define PEP_PE_H

Public b8 PE_Image_Parse(void* inMem, PE_Image *outPE) {
    outPE->baseAddrImage = (byte*)inMem;

    outPE->DOSHeader = (DOS_Header *)outPE->baseAddrImage;
    outPE->NTHeaders = (NT_Headers *)(outPE->baseAddrImage + outPE->DOSHeader->e_lfanew);

         if ( outPE->NTHeaders->OptionalHeaderMagic == PE_WIN_32  ) { outPE->format = OHF_32;  }
    else if ( outPE->NTHeaders->OptionalHeaderMagic == PE_WIN_64  ) { outPE->format = OHF_64;  }
    else if ( outPE->NTHeaders->OptionalHeaderMagic == PE_WIN_ROM ) { outPE->format = OHF_ROM; }
    else {
        LogError("Optional Header magic is an invalid value. The bin file may be corrupted.\n");
        return JENH_FALSE;
    }

    outPE->sectionHeaderCount = outPE->NTHeaders->FileHeader.NumberOfSections;
    outPE->sectionHeaders     = (Section_Header*)(&outPE->NTHeaders->firstByteOptionalHeader +
                                                  outPE->NTHeaders->FileHeader.SizeOfOptionalHeader);

    u32 symbolTableOffset = (u32)outPE->NTHeaders->FileHeader.PointerToSymbolTable;
    byte *symbolTable = outPE->baseAddrImage + symbolTableOffset;

    u32 numberOfSymbols = (u32)outPE->NTHeaders->FileHeader.NumberOfSymbols;
    outPE->strTable = (char *)(symbolTable + (numberOfSymbols * SYMBOL_SIZE));

    return JENH_TRUE;
}

Public CString PE_Image_Get_Section_Name(Section_Header *inSectionHeader, char *inStrTable) {
    char *scan = inSectionHeader->Name;

    if (scan[0] == '/') {
        ++scan;

        String offsetStr = Str(scan, SHORT_NAME_SIZE - 1);
        offsetStr.size = Min(CStrLen(offsetStr.str), offsetStr.size);

        u32 offset = (u32)Str_To_S32(offsetStr);
        CString name = inStrTable + offset;

        return name;
    } else {
        return inSectionHeader->Name;
    }
}

Public Section_Header* PE_Get_Section_Header(PE_Image* inPE, CString inName) {
    Foreach (Section_Header, sectionHeader, inPE->sectionHeaders, inPE->sectionHeaderCount ) {
        if ( CStr_Equal(inName, PE_Image_Get_Section_Name(sectionHeader, inPE->strTable)) ) {
            return sectionHeader;
        }
    }

    LogError("Failed to find a section header with the name: %s", inName);
    return 0;
}

Public void* PE_Image_Get_Section_Data(PE_Image* inPE, CString inName, u32* outSize) {
    void* retAddress;

    Section_Header* sectionHeader;
    Check( (sectionHeader = PE_Get_Section_Header(inPE, inName)), != 0 );

    LogInfo("data address: %p", inPE->baseAddrImage + sectionHeader->VirtualAddress);
    LogInfo("data size: 0x%08x", sectionHeader->Misc.VirtualSize);

    *outSize = sectionHeader->Misc.VirtualSize;
    retAddress = inPE->baseAddrImage + sectionHeader->VirtualAddress;

    return retAddress;
}

// TODO(JENH): The magic number is to not copy the "module_local_atexit_table" global that it's present in the ucrt.
//             This is very hacky and probably buggy way to solve this issue. Find a better solution.
Public Global const u32 exitTableThing = 0x05d8;

Public void* PE_Get_Global_Data_Section(PE_Image* inPE, u32* outSize) {
    void* retAddress;

    Section_Header* sectionHeader;
    Check( (sectionHeader = PE_Get_Section_Header(inPE, ".data")), != 0 );

    *outSize = sectionHeader->Misc.VirtualSize - (sectionHeader->SizeOfRawData + exitTableThing);
    retAddress = (inPE->baseAddrImage + (sectionHeader->VirtualAddress + sectionHeader->SizeOfRawData));

    return retAddress;
}

Public void PE_Image_Print_DOS_Header(DOS_Header* inDOSHeader) {
    //DOS HEADER
    printf("__DOS HEADER__\n");
    printf("    e_magic    = %04x\n", inDOSHeader->e_magic);
    printf("    e_cblp     = %04x\n", inDOSHeader->e_cblp);
    printf("    e_cp       = %04x\n", inDOSHeader->e_cp);
    printf("    e_crlc     = %04x\n", inDOSHeader->e_crlc);
    printf("    e_cparhdr  = %04x\n", inDOSHeader->e_cparhdr);
    printf("    e_minalloc = %04x\n", inDOSHeader->e_minalloc);
    printf("    e_maxalloc = %04x\n", inDOSHeader->e_maxalloc);
    printf("    e_ss       = %04x\n", inDOSHeader->e_ss);
    printf("    e_sp       = %04x\n", inDOSHeader->e_sp);
    printf("    e_csum     = %04x\n", inDOSHeader->e_csum);
    printf("    e_ip       = %04x\n", inDOSHeader->e_ip);
    printf("    e_cs       = %04x\n", inDOSHeader->e_cs);
    printf("    e_lfarlc   = %04x\n", inDOSHeader->e_lfarlc);
    printf("    e_ovno     = %04x\n", inDOSHeader->e_ovno);
    //printf("%04x\n",PE->inDOSHeader->e_res);
    printf("    e_oemid    = %04x\n", inDOSHeader->e_oemid);
    printf("    e_oeminfo  = %04x\n", inDOSHeader->e_oeminfo);
    //printf("%04x\n",PE->inDOSHeader->e_res2);
    printf("    e_lfanew   = %08x\n\n", (u32)inDOSHeader->e_lfanew);
}

Public void PE_Image_Print_File_Header(File_Header* inFileHeader) {
    //printf("    Signature            = %08x\n",   (u32)PE->NTHeaders->Signature);
    printf("__FILE HEADER__\n");
    printf("    Machine              = %04x\n", (u16)inFileHeader->Machine);
    printf("    NumberOfSections     = %04x\n", (u16)inFileHeader->NumberOfSections);
    printf("    TimeDateStamp        = %08x\n", (u32)inFileHeader->TimeDateStamp);
    printf("    PointerToSymbolTable = %08x\n", (u32)inFileHeader->PointerToSymbolTable);
    printf("    NumberOfSymbols      = %08x\n", (u32)inFileHeader->NumberOfSymbols);
    printf("    SizeOfOptionalHeader = %04x\n", (u16)inFileHeader->SizeOfOptionalHeader);
    printf("    Characteristics      = %04x\n", (u16)inFileHeader->Characteristics);
    printf("\n");
}

Public void PE_Image_Print_Optional_Header(void* inOptionalHeader, Optional_Header_Format inFormat) {
    Optional_Header32* optionalHeader32 = (Optional_Header32*)inOptionalHeader;

    printf("__OPTIONAL HEADER__\n");
    printf("    Magic                       = %04x\n", optionalHeader32->Magic);
    printf("    MajorLinkerVersion          = %02x\n", optionalHeader32->MajorLinkerVersion);
    printf("    MinorLinkerVersion          = %02x\n", optionalHeader32->MinorLinkerVersion);
    printf("    SizeOfCode                  = %08x\n", optionalHeader32->SizeOfCode);
    printf("    SizeOfInitializedData       = %08x\n", optionalHeader32->SizeOfInitializedData);
    printf("    SizeOfUninitializedData     = %08x\n", optionalHeader32->SizeOfUninitializedData);
    printf("    AddressOfEntryPoint         = %08x\n", optionalHeader32->AddressOfEntryPoint);
    printf("    BaseOfCode                  = %08x\n", optionalHeader32->BaseOfCode);

    if ( inFormat == OHF_32 ) {
        printf("    BaseOfData                  = %08x\n", optionalHeader32->BaseOfData);
        printf("    ImageBase                   = %08x\n", optionalHeader32->ImageBase);
        printf("    SectionAlignment            = %08x\n", optionalHeader32->SectionAlignment);
        printf("    FileAlignment               = %08x\n", optionalHeader32->FileAlignment);
        printf("    MajorOperatingSystemVersion = %04x\n", optionalHeader32->MajorOperatingSystemVersion);
        printf("    MinorOperatingSystemVersion = %04x\n", optionalHeader32->MinorOperatingSystemVersion);
        printf("    MajorImageVersion           = %04x\n", optionalHeader32->MajorImageVersion);
        printf("    MinorImageVersion           = %04x\n", optionalHeader32->MinorImageVersion);
        printf("    MajorSubsystemVersion       = %04x\n", optionalHeader32->MajorSubsystemVersion);
        printf("    MinorSubsystemVersion       = %04x\n", optionalHeader32->MinorSubsystemVersion);
        printf("    Win32VersionValue           = %08x\n", optionalHeader32->Win32VersionValue);
        printf("    SizeOfImage                 = %08x\n", optionalHeader32->SizeOfImage);
        printf("    SizeOfHeaders               = %08x\n", optionalHeader32->SizeOfHeaders);
        printf("    CheckSum                    = %08x\n", optionalHeader32->CheckSum);
        printf("    Subsystem                   = %04x\n", optionalHeader32->Subsystem);
        printf("    DllCharacteristics          = %04x\n", optionalHeader32->DllCharacteristics);
        printf("    SizeOfStackReserve          = %08x\n", optionalHeader32->SizeOfStackReserve);
        printf("    SizeOfStackCommit           = %08x\n", optionalHeader32->SizeOfStackCommit);
        printf("    SizeOfHeapReserve           = %08x\n", optionalHeader32->SizeOfHeapReserve);
        printf("    SizeOfHeapCommit            = %08x\n", optionalHeader32->SizeOfHeapCommit);
        printf("    LoaderFlags                 = %08x\n", optionalHeader32->LoaderFlags);
        printf("    NumberOfRvaAndSizes         = %08x\n", optionalHeader32->NumberOfRvaAndSizes);

        // DATA DIRECTORIES
        printf("\n__DATA_DIRECTORIES__\n");

        for (u32 i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; ++i) {
            Data_Directory* dataDir = &optionalHeader32->DataDirectory[i];
            printf("    %d. VirtualAddress = %08x | Size = %08x\n", i + 1, (u32)dataDir->VirtualAddress, (u32)dataDir->Size);
        }
    } else if ( inFormat == OHF_64 ) {
        Optional_Header64* optionalHeader64 = (Optional_Header64*)inOptionalHeader;

        printf("    ImageBase                   = %016llx\n", optionalHeader64->ImageBase);
        printf("    SectionAlignment            = %08x\n",    optionalHeader64->SectionAlignment);
        printf("    FileAlignment               = %08x\n",    optionalHeader64->FileAlignment);
        printf("    MajorOperatingSystemVersion = %04x\n",    optionalHeader64->MajorOperatingSystemVersion);
        printf("    MinorOperatingSystemVersion = %04x\n",    optionalHeader64->MinorOperatingSystemVersion);
        printf("    MajorImageVersion           = %04x\n",    optionalHeader64->MajorImageVersion);
        printf("    MinorImageVersion           = %04x\n",    optionalHeader64->MinorImageVersion);
        printf("    MajorSubsystemVersion       = %04x\n",    optionalHeader64->MajorSubsystemVersion);
        printf("    MinorSubsystemVersion       = %04x\n",    optionalHeader64->MinorSubsystemVersion);
        printf("    Win32VersionValue           = %08x\n",    optionalHeader64->Win32VersionValue);
        printf("    SizeOfImage                 = %08x\n",    optionalHeader64->SizeOfImage);
        printf("    SizeOfHeaders               = %08x\n",    optionalHeader64->SizeOfHeaders);
        printf("    CheckSum                    = %08x\n",    optionalHeader64->CheckSum);
        printf("    Subsystem                   = %04x\n",    optionalHeader64->Subsystem);
        printf("    DllCharacteristics          = %04x\n",    optionalHeader64->DllCharacteristics);
        printf("    SizeOfStackReserve          = %016llx\n", optionalHeader64->SizeOfStackReserve);
        printf("    SizeOfStackCommit           = %016llx\n", optionalHeader64->SizeOfStackCommit);
        printf("    SizeOfHeapReserve           = %016llx\n", optionalHeader64->SizeOfHeapReserve);
        printf("    SizeOfHeapCommit            = %016llx\n", optionalHeader64->SizeOfHeapCommit);
        printf("    LoaderFlags                 = %08x\n",    optionalHeader64->LoaderFlags);
        printf("    NumberOfRvaAndSizes         = %08x\n",    optionalHeader64->NumberOfRvaAndSizes);

        // DATA DIRECTORIES
        printf("\n__DATA_DIRECTORIES__\n");

        for (u32 i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; ++i) {
            Data_Directory* dataDir = &optionalHeader64->DataDirectory[i];
            printf("    %d. VirtualAddress = %08x | Size = %08x\n", i + 1, (u32)dataDir->VirtualAddress, (u32)dataDir->Size);
        }
    } NO_ELSE
}

Public void PE_Image_Print_Section_Headers(u32 inSectionHeaderCount, Section_Header* inSectionHeaders, char* inStrTable) {
    // Sections
    printf("\n__SECTIONS__\n\n");

    for (u32 i = 0; i < inSectionHeaderCount; ++i) {
        Section_Header* sectionHeader = &inSectionHeaders[i];

        printf("Name: \"%s\"\n", PE_Image_Get_Section_Name(sectionHeader, inStrTable));
        printf("    VirtualSize          = %08x\n", (u32)sectionHeader->Misc.VirtualSize);
        printf("    VirtualAddress       = %08x\n", (u32)sectionHeader->VirtualAddress);
        printf("    SizeOfRawData        = %08x\n", (u32)sectionHeader->SizeOfRawData);
        printf("    PointerToRawData     = %08x\n", (u32)sectionHeader->PointerToRawData);
        printf("    PointerToRelocations = %08x\n", (u32)sectionHeader->PointerToRelocations);
        printf("    PointerToLinenumbers = %08x\n", (u32)sectionHeader->PointerToLinenumbers);
        printf("    NumberOfRelocations  = %04x\n", (u32)sectionHeader->NumberOfRelocations);
        printf("    NumberOfLinenumbers  = %04x\n", (u32)sectionHeader->NumberOfLinenumbers);
        printf("    Characteristics      = %08x\n", (u32)sectionHeader->Characteristics);
        printf("\n");
    }
    printf("\n");
}

Public void PE_Image_Print(PE_Image* inPE) {
    printf("DLL address: %p\n", inPE->baseAddrImage);
    PE_Image_Print_DOS_Header(inPE->DOSHeader);
    PE_Image_Print_File_Header(&inPE->NTHeaders->FileHeader);
    PE_Image_Print_Optional_Header(&inPE->NTHeaders->firstByteOptionalHeader, inPE->format);
    PE_Image_Print_Section_Headers(inPE->sectionHeaderCount, inPE->sectionHeaders, inPE->strTable);
}

#endif //PEP_PE_H
