#ifndef COLOR_BLENDING_H
#define COLOR_BLENDING_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>


/**
 *  配置颜色混合：如何从fragment shader中得到的fragment值得到最终pixel的颜色值
 * */ 
void configure_color_blending(VkPipelineColorBlendStateCreateInfo &colorBlending);

#endif