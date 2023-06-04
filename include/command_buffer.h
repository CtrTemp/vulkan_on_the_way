#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

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
#include "swapchain.h"
#include "render_passes.h"
#include "graphic_pipeline.h"
#include "frame_buffer.h"

/*
    Introduction：
    注意，绘制操作在Vulkan中并不是通过函数调用的方式来实现的，而是通过将全部的绘制相关命令放入一个叫做
“命令缓冲区”（command buffer）的地方，一次性执行这些所有的命令。好处就是，一旦我们确定了要执行哪些操作，
这些操作会一次性被提交并执行，内部的优化会带来很好的效率提升
    这时你之前所配置的指令集队列就派上用场了？
*/

extern VkCommandPool commandPool;

// extern VkCommandBuffer commandBuffer;
extern std::vector<VkCommandBuffer> commandBuffers;

/**
 *  创建命令池
 * */
void createCommandPool();

/**
 *  创建命令缓冲区
 * */
void createCommandBuffer();
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void cleanupCommandPool();

#endif
