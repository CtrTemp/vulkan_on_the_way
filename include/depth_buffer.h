#ifndef DEPTH_BUFFER_H
#define DEPTH_BUFFER_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
/*
    第四步，首先在头文件引入时添加如下宏定义 GLM_FORCE_DEPTH_ZERO_TO_ONE，默认情况下，GLM生
成的矩阵将使用-1.0～1.0的OpenGL默认深度范围，在Vulkan中，我们就使用Vulkan风格的0～1，所以添加
这个宏定义。
*/
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "physical_device_queue.h"
#include "image_view.h"

extern VkImage depthImage;
extern VkDeviceMemory depthImageMemory;
extern VkImageView depthImageView;


void createDepthResources();
VkFormat findDepthFormat();



#endif
