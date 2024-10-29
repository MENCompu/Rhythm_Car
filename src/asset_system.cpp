#define ASSET_SYSTEM_MAX_TEXTURE 256
#define ASSET_SYSTEM_MAX_MESH 256
#define ASSET_SYSTEM_MAX_ID (ASSET_SYSTEM_MAX_TEXTURE + ASSET_SYSTEM_MAX_MESH)

#define INVALID_THING 0xffffffff
#define DEFAULT_ASSET_ID 0

#define ASSET_SYSTEM_DEFAULT_TEXTURE_ID 0
#define ASSET_SYSTEM_FIRST_VALID_TEXTURE_ID 1

#define ASSET_SYSTEM_DEFAULT_MODEL_ID 0
#define ASSET_SYSTEM_FIRST_VALID_MESH_ID 1

#define ASSET_FILE_NAME_SIZE 8

#define ASSET_MATERIAL_MAX_COUNT 4096
#define ASSET_MESH_MAX_COUNT 4096

typedef struct {
    u32 elemCount;
    u32 elementSize;
    void* pool;
    u32 freeList;
} Asset_Memory;

typedef struct {
    u32 assetCount;
    Asset* assets;

    union {
        struct {
            Asset_Memory nul; // TODO(JENH): ?????
            Asset_Memory texture;
            Asset_Memory model;
            Asset_Memory sound;
            Asset_Memory font;
        };
        Asset_Memory E[4];
    } assetMems;

    Free_List meshAllocator;
    Mesh* meshes;

    Free_List materialAllocator;
    Material* materials;

    File_Handle assetFile;

    Texture defaultTex;
    Mesh defaultMesh;
} Asset_System;

Private void Asset_System_Create_Asset(Asset_Type type, String path, Asset* asset);
Private void* Asset_System_ID_To_Address(Asset_Memory* assetMem, u32 ID);
Private u32 Asset_System_Get_ID(Asset_Memory* assetMem);
Private void Asset_System_Free_ID(Asset_Memory* assetMem, u32 ID);

Private void Asset_System_Create_Default_Textures();
Private void Asset_System_Create_Default_Model();

Private b8 Asset_System_Has_Asset(Asset *asset);

Private Global volatile b8 gAssetSystemIsInitialize;

Private Global Asset_System gAssetSystem;

Private CString Asset_Get_Type_Name(Asset_Type inType) {
    switch (inType) {
        case AT_Nul: {
            return (CString)"Nul";
        } break;

        case AT_Texture: {
            return (CString)"Texture";
        } break;

        case AT_Model: {
            return (CString)"Model";
        } break;

        case AT_Sound: {
            return (CString)"Sound";
        } break;

        case AT_Bitmap_Font: {
            return (CString)"Bitmap Font";
        } break;

        case AT_Mesh: {
            return (CString)"Mesh";
        } break;

        case AT_Material: {
            return (CString)"Material";
        } break;

        case ASSET_TYPE_COUNT:
        NO_DEFAULT
    }

    return (CString)"Nul";
}

