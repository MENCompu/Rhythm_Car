#define VKAllocator 0

#define INVALID_ID MAX_U32

Private Global CString layersVK[] = { "VK_LAYER_KHRONOS_validation" };
Private Global CString deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
};

Private Global Graphics_API_Context gCtxGAPI;

Public void VK_Swapchain_Recreate(s32x2 inWinDims);

void VK_Set_Line_Width(f32 inlineWidth) {
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];

    vkCmdSetLineWidth(cmdBuf, inlineWidth);
}

Private void VK_Render_Pass_Create(u32x2 pos, u32x2 dims, f32x4 clearColor, f32 depth, u32 stencil,
                                   VK_Render_Pass_Config_Flags config, VK_Render_Pass *renderPass) {
    renderPass->pos = pos;
    renderPass->dims = dims;
    renderPass->clearColor = clearColor;
    renderPass->config = config;
    renderPass->depth = depth;
    renderPass->stencil = stencil;

    VkSubpassDescription subPass;
    subPass.flags = 0;
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    u32 attachmentDescCount = 0;
    VkAttachmentDescription attachments[2];

    VkAttachmentDescription* colorAttachment = &attachments[attachmentDescCount++];
    colorAttachment->flags = 0;
    colorAttachment->format = gCtxGAPI.swapchain.surface.format.format;
    colorAttachment->samples = VK_SAMPLE_COUNT_1_BIT;

#if 1
    colorAttachment->loadOp = Flags_Has_All(renderPass->config, RPCF_Color_Buffer)
        ? VK_ATTACHMENT_LOAD_OP_CLEAR
        : VK_ATTACHMENT_LOAD_OP_LOAD;
#else
    colorAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
#endif

    colorAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment->initialLayout = Flags_Has_All(renderPass->config, RPCF_Has_Prev_Pass)
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment->finalLayout = Flags_Has_All(renderPass->config, RPCF_Has_Next_Pass)
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;

    if (Flags_Has_All(renderPass->config, RPCF_Depth_Buffer)) {
        VkAttachmentDescription* depthAttachment = &attachments[attachmentDescCount++];

        depthAttachment->flags = 0;
        depthAttachment->format = gCtxGAPI.devices.depthFormat;
        depthAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
#if 1
        depthAttachment->loadOp = Flags_Has_All(renderPass->config, RPCF_Color_Buffer)
            ? VK_ATTACHMENT_LOAD_OP_CLEAR
            : VK_ATTACHMENT_LOAD_OP_LOAD;
#else
        depthAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
#endif
        depthAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef;
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subPass.pDepthStencilAttachment = &depthAttachmentRef;
    } else {
        Mem_Zero_Type(&attachments[attachmentDescCount]);
        subPass.pDepthStencilAttachment = 0;
    }

    // TODO(JENH): Other attachment types (input, resolve, preserve).
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = 0;
    subPass.pResolveAttachments = 0;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = 0;

    VkSubpassDependency subPassDependency;
    subPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependency.dstSubpass = 0;
    subPassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subPassDependency.srcAccessMask = 0;
    subPassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subPassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI;
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.pNext = 0;
    renderPassCI.flags = 0;
    renderPassCI.attachmentCount = attachmentDescCount;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subPass;
    renderPassCI.dependencyCount = 1;
    renderPassCI.pDependencies = &subPassDependency;

    VK_Check(vkCreateRenderPass(gCtxGAPI.devices.logical, &renderPassCI, VKAllocator, &renderPass->handle));
}

Private void VK_Render_Pass_Destroy(VK_Render_Pass *renderPass) {
    Assert(renderPass->handle);
    vkDestroyRenderPass(gCtxGAPI.devices.logical, renderPass->handle, VKAllocator);

#ifdef JENH_SAFE
    Mem_Zero_Type(renderPass);
    renderPass->handle = VK_NULL_HANDLE;
#endif
}

Private void VK_Render_Pass_Begin(VK_Render_Pass *renderPass, VkCommandBuffer cmdBuf, VkFramebuffer frameBuffer) {
    VkRenderPassBeginInfo renderPassBeginCI;
    renderPassBeginCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginCI.pNext = 0;
    renderPassBeginCI.renderPass = renderPass->handle;
    renderPassBeginCI.framebuffer = frameBuffer;

    VkExtent2D winDims = gCtxGAPI.swapchain.surface.dims;

    //renderPassBeginCI.renderArea.offset = { (s32)renderPass->pos.x, (s32)renderPass->pos.y };
    //renderPassBeginCI.renderArea.extent = { renderPass->dims.width, renderPass->dims.height };
    renderPassBeginCI.renderArea.offset = { 0, 0 };
    renderPassBeginCI.renderArea.extent = winDims;
    renderPassBeginCI.clearValueCount = 0;
    renderPassBeginCI.pClearValues = 0;

#if 1
    VkClearValue clearValues[2];

    if (Flags_Has_All(renderPass->config, RPCF_Color_Buffer)) {
        VkClearValue *clearValue = &clearValues[renderPassBeginCI.clearValueCount];
        Mem_Copy_Forward(&clearValue->color.float32, renderPass->clearColor.E, sizeof(f32x4));
        ++renderPassBeginCI.clearValueCount;
    }

    if (Flags_Has_All(renderPass->config, RPCF_Depth_Buffer)) {
        VkClearValue *clearValue = &clearValues[renderPassBeginCI.clearValueCount];
        Mem_Copy_Forward(&clearValue->color.float32, renderPass->clearColor.E, sizeof(f32x4));

        clearValue->depthStencil.depth = renderPass->depth;
        clearValue->depthStencil.stencil = Flags_Has_All(renderPass->config, RPCF_Stencil_Buffer)
            ? renderPass->stencil
            : 0;

        ++renderPassBeginCI.clearValueCount;
    }

    if (renderPassBeginCI.clearValueCount > 0) {
        renderPassBeginCI.pClearValues = clearValues;
    }
#endif

    vkCmdBeginRenderPass(cmdBuf, &renderPassBeginCI, VK_SUBPASS_CONTENTS_INLINE);
}

Private void VK_Render_Pass_End(VkCommandBuffer cmdBuf) {
    vkCmdEndRenderPass(cmdBuf);
}

#if 0
Public void VK_GAPI_Render_Pass_Begin(Render_Pass_Type type) {
    VK_Render_Pass* renderPass;
    VkFramebuffer frameBuffer;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentImageIndex];

    switch (type) {
        case RPT_World: {
            renderPass = &gCtxGAPI.renderPassUI;
            frameBuffer = gCtxGAPI.worldFrameBuffers[gCtxGAPI.currentImageIndex];
        } break;

        case RPT_UI: {
            renderPass = &gCtxGAPI.renderPassWorld;
            frameBuffer = gCtxGAPI.swapchain.frameBuffers[gCtxGAPI.currentImageIndex];
        } break;

        NO_DEFAULT
    }

    VK_Render_Pass_Begin(renderPass, cmdBuf, frameBuffer);

    switch (type) {
        case RPT_World: {
            renderPass = &gCtxGAPI.renderPassUI;
            frameBuffer = gCtxGAPI.worldFrameBuffers[gCtxGAPI.currentImageIndex];
        } break;

        case RPT_UI: {
            renderPass = &gCtxGAPI.renderPassWorld;
            frameBuffer = gCtxGAPI.swapchain.frameBuffers[gCtxGAPI.currentImageIndex];
        } break;

        NO_DEFAULT
    }
}

Public void VK_GAPI_Render_Pass_End(Render_Pass_Type type) {
    VK_Render_Pass* renderPass;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentImageIndex];
    (void)renderPass;

#if 0
    switch (type) {
        case RPT_World: {
            renderPass = &gCtxGAPI.renderPassUI;
        } break;

        case RPT_UI: {
            renderPass = &gCtxGAPI.renderPassWorld;
        } break;

        NO_DEFAULT
    }
#endif

    VK_Render_Pass_End(cmdBuf);
}
#endif

// Command Buffers
Private void VK_Command_Buffer_Alloc(VkCommandPool cmdPool, b8 isSecondary, VkCommandBuffer *cmdBuf) {
    VkCommandBufferAllocateInfo allocI = {0};
    allocI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocI.pNext = 0;
    allocI.commandPool = cmdPool;
    allocI.level = (isSecondary) ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocI.commandBufferCount = 1;

    vkAllocateCommandBuffers(gCtxGAPI.devices.logical, &allocI, cmdBuf);
}

Private inline void VK_Command_Buffer_Free(VkCommandPool cmdPool, VkCommandBuffer *cmdBuf) {
    vkFreeCommandBuffers(gCtxGAPI.devices.logical, cmdPool, 1, cmdBuf);
#ifdef JENH_SAFE
    cmdBuf = VK_NULL_HANDLE;
#endif
}

Private void VK_Command_Buffer_Begin(VkCommandBuffer cmdBuf, VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo beginI = {0};
    beginI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginI.pNext = 0;
    beginI.flags = flags;
    beginI.pInheritanceInfo = 0;

    vkBeginCommandBuffer(cmdBuf, &beginI);
}

Private void VK_Command_Buffer_End(VkCommandBuffer cmdBuf) {
    VK_Check(vkEndCommandBuffer(cmdBuf));
}

