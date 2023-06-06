#include "buffers/buffers_operation.h"

/**
 *  在device上创建一个特定功用的buffer，并为其分配合适类型的内存空间
 * */
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    // 在 GPU device 上创建一个 buffer 实例
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    // 为以上的 buffer 实例申请GPU内存空间
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 将刚刚申请到的内存空间分配给创建的buffer实例（让二者绑定到一起）
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

/**
 *  GPU上的buffer数据拷贝（device to device）
 *  注意，数据拷贝在vulkan中也是通过命令上传命令队列来实现的，好在graphic queue一般都支持这个指令，所以
 * 我们不需要额外为其创建队列。
 * */
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    // 创建一个 command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // 开始对command buffer进行命令填充
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 这个命令队列只提交一次
    // 开始填充
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // 在command buffer中加入拷贝命令
    VkBufferCopy copyRegion{};
    // copyRegion.srcOffset = 0; // Optional 表示src地址偏移（从哪里开始拷贝）
    // copyRegion.dstOffset = 0; // Optional 表示dst地址偏移（从哪里开始接收）
    copyRegion.size = size; // 总共要传输的字节数
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // 结束填充命令
    vkEndCommandBuffer(commandBuffer);

    // 下面创建提交相关的指令
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;           // 我们就单次提交一个命令缓冲区
    submitInfo.pCommandBuffers = &commandBuffer; // 提交哪个
    // 向图形队列提交这个命令
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    /*
        以下的这个函数相当于一个barrier，必须等待这个图形队列中所有指令执行完毕才允许程序返回。这里可能是为了保证我们
    的具体数据已经正确传输，从而使得之后对数据的读取/绘制能够正常进行。
    */
    vkQueueWaitIdle(graphicsQueue);

    // 注意，由于我们创建的是一个“临时的”命令提交，所以在函数末尾记得将其free掉（这里就不用在cleanup函数中写了）
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

/**
 *  找出当前buffer最合适的分配内存类型
 * */
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
