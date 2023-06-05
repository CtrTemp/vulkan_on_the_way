#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

#include <cstdint>   // Necessary for uint32_t
#include <limits>    // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include "init_window.h"
#include "surface.h"
#include "physical_device_queue.h"
#include "logical_device_queue.h"


/*
    Brief Introduction：
    由于Vulkan不提供默认的 framebuffer，所以在将结果渲染到屏幕前，我们必须申请创建一个基础设施，这个基础设施将
在屏幕可视化结果之前，将计算结果放入framebuffer（帧缓冲区），我们称这个“基础设施”为交换链（Swap Chain）。
    该设施必须被明确创建。它其实是一个队列，队列中存放着已经计算好的结果，这些结果将被依次放入framebuffer，并渲染
到屏幕空间。
    这个队列应该怎样工作，一张图片应该在何种条件下被展示到屏幕，这都有赖于我们如何去设置这个Swap Chain。但交换链
的一般目的是进行帧同步，使得图像的呈现与屏幕的刷新率同步。
    交换链swap chain在vulkan中充当着类似于framebuffer以及其控件的作用，这在physcial device创建核验的过程中
已经进行过介绍，在此不再赘述。
*/



extern VkSwapchainKHR swapChain; // 声明交换链对象

/**
 *  01：为swap chain选择合适的界面展示格式--像素RGB排列顺序、使用哪种色域
 *  这里我们认为format为 VK_FORMAT_B8G8R8A8_SRGB 且 colorSpace为 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 最优
 * 如果以上不满足，则从待选项中选择第一个即可。（有最优解选最优解，没有就随机挑一个）
 *
 * */
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
extern VkSurfaceFormatKHR swapChainSurfaceFormat;

/**
 *  02：为swap chain设置展示策略，即：何种情况下将队列中渲染好的图像“交换”到屏幕进行显示
 *  具体选择详见函数定义
 * */
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
extern VkPresentModeKHR swapChainPresentMode;

/**
 *  03：交换范围
 *  窗口设置，当窗口“以屏幕坐标定义的长宽”和“以像素定义的长宽”不匹配的时候，使用到该配置。
 *  该部分可以理解为一个跨平台的自适应屏幕分辨率自动匹配。（具体没有太理解）
 * */
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
extern VkExtent2D swapChainExtent;





// 声明用于填入swap chain的Image
extern std::vector<VkImage> swapChainImages;

// 声明用于填充swap chain的Image的格式（这个与之前配置的swap chain中的格式保持一致即可）
extern VkFormat swapChainImageFormat;



/**
 *  swap chain 实例的创建
 * */
void createSwapChain();

/**
 *  swap chain 实例的销毁
 * */
void cleanupSwapChain();

#endif