Private inline void VK_Command_Buffer_Single_Use_Create(VkCommandPool cmdPool, VkCommandBuffer *cmdBuf) {
    VK_Command_Buffer_Alloc(cmdPool, JENH_FALSE, cmdBuf);
    VK_Command_Buffer_Begin(*cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

Private inline void VK_Command_Buffer_Single_Use_Destroy(VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer *cmdBuf) {
    VK_Command_Buffer_End(*cmdBuf);

    VkSubmitInfo submitI = {0};
    submitI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitI.pNext = 0;
    submitI.waitSemaphoreCount = 0;
    submitI.pWaitSemaphores = 0;
    submitI.pWaitDstStageMask = 0;
    submitI.commandBufferCount = 1;
    submitI.pCommandBuffers = cmdBuf;
    submitI.signalSemaphoreCount = 0;
    submitI.pSignalSemaphores = 0;

    vkQueueSubmit(queue, 1, &submitI, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    VK_Command_Buffer_Free(cmdPool, cmdBuf);
}

// Buffers
Private u32 VK_Device_Memory_Find_Type(u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gCtxGAPI.devices.physical, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i) {
        VkMemoryPropertyFlags memPropFlags = memProperties.memoryTypes[i].propertyFlags;
        if (typeFilter & (1 << i) && Flags_Has_All(memPropFlags, properties)) {
            return i;
        }
    }

    INVALID_PATH("Failed to find suitable memory type.");
    return MAX_U32;
}

Private b8 VK_Buffer_Create(u64 capacity, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VK_Buffer *buffer) {
    buffer->capacity = capacity;
    buffer->usage = usage;

    VkBufferCreateInfo bufferCI = {0};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size  = capacity;
    bufferCI.usage = usage;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_Check(vkCreateBuffer(gCtxGAPI.devices.logical, &bufferCI, 0, &buffer->handle));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(gCtxGAPI.devices.logical, buffer->handle, &requirements);

    u32 memoryTypeIndex = VK_Device_Memory_Find_Type(requirements.memoryTypeBits, properties);
    if (memoryTypeIndex == MAX_U32) {
        return JENH_FALSE;
    }

    VkMemoryAllocateInfo allocI = {0};
    allocI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocI.pNext = 0;
    allocI.allocationSize = requirements.size;
    allocI.memoryTypeIndex = memoryTypeIndex;

    VK_Check(vkAllocateMemory(gCtxGAPI.devices.logical, &allocI, 0, &buffer->deviceMemHandle));
    return JENH_TRUE;
}

Private void VK_Buffer_Destroy(VK_Buffer *buffer) {
    vkDestroyBuffer(gCtxGAPI.devices.logical, buffer->handle, VKAllocator);
    vkFreeMemory(gCtxGAPI.devices.logical, buffer->deviceMemHandle, VKAllocator);
#ifdef JENH_SAFE
    Mem_Zero_Type(buffer);
#endif
}

Private void VK_Buffer_Full_Src_Copy(VK_Buffer *dstBuf, VK_Buffer *srcBuf, u64 dstOffset, VkCommandPool cmdPool, VkQueue queue) {
#ifdef JENH_SAFE
    //vkQueueWaitIdle(queue);
#endif

    Assert(srcBuf->capacity <= dstBuf->capacity);

    VkCommandBuffer cmdBuf;
    VK_Command_Buffer_Single_Use_Create(cmdPool, &cmdBuf);

    VkBufferCopy bufferCopy = {0};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = dstOffset;
    bufferCopy.size = srcBuf->capacity;
    vkCmdCopyBuffer(cmdBuf, srcBuf->handle, dstBuf->handle, 1, &bufferCopy);

    VK_Command_Buffer_Single_Use_Destroy(cmdPool, queue, &cmdBuf);
}

Private inline b8 VK_Buffer_Create_And_Bind(u64 capacity, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                           VK_Buffer *buffer) {
    if ( !VK_Buffer_Create(capacity, usage, properties, buffer) ) {
        return JENH_FALSE;
    }

    VK_Check( vkBindBufferMemory(gCtxGAPI.devices.logical, buffer->handle, buffer->deviceMemHandle, 0) );
    return JENH_TRUE;
}

Private void VK_Buffer_Load_Mem(VK_Buffer *buf, u64 offset, void *mem, u32 size) {
    void *memGPU;
    vkMapMemory(gCtxGAPI.devices.logical, buf->deviceMemHandle, offset, size, 0, &memGPU);
    Mem_Copy_Forward(memGPU, mem, (u32)size);
    vkUnmapMemory(gCtxGAPI.devices.logical, buf->deviceMemHandle);
}

Private void VK_Send_Mem_To_GPU(VkCommandPool cmdPool, VkQueue queue, VK_Buffer *buffer, u64 offset, void *mem, u64 size) {
    VK_Buffer stanging;
    VK_Buffer_Create_And_Bind(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stanging);

    VK_Buffer_Load_Mem(&stanging, 0, mem, (u32)size);

    VK_Buffer_Full_Src_Copy(buffer, &stanging, offset, cmdPool, queue);

    VK_Buffer_Destroy(&stanging);
}

Private u64 VK_Dynamic_Buffer_Alloc(VK_Dynamic_Buffer* dyBuffer, u64 size) {
    return Free_List_Alloc(&dyBuffer->freeList, size);
}

Private void VK_Dynamic_Buffer_Free(VK_Dynamic_Buffer* dyBuffer, u64 offset, u64 size) {
    Free_List_Free(&dyBuffer->freeList, offset, size);
}

Private u64 VK_Dynamic_Buffer_Alloc_And_Transfer_To_GPU(VK_Dynamic_Buffer* dyBuffer, VkCommandPool cmdPool,
                                                        VkQueue queue, void *mem, u64 size) {
    u64 offset = VK_Dynamic_Buffer_Alloc(dyBuffer, size);

    VK_Buffer stanging;
    VK_Buffer_Create_And_Bind(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stanging);

    VK_Buffer_Load_Mem(&stanging, 0, mem, (u32)size);

    VK_Buffer_Full_Src_Copy(&dyBuffer->buffer, &stanging, offset, cmdPool, queue);

    VK_Buffer_Destroy(&stanging);

    return offset;
}

#if 0
Public Fn_Prot_GAPI_Buffer_Create(VK_GAPI_Buffer_Create) {
    VK_Buffer* bufferVK = VK_GAPI_Buffer_Get_Data(ioBuffer);

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    if ( inType == RBT_Vertex ) { Flags_Add(usage, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT); }
    if ( inType == RBT_Index  ) { Flags_Add(usage, VK_BUFFER_USAGE_INDEX_BUFFER_BIT); }

    (void)VK_Buffer_Create_And_Bind(ioBuffer->capacity, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferVK);
}

Public Fn_Prot_GAPI_Buffer_Upload_Mem(VK_GAPI_Buffer_Upload_Mem) {
    VK_Buffer* bufferVK = VK_GAPI_Buffer_Get_Data(inBuffer);

    VK_Send_Mem_To_GPU(gCtxGAPI.cmdPool, gCtxGAPI.devices.queues.transfer, bufferVK, 0, inMem, inSize);
    inBuffer->size += inSize;
}
#endif

// Image Views
Private void VK_Image_View_Create(VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageView *imageView) {
    VkImageViewCreateInfo imageViewCI = {0};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.pNext = 0;
    imageViewCI.image = image;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.format = format;
    imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.subresourceRange.aspectMask = aspect; // VK_IMAGE_ASPECT_COLOR_BIT
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;

    VK_Check(vkCreateImageView(gCtxGAPI.devices.logical, &imageViewCI, VKAllocator, imageView));
}

// Images.
Private void VK_Image_Create(VkImageType type, u32 width, u32 height, VkFormat format, VkImageTiling tiling,
                             VkImageUsageFlags usage, VkMemoryPropertyFlags inProperties, VK_Image *image) {
    //image->format = format;
    image->layer_count = 0;
    image->mip_levels = 0;

    VkImageCreateInfo imageCI = {0};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = 0;
    imageCI.imageType = type;
    imageCI.extent = {width, height, 1};
    imageCI.mipLevels = 4;
    imageCI.arrayLayers = 1;
    imageCI.format = format;
    imageCI.tiling = tiling;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCI.usage = usage;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_Check(vkCreateImage(gCtxGAPI.devices.logical, &imageCI, VKAllocator, &image->handle));

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(gCtxGAPI.devices.logical, image->handle, &requirements);

    u32 memoryTypeIndex = VK_Device_Memory_Find_Type(requirements.memoryTypeBits, inProperties);
    Assert(memoryTypeIndex != MAX_U32);

    VkMemoryAllocateInfo memAllocI = {0};
    memAllocI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocI.allocationSize = requirements.size;
    memAllocI.memoryTypeIndex = memoryTypeIndex;

    VK_Check(vkAllocateMemory(gCtxGAPI.devices.logical, &memAllocI, VKAllocator, &image->deviceMemHandle));

    vkBindImageMemory(gCtxGAPI.devices.logical, image->handle, image->deviceMemHandle, 0);
}

Private inline void VK_Image_Create_With_View(VkImageType type, u32 width, u32 height, VkFormat format, VkImageTiling tiling,
                                             VkImageUsageFlags usage, VkMemoryPropertyFlags inProperties,
                                             VkImageAspectFlags aspect, VK_Image *image) {
    VK_Image_Create(type, width, height, format, tiling, usage, inProperties, image);
    VK_Image_View_Create(image->handle, format, aspect, &image->view);
}

Private void VK_Image_Transition_Layout(VK_Image *image, VkCommandBuffer cmdBuf, VkFormat format, VkImageLayout oldLayout,
                                       VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = 0;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = gCtxGAPI.devices.queues.familyIndices.graphics;
    barrier.dstQueueFamilyIndex = gCtxGAPI.devices.queues.familyIndices.graphics;
    barrier.image = image->handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = 0;
    VkPipelineStageFlags dstStage = 0;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } NO_ELSE

    vkCmdPipelineBarrier(cmdBuf, srcStage, dstStage, 0, 0, 0, 0, 0, 1, &barrier);
}

Private void VK_Image_Copy_From_Buffer(VK_Image *image, u32 width, u32 height, VkCommandBuffer cmdBuf, VK_Buffer *buffer) {
    VkBufferImageCopy copyRegion = {0};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;

    copyRegion.imageOffset = {0, 0, 0};
    copyRegion.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(cmdBuf, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}

Private void VK_Image_Destroy(VK_Image *image) {
    if (image->view != VK_NULL_HANDLE) {
        vkDestroyImageView(gCtxGAPI.devices.logical, image->view, VKAllocator);
    }

    Assert(image->deviceMemHandle != VK_NULL_HANDLE);
    vkFreeMemory(gCtxGAPI.devices.logical, image->deviceMemHandle, VKAllocator);

    Assert(image->handle != VK_NULL_HANDLE);
    vkDestroyImage(gCtxGAPI.devices.logical, image->handle, VKAllocator);

#ifdef JENH_SAFE
    Mem_Zero_Type(image);
#endif
}

// Textures.
Fn_Prot_GAPI_Texture_Create(VK_GAPI_Texture_Create) {
    VK_Texture* texVK = VK_GAPI_Texture_Get_Data(ioTex);

    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    VK_Image_Create_With_View(VK_IMAGE_TYPE_2D, ioTex->width, ioTex->height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT, &texVK->image);

    VkCommandBuffer cmdBuf;
    VK_Command_Buffer_Single_Use_Create(gCtxGAPI.cmdPool, &cmdBuf);

    VK_Image_Transition_Layout(&texVK->image, cmdBuf, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    u32 imageSize = (u32)(ioTex->width * ioTex->height * ioTex->channelCount);

    VK_Buffer stanging;
    VK_Buffer_Create_And_Bind(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stanging);
    VK_Buffer_Load_Mem(&stanging, 0, inTexels, imageSize);

    VK_Image_Copy_From_Buffer(&texVK->image, ioTex->width, ioTex->height, cmdBuf, &stanging);

    VK_Image_Transition_Layout(&texVK->image, cmdBuf, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VK_Command_Buffer_Single_Use_Destroy(gCtxGAPI.cmdPool, gCtxGAPI.devices.queues.transfer, &cmdBuf);

    VK_Buffer_Destroy(&stanging);

    VkSamplerCreateInfo samplerCI;
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.pNext = 0;
    samplerCI.flags = 0;
    samplerCI.magFilter = VK_FILTER_LINEAR;
    samplerCI.minFilter = VK_FILTER_LINEAR;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // TODO(JENH): Configurable
    samplerCI.anisotropyEnable = VK_FALSE; // VK_TRUE
    samplerCI.maxAnisotropy = 0; // 16
    samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    //samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCI.unnormalizedCoordinates = VK_FALSE;
    samplerCI.compareEnable = VK_FALSE;
    samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCI.mipLodBias = 0.0f;
    // Use the full range of mips available.
    samplerCI.minLod = 0.0f;
    // NOTE: Uncomment the following line to test the lowest mip level.
    /* sampler_info.minLod = map->texture->mip_levels > 1 ? map->texture->mip_levels : 0.0f; */
    //samplerCI.maxLod = map->texture->mip_levels;
    samplerCI.maxLod = 0.0f;

    VK_Check(vkCreateSampler(gCtxGAPI.devices.logical, &samplerCI, VKAllocator, &texVK->sampler));
#if 0
    if (!vulkan_result_is_success(VK_SUCCESS)) {
        KERROR("Error creating texture sampler: %s", vulkan_result_string(result, true));
        return false;
    }
#endif
}

Fn_Prot_GAPI_Texture_Destroy(VK_GAPI_Texture_Destroy) {
    vkDeviceWaitIdle(gCtxGAPI.devices.logical);

    VK_Texture* textureVK = VK_GAPI_Texture_Get_Data(inTex);
    VK_Image_Destroy(&textureVK->image);
    vkDestroySampler(gCtxGAPI.devices.logical, textureVK->sampler, VKAllocator);
#ifdef JENH_SAFE
    textureVK->sampler = VK_NULL_HANDLE;
    Mem_Zero_Type(inTex);
#endif
}

Private b8 VK_Material_Alloc_Descriptor_Sets(VK_Pipeline *inPipeline, Material* inMaterial) {
    VkDescriptorSetLayout layouts[] = {
        inPipeline->descriptorManager->layout,
        inPipeline->descriptorManager->layout,
        inPipeline->descriptorManager->layout,
    };

    VkDescriptorSetAllocateInfo descriptorAllocI;
    descriptorAllocI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocI.pNext = 0;
    descriptorAllocI.descriptorPool = inPipeline->descriptorManager->pool;
    descriptorAllocI.descriptorSetCount = ArrayCount(layouts);
    descriptorAllocI.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(gCtxGAPI.devices.logical, &descriptorAllocI, inMaterial->descriptorSets)) {
        LogError("Failed allocating material descriptor sets!");
        return JENH_FALSE;
    }

    return JENH_TRUE;
}

Private void VK_Material_Free_Descriptor_Sets(VK_Pipeline* inPipeline, Material* inMaterial) {
    VK_Check( vkFreeDescriptorSets(gCtxGAPI.devices.logical, inPipeline->descriptorManager->pool,
                                   ArrayCount(inMaterial->descriptorSets), inMaterial->descriptorSets) );
}

Fn_Prot_GAPI_Mesh_Create(VK_GAPI_Mesh_Create) {
    Assert(inVertices);
    Assert(ioMesh->vertexSize != 0);
    Assert(ioMesh->vertexCount != 0);

    VkCommandPool cmdPool = gCtxGAPI.cmdPool;
    VkQueue queue = gCtxGAPI.devices.queues.transfer;

    u32 verticesSizeInBytes = ioMesh->vertexSize * ioMesh->vertexCount;

    ioMesh->vertexOffset = (u32)VK_Dynamic_Buffer_Alloc_And_Transfer_To_GPU(&gCtxGAPI.vertexBuffer, cmdPool, queue, inVertices, verticesSizeInBytes);

    if ( ioMesh->indexCount != 0 ) {
        Assert(inIndices);
        Assert(ioMesh->indexSize != 0);

        u32 indicesSizeInBytes = ioMesh->indexSize * ioMesh->indexCount;

        ioMesh->indexOffset = (u32)VK_Dynamic_Buffer_Alloc_And_Transfer_To_GPU(&gCtxGAPI.indexBuffer, cmdPool, queue, inIndices, indicesSizeInBytes);
    }
}

Fn_Prot_GAPI_Mesh_Destroy(VK_GAPI_Mesh_Destroy) {
    VK_Dynamic_Buffer_Free(&gCtxGAPI.vertexBuffer, inMesh->vertexOffset, inMesh->vertexCount * inMesh->vertexSize);
    VK_Dynamic_Buffer_Free(&gCtxGAPI.indexBuffer, inMesh->indexOffset, inMesh->indexCount * inMesh->indexSize);
    inMesh->vertexOffset = 0;
    inMesh->indexOffset = 0;

#ifdef JENH_SAFE
    //Mem_Zero_Type(inMesh);
#endif
}

Fn_Prot_GAPI_Material_Create(VK_GAPI_Material_Create) {
    outMaterial->pipelineID = inPipelineIndex;

    VK_Material_Alloc_Descriptor_Sets(&gCtxGAPI.pipelines[inPipelineIndex], outMaterial);
    outMaterial->uniformOffset = MAX_U32;
    outMaterial->upToDate = JENH_FALSE;
}

Fn_Prot_GAPI_Material_Destroy(VK_GAPI_Material_Destroy) {
    VK_Material_Free_Descriptor_Sets(&gCtxGAPI.pipelines[inMaterial->pipelineID], inMaterial);
    inMaterial->uniformOffset = MAX_U32;
}

Private VkFormat VK_Get_Depth_Format() {
    VkFormat formats[] {
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u32 i = 0; i < ArrayCount(formats); ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(gCtxGAPI.devices.physical, formats[i], &properties);

        if (Flags_Has_All(properties.linearTilingFeatures, flags)) {
            return formats[i];
        } else if (Flags_Has_All(properties.optimalTilingFeatures, flags)) {
            return formats[i];
        }
    }

    return VK_FORMAT_UNDEFINED;
}

// Surface.
Private b8 VK_Surface_Check_Device_Support(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkResult result;

    VkSurfaceCapabilitiesKHR capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    if (result != VK_SUCCESS) {
        return JENH_FALSE;
    }

    u32 formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, 0);
    if (formatsCount == 0) {
        return JENH_FALSE;
    }

    u32 presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, 0);
    if (presentModesCount == 0) {
        return JENH_FALSE;
    }

    return JENH_TRUE;
}

Private void VK_Surface_Configure_Info(VkPhysicalDevice physicalDevice, VK_Surface *surface, s32x2 inWinDims) {
    Array_VkSurfaceFormatKHR formats;
    Array_VkPresentModeKHR presentModes;

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->handle, &formats.size, 0);
    Assert(formats.size > 0);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->handle, &presentModes.size, 0);
    Assert(presentModes.size > 0);

    Memory_Arena *tempArena = AllocTempArena((formats.size * sizeof(VkSurfaceFormatKHR)) +
                                             (presentModes.size * sizeof(VkPresentModeKHR)));

    formats.A = ArenaPushArray(tempArena, VkSurfaceFormatKHR, formats.size);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->handle, &formats.size, formats.A);

    presentModes.A = ArenaPushArray(tempArena, VkPresentModeKHR, presentModes.size);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->handle, &presentModes.size, presentModes.A);

    // Get surface dimentions.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface->handle, &surface->capabilities);

    if (surface->capabilities.currentExtent.width != MAX_U32) {
        surface->dims = surface->capabilities.currentExtent;
    } else {
        VkExtent2D actualDims = { (u32)inWinDims.width, (u32)inWinDims.height };

        VkExtent2D minDims = surface->capabilities.minImageExtent;
        VkExtent2D maxDims = surface->capabilities.maxImageExtent;

        surface->dims.width  = Clamp(minDims.width, actualDims.width, maxDims.width);
        surface->dims.height = Clamp(minDims.height, actualDims.height, maxDims.height);
    }

    // Get surface format.
    surface->format = formats.A[0];
    foreach (VkSurfaceFormatKHR, format, formats) {
        if (format->format == VK_FORMAT_B8G8R8A8_SRGB && format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surface->format = *format;
            break;
        }
    }

    // Get surface format.
    surface->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    foreach (VkPresentModeKHR, presentMode, presentModes) {
        if (*presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            surface->presentMode = *presentMode;
            break;
        }
    }

    FreeTempArena(tempArena);
}

