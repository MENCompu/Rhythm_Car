#ifndef RC_GAPI_INTERFACE_TYPES_H
#define RC_GAPI_INTERFACE_TYPES_H

#define GAPI_VULKAN
//#define GAPI_DIRECTX_12
//#define GAPI_RUNTIME_OPTION

#if defined(GAPI_VULKAN)
    typedef VK_Texture GAPI_Texture;
    typedef VK_Buffer GAPI_Buffer;
#elif defined(GAPI_RUNTIME_OPTION)
    typedef void* GAPI_Texture;
    typedef void* GAPI_Buffer;
#endif

#endif // RC_GAPI_INTERFACE_TYPES_H
