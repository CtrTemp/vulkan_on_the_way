#include "logical_device_queue.h"

VkDevice device; // 逻辑设备，物理设备的映射，一个物理设备可以映射到多个逻辑设备


/**
 *  逻辑设备创建：在这里我们可以看到
 *  1、一个物理设备可以对应多个逻辑设备（当然目前还只有一个逻辑设备）
 *  2、一个逻辑设备对应多个指令队列（这是合情合理的，需要有多种不同需求的指令集需要被不同的队列进行处理）
 * */
void createLogicalDevice()
{
    // 队列信息返回，这里我们使用的是物理设备对应的property队列信息
    // QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};

    // Vulkan使用一个 0.0～1.0 的浮点数来为队列分配优先级，从而影响命令缓冲区执行的调度
    // 注意，即使你只有一个队列，这里为其分配优先级也是必须的
    float queuePriority = 1.0f;

    // 以下对于集合中的每一个队列都进行处理（现在就俩队列，一个用于绘制指令集，一个用于展示指令集），
    // 并将其放入队列创建的数据结构体，以供之后方便创建
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 接下来定义一组设备功能（不过目前我们还不需要对其进行配置，先对其留空）
    VkPhysicalDeviceFeatures deviceFeatures{};

    // 开始创建逻辑设备
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = uniqueQueueFamilies.size();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // 注意，我们在此进行了修改，使能的设备扩展不再是0,而是当前所需设备扩展数（声明在文件中，全局变量）
    // createInfo.enabledExtensionCount = 0;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    // 并且加入扩展名称
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

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

    // 实例化逻辑设备后，我们将其指令集队列映射到我们预先定义好的两个指令集队列上（相当于之前只是声明了但没有初始化，这里初始化其对象）。
    vkGetDeviceQueue(device, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueIndices.presentFamily.value(), 0, &presentQueue);
}

// 析构销毁
void logicalDeviceCleanup()
{
    vkDestroyDevice(device, nullptr);
}