Private void CreateBuffers() {
    u64 vertexBufferCapacity = MiB(64);
    if (!VK_Buffer_Create_And_Bind(vertexBufferCapacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT|
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   &gCtxGAPI.vertexBuffer.buffer)) {
        LogError("Failed to create vertex buffer");
    }

    Free_List_Init(vertexBufferCapacity, &gCtxGAPI.vertexBuffer.freeList);

    u64 indexBufferCapacity = MiB(64);
    if (!VK_Buffer_Create_And_Bind(indexBufferCapacity, VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT|
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   &gCtxGAPI.indexBuffer.buffer)) {
        LogError("Failed to create index buffer");
    }

    Free_List_Init(indexBufferCapacity, &gCtxGAPI.indexBuffer.freeList);
}

Private void VK_Swapchain_Cleanup() {
    for (u32 i = 0; i < gCtxGAPI.swapchain.imageCount; i++) {
        vkDestroyFramebuffer(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.frameBuffers[i], VKAllocator);
        //vkDestroyFramebuffer(gCtxGAPI.devices.logical, gCtxGAPI.worldFrameBuffers[i], VKAllocator);
        vkDestroyImageView(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.imageViews[i], VKAllocator);
    }

    VK_Image_Destroy(&gCtxGAPI.swapchain.depth);

    vkDestroySwapchainKHR(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.handle, VKAllocator);
}

