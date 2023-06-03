#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

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

#include "graphic_pipeline/utils.h"


/**
 *  自定义 fragment shader 配置变量
 * */ 
VkShaderModule configure_fragment_shader(VkPipelineShaderStageCreateInfo &fragShaderStageInfo);


#endif