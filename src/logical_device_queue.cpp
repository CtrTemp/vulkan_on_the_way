#include "logical_device_queue.h"

VkDevice device; // 逻辑设备，物理设备的映射，一个物理设备可以映射到多个逻辑设备

VkQueue graphicsQueue; // 理解？？？

// 逻辑设备创建
void createLogicalDevice()
{
    // 队列信息返回，这里我们使用的是物理设备对应的property队列信息
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    // 在这里我们只需要关注最其基本的图形相关的功能即可，所以我们只选择其中的一个队列
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    // Vulkan使用一个 0.0～1.0 的浮点数来为队列分配优先级，从而影响命令缓冲区执行的调度
    // 注意，即使你只有一个队列，这里为其分配优先级也是必须的
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // 接下来定义一组设备功能（不过目前我们还不需要对其进行配置，先对其留空）
    VkPhysicalDeviceFeatures deviceFeatures{};


    // 开始创建逻辑设备
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) // 如果使能了验证层，则需要将其插入到逻辑设备的创建中，以便对其进行监测
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // 这个函数将初始化文件全局变量 graphicsQueue
    // 输入参数依次为 逻辑设备/队列系列/队列索引/指向存储队列句柄的变量指针
    // 因为我们只需要一个队列序列，故这里传入的队列索引为0
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

// 析构销毁
void logicalDeviceCleanup()
{
    vkDestroyDevice(device, nullptr);
}