VK_Pipeline* VK_Pipeline_Get(u32 inID) {
    return &gCtxGAPI.pipelines[inID];
}

void VK_Viewport_Set(Pipeline_ID inID, f32x2 inPos, f32x2 inDims) {
    VK_Pipeline* pipeline = &gCtxGAPI.pipelines[inID];

    pipeline->viewportPos.x = inPos.x;
    pipeline->viewportPos.y = inPos.y + inDims.height;
    pipeline->viewportDims.width = inDims.width;
    pipeline->viewportDims.height = inDims.height;
}

void VK_Pipeline_Bind(VK_Pipeline* inPipeline) {
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline->handle);

    if ( inPipeline->adjustToSwapChain ) {
        VkViewport viewport = {0};
        viewport.x = 0.0f;
        viewport.y = (f32)gCtxGAPI.swapchain.surface.dims.height;
        viewport.width = (f32)gCtxGAPI.swapchain.surface.dims.width;
        viewport.height = -(f32)gCtxGAPI.swapchain.surface.dims.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    } else {
        VkViewport viewport = {0};
        viewport.x = inPipeline->viewportPos.x * (f32)gCtxGAPI.swapchain.surface.dims.width;
        viewport.y = inPipeline->viewportPos.y * (f32)gCtxGAPI.swapchain.surface.dims.height;
        viewport.width = inPipeline->viewportDims.width * (f32)gCtxGAPI.swapchain.surface.dims.width;
        viewport.height = -inPipeline->viewportDims.height * (f32)gCtxGAPI.swapchain.surface.dims.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    }

    VkRect2D scissor = {0};
    scissor.offset = {0, 0};
    scissor.extent = gCtxGAPI.swapchain.surface.dims;
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
}

void VK_Pipeline_Unbind() {
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];
}

#if 0
Private void VK_Clear_Rect() {
    VkClearRect clearRect = { 0 };
#if 1
    clearRect.rect.offset.x = (s32)(pipeline->viewportPos.x * (f32)gCtxGAPI.swapchain.surface.dims.width);
    clearRect.rect.offset.y = (s32)((pipeline->viewportPos.y - pipeline->viewportDims.height) * (f32)gCtxGAPI.swapchain.surface.dims.height);
    clearRect.rect.extent.width = (u32)(pipeline->viewportDims.width * (f32)gCtxGAPI.swapchain.surface.dims.width);
    clearRect.rect.extent.height = (u32)(pipeline->viewportDims.height * (f32)gCtxGAPI.swapchain.surface.dims.height);
#else
    clearRect.rect.offset.x = (s32)gCtxGAPI.swapchain.surface.dims.width / 2;
    clearRect.rect.offset.y = (s32)gCtxGAPI.swapchain.surface.dims.height / 2;
    clearRect.rect.extent.width = (u32)gCtxGAPI.swapchain.surface.dims.width / 4;
    clearRect.rect.extent.height = (u32)gCtxGAPI.swapchain.surface.dims.height / 4;
#endif

    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    VK_Render_Pass* renderPass = &gCtxGAPI.renderPassWorld;

    VkClearAttachment clearAttachments[2];
    clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachments[0].colorAttachment = 0;
    Mem_Copy_Forward(&clearAttachments[0].clearValue.color.float32, &renderPass->clearColor, sizeof(f32x4));

    clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;
    clearAttachments[1].colorAttachment = 0;
    clearAttachments[1].clearValue.depthStencil.depth = renderPass->depth;
    clearAttachments[1].clearValue.depthStencil.stencil = renderPass->stencil;

    vkCmdClearAttachments(cmdBuf, ArrayCount(clearAttachments), clearAttachments, 1, &clearRect);
}
#endif

void VK_Draw_Call_Begin() {
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];

    VK_Command_Buffer_Begin(cmdBuf, 0);

    VK_Pipeline* pipeline = &gCtxGAPI.pipelineMaterial;

    VK_Render_Pass_Begin(&gCtxGAPI.renderPassWorld, cmdBuf, gCtxGAPI.swapchain.frameBuffers[gCtxGAPI.currentImageIndex]);
}

void VK_Draw_Call_End() {
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];

    VK_Render_Pass_End(cmdBuf);
    VK_Command_Buffer_End(cmdBuf);
}

void VK_Begin_Frame(s32x2 inWinDims) {
    VkResult result;

    vkWaitForFences(gCtxGAPI.devices.logical, 1, &gCtxGAPI.inFlightFences[gCtxGAPI.currentFrame], VK_TRUE, MAX_U64);

    result = vkAcquireNextImageKHR(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.handle, MAX_U64,
                                   gCtxGAPI.imageAvailableSemaphores[gCtxGAPI.currentFrame],
                                   VK_NULL_HANDLE, &gCtxGAPI.currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        VK_Swapchain_Recreate(inWinDims);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LogError("Couldn't present image: %s.\n", string_VkResult(result));
    }

    vkResetFences(gCtxGAPI.devices.logical, 1, &gCtxGAPI.inFlightFences[gCtxGAPI.currentFrame]);

    vkResetCommandBuffer(gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame], 0);
}

