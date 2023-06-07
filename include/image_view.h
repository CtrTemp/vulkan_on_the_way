#ifndef IMAGE_H
#define IMAGE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

#include "logical_device_queue.h"
#include "swapchain.h"
#include "buffers/buffers_operation.h"

/*
    Beief Introduction：
    为了使用包括在交换链中的任何的VkImage，在整个渲染管线中，我们需要创建一个VkImageView对象。
该对象将描述如何访问图像以及要访问图像的哪些部分。例如，如果应该将其视为2D深度纹理，mipmap就不会被使用。
*/

// 每个swap chain中的Image都要对应一个ImageView
extern std::vector<VkImageView> swapChainImageViews;

/**
 *  ImageView相关实例的创建
 * */
void createImageViews();

/**
 *  创建单一图像实例
 * */
void createImage(uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties,
                 VkImage &image,
                 VkDeviceMemory &imageMemory);

/**
 *  创建单一ImageView实例
 * */
VkImageView createImageView(VkImage image, VkFormat format);

/**
 *  ImageView相关实例的销毁
 * */
void cleanupImageView();

#endif
