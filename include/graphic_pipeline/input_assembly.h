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

void configure_input_assembly(VkPipelineInputAssemblyStateCreateInfo &inputAssembly);


#endif