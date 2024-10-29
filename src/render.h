#ifndef RC_RENDER_H
#define RC_RENDER_H

typedef enum {
    UT_Nul        = 0x00,
    UT_F32        = 0x01,
    UT_F32X2      = 0x02,
    UT_F32X3      = 0x03,
    UT_F32X4      = 0x04,
    UT_S32        = 0x05,
    UT_S32X2      = 0x06,
    UT_S32X3      = 0x07,
    UT_S32X4      = 0x08,
    UT_U32        = 0x09,
    UT_U32X2      = 0x0a,
    UT_U32X3      = 0x0b,
    UT_F32X4X4    = 0x0c,
    UT_Sampler_1D = 0x0d,
    UT_Sampler_2D = 0x0f,
    UT_Sampler_3D = 0x10,
    UT_Buffer     = 0x10,
} Uniform_Type;

typedef enum {
    RPT_World = 0x00,
    RPT_UI    = 0x01,
} Render_Pass_Type;

typedef struct {
    f32x4x4 PVM;
} Uniform;

typedef struct {
    u32 objectID;
    f32x3 diffuseColor;
    struct Texture* textures[16];
} Geometry_Render_Data;

typedef struct {
    f32x4 diffuseColor;
    //f32x4 padding1;
    //f32x4 padding2;
    //f32x4 padding3;
} Object_Uniform_Object;

typedef struct {
    union {
        struct {
            f32x3 a;
            f32x3 b;
        };
        f32x3 points[2];
    };

    f32 width;

    Material* material;
} Line;

#if 0
typedef enum Renderer_Buffer_Type {
    RBT_Vertex,
    RBT_Index,
} Renderer_Buffer_Type;

typedef struct Renderer_Buffer {
    u64 capacity;
    u64 size;

    GAPI_Buffer dataGAPI;
} Renderer_Buffer;
#endif

typedef struct {
    f32x4x4* PVM; // TODO(JENH): Remember that matrices have an aling of a cache line. This should be in its own array.
    Mesh* mesh;
    Material* material;
    u32 pipelineID;
} Renderer_Draw_Data;

Public void Change_Material_Color(f32x4 inColor, Material* outMaterial);
Public void Create_Quad(f32x4 inColor, Mesh* outQuad, Material* outMaterial);
Public void Renderer_Draw_Line(Line* inLine);

Public Material* Renderer_Material_From_Color(f32x4 inColor, u32 inPipelineID);

#endif //RC_RENDER_H