Public void Asset_System_Init(String inAssetTableFilePath) {
    // TODO(JENH): this shouldn't be always open.
    gAssetSystem.assetFile = File_Open(inAssetTableFilePath.str);

    RCAT_Header header;

    if ( !File_Read_At(gAssetSystem.assetFile, 0, sizeof(RCAT_Header), &header) ) {
        LogError("Failed to read asset file header");
        return;
    }

    Assert(header.magic == Magic_4("RCAT"));
    Assert(header.version == RCAT_VERSION);

    RCAT_Asset* fileAssets = ArenaPushArray(&dyMem.temp.arena, RCAT_Asset, header.assetCount);

    if ( !File_Read_At(gAssetSystem.assetFile, FieldOffset(RCAT_Header, assets), header.assetCount * sizeof(RCAT_Asset), fileAssets) ) {
        LogError("Failed to read asset file header");
        return;
    }

    // NOTE(JENH): Adds 1 for the nul asset.
    gAssetSystem.assetCount = header.assetCount + 1;

    // TODO(JENH): Better memory handling.
    gAssetSystem.assets = ArenaPushArray(&dyMem.perma.assets, Asset, MiB(1));

    gAssetSystem.assets[0].state = AS_Unloaded;
    gAssetSystem.assets[0].type = AT_Texture;
    gAssetSystem.assets[0].fileID = 0x0000000000000000;

    for (u32 i = 1; i < header.assetCount; ++i) {
        Asset* asset = &gAssetSystem.assets[i];
        RCAT_Asset* fileAsset = &fileAssets[i];

        asset->dataID = INVALID_THING;
        asset->state = AS_Unloaded;
        asset->type = fileAsset->type;
        asset->defaultID = (u8)fileAsset->defaultID;
        asset->fileID = fileAsset->fileID;
    }

    // Asset Memory initialization.
    Asset_Memory* assetMem;

    assetMem = &gAssetSystem.assetMems.texture;
    assetMem->elemCount = 4096;
    assetMem->elementSize = sizeof(Texture);
    assetMem->pool = ArenaPushMem(&dyMem.perma.arena, assetMem->elemCount * assetMem->elementSize);

    assetMem = &gAssetSystem.assetMems.model;
    assetMem->elemCount = 512;
    assetMem->elementSize = sizeof(Model);
    assetMem->pool = ArenaPushMem(&dyMem.perma.arena, assetMem->elemCount * assetMem->elementSize);

    assetMem = &gAssetSystem.assetMems.sound;
    assetMem->elemCount = 4096;
    assetMem->elementSize = sizeof(Sound);
    assetMem->pool = ArenaPushMem(&dyMem.perma.arena, assetMem->elemCount * assetMem->elementSize);

    assetMem = &gAssetSystem.assetMems.font;
    assetMem->elemCount = 32;
    assetMem->elementSize = sizeof(Font);
    assetMem->pool = ArenaPushMem(&dyMem.perma.arena, assetMem->elemCount * assetMem->elementSize);

    gAssetSystem.meshes = ArenaPushArray(&dyMem.perma.arena, Mesh, ASSET_MESH_MAX_COUNT);
    Free_List_Init(sizeof(Mesh) * ASSET_MESH_MAX_COUNT, &gAssetSystem.meshAllocator);

    gAssetSystem.materials = ArenaPushArray(&dyMem.perma.arena, Material, ASSET_MATERIAL_MAX_COUNT);
    Free_List_Init(sizeof(Material) * ASSET_MATERIAL_MAX_COUNT, &gAssetSystem.materialAllocator);

    for (u32 memIndex = 0; memIndex < ArrayCount(gAssetSystem.assetMems.E); ++memIndex) {
        Asset_Memory* assetMem = &gAssetSystem.assetMems.E[memIndex];

        // TODO(JENH): Proper handling of default resources.
        assetMem->freeList = ASSET_SYSTEM_FIRST_VALID_TEXTURE_ID;

        for (u32 i = assetMem->freeList; i < assetMem->elemCount; ++i) {
            u32* freeNode = (u32*)Asset_System_ID_To_Address(assetMem, i);
            *freeNode = i+1;
        }
    }

    Asset_System_Create_Default_Textures();
    Asset_System_Create_Default_Model();

    gAssetSystemIsInitialize = JENH_TRUE;
}

Public void Asset_System_Cleanup() {
    for (u32 i = 0; i < gAssetSystem.assetCount; ++i) {
        Asset* asset = &gAssetSystem.assets[i];

        Assert(asset->state != AS_Qeued);
        if (asset->state == AS_Loaded) {
            Asset_Memory* assetMem = &gAssetSystem.assetMems.E[asset->type];

            switch (asset->type) {
                case AT_Texture: {
                    Texture* tex = (Texture*)Asset_System_ID_To_Address(assetMem, asset->dataID);
                    GAPI_Texture_Destroy(tex);
                } break;

                // TODO(JENH): Should cleanup these.
                case AT_Sound: {
                    INVALID_PATH("sound cleanup not supported");
                } break;

                case AT_Model: {
                    Model* model = (Model*)Asset_System_ID_To_Address(assetMem, asset->dataID);
                    //Free_List_Free(&gAssetSystem.meshAllocator, (model->meshes));
                    //Free_List_Free(&gAssetSystem.materialAllocator, (model->materials));

                    //INVALID_PATH("model cleanup not supported");
                } break;
                //

                case AT_Bitmap_Font: {
                    //INVALID_PATH("bitmap font cleanup not supported");
                } break;

                case AT_Mesh:
                case AT_Material:
                case AT_Nul:
                case ASSET_TYPE_COUNT:
                NO_DEFAULT
            }
        }

        asset->state = AS_Unloaded;
    }

    Mesh* mesh = (Mesh*)Asset_System_ID_To_Address(&gAssetSystem.assetMems.E[AT_Mesh], ASSET_SYSTEM_DEFAULT_MODEL_ID);
    GAPI_Mesh_Destroy(mesh);

    Texture* tex = (Texture*)Asset_System_ID_To_Address(&gAssetSystem.assetMems.texture, ASSET_SYSTEM_DEFAULT_TEXTURE_ID);
    GAPI_Texture_Destroy(tex);
}