void VK_End_Frame(s32x2 inWinDims) {
    VkSubmitInfo submitI = {0};
    submitI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { gCtxGAPI.imageAvailableSemaphores[gCtxGAPI.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitI.waitSemaphoreCount = 1;
    submitI.pWaitSemaphores = waitSemaphores;
    submitI.pWaitDstStageMask = waitStages;
    submitI.commandBufferCount = 1;
    submitI.pCommandBuffers = &gCtxGAPI.cmdBuffers[gCtxGAPI.currentFrame];

    VkSemaphore signalSemaphores[] = { gCtxGAPI.renderFinishedSemaphores[gCtxGAPI.currentFrame] };
    submitI.signalSemaphoreCount = 1;
    submitI.pSignalSemaphores = signalSemaphores;

    VK_Check(vkQueueSubmit(gCtxGAPI.devices.queues.graphics, 1, &submitI,
                           gCtxGAPI.inFlightFences[gCtxGAPI.currentFrame]));

    VkPresentInfoKHR presentI = {0};
    presentI.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentI.waitSemaphoreCount = 1;
    presentI.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { gCtxGAPI.swapchain.handle };
    presentI.swapchainCount = 1;
    presentI.pSwapchains = swapchains;
    presentI.pImageIndices = &gCtxGAPI.currentImageIndex;
    presentI.pResults = 0;

    VkResult result;
    result = vkQueuePresentKHR(gCtxGAPI.devices.queues.present, &presentI);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        VK_Swapchain_Recreate(inWinDims);
    } else if (result != VK_SUCCESS) {
        LogError("Couldn't present image: %s.\n", string_VkResult(result));
    }

    gCtxGAPI.currentFrame = (gCtxGAPI.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VK_Update_Draw_Call_Uniforms(VK_Pipeline* inPipeline, f32x4x4 inPVM) {
    u32 imageIndex = gCtxGAPI.currentFrame;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[imageIndex];

    vkCmdPushConstants(cmdBuf, inPipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(f32x4x4), &inPVM);
}

void VK_Update_Material_Uniforms(VK_Pipeline* inPipeline, Texture* inTextures[2], Material* inMaterial, u32 inTexturesCount) {
    u32 imageIndex = gCtxGAPI.currentFrame;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[imageIndex];

    if ( inMaterial->upToDate ) {
        VkDescriptorSet descriptorSet = inMaterial->descriptorSets[inMaterial->indexSetCurrentlyUsed];
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline->layout, 0, 1, &descriptorSet, 0, 0);
        return;
    }

    u32 setIndex = (inMaterial->indexSetCurrentlyUsed + 1) % 3;
    VkDescriptorSet descriptorSet = inMaterial->descriptorSets[setIndex];
    inMaterial->indexSetCurrentlyUsed = setIndex;

    VkWriteDescriptorSet descriptorWrites[VK_SHADER_DESCRIPTOR_COUNT];
    Mem_Zero(descriptorWrites, sizeof(descriptorWrites));
    u32 descriptorCount = 0;
    u32 descriptorBinding = 0;

    u32 range  = sizeof(Object_Uniform_Object);

    u32 offset;
    if ( inMaterial->uniformOffset != MAX_U32 ) {
        offset = inMaterial->uniformOffset;
    } else {
        offset = gCtxGAPI.uniformOffset;
        gCtxGAPI.uniformOffset += range;
    }

    inMaterial->uniformOffset = offset;

#if 1
    Object_Uniform_Object ouo;

    ouo.diffuseColor = inMaterial->diffuseColor;

    VK_Buffer* uniformBuffer = &gCtxGAPI.uniformBuffers[imageIndex];

    VK_Buffer_Load_Mem(uniformBuffer, offset, &ouo, range);

    VkDescriptorBufferInfo bufferI;
    bufferI.buffer = uniformBuffer->handle;
    bufferI.offset = offset;
    bufferI.range  = range;

    VkWriteDescriptorSet* write = &descriptorWrites[descriptorCount++];
    write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write->pNext = 0;
    write->dstSet = descriptorSet;
    write->dstBinding = descriptorBinding;
    write->dstArrayElement = 0;
    write->descriptorCount = 1;
    write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    write->pImageInfo = 0;
    write->pBufferInfo = &bufferI;
    write->pTexelBufferView = 0;
#endif

    ++descriptorBinding;

#define SAMPLER_COUNT 2
    VkDescriptorImageInfo imageInfos[SAMPLER_COUNT];
    Assert( inTexturesCount <= SAMPLER_COUNT );
    for (u32 samplerIndex = 0; samplerIndex < inTexturesCount; ++samplerIndex) {
        Texture* tex = inTextures[samplerIndex];

        VK_Texture* texVK = VK_GAPI_Texture_Get_Data(tex);

        imageInfos[samplerIndex].sampler = texVK->sampler;
        imageInfos[samplerIndex].imageView = texVK->image.view;
        imageInfos[samplerIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet* write = &descriptorWrites[descriptorCount++];
        write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write->pNext = 0;
        write->dstSet = descriptorSet;
        write->dstBinding = descriptorBinding;
        write->dstArrayElement = 0;
        write->descriptorCount = 1;
        write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        write->pImageInfo = &imageInfos[samplerIndex];
        write->pBufferInfo = 0;
        write->pTexelBufferView = 0;

        ++descriptorBinding;
    }

    if (descriptorCount > 0) {
        vkUpdateDescriptorSets(gCtxGAPI.devices.logical, descriptorCount, descriptorWrites, 0, 0);
    }

    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline->layout, 0, 1, &descriptorSet, 0, 0);

    inMaterial->upToDate = JENH_TRUE;
}

void VK_Mesh_Draw(Mesh* inMesh) {
    u32 imageIndex = gCtxGAPI.currentFrame;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[imageIndex];

    VkDeviceSize offsets[] = { inMesh->vertexOffset };
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, &gCtxGAPI.vertexBuffer.buffer.handle, offsets);

    if ( inMesh->indexCount != 0 ) {
        vkCmdBindIndexBuffer(cmdBuf, gCtxGAPI.indexBuffer.buffer.handle, inMesh->indexOffset, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmdBuf, inMesh->indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(cmdBuf, inMesh->vertexCount, 1, 0, 0);
    }
}

#if 0
Public void VK_Buffers_Draw(Renderer_Buffer* inVertex, Renderer_Buffer* inIndex) {
    VK_Buffer* inVertexVK = VK_GAPI_Buffer_Get_Data(inVertex);
    VK_Buffer* inIndexVK  = VK_GAPI_Buffer_Get_Data(inIndex);

    u32 imageIndex = gCtxGAPI.currentFrame;
    VkCommandBuffer cmdBuf = gCtxGAPI.cmdBuffers[imageIndex];

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, &inVertexVK->handle, offsets);

    vkCmdBindIndexBuffer(cmdBuf, inIndexVK->handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuf, (u32)(inIndex->size / sizeof(u32)), 1, 0, 0, 0);
}
#endif

Private void AssertValidationLayers(Memory_Arena *arena) {
    Array_VkLayerProperties availableLayers;
    vkEnumerateInstanceLayerProperties(&availableLayers.size, 0);

    availableLayers.A = ArenaPushArray(arena, VkLayerProperties, availableLayers.size);

    vkEnumerateInstanceLayerProperties(&availableLayers.size, availableLayers.A);

    for (u32 i = 0; i < ArrayCount(layersVK); ++i) {
        CString layer = layersVK[i];
        foreach (VkLayerProperties, layerProp, availableLayers) {
            if ( CStr_Equal(layer, layerProp->layerName) ) { goto is_available; }
        }

        LogError("%s layer is not pressent", layer);
is_available:;
    }
}

// Validation layers
Private VKAPI_ATTR VkBool32 VKAPI_CALL
VK_Debug_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LogError("validation layer: %s", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

Private VkResult
VK_Debug_Utils_Messenger_EXT_Create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
                                               (void*)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    Assert(func != 0);
    return func(instance, pCreateInfo, pAllocator, pMessenger);
}

Private void VK_Debug_Utils_Messenger_EXT_Destroy(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                 const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                                                (void*)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    Assert(func != 0);
    func(instance, messenger, pAllocator);
}

Array(Array_VkQueueFamilyProperties, VkQueueFamilyProperties);

Private b8 VK_Device_Indices(Memory_Arena *arena, VkPhysicalDevice device, VK_Queue_Family_Indices *familyIndices) {
    familyIndices->graphics = MAX_U32;
    familyIndices->present  = MAX_U32;
    familyIndices->transfer = MAX_U32;

    Array_VkQueueFamilyProperties queueFamilyProps;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyProps.size, 0);

    queueFamilyProps.A = ArenaPushArray(arena, VkQueueFamilyProperties, queueFamilyProps.size);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyProps.size, queueFamilyProps.A);

    s32 maxTransferScore = -1;

    for (u32 i = 0; i < queueFamilyProps.size; ++i) {
        s32 transferScore = 2;

        if (familyIndices->graphics != MAX_U32 && familyIndices->present != MAX_U32 &&
            familyIndices->transfer != MAX_U32) {
            return JENH_TRUE;
        }

        VkQueueFamilyProperties *queueFamily = &queueFamilyProps.A[i];

        if (queueFamily->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            familyIndices->graphics = i;
            transferScore -= 1;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, gCtxGAPI.swapchain.surface.handle, &presentSupport);
        if (presentSupport) {
            familyIndices->present = i;
            transferScore -= 1;
        }

        if (transferScore > maxTransferScore) {
            familyIndices->transfer = i;
            maxTransferScore = transferScore;
        }
    }

    return false;
}

Private b8 VK_Device_Check_Extension_Support(Memory_Arena *arena, VkPhysicalDevice device) {
    u32 extensionCount;
    VkExtensionProperties* extensions;

    vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, 0);

    extensions = ArenaPushArray(arena, VkExtensionProperties, extensionCount);

    vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, extensions);

    for (u32 i = 0; i < ArrayCount(deviceExtensions); ++i) {
        CString extension = deviceExtensions[i];

        for (u32 j = 0; j < extensionCount; ++j) {
            CString availableExtension = extensions[j].extensionName;

            if ( CStr_Equal(availableExtension, extension) ) {
                goto found;
            }
        }

        return false;
        found:;
    }

    return JENH_TRUE;
}

Private b8 VK_Device_Check(Memory_Arena *arena, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VK_Queue_Family_Indices indices;
    b8 resDI = VK_Device_Indices(arena, physicalDevice, &indices);
    b8 resCDES = VK_Device_Check_Extension_Support(arena, physicalDevice);

    return (resDI && resCDES && VK_Surface_Check_Device_Support(physicalDevice, surface));
}

Private void CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreCI = {0};
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCI = {0};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VK_Check(vkCreateSemaphore(gCtxGAPI.devices.logical, &semaphoreCI, 0, &gCtxGAPI.imageAvailableSemaphores[i]));
        VK_Check(vkCreateSemaphore(gCtxGAPI.devices.logical, &semaphoreCI, 0, &gCtxGAPI.renderFinishedSemaphores[i]));
        VK_Check(vkCreateFence(gCtxGAPI.devices.logical, &fenceCI, 0, &gCtxGAPI.inFlightFences[i]));
    }
}

Private void VK_Swapchain_Create(s32x2 inWinDims) {
    VK_Surface_Configure_Info(gCtxGAPI.devices.physical, &gCtxGAPI.swapchain.surface, inWinDims);

    u32 maxImageCount = (gCtxGAPI.swapchain.surface.capabilities.maxImageCount == 0)
                      ? MAX_U32
                      : gCtxGAPI.swapchain.surface.capabilities.maxImageCount;
    u32 imageCount = Min(maxImageCount, gCtxGAPI.swapchain.surface.capabilities.minImageCount + 1);

    VkSwapchainCreateInfoKHR swapchainCI = {0};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = gCtxGAPI.swapchain.surface.handle;
    swapchainCI.minImageCount = imageCount;
    swapchainCI.imageFormat = gCtxGAPI.swapchain.surface.format.format;
    swapchainCI.imageColorSpace = gCtxGAPI.swapchain.surface.format.colorSpace;
    swapchainCI.imageExtent = gCtxGAPI.swapchain.surface.dims;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndices[] = { gCtxGAPI.devices.queues.familyIndices.graphics,
                                 gCtxGAPI.devices.queues.familyIndices.transfer };

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.queueFamilyIndexCount = 0;
        swapchainCI.pQueueFamilyIndices = 0;
    }

    swapchainCI.preTransform = gCtxGAPI.swapchain.surface.capabilities.currentTransform;

    // TODO(JENH): alpha blending with rest of windows.
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = gCtxGAPI.swapchain.surface.presentMode;
    swapchainCI.clipped = VK_TRUE;

    gCtxGAPI.devices.depthFormat = VK_Get_Depth_Format();
    if (gCtxGAPI.devices.depthFormat == VK_FORMAT_UNDEFINED) {
        LogFatal("Failed to find depth stencil image format");
        return;
    }

    VK_Image_Create_With_View(VK_IMAGE_TYPE_2D, gCtxGAPI.swapchain.surface.dims.width,
                              gCtxGAPI.swapchain.surface.dims.height, gCtxGAPI.devices.depthFormat, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_DEPTH_BIT, &gCtxGAPI.swapchain.depth);

    // TODO(JENH): When recreating the swapchain, figure out how to properly save the old one until it finish.
    swapchainCI.oldSwapchain = VK_NULL_HANDLE;

    VK_Check(vkCreateSwapchainKHR(gCtxGAPI.devices.logical, &swapchainCI, 0, &gCtxGAPI.swapchain.handle));
}

