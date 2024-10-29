#ifndef RC_ASSET_SYSTEM_H
#define RC_ASSET_SYSTEM_H

typedef struct {
    f32x3 pos;
    f32x3 normal;
    f32x2 tex;
} Vertex_Data;

typedef union {
    struct {
        u32 i1;
        u32 i2;
        u32 i3;
    };
    u32 E[3];
} Tri_Indices;

#define ASSET_NULL_HANDLE Asset_Get_Handle(0)

typedef struct {
    u64 assetID;
} Asset_Handle;

// Sound

typedef struct {
    u32 sampleCount;
    u32 channelCount;

    s16 maxAmplitude;
    s16 minAmplitude;

    u32 sampleRate;

    s16* samples;
} Sound;

// Materials

#if 0
typedef struct {
    f32x3 diffuseColor;
    f32x3 specularColor;
} Material_Effects;
#endif

typedef struct Material {
    f32x4 diffuseColor;
    f32x4 specularColor;

    union {
        struct {
            Asset_Handle diffuseTex;
            Asset_Handle normalTex;
            Asset_Handle specularTex;
        };

        Asset_Handle textures[3];
    };

    u32 pipelineID;

    u32 uniformOffset;
    b8  upToDate;
    u32 indexSetCurrentlyUsed;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
} Material;

// Meshes

#if 1
typedef struct {
    u32 vertexCount;
    void* vertices;

    u32 indexCount;
    void* indices;

    b8 isDirty;
} Raw_Mesh;
#endif

typedef struct Mesh {
    // TODO(JENH): Could mix this two.
    u8  vertexSize;
    u32 vertexCount;
    u32 vertexOffset;
    //

    u8  indexSize;
    u32 indexCount;
    u32 indexOffset;

    //u32 materialID;
    Material* material;

    // AABB
    f32x3 center;
    f32x3 halfDim;
} Mesh;

// Textures

typedef struct Texture {
    u16 width;
    u16 height;
    u8 channelCount;
    b8 hasTransparency;
    GAPI_Texture dataGAPI;
} Texture;

// Models

typedef struct Model {
    u16 pipelineIndex;

    u32 meshCount;
    Mesh* meshes;

    u32 materialCount;
    Material* materials;
} Model;

// Bitmap Fonts

typedef struct {
    char codePoint;
    u16 xPos;
    u16 yPos;
    u16 width;
    u16 height;
    s16 xOffset;
    s16 yOffset;
    u16 xAdvance;
} Code_Point_Data;

struct RCAF_Code_Point_Data;

typedef struct {
    Asset_Handle bitmap;
    u16 bitmapWidth;
    u16 bitmapHeight;

    u16 lineHeight;
    u32 codePointCount;
    RCAF_Code_Point_Data* table;
} Font;

// Assets

typedef enum {
    AS_Unloaded      = 0x00,
    AS_Qeued         = 0x01,
    AS_Loaded        = 0x02,
    AS_Using_Default = 0x03,
} Asset_State;

typedef enum {
    AT_Nul,
    AT_Texture,
    AT_Model,
    AT_Sound,
    AT_Bitmap_Font,

    AT_Mesh,
    AT_Material,

    ASSET_TYPE_COUNT,
} Asset_Type;

typedef struct {
    Asset_State state;
    // TODO(JENH): Mix this in a bit field.
    Asset_Type type;
    u8 defaultID;
    //

    u64 fileID;
    u32 dataID;
} Asset;

Public void Asset_System_Init();
Public void Asset_System_Cleanup();
Public Asset_Handle Asset_Get_Handle(u64 assetID);

Public Font* Asset_Get_Font(Asset_Handle inAsset);

Public Mesh* Asset_System_Get_Mesh(Asset_Handle inAsset);
Public Material* Asset_System_Get_Material(u32 inMaterialID);
//Public void Asset_System_Release_Mesh(Asset_Handle inAsset);

Public Texture* Asset_System_Get_Texture(Asset_Handle inAsset);
//Public void Asset_System_Release_Texture(Asset_Handle inAsset);

Public Model* Asset_System_Get_Model(Asset_Handle inAsset);
//Public void Asset_System_Release_Model(Asset_Handle inAsset);

Public b8 Texture_Create_Form_RCT(u64 fileID, Texture* ioTex);
Public b8 Font_Create_Form_RCF(u64 inFileID, Font* outFont);
Public b8 Model_Create_From_RCM(u64 fileID, Model* ioModel);
Public void Texture_Destroy_Form_RCT(Texture* inTex);
Public void Model_Destroy_From_RCM(Model* inModel);
//Public void Mesh_Destroy_From_RCM(Mesh* resource);

Public void* Asset_System_Get_Asset(Asset_Handle handle);
Public void Asset_Reload(u64 inAssetID);

Public void Asset_Get_File_Name(u64 inFileID, char* outFileName);

// Rhythm Cat File Format (RCAF)

// NOTE(JENH); The higher 4 bits should equal the asset type.
typedef enum {
    RF_Nul = 0x0000,

    // Image
    RF_BMP = 0x1001,
    RF_PNG = 0x1002,
    RF_TGA = 0x1003,

    // Model
    //RF_RCM = 0x2000,
    RF_OBJ = 0x2001,

    // Audio
    //RF_RCS = 0x3000,
    RF_WAV = 0x3001,

    // Bitmap Font
    RF_Fnt = 0x4001,
} Resource_Format;

#define RCAT_VERSION 1

typedef u32 File_Offset;

#pragma pack(push, 1)
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

typedef struct {
    u16 width;
    u16 height;
    u8 channelCount;
    b8 hasTransparency;
} RCAF_Texture;

#if 0
typedef struct {
    f32x3 diffuseColor;
    f32x3 specularColor;
} RCAF_Material_Effects;
#endif

typedef struct {
    f32x4 diffuseColor;
    f32x4 specularColor;

    Asset_Handle diffuseTex;
    Asset_Handle specularTex;
    Asset_Handle normalTex;
} RCAF_Material;

typedef struct {
    // AABB
    f32x3 center;
    f32x3 halfDim;

    // TODO(JENH): Could mix this two.
    u8  vertexSize;
    u32 vertexCount;
    //
    u32 vertexOffset;

    u8  indexSize;
    u32 indexCount;
    u32 indexOffset;

    // material
    u32 materialID;
} RCAF_Mesh;

typedef struct {
    //u16 pipelineIndex;

    u32 meshCount;
    u32 meshesOffset;

    //u32 materialEffectsCount;
    //u32 materialEffectsOffset;

    u32 materialCount;
    RCAF_Material materials[];
} RCAF_Model;

typedef struct {
    u32 sampleCount;
} RCAF_Sound;

typedef struct {
    // NOTE(JENH): This shouldn't be modified for the converter. The app itself should be in charge of that.
    u32 defaultID;
    //

    Asset_Type type;
    u64 fileID;
} RCAT_Asset;

typedef struct {
    u32 magic;
    u32 version;

    u32 assetCount;
    RCAT_Asset assets[0]; // variable length array (this come next in the file format)
} RCAT_Header;
#pragma pack(pop)

#endif // RC_ASSET_SYSTEM_H
