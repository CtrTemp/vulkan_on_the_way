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
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
}

/**
 *  之前提到，Vulkan中的所有操作所要执行的命令都要上传队列，这些队列有不同的类型，某一类型的队列只负责处理一系列特定的命令集，
 * 接下来我们要检查设备支持哪些队列系列，以及其中哪个队列系列支持我们将要使用的命令，如下函数将做这些事情。
 * 
 *  下面的函数主要做了这几件事情：
 * 
 *  1、获取所有当前物理显卡所支持的指令(cmd)集合的队列，打印输出看一看
 *  2、从这些队列中找到一个支持“图形绘制指令”集合的队列
 *  3、从这些队列中找到一个支持“图形展示指令”集合的队列
 *  4、我们无法保证一个满足“图形绘制指令”集合的队列也可以同时胜任“图形展示指令”集合，所以我们需要在下面对集合进行遍历，找到满足
 * 以上两种需求的队列至少各一个（可以重合）后便可以返回
 *  5、无论是否合乎我们的要求（基本上现代图形显卡都满足要求），我们都将得到的数据打包返回。
 *
 * */
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices; // 在当前选中的设备卡中，指向一个可用的设备特性队列

    // 获取“显卡设备支持特性”的队列长度
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    // 填充数组，将获取这些队列的具体属性
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // 打印这些队列
    // 以下打印出当前显卡所支持的所有扩展
    std::cout << "available devices' cmd queue family: \n"
              << "queue family count = " << queueFamilyCount << " <> " << queueFamilies.size() << std::endl;
    for (const auto &queue : queueFamilies)
    {
        std::cout << "\t" << queue.queueFlags << std::endl; // 打印输出其置位的二进制数（输出还是十进制的需要自己解码）
    }

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        // 我们需要找到一个支持 VK_QUEUE_GRAPHICS_BIT 类型的队列（支持图形绘制指令）
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i; // 然后让 indices 指向这个队列
        }

        VkBool32 presentSupport = false;
        // 使用以下函数获取一个支持图形展示指令的队列
        // 由于一个队列支持绘制指令并不意味着它一定支持展示指令，所以以下的验证是必须的。
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport)
        {
            indices.presentFamily = i;
        }

        // 当满足条件的队列都被找到，则可以提前返回
        if (indices.isComplete()) 
        {
            break;
        }
        i++;
    }
    // 如果以上循环结束，得到的这个indices判isComplete得到的一定是一个false
    return indices;
}