Private void CreateImageViews() {
    Array_VkImage swapchainImages = {0};
    vkGetSwapchainImagesKHR(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.handle, &swapchainImages.size, 0);

    Assert(swapchainImages.size <= SWAPCHAIN_IMAGES_MAX);

    Memory_Arena *tempArena;
    swapchainImages.A = AllocTempArray(&tempArena, VkImage, swapchainImages.size);

    vkGetSwapchainImagesKHR(gCtxGAPI.devices.logical, gCtxGAPI.swapchain.handle,
                            &swapchainImages.size, swapchainImages.A);
    gCtxGAPI.swapchain.imageCount = swapchainImages.size;

    VkFormat format = gCtxGAPI.swapchain.surface.format.format;

    for (u32 i = 0; i < gCtxGAPI.swapchain.imageCount; ++i) {
        VK_Image_View_Create(swapchainImages.A[i], format, VK_IMAGE_ASPECT_COLOR_BIT, &gCtxGAPI.swapchain.imageViews[i]);
    }

    FreeTempArena(tempArena);
}

Private void CreateFrameBuffers() {
    for (u32 i = 0; i < gCtxGAPI.swapchain.imageCount; ++i) {
        VkImageView worldAttachments[] = {
            gCtxGAPI.swapchain.imageViews[i],
            gCtxGAPI.swapchain.depth.view,
        };

        VkFramebufferCreateInfo framebufferCI = {0};
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.pNext = 0;
        framebufferCI.flags = 0;
        framebufferCI.renderPass = gCtxGAPI.renderPassWorld.handle;
        framebufferCI.attachmentCount = ArrayCount(worldAttachments);
        framebufferCI.pAttachments = worldAttachments;
        framebufferCI.width = gCtxGAPI.swapchain.surface.dims.width;
        framebufferCI.height = gCtxGAPI.swapchain.surface.dims.height;
        framebufferCI.layers = 1;

        VK_Check(vkCreateFramebuffer(gCtxGAPI.devices.logical, &framebufferCI, 0, &gCtxGAPI.swapchain.frameBuffers[i]));

#if 0
        VkImageView uiAttachments[] = {
            gCtxGAPI.swapchain.imageViews[i],
        };

        VkFramebufferCreateInfo scFramebufferCI;
        scFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        scFramebufferCI.pNext = 0;
        scFramebufferCI.flags = 0;
        scFramebufferCI.renderPass = gCtxGAPI.renderPassUI.handle;
        scFramebufferCI.attachmentCount = ArrayCount(uiAttachments);
        scFramebufferCI.pAttachments = uiAttachments;
        scFramebufferCI.width = gCtxGAPI.swapchain.surface.dims.width;
        scFramebufferCI.height = gCtxGAPI.swapchain.surface.dims.height;
        scFramebufferCI.layers = 1;

        VK_Check(vkCreateFramebuffer(gCtxGAPI.devices.logical, &scFramebufferCI,
                                     VKAllocator, &gCtxGAPI.swapchain.frameBuffers[i]));
#endif
    }
}

Public void VK_Swapchain_Recreate(s32x2 inWinDims) {
    vkDeviceWaitIdle(gCtxGAPI.devices.logical);

    VK_Swapchain_Cleanup();

    VK_Swapchain_Create(inWinDims);
    CreateImageViews();
    CreateFrameBuffers();
}

#if 0
Private VkDescriptorType Uniform_Type_To_VK(Uniform_Type inType) {
    switch (inType) {
        case UT_F32: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_F32X2: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_F32X3: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_F32X4: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_S32: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_S32X2: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_S32X3: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_S32X4: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_U32: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_U32X2: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_U32X3: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_F32X4X4: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_Sampler_1D: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_Sampler_2D: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_Sampler_3D: {
            INVALID_PATH("uniform type no supported in vulkan");
        } break;

        case UT_Buffer: {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        } break;

        case UT_Nul:
        NO_DEFAULT
    }
}
#endif

Private VK_Descriptor_Manager* VK_Descriptor_Manager_Create(u32 inBindingCount, VkDescriptorType* inDescriptorTypes, u32 inMaxSetCount) {
    VK_Descriptor_Manager* retDescriptorManager = &gCtxGAPI.descriptorManagers[gCtxGAPI.descriptorManagersCount++];

    Assert( inBindingCount <= VK_SHADER_DESCRIPTOR_COUNT );

    // TODO(JENH): This should be base in 'inBindingCount'.
    VkDescriptorSetLayoutBinding bindings[VK_SHADER_DESCRIPTOR_COUNT];
    VkDescriptorPoolSize poolSizes[VK_SHADER_DESCRIPTOR_COUNT];

    Mem_Zero(bindings, sizeof(bindings));

    for (u32 i = 0; i < inBindingCount; ++i) {
        VkDescriptorSetLayoutBinding *binding = &bindings[i];
        binding->binding = i;
        binding->descriptorCount = 1;
        binding->descriptorType = inDescriptorTypes[i];
        binding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding->pImmutableSamplers = 0;

        poolSizes[i].type = inDescriptorTypes[i];
        poolSizes[i].descriptorCount = inMaxSetCount;
    }

    VkDescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCI.pNext = 0;
    layoutCI.flags = 0;
    layoutCI.bindingCount = inBindingCount;
    layoutCI.pBindings = bindings;

    VK_Check( vkCreateDescriptorSetLayout(gCtxGAPI.devices.logical, &layoutCI, VKAllocator, &retDescriptorManager->layout) );

    VkDescriptorPoolCreateInfo poolCI;
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.pNext = 0;
    poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolCI.poolSizeCount = inBindingCount;
    poolCI.pPoolSizes = poolSizes;
    poolCI.maxSets = inMaxSetCount;

    VK_Check( vkCreateDescriptorPool(gCtxGAPI.devices.logical, &poolCI, VKAllocator, &retDescriptorManager->pool) );

    return retDescriptorManager;
}

typedef struct {
    b8 hasDepthTest;
    CString vertexShaderFilePath;
    CString fragmentShaderFilePath;

    u32 descriptorTypeCount;
    VkDescriptorType* descriptorTypes;

    u32 vertexAttributeFormatCount;
    VkFormat* vertexAttributeFormats;

    b8 isWireframe;
    VkPrimitiveTopology topology;
    b8 shouldDrawBack;
} VK_Pipeline_Create_Args;

