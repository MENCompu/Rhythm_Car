#ifndef RC_VULKAN_H
#define RC_VULKAN_H

#pragma warning(push, 1)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#pragma warning(pop)

#define MAX_FRAMES_IN_FLIGHT 3

#define VK_VERTEX_INPUT_FORMAT_F32X2 VK_FORMAT_R32G32_SFLOAT
#define VK_VERTEX_INPUT_FORMAT_F32X3 VK_FORMAT_R32G32B32_SFLOAT

// TODO(JENH): Use this structure for memory usage reporting instead.
#if 0
typedef struct {
    VkDeviceMemory handle;

    VkMemoryPropertyFlags propertyFlags;
    VkMemoryRequirements requirements;
} VK_Device_Memory;
#endif

typedef struct {
    u64 capacity;
    VkBuffer handle;

    VkBufferUsageFlags usage;

    VkDeviceMemory deviceMemHandle;
} VK_Buffer;

typedef struct {
    VkImage handle;
    VkImageView view;

    VkDeviceMemory deviceMemHandle;

    //VkFormat format;
    u16 layer_count;
    u32 mip_levels;
} VK_Image;

typedef struct {
    union {
        struct {
            u32 graphics;
            u32 transfer;
            u32 present;
        };
        u32 E[3];
    };
} VK_Queue_Family_Indices;

typedef struct {
    VK_Queue_Family_Indices familyIndices;

    VkQueue graphics;
    VkQueue present;
    VkQueue transfer;
} VK_Queues;

typedef struct {
    VkPhysicalDevice physical;
    VkDevice logical;

    VK_Queues queues;
    VkFormat depthFormat;
} VK_Devices;

#define SWAPCHAIN_IMAGES_MAX 3

typedef struct {
    VkSurfaceKHR handle;

    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D dims;
} VK_Surface;

typedef struct {
    VkSwapchainKHR handle;
    VK_Surface surface;

    u32 imageCount;
    VkImageView imageViews[SWAPCHAIN_IMAGES_MAX];
    VkFramebuffer frameBuffers[SWAPCHAIN_IMAGES_MAX];

    VK_Image depth;
} VK_Swap_Chain;

#define VK_SHADER_DESCRIPTOR_COUNT 3
#define VK_SHADER_SAMPLER_COUNT 2

typedef struct {
    u32 generations[3];
} VK_Descriptor_State;

typedef struct {
    // NOTE(JENH): Per frame.
    VkDescriptorSet descriptorSets[3];
} VK_Shader_Object_State;

#define VK_OBJECT_MAX_OBJECT_COUNT 512
#define SHADER_STAGE_COUNT 2

typedef struct {
    VkDescriptorPool pool;
    VkDescriptorSetLayout layout;

    u32 firstUnusedSetIndex;
    b8 isStateValid[VK_OBJECT_MAX_OBJECT_COUNT];
    VkDescriptorSet sets[VK_OBJECT_MAX_OBJECT_COUNT];

    //Uniform_Type uniformLayout[3];
} VK_Descriptor_Manager;

typedef enum {
    RPCF_Nul            = Bit_Pos(0),
    RPCF_Color_Buffer   = Bit_Pos(1),
    RPCF_Depth_Buffer   = Bit_Pos(2),
    RPCF_Stencil_Buffer = Bit_Pos(3),

    RPCF_Has_Prev_Pass  = Bit_Pos(4),
    RPCF_Has_Next_Pass  = Bit_Pos(5),
} _VK_Render_Pass_Config_Flags;
typedef u8 VK_Render_Pass_Config_Flags;

typedef struct {
    VkRenderPass handle;

    u32x2 pos;
    u32x2 dims;
    f32x4 clearColor;

    VK_Render_Pass_Config_Flags config;
    VkFramebuffer worldFrameBuffers[3];

    f32 depth;
    u32 stencil;
} VK_Render_Pass;

typedef struct {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule shaderModules[SHADER_STAGE_COUNT];

    VK_Render_Pass* renderPass;
    VkFramebuffer*  frameBuffers;

    VK_Descriptor_Manager* descriptorManager;

    b8 adjustToSwapChain;

    f32x2 viewportPos;
    f32x2 viewportDims;
} VK_Pipeline;

typedef struct {
    VK_Image image;
    VkSampler sampler;
} VK_Texture;

typedef struct {
    u32 size;
    u32 sizeBytes;
    u32 bufferOffset;
} VK_Array_Vertex, VK_Array_Indices;

typedef struct {
    u64 vertexBufferOffset;
    u64 indexBufferOffset;
} VK_Mesh;

typedef struct {
    VK_Buffer buffer;
    Free_List freeList;
} VK_Dynamic_Buffer;

