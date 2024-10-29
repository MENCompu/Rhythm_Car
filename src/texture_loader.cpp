#define ALPHA_MASK 0xff000000
#define ALPHA_255  0xff000000

Public void Image_BGR_To_RGBA(void* inBGR, u32 inTexelCount, void* outRGBA) {
    u32* srcScan = (u32*)inBGR;
    u32* dstScan = (u32*)outRGBA;

    for (u32 i = 0; i < inTexelCount; ++i) {
        u8 fileB = (u8)((*srcScan & 0x000000ff) >>  0);
        u8 fileG = (u8)((*srcScan & 0x0000ff00) >>  8);
        u8 fileR = (u8)((*srcScan & 0x00ff0000) >> 16);

        dstScan[i] = (u32)(( fileR <<  0 ) |
                           ( fileG <<  8 ) |
                           ( fileB << 16 ) |
                           ( 0xff  << 24 ));

        srcScan = (u32*)((byte*)srcScan + 3);
    }
}

Public void Image_BGRA_To_RGBA(void* inBGRA, u32 inTexelCount, void* outRGBA) {
    u32* srcScan = (u32*)inBGRA;
    u32* dstScan = (u32*)outRGBA;

    for (u32 i = 0; i < inTexelCount; ++i) {
        u8 fileB = (u8)((*srcScan & 0x000000ff) >>  0);
        u8 fileG = (u8)((*srcScan & 0x0000ff00) >>  8);
        u8 fileR = (u8)((*srcScan & 0x00ff0000) >> 16);
        u8 fileA = (u8)((*srcScan & 0xff000000) >> 24);

        dstScan[i] = (u32)(( fileR <<  0 ) |
                           ( fileG <<  8 ) |
                           ( fileB << 16 ) |
                           ( fileA << 24 ));

        ++srcScan;
    }
}

Public void Image_Grey_Scale_To_RGBA(void* inGreyScale, u32 inTexelCount, void* outRGBA) {
    u8*  srcScan = (u8*)inGreyScale;
    u32* dstScan = (u32*)outRGBA;

    for (u32 i = 0; i < inTexelCount; ++i) {
        dstScan[i] = (u32)(( *srcScan <<  0 ) |
                           ( *srcScan <<  8 ) |
                           ( *srcScan << 16 ) |
                           ( *srcScan << 24 )); // ????

        ++srcScan;
    }
}

Public void Image_RGBA_Flip_Vertically(void* inTexels, u32 inWidth, u32 inHeight) {
    u32* top = (u32*)inTexels;
    u32* bottom = top + (inHeight * (inWidth - 1));

    for (u32 y = 0; y < (inHeight / 2); ++y) {
        for (u32 x = 0; x < inWidth; ++x) {
            Swap(top, bottom, u32);

            ++top;
            ++bottom;
        }

        //top += inWidth;
        bottom -= 2 * inWidth;
    }
}

Private b8 Image_Has_Transparency(void* inTexels, u32 inTexelCount) {
    u32* scan = (u32*)inTexels;

    for (u32 i = 0; i < inTexelCount; ++i) {
        if ( (*scan & ALPHA_MASK) != ALPHA_255 ) {
            return JENH_TRUE;
        }

        ++scan;
    }

    return JENH_FALSE;
}

#pragma pack(push, 1)
typedef struct {
    u16 magic;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 texelsOffset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
    u32 compression;
    u32 imageSize;
    s32 horResolution;
    s32 VerResolution;
    u32 colorCount;
    u32 importantColorCount;
} BMP_Header;

typedef struct {
    u32 r;
    u32 g;
    u32 b;
} BMP_RGB_Bit_Fields;

typedef struct {
    u32 r;
    u32 g;
    u32 b;
    u32 a;
} BMP_RGBA_Bit_Fields;
#pragma pack(pop)

typedef enum {
    BMP_RGB            =  0,
	BMP_RLE8           =  1,
	BMP_RLE4           =  2,
	BMP_BITFIELDS      =  3,
	BMP_JPEG           =  4,
	BMP_PNG            =  5,
	BMP_ALPHABITFIELDS =  6,
	BMP_CMYK           = 11,
	BMP_CMYKRLE8       = 12,
	BMP_CMYKRLE4       = 13
} BMP_Compressoin_Type;

#if 0
typedef struct {

} Resource_Memory;

Public void Image_Converter_Parse_BMP_Header() {
}
#endif

