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
}

/**
 *  实例对象的析构销毁，基本遵循先创建的后销毁，后创建的先销毁
 * */
void cleanupVulkan()
{
    cleanupImageView();

    cleanupSwapChain();

    logicalDeviceCleanup();

    surfaceCleanUp();

    vkInstanceCleanUp();

    windowCleanup();
}
