#include "physical_device_queue.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理设备对象
QueueFamilyIndices queueIndices;                  // 指令集队列集合对象
SwapChainSupportDetails swapChainDetails;         // 支持交换链具体细节对象

// 必要的设备扩展支持：
// 1、对 swap chain 的支持
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/**
 *  图形卡的选取以及核验
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
 *  GPU核验评估
 * */
bool isDeviceSuitable(VkPhysicalDevice device)
{
    // 01：核验GPU是否同时具有支持“图形绘制”和“图形展示”的指令集队列
    queueIndices = findQueueFamilies(device);

    // 02：核验GPU是否支持 swap chain 扩展
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    // 03：若GPU支持swap chain扩展，则核验swap chain是否满足功能性要求
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        swapChainDetails = querySwapChainSupport(device);
        swapChainAdequate = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
    }
    /**
     *  04：即使在现代图形显卡中很难找到不支持“各项异性采样”的，我们还是决定进行以下的验证保证程序能够
     * 正确运行，且在无法运行时给出正确报错。
     * */
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    // 以上核验均通过则认定GPU设备合格
    return queueIndices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    // return queueIndices.isComplete() && extensionsSupported && swapChainAdequate;
}

/**
 * 01：核验GPU是否同时具有支持“图形绘制”和“图形展示”的指令集队列
 *  Vulkan中的所有操作所要执行的命令都要上传队列，这些队列有不同的类型，某一类型的队列只负责处理一系列特定的命令集，
 * 以下函数负责检查设备支持哪些队列系列，并返回所支持队列的列表。
 *  （在这里仅验证是否支持“图形绘制指令集”队列，以及“图形展示指令集”队列）
 * */
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    std::cout << "available devices' cmd queue family: \n"
              << "queue family count = " << queueFamilyCount << " <> " << queueFamilies.size() << std::endl;
    for (const auto &queue : queueFamilies)
    {
        // 打印输出其置位的二进制数（输出还是十进制的需要自己解码）
        std::cout << "\t" << queue.queueFlags << std::endl;
    }
    std::cout << std::endl;

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        // 找到一个支持 VK_QUEUE_GRAPHICS_BIT 类型的队列（支持图形绘制指令）
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i; // 然后让 indices 指向这个队列
        }

        // 找到一个支持图形展示指令集的队列（支持图形绘制指令集的队列不一定支持图形展示指令，但二者也有可能重合）
        VkBool32 presentSupport = false;
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
    return indices;
}

/**
 *  02：验证GPU对 swap chain 的支持
 * */
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

/**
 *  03：验证 swap chain 特性是否满足要求
 *  1、swap chain 的基本界面能力，如：swap-chain能最多/最少能容纳的图片数量，swap-chain能够接受的最大图像宽度/高度
 *  2、swap chain 支持的基本界面格式，如：像素类型，色域
 *  3、swap chain 可用的输出展示模式
 * */
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    // 基本界面能力获取
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // 支持基本界面格式获取
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // 可用的输出展示模式获取
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}
