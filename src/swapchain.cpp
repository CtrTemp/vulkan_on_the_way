#include "swapchain.h"

VkSwapchainKHR swapChain; // 定义交换链对象

// 1、swap chain 支持的基本界面格式，如：像素类型，色域
VkSurfaceFormatKHR swapChainSurfaceFormat;
// 2、swap chain 可用的输出展示模式
VkPresentModeKHR swapChainPresentMode;
// 3、swap chain 的交换区域（哪部分图像将会被交换到屏幕进行展示或者进行绘制处理，可以理解为图像size）
VkExtent2D swapChainExtent;

// 用于填入swap chain的Image
std::vector<VkImage> swapChainImages;

// 用于填充swap chain的Image的格式（这个与之前配置的swap chain中的格式保持一致即可）
VkFormat swapChainImageFormat;

// 每个swap chain中的Image都要对应一个ImageView
std::vector<VkImageView> swapChainImageViews;

/**
 *  创建 swap chain
 * */
void createSwapChain()
{
    // 获取当前硬件设备的 swap chain 所支持的细节
    swapChainDetails = querySwapChainSupport(physicalDevice);

    // 其次根据实际情况，给出当前创建swap chain最优的配置选择
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
    swapChainPresentMode = chooseSwapPresentMode(swapChainDetails.presentModes);
    swapChainExtent = chooseSwapExtent(swapChainDetails.capabilities);

    // 确定在交换链中存储的图片数量，这里我们初始化为最小值 2
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount;
    std::cout << "minimum image count in swap chain = " << imageCount << std::endl
              << std::endl;

    if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
    {
        // 保证 image count 不大于 swap chain 中存储图片的最大存储能力
        imageCount = swapChainDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapChainSurfaceFormat.format;
    createInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    // imageArrayLayers字段 指定每个图像包含的层的数量。除非您正在开发立体3D应用程序，否则此值始终为1。
    createInfo.imageArrayLayers = 1;
    // imageUsage字段 指定交换链中的图像将在接下来进行何种操作，根据操作类型不同会做图片存储优化
    /**
     *  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT：直接进行展示
     *  VK_IMAGE_USAGE_TRANSFER_DST_BIT：图像传输
     * */
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /**
     *  imageSharingMode字段：决定对swapchain中的图像的处理是否是“队列独占的”。由于具有“绘制”功能的指令集队列与
     * 提供“展示”功能的指令集队列可能不为同一个队列。此时队列不能独占交换链中的image，应该设为共享模式，以便在执行完
     * 绘制指令后交付图像展示队列进行下一步操作。
     *
     *  注：在大多数硬件上两个队列是重合的，所以一般情况下我们使用的是独占模式。
     *
     *  VK_SHARING_MODE_CONCURRENT：共享模式，图像一次只能由一个队列系列拥有，在将其用于另一个队列系列之前，必须
     * 明确将其所有权转移。这种模式下提供最佳的性能。
     *
     *  VK_SHARING_MODE_EXCLUSIVE：独占模式，图像可以被多个队列系列同时拥有并操作，并不需要明确的所有权转移。
     * */
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = swapChainPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }
    /*
        第六步
        在创建交换链成功后，立刻添加代码用于检索当前函数createSwapChain本身在函数末尾的句柄（没理解）
        （以下进行摘抄）
        我将在vkCreateSwapchainKHR调用之后添加代码以检索createSwapChain函数末尾的句柄。
    检索它们与我们从Vulkan检索对象数组的其他时间非常相似。请记住，我们只在交换链中指定了最少数量的图像，
    因此允许实现创建具有更多图像的交换链。这就是为什么我们将首先使用vkGetSwapchainImagesKHR查询最终数量
    的图像，然后调整容器大小，最后再次调用它以检索句柄。
    */

    // // 现在我将以上的语句加到后面image view的创建（反正swapChain是全局可访问的）（成功）
    // vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    // swapChainImages.resize(imageCount);
    // vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = swapChainSurfaceFormat.format;
    swapChainExtent = swapChainExtent;
}

/**
 *  为swap chain选择合适的界面格式（颜色位深）
 *  这里选择：
 *  VK_FORMAT_B8G8R8A8_SRGB：B/G/R/A 顺序依次存储的各值，且每个占8位宽，共32位存储
 *  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR：？？？
 * */
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // 没有找到最优解，返回第一个可选项作为次忧解
    return availableFormats[0];
}

// 这个函数对应第四步-第二项：显示（展示）模式（即：何种情况下将图像“交换”到屏幕进行显示）
// 注意，这几乎可以说是Vulkan交换链中最重要的设置了，因为它代表了向屏幕显示图像的实际条件（在哪些条件下进行展示）
/*
 */

/**
 *  何种情况下，将图像交换到屏幕进行显示？
 *  在这里我们罗列其中可选的四种模式：
 *  VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
 *  这种情况下，你程序上传的图像会被立刻上传到屏幕上，但有可能造成“撕裂”现象。
 *
 *  VK_PRESENT_MODE_MAILBOX_KHR = 1,
 *  仅在队列排满时与以下 VK_PRESENT_MODE_FIFO_KHR 存在一些差异。程序会简单的用较新的图片替代那些在队列中正
 * 在排队等待的图片，而非停止。这种模式多用于尽可能快的渲染，同时仍能避免“撕裂”现象。这种技术多被称为“三重缓冲”。
 *
 * VK_PRESENT_MODE_FIFO_KHR = 2,
 *  交换链是一个队列的形式，程序提交的图片被插入到队尾，最前面的图片被逐次提交到屏幕，如果队列满了，程序会等待
 * 这与现代游戏中的垂直同步最为相似，刷新显示的瞬间被称为“垂直空白”。
 *
 * VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
 *  仅在程序产生延迟时（队列为空，但下一张图片还没计算出来）与以上 VK_PRESENT_MODE_FIFO_KHR 存在一些差异。
 * 下一张图片传入后会被立刻送往屏幕，而不是等待下一个用于同步的“垂直空白”到来后再进行图像上传。
 * */

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    // VK_PRESENT_MODE_FIFO_KHR 最优解
    // K_PRESENT_MODE_FIFO_KHR 备选次优解
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

/**
 *  设置交换范围：
 *  窗口设置，当窗口“以屏幕坐标定义的长宽”和“以像素定义的长宽”不匹配的时候，使用到该配置。
 *  该部分可以理解为一个跨平台的自适应屏幕分辨率自动匹配。（具体没有太理解）
 * */

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

/**
 *  注销swap chain实例
 * */
void cleanupSwapChain()
{
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