Private void VK_Graphics_Pipeline_Create(VK_Render_Pass* inRenderPass, VK_Pipeline_Create_Args* inArgs, VK_Pipeline* outPipeline) {
#if 0
    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.pImmutableSamplers = 0;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descSetLayoutCI;
    descSetLayoutCI.pNext = 0;
    descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutCI.flags = 0;
    descSetLayoutCI.bindingCount = 1;
    descSetLayoutCI.pBindings = &binding;

    VK_Check(vkCreateDescriptorSetLayout(g.ctxGAPI.devices.logical, &descSetLayoutCI, VKAllocator, &pipeline->descSetLayout));

    VkDescriptorPoolSize descPoolSize;
    descPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descPoolSize.descriptorCount = gCtxGAPI.swapchain.imageCount;

    VkDescriptorPoolCreateInfo descPoolCI;
    descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolCI.pNext = 0;
    descPoolCI.flags = 0;
    descPoolCI.poolSizeCount = 1;
    descPoolCI.pPoolSizes = &descPoolSize;
    descPoolCI.maxSets = gCtxGAPI.swapchain.imageCount;

    VK_Check(vkCreateDescriptorPool(gCtxGAPI.devices.logical, &descPoolCI, VKAllocator, &pipeline->descPool));
#endif

    // Descriptor set allocation.
    // TODO(JENH): Free list?.

    Shader vertex;
    Asset_Loader_Load_SPIR_V(inArgs->vertexShaderFilePath, &vertex);
    Shader fragment;
    Asset_Loader_Load_SPIR_V(inArgs->fragmentShaderFilePath, &fragment);

    size_t shaderSizes[] = {
        vertex.size,
        fragment.size,
    };

    u32* shaderCodes[] {
        (u32*)vertex.code,
        (u32*)fragment.code,
    };

    u32 maxDescriptorSets = VK_OBJECT_MAX_OBJECT_COUNT * 3;
    outPipeline->descriptorManager = VK_Descriptor_Manager_Create(inArgs->descriptorTypeCount, inArgs->descriptorTypes, maxDescriptorSets);

    for (u32 i = 0; i < ArrayCount(shaderSizes); ++i) {
        // TODO(JENH): The code memory MUST be alinged to a 4 byte boundary.
        Assert( ((u64)shaderCodes[i] & (4 - 1)) == 0 );

        VkShaderModuleCreateInfo shaderModuleCI = {0};
        shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCI.pNext = 0;
        shaderModuleCI.codeSize = shaderSizes[i];
        shaderModuleCI.pCode = shaderCodes[i];

        VK_Check( vkCreateShaderModule(gCtxGAPI.devices.logical, &shaderModuleCI, 0, &outPipeline->shaderModules[i]) );
    }

    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkPipelineShaderStageCreateInfo shaderStageCIs[2];

    for (u32 i = 0; i < ArrayCount(shaderStageCIs); ++i) {
        VkPipelineShaderStageCreateInfo* CI = &shaderStageCIs[i];

        CI->sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        CI->pNext  = 0;
        CI->flags  = 0;
        CI->stage  = stages[i];
        CI->module = outPipeline->shaderModules[i];
        CI->pName  = "main";
        CI->pSpecializationInfo = 0;
    }

#define ATTRIBUTE_COUNT 3
    VkVertexInputAttributeDescription attributes[ATTRIBUTE_COUNT];
    Assert( inArgs->vertexAttributeFormatCount <= ATTRIBUTE_COUNT );

    u32 offset = 0;

    for (u32 i = 0; i < inArgs->vertexAttributeFormatCount; ++i) {
        VkVertexInputAttributeDescription *attribute = &attributes[i];
        VkFormat format = inArgs->vertexAttributeFormats[i];

        attribute->binding  = 0;
        attribute->location = i;
        attribute->format   = format;
        attribute->offset   = offset;

        if ( format == VK_VERTEX_INPUT_FORMAT_F32X3 ) {
            offset += sizeof(f32x3);
        } else if ( format == VK_VERTEX_INPUT_FORMAT_F32X2 ) {
            offset += sizeof(f32x2);
        } NO_ELSE
    }

    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;
    bindingDescription.stride = offset;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // descriptor set layouts
    VkDescriptorSetLayout layouts[] = {
        outPipeline->descriptorManager->layout,
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {0};
    vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCI.vertexBindingDescriptionCount = 1;
    vertexInputStateCI.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCI.vertexAttributeDescriptionCount = inArgs->vertexAttributeFormatCount;
    vertexInputStateCI.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {0};
    inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCI.topology = inArgs->topology;
    inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    VkPipelineDynamicStateCreateInfo pDynamicStateCI = {0};
    pDynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pDynamicStateCI.dynamicStateCount = ArrayCount(dynamicStates);
    pDynamicStateCI.pDynamicStates = dynamicStates;

    VkPipelineViewportStateCreateInfo viewportStateCI = {0};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCI.viewportCount = 1;
    viewportStateCI.pViewports = 0;
    viewportStateCI.scissorCount = 1;
    viewportStateCI.pScissors = 0;

    VkPipelineRasterizationStateCreateInfo rasterizerCI = {0};
    rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCI.depthClampEnable = VK_FALSE;
    rasterizerCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCI.polygonMode = ( inArgs->isWireframe ) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizerCI.lineWidth = 5.0f;
    rasterizerCI.cullMode = ( inArgs->shouldDrawBack ) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

    // TODO(JENH): IF YOU ARE STRUCK SEE THIS!!!!
    // VK_FRONT_FACE_CLOCKWISE
    rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizerCI.depthBiasEnable = VK_FALSE;
    rasterizerCI.depthBiasConstantFactor = 0.0f;
    rasterizerCI.depthBiasClamp = 0.0f;
    rasterizerCI.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisamplingCI = {0};
    multisamplingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCI.sampleShadingEnable = VK_FALSE;
    multisamplingCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingCI.minSampleShading = 1.0f;
    multisamplingCI.pSampleMask = 0;
    multisamplingCI.alphaToCoverageEnable = VK_FALSE;
    multisamplingCI.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil;
    if ( inArgs->hasDepthTest ) {
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.pNext = 0;
        depthStencil.flags = 0;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {0};
        depthStencil.back = {0};
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 0.0f;
    } else {
        depthStencil = {0};
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachmentCI = {0};
    colorBlendAttachmentCI.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentCI.blendEnable = VK_TRUE;
    colorBlendAttachmentCI.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentCI.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentCI.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentCI.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentCI.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentCI.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendingCI = {0};
    colorBlendingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingCI.logicOpEnable = VK_FALSE;
    colorBlendingCI.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingCI.attachmentCount = 1;
    colorBlendingCI.pAttachments = &colorBlendAttachmentCI;
    colorBlendingCI.blendConstants[0] = 0.0f;
    colorBlendingCI.blendConstants[1] = 0.0f;
    colorBlendingCI.blendConstants[2] = 0.0f;
    colorBlendingCI.blendConstants[3] = 0.0f;

    VkPushConstantRange constantRange = {0};
    constantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    constantRange.offset = 0;
    constantRange.size   = sizeof(f32x4x4);

    VkPipelineLayoutCreateInfo pipelineLayoutCI = {0};
    pipelineLayoutCI.pNext = 0;
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.setLayoutCount = ArrayCount(layouts);
    pipelineLayoutCI.pSetLayouts = layouts;
    pipelineLayoutCI.pushConstantRangeCount = 1;
    pipelineLayoutCI.pPushConstantRanges = &constantRange;

    VK_Check( vkCreatePipelineLayout(gCtxGAPI.devices.logical, &pipelineLayoutCI, 0, &outPipeline->layout) );

    VkGraphicsPipelineCreateInfo pipelineCI = {0};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = 0;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStageCIs;
    pipelineCI.pVertexInputState = &vertexInputStateCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizerCI;
    pipelineCI.pMultisampleState = &multisamplingCI;
    pipelineCI.pDepthStencilState = ( inArgs->hasDepthTest ) ? &depthStencil : 0;
    pipelineCI.pColorBlendState = &colorBlendingCI;
    pipelineCI.pDynamicState = &pDynamicStateCI;
    pipelineCI.layout = outPipeline->layout;

    // TODO(JENH): See what do whit multiple render passes.
    outPipeline->renderPass = inRenderPass;
    pipelineCI.renderPass = outPipeline->renderPass->handle;

    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.basePipelineIndex = -1;

    VK_Check(vkCreateGraphicsPipelines(gCtxGAPI.devices.logical, VK_NULL_HANDLE, 1, &pipelineCI, 0, &outPipeline->handle));
}

Private void VK_Pipeline_Destroy(VK_Pipeline* inPipeline) {
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        Assert( inPipeline->shaderModules[i] != VK_NULL_HANDLE );
        vkDestroyShaderModule(gCtxGAPI.devices.logical, inPipeline->shaderModules[i], VKAllocator);
    }

    vkDestroyPipeline(gCtxGAPI.devices.logical, inPipeline->handle, VKAllocator);
    vkDestroyPipelineLayout(gCtxGAPI.devices.logical, inPipeline->layout, VKAllocator);
}

void VK_Init(HWND inWindow, HINSTANCE inInstance, s32x2 inWinDims) {
    Assert(sizeof(VK_Queue_Family_Indices) == sizeof(Field(VK_Queue_Family_Indices, E)));

    Memory_Arena *tempArena = AllocTempArena(KiB(256));

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Rhythym Car";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    CString extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifndef JENH_RELEASE
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    VkInstanceCreateInfo insCreateInfo = {0};
    insCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    insCreateInfo.pApplicationInfo = &appInfo;
    insCreateInfo.enabledExtensionCount = ArrayCount(extensions);
    insCreateInfo.ppEnabledExtensionNames = extensions;

#ifndef JENH_RELEASE
    AssertValidationLayers(tempArena);
    Arena_Clear(tempArena);

    insCreateInfo.enabledLayerCount = ArrayCount(layersVK);
    insCreateInfo.ppEnabledLayerNames = layersVK;
#endif

    VK_Check( vkCreateInstance(&insCreateInfo, 0, &gCtxGAPI.instance) );

    VkDebugUtilsMessengerCreateInfoEXT DUMcreateInfo = {0};
    DUMcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DUMcreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    DUMcreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DUMcreateInfo.pfnUserCallback = VK_Debug_Callback;
    DUMcreateInfo.pUserData = 0;

    VK_Check(VK_Debug_Utils_Messenger_EXT_Create(gCtxGAPI.instance, &DUMcreateInfo, 0, &gCtxGAPI.debugMessenger));

    VkWin32SurfaceCreateInfoKHR surfaceCI = {0};
    surfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

    // TODO(JENH): Problems with concurrency.
    surfaceCI.hwnd = inWindow;
    surfaceCI.hinstance = inInstance;

    VK_Check(vkCreateWin32SurfaceKHR(gCtxGAPI.instance, &surfaceCI, 0, &gCtxGAPI.swapchain.surface.handle));

    Array_VkPhysicalDevice physicalDevices;
    vkEnumeratePhysicalDevices(gCtxGAPI.instance, &physicalDevices.size, 0);

    physicalDevices.A = ArenaPushArray(tempArena, VkPhysicalDevice, physicalDevices.size);
    vkEnumeratePhysicalDevices(gCtxGAPI.instance, &physicalDevices.size, physicalDevices.A);

    foreach (VkPhysicalDevice, physicalDevice, physicalDevices) {
        if (VK_Device_Check(tempArena, *physicalDevice, gCtxGAPI.swapchain.surface.handle)) {
            gCtxGAPI.devices.physical = *physicalDevice;
            break;
        }

        Arena_Clear(tempArena);
    }

    Arena_Clear(tempArena);

    Assert(gCtxGAPI.devices.physical != VK_NULL_HANDLE);

    //NOTE(JENH): logical device
    Assert(VK_Device_Indices(tempArena, gCtxGAPI.devices.physical, &gCtxGAPI.devices.queues.familyIndices) == JENH_TRUE);

    Array_u32 uniqueFamilyIndices;
    uniqueFamilyIndices.size = 0;
    uniqueFamilyIndices.A = ArenaPushArray(tempArena, u32, ArrayCount(Field(VK_Queue_Family_Indices, E)));

    u32 uniqueIndex = 0;
    for (u32 i = 0; i < ArrayCount(Field(VK_Queue_Family_Indices, E)); ++i) {
        foreach (u32, uniquefamilyIndex, uniqueFamilyIndices) {
            if (gCtxGAPI.devices.queues.familyIndices.E[i] == *uniquefamilyIndex) {
                goto skip_index;
            }
        }

        uniqueFamilyIndices.A[uniqueFamilyIndices.size++] = gCtxGAPI.devices.queues.familyIndices.E[i];

skip_index:;
    }

    Array_VkDeviceQueueCreateInfo queuesCI;
    queuesCI.size = uniqueFamilyIndices.size;
    queuesCI.A = ArenaPushArray(tempArena, VkDeviceQueueCreateInfo, queuesCI.size);

    f32 queuePriority = 1.0f;
    for (u32 i = 0; i < queuesCI.size; ++i) {
        VkDeviceQueueCreateInfo *queueCI = &queuesCI.A[i];

        queueCI->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCI->pNext = 0;
        queueCI->flags = 0;
        queueCI->queueFamilyIndex = gCtxGAPI.devices.queues.familyIndices.E[i];
        queueCI->queueCount = 1;
        queueCI->pQueuePriorities = &queuePriority;
    }

    VkDeviceCreateInfo deviceCI = {0};
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pQueueCreateInfos = queuesCI.A;
    deviceCI.queueCreateInfoCount = queuesCI.size;

    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;

    deviceCI.pEnabledFeatures = &deviceFeatures;
    deviceCI.enabledExtensionCount = ArrayCount(deviceExtensions);
    deviceCI.ppEnabledExtensionNames = deviceExtensions;

    deviceCI.enabledLayerCount = ArrayCount(layersVK);
    deviceCI.ppEnabledLayerNames = layersVK;

    VK_Check(vkCreateDevice(gCtxGAPI.devices.physical, &deviceCI, 0, &gCtxGAPI.devices.logical));

    vkGetDeviceQueue(gCtxGAPI.devices.logical, gCtxGAPI.devices.queues.familyIndices.graphics,
                     0, &gCtxGAPI.devices.queues.graphics);
    vkGetDeviceQueue(gCtxGAPI.devices.logical, gCtxGAPI.devices.queues.familyIndices.present,
                     0, &gCtxGAPI.devices.queues.present);
    vkGetDeviceQueue(gCtxGAPI.devices.logical, gCtxGAPI.devices.queues.familyIndices.transfer,
                     0, &gCtxGAPI.devices.queues.transfer);

    VK_Swapchain_Create(inWinDims);

    VK_Queue_Family_Indices indices = gCtxGAPI.devices.queues.familyIndices;

    VkCommandPoolCreateInfo commandPoolCI = {0};
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCI.queueFamilyIndex = gCtxGAPI.devices.queues.familyIndices.graphics;

    VK_Check(vkCreateCommandPool(gCtxGAPI.devices.logical, &commandPoolCI, 0, &gCtxGAPI.cmdPool));

    VkCommandBufferAllocateInfo cmdBufferAllocCI = {0};
    cmdBufferAllocCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocCI.commandPool = gCtxGAPI.cmdPool;
    cmdBufferAllocCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocCI.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VK_Check(vkAllocateCommandBuffers(gCtxGAPI.devices.logical, &cmdBufferAllocCI, gCtxGAPI.cmdBuffers));

    // NOTE(JENH): Creating swap chain.
    CreateImageViews();

    CreateBuffers();

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (!VK_Buffer_Create_And_Bind(sizeof(Object_Uniform_Object) * VK_OBJECT_MAX_OBJECT_COUNT * 2,
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gCtxGAPI.uniformBuffers[i])) {
            LogError("Failed to create uniform buffer");
            return;
        }
    }

    u32x2 renderPassDims = U32x2(gCtxGAPI.swapchain.surface.dims.width, gCtxGAPI.swapchain.surface.dims.height);

    //VK_Render_Pass_Create(U32x2(0, 0), renderPassDims, F32x4(0.0f, 0.0f, 0.0f, 0.0f),
                          //1.0f, 0, RPCF_Has_Next_Pass, &gCtxGAPI.renderPassUI);

    // |RPCF_Has_Prev_Pass
    VK_Render_Pass_Create(U32x2(0, 0), renderPassDims, F32x4(0.0f, 0.0f, 0.2f, 1.0f), 1.0f, 0,
                          RPCF_Color_Buffer|RPCF_Depth_Buffer|RPCF_Stencil_Buffer, &gCtxGAPI.renderPassWorld);

    VkDescriptorType matDescriptorTypes[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkFormat matFormats[] = {
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X2,
    };

    VK_Pipeline_Create_Args materialArgs;
    materialArgs.hasDepthTest = JENH_TRUE;
    materialArgs.vertexShaderFilePath = "..\\bin\\shader.spv.vert";
    materialArgs.fragmentShaderFilePath = "..\\bin\\shader.spv.frag";
    materialArgs.descriptorTypeCount = ArrayCount(matDescriptorTypes);
    materialArgs.descriptorTypes = matDescriptorTypes;
    materialArgs.vertexAttributeFormatCount = ArrayCount(matFormats);
    materialArgs.vertexAttributeFormats = matFormats;
    materialArgs.isWireframe = JENH_FALSE;
    materialArgs.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    materialArgs.shouldDrawBack = JENH_FALSE;

    VK_Graphics_Pipeline_Create(&gCtxGAPI.renderPassWorld, &materialArgs, &gCtxGAPI.pipelineMaterial);
    gCtxGAPI.pipelineMaterial.adjustToSwapChain = JENH_FALSE;

    gCtxGAPI.pipelineMaterial.viewportPos.x = 0.0f;
    gCtxGAPI.pipelineMaterial.viewportPos.y = 1.0f;
    gCtxGAPI.pipelineMaterial.viewportDims.width = 1.0f;
    gCtxGAPI.pipelineMaterial.viewportDims.height = 1.0f;

    VkDescriptorType uiDescriptorTypes[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkFormat uiFormats[] = {
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X2,
    };

    VK_Pipeline_Create_Args uiArgs;
    uiArgs.hasDepthTest = JENH_TRUE;
    uiArgs.vertexShaderFilePath = "..\\bin\\ui.spv.vert";
    uiArgs.fragmentShaderFilePath = "..\\bin\\ui.spv.frag";
    uiArgs.descriptorTypeCount = ArrayCount(uiDescriptorTypes);
    uiArgs.descriptorTypes = uiDescriptorTypes;
    uiArgs.vertexAttributeFormatCount = ArrayCount(uiFormats);
    uiArgs.vertexAttributeFormats = uiFormats;
    uiArgs.isWireframe = JENH_FALSE;
    uiArgs.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    uiArgs.shouldDrawBack = JENH_FALSE;

    VK_Graphics_Pipeline_Create(&gCtxGAPI.renderPassWorld, &uiArgs, &gCtxGAPI.pipelineUI);
    gCtxGAPI.pipelineUI.adjustToSwapChain = JENH_TRUE;

    VkDescriptorType wireframeDescriptorTypes[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkFormat wireframeFormats[] = {
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X3,
        VK_VERTEX_INPUT_FORMAT_F32X2,
    };

    VK_Pipeline_Create_Args wireframeArgs;
    wireframeArgs.hasDepthTest = JENH_TRUE;
    wireframeArgs.vertexShaderFilePath = "..\\bin\\wireframe.spv.vert";
    wireframeArgs.fragmentShaderFilePath = "..\\bin\\wireframe.spv.frag";
    wireframeArgs.descriptorTypeCount = ArrayCount(wireframeDescriptorTypes);
    wireframeArgs.descriptorTypes = wireframeDescriptorTypes;
    wireframeArgs.vertexAttributeFormatCount = ArrayCount(wireframeFormats);
    wireframeArgs.vertexAttributeFormats = wireframeFormats;
    wireframeArgs.isWireframe = JENH_TRUE;
    wireframeArgs.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    wireframeArgs.shouldDrawBack = JENH_TRUE;

    VK_Graphics_Pipeline_Create(&gCtxGAPI.renderPassWorld, &wireframeArgs, &gCtxGAPI.wireframePipeline);
    gCtxGAPI.wireframePipeline.adjustToSwapChain = JENH_TRUE;

    CreateFrameBuffers();

    CreateSyncObjects();

    FreeTempArena(tempArena);
}

void VK_Cleanup() {
    vkDeviceWaitIdle(gCtxGAPI.devices.logical);

#ifndef JENH_RELEASE
    VK_Debug_Utils_Messenger_EXT_Destroy(gCtxGAPI.instance, gCtxGAPI.debugMessenger, VKAllocator);
#endif
    VK_Swapchain_Cleanup();

    VK_Pipeline_Destroy(&gCtxGAPI.pipelineMaterial);
    VK_Pipeline_Destroy(&gCtxGAPI.pipelineUI);

    for (u32 i = 0; i < gCtxGAPI.descriptorManagersCount; ++i) {
        VK_Descriptor_Manager* manager = &gCtxGAPI.descriptorManagers[i];

        vkDestroyDescriptorSetLayout(gCtxGAPI.devices.logical, manager->layout, VKAllocator);
        vkDestroyDescriptorPool(gCtxGAPI.devices.logical, manager->pool, VKAllocator);
    }

    for (u32 i = 0; i < ArrayCount(gCtxGAPI.uniformBuffers); ++i) {
        VK_Buffer_Destroy(&gCtxGAPI.uniformBuffers[i]);
    }

    vkDestroyRenderPass(gCtxGAPI.devices.logical, gCtxGAPI.renderPassWorld.handle, VKAllocator);
    //vkDestroyRenderPass(gCtxGAPI.devices.logical, .renderPassUI.handle, VKAllocator);

    vkDestroyBuffer(gCtxGAPI.devices.logical, gCtxGAPI.indexBuffer.buffer.handle, VKAllocator);
    vkFreeMemory(gCtxGAPI.devices.logical, gCtxGAPI.indexBuffer.buffer.deviceMemHandle, VKAllocator);

    vkDestroyBuffer(gCtxGAPI.devices.logical, gCtxGAPI.vertexBuffer.buffer.handle, VKAllocator);
    vkFreeMemory(gCtxGAPI.devices.logical, gCtxGAPI.vertexBuffer.buffer.deviceMemHandle, VKAllocator);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(gCtxGAPI.devices.logical, gCtxGAPI.imageAvailableSemaphores[i], VKAllocator);
        vkDestroySemaphore(gCtxGAPI.devices.logical, gCtxGAPI.renderFinishedSemaphores[i], VKAllocator);
        vkDestroyFence(gCtxGAPI.devices.logical, gCtxGAPI.inFlightFences[i], VKAllocator);
    }

    vkDestroyCommandPool(gCtxGAPI.devices.logical, gCtxGAPI.cmdPool, VKAllocator);

    vkDestroySurfaceKHR(gCtxGAPI.instance, gCtxGAPI.swapchain.surface.handle, VKAllocator);
    vkDestroyDevice(gCtxGAPI.devices.logical, VKAllocator);
    vkDestroyInstance(gCtxGAPI.instance, VKAllocator);
}
