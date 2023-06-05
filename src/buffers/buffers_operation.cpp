#include "buffers/buffers_operation.h"

/*
    第一步，
    由于我们涉及多个缓冲区的创建，现在最好是将这些缓冲区的创建分离开来，于是将以上 createVertexBuffer() 代码中
的部分放入下面的 creaetBuffer() 函数中。
    creaetBuffer() 函数的多个输入参数使其可以灵活创建多种用途的缓冲区，它是通用的。
*/
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

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

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

/*
    第三步，真正执行从“分段缓冲区”到“顶点缓冲区”拷贝数据的操作
    参数指明 source 和 destination 两个buffer，以及要拷贝的字节数（大小）

    需要注意的是，这种拷贝操作在Vulkan中也必须符合规范，我们要单独为其设置一个暂时的（生命周期在函数内）命令缓冲区。
通过提交这个命令，来达到拷贝操作的目的。
*/
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    /*
        创建好这个命令缓冲区后，我们将记录/填充这个缓冲区，下面的begininfo配置一些命令缓冲区提交时的一些设置，注意到
    它其实并不是一个真实的命令。
    */
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // 注意，以下的这个字段表示 我们只对该命令提交 一次
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // 以下开始创建这个具体的拷贝命令，输入一些具体参数（这才是真实的命令）
    VkBufferCopy copyRegion{};
    // copyRegion.srcOffset = 0; // Optional 表示src地址偏移（从哪里开始拷贝）
    // copyRegion.dstOffset = 0; // Optional 表示dst地址偏移（从哪里开始接收）
    copyRegion.size = size; // 总共要传输的字节数
    // 创建指令，并将其填充进 commandBuffer
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    // 以上创建完毕，可见这个命令缓冲区只包含一个缓冲区数据拷贝相关的命令
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

/*
    第三步：
    由于GPU可以提供给我们不同类型的内存分配。每种类型的内存在允许的操作和特性方面都有所不同。我们需要结合
缓冲区的需求和我们自己程序的需求来找到最合适的内存类型。以下函数的功用即在此：
*/
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // 首先要询问可选用的内存分配类型，存在一个 VkPhysicalDeviceMemoryProperties 类型的结构体中
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // 使用一个二进制的Fliter来筛出我们想要的那个type
        // 通过位操作，选出合适的type
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    // 如果都没有，则抛出错误
    throw std::runtime_error("failed to find suitable memory type!");
}

