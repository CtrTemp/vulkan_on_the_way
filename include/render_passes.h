#ifndef RENDER_PASSES_H
#define RENDER_PASSES_H


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

#include "logical_device_queue.h"
#include "image_view.h"
#include "swapchain.h"

/*
    Introduction：
    在完成渲染管道的创建之前，我们要告诉Vulkan在渲染时要使用的framebuffer组件，并对其进行配置。
这主要包括：指定需要多少颜色/深度缓冲区，每个缓冲区要使用多少个样本，以及在整个渲染过程中如何处理
它们的内容。
    以上的配置将封装在一个Render Pass中。注意，在创建图形管线之前，我们就要从initVulkan函数中
调用该创建RenderPass的函数。

    实际上这部分的核心就是配置 Framebuffer
*/


extern VkRenderPass renderPass;

void createRenderPass();
void cleanupRenderPass();



#endif