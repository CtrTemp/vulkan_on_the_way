#ifndef DEPTH_STENCIL_H
#define DEPTH_STENCIL_H


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


void configure_depth_stencil(VkPipelineDepthStencilStateCreateInfo &depthStencil);


#endif