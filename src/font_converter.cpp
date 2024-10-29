#if 0
typedef struct RCAF_Code_Point_Data {
    char codePoint;
    u16 xPos;
    u16 yPos;
    u16 width;
    u16 height;
    s16 xOffset;
    s16 yOffset;
    u16 xAdvance;
} RCAF_Code_Point_Data;

typedef struct {
    Asset_Handle bitmap;
    u16 bitmapWidth;
    u16 bitmapHeight;

    u16 lineHeight;

    u32 codePointCount;
    RCAF_Code_Point_Data table[];
} RCAF_Font;
#endif

Public b8 Font_Converter_Fnt_To_RCF(void* inFileMem, u32 inFileSize, File_Handle inDstFile, Memory_Arena* inArena) {
    u32 lineNumber = 0;

    RCAF_Code_Point_Data* codePointTable = 0;
    u32 codePointCount = 0;
    RCAF_Code_Point_Data* codePointData = 0;

    RCAF_Font fontHeader;

    for (String_Scan fileScan = StrScan((char*)inFileMem, (char*)inFileMem + inFileSize); !StrScanEOS(fileScan);) {
        StrScanAdvance(&fileScan, FindAnyCharForward(fileScan.scan, fileScan.end, LitToStr("\r\n")));
        String_Scan line = StrToStrScan(fileScan.data);

        if (fileScan.scan[0] == '\n') { ++fileScan.scan; }

        StrScanAdvance(&line, FindCharForward(line.scan, line.end, ' '));
        ++lineNumber;

        if (StrScanEOS(line)) { continue; }
        //if ( lineNumber <= 3 ) { continue; }

        if ( Str_Equal(line.data, LitToStr("common")) ) {
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            fontHeader.lineHeight = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            fontHeader.bitmapWidth = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            fontHeader.bitmapHeight = (u16)Str_To_S32(line.data);
        } else if ( Str_Equal(line.data, LitToStr("page")) ) {
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '\"'));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '\"'));

            fontHeader.bitmap = Converter_Get_Asset_Handle(line.data);
        } else if ( Str_Equal(line.data, LitToStr("chars")) ) {
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, FindAnyCharForward(line.scan, line.end, LitToStr("\r\n")));

            codePointTable = ArenaPushArray(inArena, RCAF_Code_Point_Data, (u32)Str_To_S32(line.data));
        } else if ( Str_Equal(line.data, LitToStr("char")) ) {
            codePointData = &codePointTable[codePointCount++];

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->codePoint = (char)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->xPos = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->yPos = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->width = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->height = (u16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->xOffset = (s16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->yOffset = (s16)Str_To_S32(line.data);

            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, '='));
            StrScanAdvance(&line, Str_Find_Char_Forward(line.scan, line.end, ' '));
            codePointData->xAdvance = (u16)Str_To_S32(line.data);
        }
    }

    fontHeader.codePointCount = codePointCount;

    File_Write_At(inDstFile, 0, sizeof(RCAF_Font), &fontHeader);
    File_Write_At(inDstFile, FieldOffset(RCAF_Font, table), sizeof(RCAF_Code_Point_Data) * fontHeader.codePointCount, codePointTable);

    return JENH_TRUE;
}
