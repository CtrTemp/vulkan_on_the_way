#include "init_vk.h"

/**
 *  当前vulkan图形工程总体的初始化配置，对一些渲染管线中必要的实例对象的创建
 * */
void initVulkan()
{
    // // 在创建 instance 之前可以先查看以下支持的扩展，并打印输出（这只是一个罗列查看，去掉也无妨）
    // checkExtension();

    createInstance(); // 创建一个 instance 它将作为你的应用与vulkan库之间的桥梁

    setupDebugMessenger(instance); //  启用验证层

    createSurface(); // 创建界面实例

    pickPhysicalDevice(); // 选择适用的物理设备（GPU Card）

    createLogicalDevice(); // 将物理设备映射到逻辑设备，创建逻辑设备实例

    createSwapChain(); // 创建交换链

    createImageViews(); // 创建配置要填充在交换链中图像实例

    createRenderPass(); // 创建渲染流

    // 对描述符池进行销毁。注意不必对描述符集进行销毁，当描述符池被销毁后它会被自动释放
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    createDescriptorSetLayout(); // 创建描述符区

    createGraphicsPipeline(); // 创建渲染图形管线

    createFramebuffers(); // 创建帧缓冲区

    createCommandPool(); // 创建命令池

    createTextureImage(); // 创建纹理贴图

    createTextureImageView(); // 为纹理图创建ImageView

    createTextureSampler(); // 创建纹理采样器

    createVertexBuffer(); // 创建顶点缓冲区

    createIndexBuffer(); // 创建索引缓冲区

    createUniformBuffers(); // 创建“统一”缓冲区

    createDescriptorPool(); // 创建描述符池
    createDescriptorSets(); // 创建描述符集合

    createCommandBuffer(); // 创建命令缓冲区

    createSyncObjects(); // 创建绘制循环中的流控制原语
}

/**
 *  实例对象的析构销毁，基本遵循先创建的后销毁，后创建的先销毁
 * */
void cleanupVulkan()
{
    cleanupSwapChain();

    cleanupTextureRelated();

    cleanupGraphicPipeline();

    cleanupRenderPass();

    cleanupUniformBuffer();

    cleanupDescriptor();

    cleanupIndexBuffer();

    cleanupVertexBuffer();

    cleanupRenderLoopRelated();

    cleanupCommandPool();

    cleanupFramebuffer();

    cleanupImageView();

    logicalDeviceCleanup();

    surfaceCleanUp();

    vkInstanceCleanUp();

    windowCleanup();
}
