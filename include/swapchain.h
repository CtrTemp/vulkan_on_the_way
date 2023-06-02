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

extern VkSwapchainKHR swapChain; // 声明交换链对象

/**
 *  交换链swap chain在vulkan中充当着类似于framebuffer以及其控件的作用，这在physcial device创建核验
 * 的过程中已经进行过介绍，在此不再赘述。
 *
 * */

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

/**
 *  swap chain 实例的创建
 * */
void createSwapChain();

/**
 *  swap chain 实例的销毁
 * */
void cleanupSwapChain();

#endif
