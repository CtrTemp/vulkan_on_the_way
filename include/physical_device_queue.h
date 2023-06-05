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

#include "vk_instance.h"
#include "surface.h"

/**
 *  图形卡的选取以及核验
 * */
void pickPhysicalDevice();

/**
 *  GPU核验评估
 * */
bool isDeviceSuitable(VkPhysicalDevice device);
extern VkPhysicalDevice physicalDevice; // 声明 物理设备对象

/**
 *  验证指标 01 ：
 *
 *  一块物理设备（GPU Card）可能支持不同的指令集合，这些集合被归纳为几个类，每个类对应一个指令队列
 * 一个队列只能处理一类指令集（当然队列与队列之间处理的指令集可能有指令重合）。
 *
 *  我们需要保证我们选取的设备至少有支持“图形绘制指令集”和“图形展示指令集”的队列各一个。
 *
 *  以下结构体记录所需两个队列具体的index。
 *
 *  在程序中我们将对设备进行验证，使用 findQueueFamilies 函数将具体的信息填充到下面声明的 queueIndices
 * 全局对象中，以供后面的程序进行验证操作。
 *
 *  当 graphicsFamily 和 presentFamily 都有值时，说明当前图形卡可以找到满足二者的指令集队列至少
 * 各一个（可以重合），那么我们认为它在命令队列这方面是“完备的”(complete)。
 *
 *  该指标是评价物理设备合格的必要条件（因为我们当前的图形应用就是要求既可以绘制又可以展示，所以二者
 * 不可或缺）。
 * */
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

extern QueueFamilyIndices queueIndices; // 声明 指令集队列集合对象

/**
 * 01：核验GPU是否同时具有支持“图形绘制”和“图形展示”的指令集队列
 * */
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

/**
 *  验证指标 02 ：
 *
 *  swap chain可以理解为vulkan提供的framebuffer以及对framebuffer的控件的集合。我们将渲染好的图像输入到
 * swap chain中，它将负责对整个流进行控制，并以某种我们设置的方式输出到屏幕空间。
 *
 *  可以看到swap chain对于我们当前所要做的这个图形应用来说是不可或缺的。所以我们需要核验它是否被我们当前的物理
 * 设备（GPU Card）所支持。
 *
 *  我们将所需的设备扩展定义在如下声明的 deviceExtensions 数组中，使用以下的 checkDeviceExtensionSupport
 * 函数进行验证，若物理设备所支持的扩展可以完全覆盖我们在数组中填充的所有需求（这里我们只需要swap chain），则认为
 * 当前物理设备在扩展支持方面是合格的。
 *
 *  这是物理设备合格的必要条件。
 *
 * */
extern const std::vector<const char *> deviceExtensions; // 声明 必要的设备扩展支持
/**
 *  02：验证GPU对 swap chain 的支持
 * */
bool checkDeviceExtensionSupport(VkPhysicalDevice device);

/**
 *  验证指标 03 ：
 *
 *  物理设备支持swap chain扩展仍然不足以说明其完全可用。仍需要对swap chain所支持的一些细节进行考察：
 *  以下使用 SwapChainSupportDetails 结构体定义的 swapChainDetails 全局对象保存当前物理设备所支持的
 * swap chain的一些细节，该对象在 querySwapChainSupport 函数中得到填充。
 * */

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

extern SwapChainSupportDetails swapChainDetails; // 声明 支持交换链具体细节对象
/**
 *  03：验证 swap chain 特性是否满足要求
 * */
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

#endif
