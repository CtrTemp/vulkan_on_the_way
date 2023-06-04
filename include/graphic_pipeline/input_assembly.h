#ifndef INPUT_ASSEMBLY_H
#define INPUT_ASSEMBLY_H


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
 *  配置输入“装配器”：描述按照何种规则将顶点连成几何
 * */ 
void configure_input_assembly(VkPipelineInputAssemblyStateCreateInfo &inputAssembly);

#endif