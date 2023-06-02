#include "physical_device_queue.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;         // 物理设备对象
QueueFamilyIndices queueIndices;         // 指令集队列集合对象
SwapChainSupportDetails swapChainDetails; // 支持交换链具体细节对象


// 必要的设备扩展支持：
// 1、对 swap chain 的支持
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/**
 *  选取一块我们需要的图形显卡（物理设备）
 * 可以是一块也可以是多块，在这里我们只选用一块
 *
 * */
void pickPhysicalDevice()
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
    std::cout << std::endl;

    // 对卡开始评估
    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device)) // 如果有显卡是合适的，那么我们就选取其中的第一张
        {
            // std::cout << "GPU physical device checked! Avaliable" << std::endl;
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
    queueIndices = findQueueFamilies(device); // 验证是否支持“图形绘制”指令集以及是否支持“图形展示”指令集

    // 验证当前设备是否支持 swap chain 扩展
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    // 以下是第三步中的一些关于swap chain细节的验证（adequate：恰当/充足）
    bool swapChainAdequate = false; // 首先假设不满足条件
    if (extensionsSupported)        // 仅当以上的 “当前设备支持交换链” 的条件满足时才进行如下的判断
    {
        swapChainDetails = querySwapChainSupport(device);
        // 当我们的交换链有至少一个可用的输出格式，且至少有一个可用的展示模式的时候，我们就认为其足够了！
        swapChainAdequate = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
    }

    return queueIndices.isComplete() && extensionsSupported && swapChainAdequate;
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
    std::cout << std::endl;

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

        if (presentSupport)
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




/******************************** 以下是物理设备关于 swap chain 特性支持的验证 ********************************/ 


bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    // 扩展数量
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    // 将“可用”扩展信息一并收集，并压入 vector （注意，可用扩展信息是实时从当前设备中获取的）
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    // 将“所需”的扩展信息存入一个set（注意，所需扩展信息是我们在最开始声明在文件中的全局变量，设备可能具备）
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // 从“所需扩展”中逐一擦除“可用扩展”
    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    // 如果此时“所需扩展”为空，则说明“所需扩展”是“可用扩展”的子集，我们的设备完全支持这些我们需要的扩展
    // 此时返回true
    return requiredExtensions.empty();
}




/*
    第三步，查询更加细节层面的swap-chain支持
    仅仅查验swap-chain扩展是否支持是远远不够的，因为即便它可用，却不能保证它和我们的窗口视图的“可共用性”。
    创建一个swap-chain同样需要远比创建一个instance复杂的多的设置，所以我们需要查验更多的细节。

    大致有以下几个方面的特性我们需要查验：
    1/基本界面能力属性（如swap-chain能最多/最少能容纳的图片数量，swap-chain能够接受的最大图像宽度/高度）
    2/界面格式（像素类型，色域）
    3/可用的输出展示模式
*/
// 对应第三步，创建一个函数，用于填充与swap chain细节相对应的那个结构体
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    // 首先对“基本界面能力”这项进行填充
    // 输入的参数是“硬件设备”和“当前界面”两个相关的核心参数
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // 其次对第二个“界面格式”这项进行填充（像素类型，色域）
    uint32_t formatCount;
    // 首先询问支持多少种“界面格式”
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        // 支持界面格式不为0时，对其每一项的细节列写/填充在数组中
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // 接下来对应的是其中第三项：可用的输出展示模式，与以上第二项相同，也需要两个步骤
    uint32_t presentModeCount;
    // 首先确定可选的展示模式有几个
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        // 再根据具体个数，配置数组中的各项细节
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    // 当收集完所有的细节，返回这个结构体
    return details;
}

