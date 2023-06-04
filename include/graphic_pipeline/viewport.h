#ifndef VIEWPORT_H
#define VIEWPORT_H

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

#include "swapchain.h"

/**
 *  配置视口以及视口截取：得到的image将展示到多大的窗口上，剪裁尺寸是多少
 * */ 
void configure_viewport(VkPipelineViewportStateCreateInfo &viewportState);

#endif