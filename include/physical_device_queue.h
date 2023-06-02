#ifndef DEVICE_QUEUE_H
#define DEVICE_QUEUE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <optional>
// 容器
#include <string>
#include <vector>

#include <cstring>

#include "surface.h"

// 验证/获取指令集队列的数据结构体
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// 验证/获取交换链信息的数据结构体
// 类似于 findQueueFamilies 函数的形式，我们使用一个结构体记录/传递这些细节（一旦被查验，我们通过结构体传参）
// 我们在“第三步”中所列写的几种所要查验的属性均被一一对应在如下结构体中
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

extern VkPhysicalDevice physicalDevice;                  // 物理设备对象
extern QueueFamilyIndices queueIndices;                  // 指令集队列集合对象
extern SwapChainSupportDetails swapChainDetails;         // 支持交换链具体细节对象
extern const std::vector<const char *> deviceExtensions; // 必要的设备扩展支持

void pickPhysicalDevice(VkInstance &instance);
bool isDeviceSuitable(VkPhysicalDevice device);



/******************************** 以下是物理设备关于 必要指令集队列 支持的验证 ********************************/ 
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);


/******************************** 以下是物理设备关于 swap chain 特性支持的验证 ********************************/ 
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

#endif