Public void* Image_Converter_BMP_To_RCAF(void* inFileMem, Memory_Arena* inArena, RCAF_Texture* outTex) {
    void* retRCAFMem = 0;

    void* fileScan = inFileMem;
    BMP_Header* header = Consume_Type(&fileScan, BMP_Header);

    Assert(header->magic == Magic_2("BM"));
    Assert(header->bitsPerPixel == 32);

    outTex->width  = (u16)header->width;
    outTex->height = (u16)header->height;
    outTex->channelCount = 4;

    u32 imageSize = (u32)(outTex->width * outTex->height * outTex->channelCount);
    void* fileTexels = (byte*)inFileMem + header->texelsOffset;

    if (header->compression == BMP_BITFIELDS) {
        BMP_RGB_Bit_Fields* bitFields = Consume_Type(&fileScan, BMP_RGB_Bit_Fields);
        u8 byteIndexR = (u8)(( (bitFields->r >> 0) == 0xff ) ? 0 : ( (bitFields->r >> 8) == 0xff ) ? 1 : ( (bitFields->r >> 16) == 0xff ) ? 2 : 3);
        u8 byteIndexG = (u8)(( (bitFields->g >> 0) == 0xff ) ? 0 : ( (bitFields->g >> 8) == 0xff ) ? 1 : ( (bitFields->g >> 16) == 0xff ) ? 2 : 3);
        u8 byteIndexB = (u8)(( (bitFields->b >> 0) == 0xff ) ? 0 : ( (bitFields->b >> 8) == 0xff ) ? 1 : ( (bitFields->b >> 16) == 0xff ) ? 2 : 3);

        if ( byteIndexB == 0 && byteIndexG == 1 && byteIndexR == 2) {
            retRCAFMem = ArenaPushMem(inArena, imageSize);

            Image_BGRA_To_RGBA(fileTexels, (u32)(outTex->width * outTex->height), retRCAFMem);
            outTex->hasTransparency = JENH_FALSE;
        }
    } else if (header->compression == BMP_ALPHABITFIELDS) {
        BMP_RGBA_Bit_Fields* bitFields = Consume_Type(&fileScan, BMP_RGBA_Bit_Fields);
        INVALID_PATH("not supported yet.");
    } else {
        retRCAFMem = fileTexels;
        outTex->hasTransparency = Image_Has_Transparency(retRCAFMem, (u32)(outTex->width * outTex->height));
    }

    return retRCAFMem;
}

Public b8 Image_Converter_PNG_To_RCAF(void* inSrcFileMem, u32 inSrcFileSize, File_Handle inDstFile, RCAF_Texture* outTex) {
    stbi_set_flip_vertically_on_load(JENH_TRUE);

    int width;
    int height;
    int channelsInFile;
    int requiredChannelCount = 4;
    void* data = stbi_load_from_memory((stbi_uc const *)inSrcFileMem, (int)inSrcFileSize, &width, &height, &channelsInFile, requiredChannelCount);

    if (!data || stbi_failure_reason()) {
        LogWarn("stbi error message \"%s\"", stbi_failure_reason());
        stbi__err(0, 0);
        return JENH_FALSE;
    }

    outTex->width  = (u16)width;
    outTex->height = (u16)height;
    outTex->channelCount = (u8)requiredChannelCount;
    outTex->hasTransparency = Image_Has_Transparency(data, (u32)(outTex->width * outTex->height));

    File_Write_At(inDstFile, 0, sizeof(RCAF_Texture), &outTex);
    File_Write_At(inDstFile, sizeof(RCAF_Texture), (u32)(outTex->width * outTex->height * outTex->channelCount), data);

    stbi_image_free(data);

    return JENH_TRUE;
}

#define ALPHA_CHANNEL_DEPTH_MASK 0b00001111
#define RIGHT_TO_LEFT_ORDERING_FLAG 0b00010000
#define TOP_TO_BOTTOM_ORDERING_FLAG 0b00100000

typedef enum {
    TIT_Nul                                   =  0,
    TIT_Uncompressed_Mapped                   =  1,
    TIT_Uncompressed_True_Color               =  2,
    TIT_Uncompressed_Black_And_White          =  3,
    TIT_Run_Length_Compressed_Mapped          =  9,
    TIT_Run_Length_Compressed_True_Color      = 10,
    TIT_Run_Length_Compressed_Black_And_White = 11,
} TGA_Image_Type;

typedef u8 TGA_Image_Type_U8;

#pragma pack(push, 1)
typedef struct {
    u16 firstEntryIndex;
    u16 length;
    u8  entrySize;
} TGA_Color_Map;

typedef struct {
    u16 originX;
    u16 originY;
    u16 width;
    u16 height;
    u8  pixelDepth;
    u8  imageDescriptor;
} TGA_Image_Spec;

typedef struct {
    u8 lengthID;
    u8 colorMapType;
    TGA_Image_Type_U8 imageType;
    TGA_Color_Map colorMap;
    TGA_Image_Spec imageSpec;
} TGA_Header;

typedef struct {
    u32 extensionOffset;
    u32 developerOffset;
    char signature[18];
} TGA_Footer;

