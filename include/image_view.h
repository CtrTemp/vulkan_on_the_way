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



extern std::vector<VkImage> swapChainImages; // 声明用于填入交换链的Image
extern VkFormat swapChainImageFormat;

// 如 Introduction 部分所说，首先添加一个 VkImageView 类型 vector 的成员变量
extern std::vector<VkImageView> swapChainImageViews;




void createImageViews();
void cleanupImageView();



#endif
