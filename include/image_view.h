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

// 声明用于填入swap chain的Image
extern std::vector<VkImage> swapChainImages;

// 声明用于填充swap chain的Image的格式（这个与之前配置的swap chain中的格式保持一致即可）
extern VkFormat swapChainImageFormat;

// 每个swap chain中的Image都要对应一个ImageView
extern std::vector<VkImageView> swapChainImageViews;

/**
 *  ImageView相关实例的创建
 * */
void createImageViews();

/**
 *  ImageView相关实例的销毁
 * */
void cleanupImageView();

#endif
