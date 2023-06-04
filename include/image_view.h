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


/**
 *  ImageView相关实例的创建
 * */
void createImageViews();

/**
 *  ImageView相关实例的销毁
 * */
void cleanupImageView();

#endif