Public Asset_Handle Asset_Get_Handle(u64 assetID) {
    Asset_Handle retAssetHandle;
    retAssetHandle.assetID = assetID;

    return retAssetHandle;
}

Public Texture* Asset_System_Get_Texture(Asset_Handle inAsset) {
    return (Texture*)Asset_System_Get_Asset(inAsset);
}

Public Mesh* Asset_System_Get_Mesh(Asset_Handle inAsset) {
    return (Mesh*)Asset_System_Get_Asset(inAsset);
}

Public Material* Asset_System_Get_Material(u32 inMaterialID) {
    return &gAssetSystem.materials[inMaterialID];
}

Public Model* Asset_System_Get_Model(Asset_Handle inAsset) {
    return (Model*)Asset_System_Get_Asset(inAsset);
}

Public Font* Asset_Get_Font(Asset_Handle inAsset) {
    return (Font*)Asset_System_Get_Asset(inAsset);
}

Public void* Asset_System_Get_Asset(Asset_Handle handle) {
    Asset* asset = &gAssetSystem.assets[handle.assetID];
    void* ret = 0;

    Asset_Memory* assetMem = &gAssetSystem.assetMems.E[asset->type];

    if (Asset_System_Has_Asset(asset)) {
        Assert(asset->dataID != INVALID_THING);
        ret = Asset_System_ID_To_Address(assetMem, asset->dataID);
    } else {
        asset->state = AS_Qeued;
        asset->dataID = Asset_System_Get_ID(assetMem);

        ret = Asset_System_ID_To_Address(assetMem, asset->dataID);

        b8 resLoad = JENH_FALSE;

        switch ( asset->type ) {
            case AT_Texture: { resLoad = Texture_Create_Form_RCT(asset->fileID, (Texture*)ret); } break;
            case AT_Mesh:    { INVALID_PATH("mesh loading not supported"); } break;
            case AT_Model:   { resLoad = Model_Create_From_RCM(asset->fileID, (Model*)ret); } break;
            case AT_Sound:   { INVALID_PATH("sound loading not supported"); } break;
            case AT_Bitmap_Font: { resLoad = Font_Create_Form_RCF(asset->fileID, (Font*)ret); } break;

            case AT_Nul:
            case AT_Material:
            case ASSET_TYPE_COUNT:
            NO_DEFAULT
        }

        if ( resLoad ) {
            asset->state = AS_Loaded;
        } else {
            // Return "memory" becouse texture loading failed.
            Asset_System_Free_ID(assetMem, asset->dataID);

            asset->dataID = asset->defaultID;
            ret = Asset_System_ID_To_Address(assetMem, asset->dataID);

            asset->state = AS_Using_Default;
        }
    }

    return ret;
}

Public void Asset_System_Release_Asset(Asset_Handle handle) {
    Asset* asset = &gAssetSystem.assets[handle.assetID];
    Assert(asset->dataID != INVALID_THING);
    Assert(Asset_System_Has_Asset(asset));

    Asset_Memory* assetMem = &gAssetSystem.assetMems.E[asset->type];

    void* resource = Asset_System_ID_To_Address(assetMem, asset->dataID);

    switch (asset->type) {
        case AT_Texture: { Texture_Destroy_Form_RCT((Texture*)resource); } break;
        case AT_Model:   { Model_Destroy_From_RCM((Model*)resource); } break;
        case AT_Sound:   { INVALID_PATH("sound release not supported"); } break;
        case AT_Bitmap_Font: { INVALID_PATH("bitmap font is not supported"); } break;

        case AT_Mesh:    { INVALID_PATH("mesh release not supported"); } break;

        case AT_Material:
        case AT_Nul:
        case ASSET_TYPE_COUNT:
        NO_DEFAULT
    }

    asset->state = AS_Unloaded;

    Asset_System_Free_ID(assetMem, asset->dataID);

    asset->dataID = INVALID_THING;
}

