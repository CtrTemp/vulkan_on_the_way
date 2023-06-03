#include "command_buffer.h"

/*
    第一步，创建“命令池”（command pool）
    在创建命令缓冲区（command buffer）之前，必须创建一个命令池。它用于管理存储缓冲区的内存，并从中
分配出命令缓冲区。在此我们添加入这个全局变量。
*/
VkCommandPool commandPool;

/*
    第三步，创建“命令缓冲区”（command buffer）
    在这一步中，我们依赖命令池创建命令缓冲区。
    注意，命令池销毁后，命令缓冲区将会被自动销毁，所以不需要我们去手动销毁
    以下添加这个全局变量
*/
VkCommandBuffer commandBuffer;

// 第二步，创建 command pool
void createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    /*
        flags 字段有两个可配置选项：
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT： 命令缓冲区常常被新命令重新记录（应该就是完全reset掉）
    这种操作很可能会改变内存。
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT： 允许单独重新记录命令缓冲区，无此标志则
    必须一同重置（reset）命令缓冲区。
        （这部分没有很理解）
        因为我们要在每一帧都记录一个命令缓冲区，所以希望可以对齐进行重置和单独重新记录，所以我们使用后者。
    */
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    /*
        命令缓冲区中的命令也是通过在设备队列上提交来执行的。每个命令池只能在单一队列中进行提交，我们这里将记
    录的是绘制命令，所以我们选择将其提交到图形队列中。
    */
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    // 根据以上结构体配置，创建命令池
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

// 第四步，创建command buffer
void createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    /*
        level 字段用于指示分配的命令缓冲区是“主命令缓冲区”还是“次级命令缓冲区”，有如下可选配置：
        VK_COMMAND_BUFFER_LEVEL_PRIMARY： 如果是“主命令缓冲区”，则可以直接提交到队列执行，但其他命令
    缓冲区无法调用（可以说是当前管线/队列所独占的）
        VK_COMMAND_BUFFER_LEVEL_SECONDARY： 如果是“次级命令缓冲区”，它无法直接提交到队列进行执行，但
    可以通过“主命令缓冲区”进行调用。
        这里我们不使用次级命令缓冲区，故配置如下
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // 因为没有“次级命令缓冲区”，故这里配置为1（只有一个“主命令缓冲区”）
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

/*
    第五步，记录命令缓冲区？？？（record 这个词这样翻译好像不太好）
    其实际作用是将要执行的命令写入命令缓冲区。
    当前还没有实际用到该函数
*/
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // /*
    //     其中flags字段有如下可配置选项：
    //     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：命令缓冲区将在一次执行后立即重新记录。（应该
    // 可以理解为执行完就清除，而后等待下一次的命令输入来填充队列）
    //     VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：辅助命令缓冲区，完全位于单个渲染过程中。
    // （目前没有使用辅助命令缓冲区，，，也不太好理解这个）
    //     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT：允许在命令缓冲区已经挂起的状态下重新提交。
    //     （但教程中说这些配置现在都不太适合我们，所以先不写，以下配置为一个无效的0）
    // */
    // beginInfo.flags = 0; // Optional
    // // pInheritanceInfo字段也是只与“次级命令缓冲区”相关，这里也可以先不管
    // beginInfo.pInheritanceInfo = nullptr; // Optional

    // // vkBeginCommandBuffer 函数的调用将隐式重置命令缓冲区（如果命令缓冲区已经被记录/填充过一次后）
    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    // /*
    //     第六步，从这里开始真正的渲染过程Render Pass
    // */
    // VkRenderPassBeginInfo renderPassInfo{};
    // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // // 指定 render pass
    // renderPassInfo.renderPass = renderPass;
    // // 指定 framebuffer 要绑定的交换链中的图片
    // renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    // // 以下两个参数指定了渲染范围，分别是偏移量以及图像大小（应该与交换链中定义的相适应）
    // renderPassInfo.renderArea.offset = {0, 0};
    // renderPassInfo.renderArea.extent = swapChainExtent;

    // VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    // renderPassInfo.clearValueCount = 1;
    // /*
    //     pClearValues字段对应的是 Render Pass创建过程中 loadOp 对应的 VK_ATTACHMENT_LOAD_OP_CLEAR
    // 参数，当图像渲染前，framebuffer中每个位置的像素颜色应该被置为常量，那么这个常量值是什么，在这里定义，如
    // 下被设置为纯黑色，透明度为1（完全不透明）
    // */
    // renderPassInfo.pClearValues = &clearColor;

    // /*
    //     根据以上的设置，现在可以开始渲染过程，注意，所有记录命令的函数都使用vkCmd作为前缀，很好识别。
    //     参数详解：
    //     参数1：指定命令缓冲区
    //     参数2：指定渲染过程的详细信息
    //     参数3：控制如何提供渲染过程中的绘图指令，有以下的可选项：
    //     VK_SUBPASS_CONTENTS_INLINE： 渲染传递的命令将嵌入/填充到主命令缓冲区中，并且不会执行次级命令缓
    // 冲区中的命令。
    //     VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS： 渲染过程中的命令将从次级命令缓冲区中执行。
    //     当前我们不使用次级命令缓冲区，所以选用前者。
    // */
    // vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // /*
    //     第七步，将基本的绘制命令绑定到渲染管线中
    //     以下其中第二个参数指明当前要绑定的管道类型，有以下两种：
    //     VK_PIPELINE_BIND_POINT_COMPUTE：计算管线
    //     VK_PIPELINE_BIND_POINT_GRAPHICS：图形管线
    //     由于当前我们的绘制命令要传入的是图形管线，所以选择后者。
    // */
    // vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    // /*
    //     vkCmdDraw参数解释：
    //     参数1：指定命令缓冲区
    //     参数2：要绘制的顶点数量，我们硬编码在shader中，就是要绘制一个三个顶点组成的三角形，所以这里是3
    //     参数3：用于实例化渲染，我们还不需要这样做，所以这里选择1。
    //     参数4：顶点缓冲区的偏移量，定义顶点缓冲区的索引值从哪里开始计数（定义gl_VertexIndex最小值）
    //     参数5：实例化渲染的偏移量，同样是索引计数起点（定义gl_InstanceIndex最小值）
    // */
    // vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // // 结束渲染过程
    // vkCmdEndRenderPass(commandBuffer);
    // // 结束命令缓冲区的记录/填充
    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
    // // 下一章节开始真正编写主循环中的过程，开始真正渲染
}

void cleanupCommandPool()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
}