#ifndef MULTI_SAMPLING_H
#define MULTI_SAMPLING_H

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
 *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
 * */
void configure_multi_sampling(VkPipelineMultisampleStateCreateInfo &multisampling);

#endif