Public Resource_Format Asset_Get_Format_By_Extension(String extension) {
           if (Chars_Equal(extension.str, S("png"), extension.size)) {
        return RF_PNG;
    } else if (Chars_Equal(extension.str, S("wav"), extension.size)) {
        return RF_WAV;
    } else if (Chars_Equal(extension.str, S("obj"), extension.size)) {
        return RF_OBJ;
    } else if (Chars_Equal(extension.str, S("bmp"), extension.size)) {
        return RF_BMP;
    } else if (Chars_Equal(extension.str, S("tga"), extension.size)) {
        return RF_TGA;
    } else if (Chars_Equal(extension.str, S("fnt"), extension.size)) {
        return RF_Fnt;
    } else if (Chars_Equal(extension.str, S("mtl"), extension.size)) {
        return RF_Nul;
    } else { // default
        LogInfo("unrecognized extension: %.*s", extension.size, extension.str);
        return RF_Nul;
    }
}

Public String Asset_Get_Extension_By_Asset_Type(Asset_Type type) {
    switch (type) {
        case AT_Texture: { return LitToStr(".rct"); } break;
        case AT_Model:   { return LitToStr(".rcm"); } break;
        case AT_Sound:   { return LitToStr(".rcs"); } break;
        case AT_Mesh:    { /* return LitToStr(".rcm"); */ } break;
        case AT_Bitmap_Font: { return LitToStr(".rcf"); } break;

        case AT_Material:
        case AT_Nul:
        case ASSET_TYPE_COUNT:
        NO_DEFAULT
    }

    return LitToStr(".bolas");
}

Public Asset_Type Asset_Get_Type_By_Reasource_Format(Resource_Format format) {
    Asset_Type assetType = (Asset_Type)((u16)format >> 12);
    return assetType;
}

Public void Asset_Get_File_Name(u64 inFileID, char* outFileName) {
    for (s32 i = 7; i >= 0; --i) {
        u8 digit = (inFileID & 0xf);
        outFileName[i] = Hex_Digit_To_Char(digit);
        inFileID = inFileID >> 4;
    }
}

Private b8 Asset_System_Has_Asset(Asset *asset) {
    Assert(AS_Unloaded < AS_Qeued && AS_Qeued < AS_Loaded && AS_Loaded < AS_Using_Default);
    b8 ret = (asset->state >= AS_Loaded);
    return ret;
}

Private void* Asset_System_ID_To_Address(Asset_Memory* inAssetMem, u32 inID) {
    void* retAddress = (void*)((byte*)inAssetMem->pool + (inAssetMem->elementSize * inID));
    return retAddress;
}

Private u32 Asset_System_Get_ID(Asset_Memory* inAssetMem) {
    u32 retFreeID = inAssetMem->freeList;

    u32 freeNode = *(u32*)Asset_System_ID_To_Address(inAssetMem, retFreeID);
    inAssetMem->freeList = freeNode;

    return retFreeID;
}

Private void Asset_System_Free_ID(Asset_Memory* inAssetMem, u32 inID) {
    u32* freeNode = (u32*)Asset_System_ID_To_Address(inAssetMem, inID);

    *freeNode = inAssetMem->freeList;
    inAssetMem->freeList = inID;
}

