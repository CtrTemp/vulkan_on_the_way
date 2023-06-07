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
#include "depth_buffer.h"

/*
    Introduction：
    在完成渲染管道的创建之前，我们要告诉Vulkan在渲染时要使用的framebuffer组件，并对其进行配置。
这主要包括：指定需要多少颜色/深度缓冲区，每个缓冲区要使用多少个样本，以及在整个渲染过程中如何处理
它们的内容。
    以上的配置将封装在一个Render Pass中。注意，在创建图形管线之前，我们就要从initVulkan函数中
调用该创建RenderPass的函数。

    实际上这部分的核心就是配置 Framebuffer。

    Render Pass 字面意思来理解就是渲染过程，该部分就是对渲染过程的配置。但注意，并不是对pipeline
的配置。主要就是对Framebuffer的配置，包括：

    关于 loadOp 字段：它表示渲染一张新的图片之前，framebuffer应该是一个怎样的状态？

        VK_ATTACHMENT_LOAD_OP_LOAD： framebuffer 中现有内容（应该是上一帧的内容）将会被保留；
        VK_ATTACHMENT_LOAD_OP_CLEAR：在开始呈现下一张图像前， framebuffer 现有值将会被清除为常量
    （但这个常量值是什么呢？下面的描述说明说将会被置为“黑色”，那么这个常量值应该就是0）；
        VK_ATTACHMENT_LOAD_OP_DONT_CARE： framebuffer 中的值会被置为一个“未定义”的状态（不确定状
    态，并且我们也不在乎其中的值）

    关于 storeOp 字段：它表示在渲染一张图片之后，framebuffer应该会被置为什么状态？

        VK_ATTACHMENT_STORE_OP_STORE： framebuffer 中的内容将存储在内存中，稍后可以读取
        VK_ATTACHMENT_STORE_OP_DONT_CARE： framebuffer 中的内容将被置为一个“未定义”状态

    initialLayout 字段：指示渲染过程（Render Pass）开始之前的图像布局？
    finalLayout 字段指示的是渲染过程（Render Pass）完成之后，图像自动转换到的布局？

        VK_IMAGE_LAYOUT_UNDEFINED 表示我们并不关心图像在之前的布局是什么样子
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR 进行设置，表示我们希望图像在渲染之后可以使用交换链进行直接显示

    Subpass部分暂时不展开讨论

*/

extern VkRenderPass renderPass; // 声明 render pass 实例

/**
 *  创建 Render Pass
 */
void createRenderPass();

/**
 *  注销 Render Pass
 */
void cleanupRenderPass();

#endif