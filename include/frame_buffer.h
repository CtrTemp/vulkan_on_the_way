#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

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
#include "image_view.h"
#include "render_passes.h"
#include "depth_buffer.h"

/*
    Introduction：
    虽然在前几个章节，尤其是Render Pass部分我们为Framebuffer相关的一些属性做了很详细的配置，比如color属性
depth属性，size大小适应swapchain中的image等等，但目前为止我们还没有真正创建一个framebuffer。
    在当前章节，我们需要做的第一步就是对Framebuffer进行创建。
*/

extern std::vector<VkFramebuffer> swapChainFramebuffers; // 声明交换链中的framebuffers

/**
 *  为交换链中的每一个 Image View 都要创建一个 framebuffer
 * */
void createFramebuffers();

/**
 *  销毁 framebuffer
 * */
void cleanupFramebuffer();

#endif