Private void Asset_System_Create_Default_Textures() {
    Texture* diffuse = (Texture*)Asset_System_ID_To_Address(&gAssetSystem.assetMems.E[AT_Texture], ASSET_SYSTEM_DEFAULT_TEXTURE_ID);
    //Texture* diffuse = &gAssetSystem.defaultTex;

    const u32 texWidth  = 256;
    const u32 texHeight = 256;
    u32 texels[texWidth * texHeight];

    u32 texelIndex = 0;
    for (u32 row = 0; row < texWidth; ++row) {
        for (u32 col = 0; col < texHeight; ++col) {
            if ((col & (0b00010000)) != 0) {
                if ((row & (0b00010000)) != 0) {
                    texels[texelIndex] = 0xffa0a0a0;
                } else {
                    texels[texelIndex] = 0xffffffff;
                }
            } else {
                if ((row & (0b00010000)) != 0) {
                    texels[texelIndex] = 0xffffffff;
                } else {
                    texels[texelIndex] = 0xffa0a0a0;
                }
            }

            ++texelIndex;
        }
    }

    diffuse->width = texWidth;
    diffuse->height = texHeight;
    diffuse->channelCount = 4;
    diffuse->hasTransparency = JENH_FALSE;

    GAPI_Texture_Create(texels, diffuse);
}

#if 0
Private void Asset_System_Create_Default_Mesh() {
    Mesh* mesh = (Mesh*)Asset_System_ID_To_Address(&gAssetSystem.assetMems.E[AT_Mesh], ASSET_SYSTEM_DEFAULT_MODEL_ID);

    Vertex_Data vertexData[] = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
    };

    Tri_Indices indexData[] = {
        {0,1,2},
        {0,3,1},
    };

    mesh->vertexSize = sizeof(Vertex_Data);
    mesh->vertexCount = ArrayCount(vertexData);
    mesh->indexSize = FieldSize(Tri_Indices, i1);
    mesh->indexCount = sizeof(indexData) / mesh->indexSize;

    GAPI_Mesh_Create(vertexData, indexData, mesh);
}
#endif

Public void Asset_System_Create_Default_Model() {
    Model* model = (Model*)Asset_System_ID_To_Address(&gAssetSystem.assetMems.E[AT_Model], ASSET_SYSTEM_DEFAULT_MODEL_ID);

    model->pipelineIndex = PI_Material;
    model->meshCount = 1;
    model->meshes = (Mesh*)((byte*)gAssetSystem.meshes + Free_List_Alloc(&gAssetSystem.meshAllocator, sizeof(Mesh)));

    model->materialCount = 1;
    model->materials = (Material*)((byte*)gAssetSystem.materials + Free_List_Alloc(&gAssetSystem.materialAllocator, sizeof(Material)));

    Vertex_Data vertexData[] = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
    };

    Tri_Indices indexData[] = {
        {0,1,2},
        {0,3,1},
    };

    Mesh* mesh = &model->meshes[0];

    mesh->vertexSize = sizeof(Vertex_Data);
    mesh->vertexCount = ArrayCount(vertexData);
    mesh->indexSize = FieldSize(Tri_Indices, i1);
    mesh->indexCount = sizeof(indexData) / mesh->indexSize;

    GAPI_Mesh_Create(vertexData, indexData, mesh);

    Material* material = &model->materials[0];

    material->diffuseColor  = F32x4(0.0f, 0.0f, 0.0f, 0.0f);
    material->specularColor = F32x4(0.0f, 0.0f, 0.0f, 0.0f);

    Asset_Handle nulHandle = { 0 };
    material->diffuseTex = nulHandle;
    material->specularTex = nulHandle;
    material->normalTex = nulHandle;
}

Public b8 Font_Create_Form_RCF(u64 inFileID, Font* outFont) {
    Local_Str(fileName, ASSET_FILE_NAME_SIZE);
    Asset_Get_File_Name(inFileID, fileName.str);
    fileName.size = ASSET_FILE_NAME_SIZE;

    Local_Str(filePath, KiB(2));
    CatStr(&filePath, LitToStr("..\\assets\\assets\\cache\\"), fileName);
    CatStr(&filePath, filePath, LitToStr(".rcf"));
    filePath.str[filePath.size] = '\0';

    File_Handle file = File_Open(filePath.str);
    if ( !File_Handle_Is_Valid(file) ) { return JENH_FALSE; }

    u32 fileSize = File_Get_Size(file);
    if ( fileSize == MAX_U32 ) {
        return JENH_FALSE;
    }

    void* fileMem = OS_Alloc_Mem(fileSize);
    if ( !File_Read(file, fileSize, fileMem) ) {
        return JENH_FALSE;
    }

    File_Close(file);

    RCAF_Font* fileFont = (RCAF_Font*)fileMem;

    outFont->bitmap = fileFont->bitmap;
    outFont->bitmapWidth = fileFont->bitmapWidth;
    outFont->bitmapHeight = fileFont->bitmapHeight;

    outFont->lineHeight = fileFont->lineHeight;

    outFont->codePointCount = fileFont->codePointCount;
    outFont->table = fileFont->table;

    return JENH_TRUE;
}

