#ifndef RC_GAPI_INTERFACE_H
#define RC_GAPI_INTERFACE_H

#define Fn_Prot_GAPI_Texture_Create(GAPI_FnName)  void GAPI_FnName(void* inTexels, Texture* ioTex)
#define Fn_Prot_GAPI_Texture_Destroy(GAPI_FnName) void GAPI_FnName(Texture* inTex)

#define Fn_Prot_GAPI_Mesh_Create(GAPI_FnName)  void GAPI_FnName(void* inVertices, void* inIndices, Mesh* ioMesh)
#define Fn_Prot_GAPI_Mesh_Destroy(GAPI_FnName) void GAPI_FnName(Mesh* inMesh)

#define Fn_Prot_GAPI_Material_Create(GAPI_FnName) void GAPI_FnName(u32 inPipelineIndex, Material* outMaterial)
#define Fn_Prot_GAPI_Material_Destroy(GAPI_FnName) void GAPI_FnName(Material* inMaterial)

#define Fn_Prot_GAPI_Buffer_Create(GAPI_FnName) void GAPI_FnName(enum Renderer_Buffer_Type inType, Renderer_Buffer* ioBuffer)
#define Fn_Prot_GAPI_Buffer_Upload_Mem(GAPI_FnName) void GAPI_FnName(Renderer_Buffer* inBuffer, u64 inSize, void* inMem)

#if defined(GAPI_VULKAN)
    Public Fn_Prot_GAPI_Texture_Create(GAPI_Texture_Create) {
        VK_GAPI_Texture_Create(inTexels, ioTex);
    }

    Public Fn_Prot_GAPI_Texture_Destroy(GAPI_Texture_Destroy) {
        VK_GAPI_Texture_Destroy(inTex);
    }

    Public Fn_Prot_GAPI_Mesh_Create(GAPI_Mesh_Create) {
        VK_GAPI_Mesh_Create(inVertices, inIndices, ioMesh);
    }

    Public Fn_Prot_GAPI_Mesh_Destroy(GAPI_Mesh_Destroy) {
        VK_GAPI_Mesh_Destroy(inMesh);
    }

    Public Fn_Prot_GAPI_Material_Create(GAPI_Material_Create) {
        VK_GAPI_Material_Create(inPipelineIndex, outMaterial);
    }

    Public Fn_Prot_GAPI_Material_Destroy(GAPI_Material_Destroy) {
        VK_GAPI_Material_Destroy(inMaterial);
    }

#if 0
    Public Fn_Prot_GAPI_Buffer_Create(GAPI_Buffer_Create) {
        VK_GAPI_Buffer_Create(inType, ioBuffer);
    }

    Public Fn_Prot_GAPI_Buffer_Upload_Mem(GAPI_Buffer_Upload_Mem) {
        VK_GAPI_Buffer_Upload_Mem(inBuffer, inSize, inMem);
    }
#endif

    // GAPI data getters
    Private inline VK_Texture* VK_GAPI_Texture_Get_Data(Texture* inTex) {
        return &inTex->dataGAPI;
    }

#if 0
    Private inline VK_Buffer* VK_GAPI_Buffer_Get_Data(Renderer_Buffer* inBuffer) {
        return &inBuffer->dataGAPI;
    }
#endif

#elif defined(GAPI_RUNTIME_ACCESS)

    typedef Fn_Prot_GAPI_Texture_Create(Fn_GAPI_Texture_Create)
    typedef Fn_Prot_GAPI_Texture_Destroy(Fn_GAPI_Texture_Destroy)

    typedef Fn_Prot_GAPI_Mesh_Create(Fn_GAPI_Mesh_Create)
    typedef Fn_Prot_GAPI_Mesh_Destroy(Fn_GAPI_Mesh_Destroy)

    typedef Fn_Prot_GAPI_Material_Create(Fn_GAPI_Material_Create)

    typedef struct {
        Fn_GAPI_Texture_Create* GAPI_Texture_Create;
        Fn_GAPI_Texture_Destroy* GAPI_Texture_Destroy;

        Fn_GAPI_Mesh_Create* GAPI_Mesh_Create;
        Fn_GAPI_Mesh_Destroy* GAPI_Mesh_Destroy;

        Fn_GAPI_Material_Create* GAPI_Material_Create;
    } GAPI_VTable;

    // GAPI data getters
    Private inline VK_Texture* VK_GAPI_Texture_Get_Data(Texture* inTex) {
        return &inTex->dataGAPI;
    }

#if 0
    Private inline VK_Buffer* VK_GAPI_Buffer_Get_Data(Renderer_Buffer* inBuffer) {
        return &inBuffer->dataGAPI;
    }
#endif

#endif // GAPI_RUNTIME_ACCESS

#endif // RC_GAPI_INTERFACE_H
