#include "physical_device_queue.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;


/**
 *  选取一块我们需要的图形显卡（物理设备）
 * 可以是一块也可以是多块，在这里我们只选用一块
 * 
 * */ 
void pickPhysicalDevice(VkInstance &instance)
{
    // 显卡个数
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    // 没有显卡的话就抛出错误提示
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    // 有显卡的话就将其罗列打印输出
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());



    // 以下打印出当前显卡所支持的所有扩展（这里打印的好像只能是PCI总线上的挂载地址）
    std::cout << "available GPU devices: \n"
              << "deviceCount = " << deviceCount << " <> " << devices.size() << std::endl;
    for (const auto &device : devices)
    {
        std::cout << "\t" << device << std::endl;
    }

    // 对卡开始评估
    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device)) // 如果有显卡是合适的，那么我们就选取其中的第一张
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

/**
 *  GPU评估，是否适合：
 *  对每一个图形卡进行评估，检查它们是否适合我们接下来想要执行的操作，因为并非所有的卡都支持vulkan或部分支持。
 * */ 
bool isDeviceSuitable(VkPhysicalDevice device)
{
    // // 基本的显卡参数返回，方法之一
    // VkPhysicalDeviceProperties deviceProperties;
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);
    // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // // 如果我们仅仅希望我们的显卡支持集合着色器，那么如下的返回就足够了
    // // 其实就是一个特定二进制位的判断
    // return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

    // 方法之二：通过队列家族查找函数来返回一个至少支持 VK_QUEUE_GRAPHICS_BIT 类型的队列系列
    // 只要所有检测到的显卡中有一个支持就行
    // （QueueFamilyIndices是我们自定义的结构体，其中只要有值，下方的isComplete函数就会放回true）
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
}

// 之前提到，Vulkan中的所有操作所要执行的命令都要上传队列
// 这些队列有不同的类型，某一类型的队列只负责处理一系列特定的命令集
// 接下来我们要检查设备支持哪些队列系列，以及其中哪个队列系列支持我们将要使用的命令，如下函数将做这些事情
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices; // 在当前选中的设备卡中，指向一个可用的设备特性队列

    // 获取“显卡设备支持特性”的队列长度
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    // 填充数组，将获取这些队列的具体属性
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        // 我们需要找到至少一个支持 VK_QUEUE_GRAPHICS_BIT 类型的队列系列
        // 也就是说，这张卡的某个特性队列要说明其至少要支持VK图形输出（flag位这一位为置位状态）
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i; // 然后让 indices 指向这个队列
        }
        if (indices.isComplete()) // 既然找到了至少一个了合乎规范了，退出就行
        {
            /* code */
            break;
        }
        i++;
    }

    return indices;
}