Public b8 Texture_Create_Form_RCT(u64 inFileID, Texture* outTex) {
    b8 ret;

    Local_Str(fileName, ASSET_FILE_NAME_SIZE);
    Asset_Get_File_Name(inFileID, fileName.str);
    fileName.size = ASSET_FILE_NAME_SIZE;

    Local_Str(filePath, KiB(2));
    CatStr(&filePath, LitToStr("..\\assets\\assets\\cache\\"), fileName);
    CatStr(&filePath, filePath, LitToStr(".rct"));
    filePath.str[filePath.size] = '\0';

    File_Handle file = File_Open(filePath.str);
    if ( !File_Handle_Is_Valid(file) ) { return JENH_FALSE; }

    u32 fileSize = File_Get_Size(file);
    if ( fileSize == MAX_U32 ) {
        return JENH_FALSE;
    }

    void* texels = OS_Alloc_Mem(fileSize);

    RCAF_Texture fileTex;
    if (!File_Read_At(file, 0, sizeof(RCAF_Texture), &fileTex)) { defer(JENH_FALSE); }

    if (!File_Read_At(file, sizeof(RCAF_Texture), fileSize - sizeof(RCAF_Texture), texels)) {
        LogWarn("Failed to stream texture: %.*s", filePath.size, filePath.str);
        defer(JENH_FALSE);
    }

    File_Close(file);

    // Convert from file type to "memory" type.
    outTex->width = fileTex.width;
    outTex->height = fileTex.height;
    outTex->channelCount = fileTex.channelCount;
    outTex->hasTransparency = fileTex.hasTransparency;

    // Decompress.

    GAPI_Texture_Create(texels, outTex);

defer:
    OS_Free_Mem(texels);

    return JENH_TRUE;
}

