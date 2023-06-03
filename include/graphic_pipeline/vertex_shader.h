#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

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
 *  自定义 vertex shader 配置变量
 * */ 
VkShaderModule configure_vertex_shader(VkPipelineShaderStageCreateInfo &vertShaderStageInfo);


#endif