#define TGA_FOOTER_SIGNATURE S("TRUEVISION-XFILE.")

typedef struct {
    u8 a;
    u8 b;
    u8 c;
} TGA_Date_Time_Stamp;

typedef struct {
    u16 extensionSize;
    char authorName[41];
    char authorComment[324];
    TGA_Date_Time_Stamp dateTimeStamp;
    byte jobID[41];
    char jobTime[6];
    byte softwareID[41];
    char SoftwareVersion[3];
    u32 keyColor;
    u32 pixelAspectRatio;
    u32 gammaValue;
    u32 colorCorrectionOffset;
    u32 postageStampOffset;
    u32 scanLineOffset;
    u8  attributesType;
} TGA_Extension_Area;

#define TGA_EXTENSION_AREA_SIZE 495
#pragma pack(pop)

Public void* Image_Converter_TGA_To_RCAF(void* inFileMem, u32 inFileSize, Memory_Arena* inArena, RCAF_Texture* outTex) {
    u32* retTexels = 0;

    TGA_Header* header;
    header = (TGA_Header*)inFileMem;

    u32 bitDepth = header->imageSpec.pixelDepth; // + (u32)(header->imageSpec.imageDescriptor & ALPHA_CHANNEL_DEPTH_MASK);

    outTex->width = header->imageSpec.width;
    outTex->height = header->imageSpec.height;
    outTex->channelCount = 4; // (u8)(bitDepth / 8);

    u32 imageSize = (u32)(outTex->width * outTex->height * outTex->channelCount);

    u32 texelsCount = (u32)(outTex->width * outTex->height);
    void* fileTexels = (byte*)inFileMem + (sizeof(TGA_Header) + header->lengthID + header->colorMap.length);

    Assert( bitDepth == 8 || bitDepth == 15 || bitDepth == 16 || bitDepth == 24 || bitDepth == 32 );

    TGA_Footer* footer = (TGA_Footer*)((byte*)inFileMem + inFileSize - sizeof(TGA_Footer));

    // Is TGA v2.0
    if ( CStr_Size_Equal(footer->signature, TGA_FOOTER_SIGNATURE, ArrayCount(TGA_FOOTER_SIGNATURE)) ) {
        if ( footer->extensionOffset != 0 ) {
            TGA_Extension_Area* extensionArea = (TGA_Extension_Area*)((byte*)inFileMem + footer->extensionOffset);
        }
        void* developerArea = (byte*)inFileMem + footer->developerOffset;
    }

    switch ( (TGA_Image_Type)header->imageType ) {
        case TIT_Uncompressed_Mapped: {
            INVALID_PATH("TIT_Uncompressed_Mapped image type is currently unsupported");
        } break;

        case TIT_Uncompressed_True_Color: {
            retTexels = (u32*)ArenaPushMem(inArena, imageSize);

            if ( bitDepth == 24 ) {
                Image_BGR_To_RGBA(fileTexels, texelsCount, retTexels);
                outTex->hasTransparency = JENH_FALSE;
            } else { // bitDepth == 32
                Image_BGRA_To_RGBA(fileTexels, texelsCount, retTexels);
                outTex->hasTransparency = Image_Has_Transparency(retTexels, (u32)(outTex->width * outTex->height));
            }
        } break;

        case TIT_Uncompressed_Black_And_White: {
            Assert( bitDepth == 8 );

            retTexels = (u32*)ArenaPushMem(inArena, imageSize);
            Image_Grey_Scale_To_RGBA(fileTexels, texelsCount, retTexels);
            outTex->hasTransparency = JENH_TRUE; // TODO(JENH): Grey scale should always be transparent?.
        } break;

        case TIT_Run_Length_Compressed_Mapped: {
            INVALID_PATH("TIT_Run_Length_Compressed_Mapped image type is currently unsupported");
        } break;

        case TIT_Run_Length_Compressed_True_Color: {
            INVALID_PATH("TIT_Run_Length_Compressed_True_Color image type is currently unsupported");
        } break;

        case TIT_Run_Length_Compressed_Black_And_White: {
            INVALID_PATH("TIT_Run_Length_Compressed_Black_And_White image type is currently unsupported");
        } break;

        case TIT_Nul:
        NO_DEFAULT
    }

    if ( Flags_Has_All(header->imageSpec.imageDescriptor, RIGHT_TO_LEFT_ORDERING_FLAG) ) {
        INVALID_PATH("Right to left images are not supported");
    }

    if ( Flags_Has_All(header->imageSpec.imageDescriptor, TOP_TO_BOTTOM_ORDERING_FLAG) ) {
        Image_RGBA_Flip_Vertically(retTexels, outTex->width, outTex->height);
    }

    return retTexels;
}