Public b8 Model_Create_From_RCM(u64 inFileID, Model* outModel) {
    Local_Str(fileName, ASSET_FILE_NAME_SIZE);
    Asset_Get_File_Name(inFileID, fileName.str);
    fileName.size = ASSET_FILE_NAME_SIZE;

    Local_Str(filePath, KiB(2));
    CatStr(&filePath, LitToStr("..\\assets\\assets\\cache\\"), fileName);
    CatStr(&filePath, filePath, LitToStr(".rcm"));
    filePath.str[filePath.size] = '\0';

    File_Handle file = File_Open(filePath.str);
    if ( !File_Handle_Is_Valid(file) ) { return JENH_FALSE; }

    u32 fileSize = File_Get_Size(file);

    byte* mem = (byte*)OS_Alloc_Mem(fileSize);

    if ( !File_Read(file, fileSize, mem) ) { return JENH_FALSE; }

    RCAF_Model* fileModel = (RCAF_Model*)mem;

    RCAF_Mesh* fileMeshes = (RCAF_Mesh*)(mem + fileModel->meshesOffset);
    RCAF_Material* fileMaterials = (RCAF_Material*)(mem + FieldOffset(RCAF_Model, materials));

#if 0
    RCAF_Mesh* fileMeshes = ArenaPushArray(arena, RCAF_Mesh, fileModel->meshCount);
    if ( !File_Read_At(file, fileModel->meshesOffset, (sizeof(RCAF_Mesh) * fileModel->meshCount), fileMeshes) ) { return JENH_FALSE; }

    RCAF_Material* fileMaterials = ArenaPushArray(arena, RCAF_Material, fileModel->materialCount);
    if ( !File_Read_At(file, FieldOffset(RCAF_Model, materials), (sizeof(RCAF_Material) * fileModel->materialCount), fileMaterials) ) {
        return JENH_FALSE;
    }
#endif

    outModel->meshCount = fileModel->meshCount;
    outModel->meshes = (Mesh*)((byte*)gAssetSystem.meshes + Free_List_Alloc(&gAssetSystem.meshAllocator, sizeof(Mesh) * outModel->meshCount));

    outModel->materialCount = fileModel->materialCount;
    outModel->materials = (Material*)((byte*)gAssetSystem.materials + Free_List_Alloc(&gAssetSystem.materialAllocator,
                                                                                      sizeof(Material) * outModel->materialCount));

    u32 pipelineIndex = 0;

    for (u32 i = 0; i < outModel->materialCount; ++i) {
        Material* material = &outModel->materials[i];
        RCAF_Material* fileMaterial = &fileMaterials[i];

        // Convert from file type to "memory" type.
        material->diffuseColor = fileMaterial->diffuseColor;
        material->specularColor = fileMaterial->specularColor;
        material->diffuseTex = fileMaterial->diffuseTex;
        material->specularTex = fileMaterial->specularTex;
        material->normalTex = fileMaterial->normalTex;

        GAPI_Material_Create(pipelineIndex, material);
    }

    // TODO(JENH): This is a hack.
    u32 baseMaterialID = (u32)(((byte*)outModel->materials - (byte*)gAssetSystem.materials) / sizeof(Material));

    for (u32 i = 0; i < outModel->meshCount; ++i) {
        Mesh* mesh = &outModel->meshes[i];
        RCAF_Mesh* fileMesh = &fileMeshes[i];

        // Convert from file type to "memory" type.
        mesh->center = fileMesh->center;
        mesh->halfDim = fileMesh->halfDim;
        mesh->vertexSize = fileMesh->vertexSize;
        mesh->vertexCount = fileMesh->vertexCount;
        mesh->indexSize = fileMesh->indexSize;
        mesh->indexCount = fileMesh->indexCount;
        //mesh->materialID = baseMaterialID + fileMesh->materialID;
        mesh->material = &outModel->materials[fileMesh->materialID];

#if 0
        void* vertices = ArenaPushMem(arena, mesh->vertexSize * mesh->vertexCount);
        if (!File_Read_At(file, fileMesh->vertexOffset, mesh->vertexCount * mesh->vertexSize, vertices)) { return JENH_FALSE; }

        void* indices = ArenaPushMem(arena, mesh->indexSize * mesh->indexCount);
        if (!File_Read_At(file, fileMesh->indexOffset, mesh->indexCount * mesh->indexSize, indices)) { return JENH_FALSE; }
#else
        void* vertices = mem + fileMesh->vertexOffset;
        void* indices = mem + fileMesh->indexOffset;
#endif

        GAPI_Mesh_Create(vertices, indices, mesh);
    }

    File_Close(file);
    OS_Free_Mem(mem);

    return JENH_TRUE;
}

Public b8 Asset_Mesh_Get_Vertices(Asset_Handle inHandle, Raw_Mesh* inRawMesh) {
    Asset* asset = &gAssetSystem.assets[inHandle.assetID];

    Local_Str(fileName, ASSET_FILE_NAME_SIZE);
    Asset_Get_File_Name(asset->fileID, fileName.str);
    fileName.size = ASSET_FILE_NAME_SIZE;

    Local_Str(filePath, KiB(2));
    CatStr(&filePath, LitToStr("..\\assets\\assets\\cache\\"), fileName);
    CatStr(&filePath, filePath, LitToStr(".rcm"));
    filePath.str[filePath.size] = '\0';

    File_Handle file = File_Open(filePath.str);
    if ( !File_Handle_Is_Valid(file) ) { return JENH_FALSE; }

    u32 fileSize = File_Get_Size(file);

    byte* mem = (byte*)OS_Alloc_Mem(fileSize);

    if ( !File_Read(file, fileSize, mem) ) { return JENH_FALSE; }

    RCAF_Model* fileModel = (RCAF_Model*)mem;

    RCAF_Mesh* fileMeshes = (RCAF_Mesh*)(mem + fileModel->meshesOffset);

    Assert( fileModel->meshCount == 1 );

    RCAF_Mesh* fileMesh = &fileMeshes[0];

    inRawMesh->vertexCount = fileMesh->vertexCount;
    inRawMesh->vertices = mem + fileMesh->vertexOffset;

    inRawMesh->indexCount = fileMesh->indexCount;
    inRawMesh->indices = mem + fileMesh->indexOffset;

    File_Close(file);
    //OS_Free_Mem(mem);

    return JENH_TRUE;
}