typedef enum {
    PI_Material = 0,
    PI_UI = 1,
    PI_Wireframe = 2,
} Pipeline_ID;

typedef struct {
    VkInstance instance;

    VK_Devices devices;
    VK_Swap_Chain swapchain;

    VK_Render_Pass renderPassUI;
    VK_Render_Pass renderPassWorld;

    VkFramebuffer worldFrameBuffers[3];

    VkRenderPass renderPass;
    VkSurfaceKHR surface;

    u64 vertexBufferOffset;
    u64 indexBufferOffset;

    //VK_Buffer vertexBuffer;
    //VK_Buffer indexBuffer;
    VK_Dynamic_Buffer vertexBuffer;
    VK_Dynamic_Buffer indexBuffer;

    u32 currentFrame;
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffers[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

    u32 currentImageIndex;

    union {
        struct {
            VK_Pipeline pipelineMaterial;
            VK_Pipeline pipelineUI;
            VK_Pipeline wireframePipeline;
        };
        VK_Pipeline pipelines[3];
    };

    u32 uniformOffset;
    VK_Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];

    u32 descriptorManagersCount;
    VK_Descriptor_Manager descriptorManagers[32];

    //VK_Shader shader;

#ifndef JENH_RELEASE
    VkDebugUtilsMessengerEXT debugMessenger;

    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
#endif
} Graphics_API_Context;

Array(Array_VkSurfaceFormatKHR, VkSurfaceFormatKHR);
Array(Array_VkPresentModeKHR, VkPresentModeKHR);
Array(Array_VkImage, VkImage);
Array(Array_VkImageView, VkImageView);
Array(Array_VkDynamicState,VkDynamicState);
Array(Array_VkFramebuffer, VkFramebuffer);
Array(Array_VkSemaphore, VkSemaphore);
Array(Array_VkFence, VkFence);
Array(Array_VkCommandBuffer, VkCommandBuffer);

Array(Array_VkLayerProperties, VkLayerProperties);
Array(Array_VkPhysicalDevice, VkPhysicalDevice);
Array(Array_VkDeviceQueueCreateInfo, VkDeviceQueueCreateInfo);
Array(Array_VkExtensionProperties, VkExtensionProperties);

#define VK_Check(resOrFunc) \
    { \
        VkResult result = (resOrFunc); \
        if (result != VK_SUCCESS) { \
            LogError("(vulkan %u) %s", result, string_VkResult(result)); \
            INVALID_PATH(""); \
        } \
    }

#include "dll_api.h"

#ifdef VULKAN_EXPORT
    #define VULKAN_API Export
#else
    #define VULKAN_API Import
#endif

struct Texture;
struct Mesh;
struct Material;
struct Material;
struct Renderer_Buffer;

VULKAN_API void VK_GAPI_Texture_Create(void* inTexels, Texture* ioTex);
VULKAN_API void VK_GAPI_Texture_Destroy(Texture* inTex);

VULKAN_API void VK_GAPI_Mesh_Create(void* inVertices, void* inIndices, Mesh* ioMesh);
VULKAN_API void VK_GAPI_Mesh_Destroy(Mesh* inMesh);

VULKAN_API void VK_GAPI_Material_Create(u32 inPipelineIndex, Material* outMaterial);
VULKAN_API void VK_GAPI_Material_Destroy(Material* inMaterial);

VULKAN_API void VK_Init(HWND inWindow, HINSTANCE inInstance);
VULKAN_API void VK_Cleanup();

VULKAN_API void VK_Draw_Call_Begin();
VULKAN_API void VK_Draw_Call_End();

VULKAN_API VK_Pipeline* VK_Pipeline_Get(u32 inID);
VULKAN_API void VK_Pipeline_Bind(VK_Pipeline* inPipeline);
VULKAN_API void VK_Pipeline_Unbind();
VULKAN_API void VK_Update_Draw_Call_Uniforms(VK_Pipeline* inPipeline, f32x4x4 inPVM);
VULKAN_API void VK_Update_Material_Uniforms(VK_Pipeline* inPipeline, Texture* inTextures[2], Material* inMaterial, u32 inTexturesCount);

VULKAN_API void VK_Mesh_Draw(Mesh* inMesh);

VULKAN_API void VK_Begin_Frame(s32x2 inWinDims);
VULKAN_API void VK_End_Frame(s32x2 inWinDims);

VULKAN_API void VK_Viewport_Set(Pipeline_ID inID, f32x2 inPos, f32x2 inDims);

VULKAN_API void VK_Set_Line_Width(f32 inlineWidth);

#endif //RC_VULKAN_H
