#ifndef DYNAMIC_STATE_H
#define DYNAMIC_STATE_H

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

extern std::vector<VkDynamicState> dynamicStates;

/**
 *  配置 graphic pipeline 确定后，还可进行动态编辑的状态列表
 * */ 
void configure_dynamic_state(VkPipelineDynamicStateCreateInfo &dynamicState);

#endif