// TODO(JENH): GET RID OF THIS THING!!!.
Public File_Handle Asset_Get_Table_File() {
    return gAssetSystem.assetFile;
}

Public b8 Sound_Create_From_RCAF(File_Handle inFile, u64 inOffset, Sound* ioSound) {
    return JENH_FALSE;
}

Public void Texture_Destroy_Form_RCT(Texture* inTex) {
}

Public void Mesh_Destroy_From_RCM(Mesh* inMesh) {
}

Public void Model_Destroy_From_RCM(Model* inModel) {
}

Public void Asset_Reload(u64 inAssetID) {
    Asset* asset = &gAssetSystem.assets[inAssetID];

    if ( asset->state == AS_Unloaded ) { return; }

    Asset_Memory* assetMem = &gAssetSystem.assetMems.E[asset->type];
    u32 dataID = asset->dataID;

    asset->state = AS_Unloaded;

    Asset_System_Free_ID(assetMem, dataID);

    switch ( asset->type ) {
        case AT_Texture: {
            // TODO(JENH): This should iterate through the allocated material. Not through all of them.
            for (u32 i = 0; i < ASSET_MATERIAL_MAX_COUNT; ++i) {
                Material* material = &gAssetSystem.materials[i];

                for (u32 j = 0; j < ArrayCount(material->textures); ++j) {
                    Asset_Handle handle = material->textures[j];
                    if ( handle.assetID == inAssetID ) {
                        material->upToDate = JENH_FALSE;
                    }
                }
            }
        } break;

        case AT_Sound:
        case AT_Model:
        case AT_Bitmap_Font:
        case AT_Mesh:
        case AT_Material:
        case AT_Nul:
        case ASSET_TYPE_COUNT: break;
    }
}

Public void Asset_Terrain_Create(u32 inQuadWidth, u32 inQuadHeight, f32x2 inScale, Mesh* outMesh) {
    Vertex_Data* vertices = OS_Alloc_Array(Vertex_Data, inQuadWidth * inQuadHeight * 4);
    u32* indices = OS_Alloc_Array(u32, inQuadWidth * inQuadHeight * 8);

    f32 advanceX = inScale.width  / (f32)(inQuadWidth / 2);
    f32 advanceY = inScale.height / (f32)(inQuadHeight / 2);

    u32 vertexBase = 0;
    u32 indexBase = 0;

    f32 y = -inScale.height;

    for (u32 qy = 0; qy < inQuadWidth; ++qy) {

        f32 x = -inScale.width;
        for (u32 qx = 0; qx < inQuadHeight; ++qx) {
            vertices[vertexBase+0] = { F32x3(x           , 0.0f, y), F32x3(0.0f, 0.0f, -1.0f), F32x2(0.0f, 0.0f) };
            vertices[vertexBase+1] = { F32x3(x + advanceX, 0.0f, y), F32x3(0.0f, 0.0f, -1.0f), F32x2(0.0f, 0.0f) };
            vertices[vertexBase+2] = { F32x3(x + advanceX, 0.0f, y + advanceY), F32x3(0.0f, 0.0f, -1.0f), F32x2(0.0f, 0.0f) };
            vertices[vertexBase+3] = { F32x3(x           , 0.0f, y + advanceY), F32x3(0.0f, 0.0f, -1.0f), F32x2(0.0f, 0.0f) };

            indices[indexBase+0] = vertexBase + 0;
            indices[indexBase+1] = vertexBase + 1;
            indices[indexBase+2] = vertexBase + 1;
            indices[indexBase+3] = vertexBase + 2;
            indices[indexBase+4] = vertexBase + 2;
            indices[indexBase+5] = vertexBase + 3;
            indices[indexBase+6] = vertexBase + 3;
            indices[indexBase+7] = vertexBase + 0;

            vertexBase += 4;
            indexBase  += 8;

            x += advanceX;
        }

        y += advanceY;
    }

    outMesh->vertexSize  = sizeof(Vertex_Data);
    outMesh->vertexCount = inQuadWidth * inQuadHeight * 4;
    outMesh->vertexOffset = 0;

    outMesh->indexSize  = sizeof(u32);
    outMesh->indexCount = inQuadWidth * inQuadHeight * 8;
    outMesh->indexOffset = 0;

    GAPI_Mesh_Create(vertices, indices, outMesh);
}
