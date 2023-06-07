#include "command_buffer.h"

VkCommandPool commandPool; // 命令池实例，用于管理命令缓冲区的内容

std::vector<VkCommandBuffer> commandBuffers; // 命令缓冲区实例

/**
 *  创建 command pool
 * */
void createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    /*
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT： 所有的command buffer，来了新指令之后一并刷新。
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT： 允许单独刷新某个command buffer。
        由于我们允许CPU领先GPU提交多任务，在一个command pool中设置了多个command buffer，所以显然后者是更合适的选择。
    */
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // 将 command pool 绑定到图形指令集队列，表示内部存储的指令都将被提交到图形队列
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void createCommandBuffer()
{
    /**
     *  由于可以不等待GPU任务执行返回，而向GPU提交超额的任务，这里command buffer的数量可以自定义：由当前CPU可以超额提交
     * 给GPU的最大任务量决定（为每一个任务都配备一个专门的command buffer可能也是为了防止command buffer使用冲突）
     * */
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    /*
        level 字段用于指示分配的命令缓冲区是“主命令缓冲区”还是“次级命令缓冲区”：
        VK_COMMAND_BUFFER_LEVEL_PRIMARY： 如果是“主命令缓冲区”，则可以直接提交到队列执行，但其他命令
    缓冲区无法调用（可以说是当前管线/队列所独占的）
        VK_COMMAND_BUFFER_LEVEL_SECONDARY： 如果是“次级命令缓冲区”，它无法直接提交到队列进行执行，但
    可以通过“主命令缓冲区”进行调用。
        在这里每个command buffer都完全负责渲染一帧图像的所有流程，所以只能是选择“PRIMARY”。
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    // 一次创建多个 command buffer
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

/**
 *  更新/填充command buffer
 *  draw time 函数，在渲染过程中更新
 * */
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // 开始command buffer填充，如果 command buffer 是一个已经被填充的状态，则原来的状态将先被清空/重置
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    /*
        第六步，从这里开始真正的渲染过程Render Pass
    */
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // 指定 render pass
    renderPassInfo.renderPass = renderPass;
    // 指定 framebuffer 要绑定的交换链中的图片
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    // 指定了渲染范围，分别是偏移量以及图像大小
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    // 指定开始渲染前framebuffer中的像素颜色，以下设置为全黑
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    /**
     * 填充指令1：启动RenderPass
     */
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    /**
     * 填充指令2：绑定graphic pipeline
     */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    /**
     * 填充指令3：设置视口大小
     */
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    /**
     * 填充指令4：设置视口截取尺寸
     */
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    /**
     * 填充指令5：绑定 vertex buffer（这将作为顶点数据源传入graphic pipeline）
     */
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    /**
     * 填充指令6：绑定 index buffer（这将作为顶点索引数据源传入graphic pipeline）
     */
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    /**
     * 填充指令7：绑定描述符，这将每帧更新的MVP变换阵以pipelineLayout作为接口传入graphic pipeline
     * 进行修改更新，并将更新后的描述符作用于每个顶点
     */
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0,
                            1,
                            &descriptorSets[currentFrame],
                            0,
                            nullptr);

    /**
     * 填充指令8：开始渲染，如果没有使用index bufer，则使用vkCmdDraw命令进行填充，否则使用vkCmdDrawIndexed
     * 命令进行填充，第二参数为要绘制的顶点数量，由于vertex buffer原数组中有顶点复用，而这里我们需要未复用的总数量，
     * 于是使用index buffer原数组的长度作为输入值。
     */
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    /**
     * 填充指令9：结束RenderPass
     */
    vkCmdEndRenderPass(commandBuffer);

    // 结束 command buffer 填充
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

/**
 *  创建一个用于提交单次指令的 command buffer
 * */
VkCommandBuffer beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

/**
 *  结束一个 用于提交单次指令的 command buffer 的填充，并将其送入 graphic queue 进行执行
 * */
void endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void cleanupCommandPool()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
}
