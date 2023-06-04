#include "swapchain.h"

VkSwapchainKHR swapChain; // 定义交换链对象

VkSurfaceFormatKHR swapChainSurfaceFormat;
VkPresentModeKHR swapChainPresentMode;
VkExtent2D swapChainExtent;



// 声明用于填入swap chain的Image
std::vector<VkImage> swapChainImages;

// 声明用于填充swap chain的Image的格式（这个与之前配置的swap chain中的格式保持一致即可）
VkFormat swapChainImageFormat;

// 每个swap chain中的Image都要对应一个ImageView
std::vector<VkImageView> swapChainImageViews;



/*
    本章是Vulkan关键的交换链部分;
    由于Vulkan不提供默认的 framebuffer，所以在将结果渲染到屏幕前，我们必须申请创建一个基础设施，这个基础设施将
在屏幕可视化结果之前，将计算结果放入framebuffer（帧缓冲区），我们称这个“基础设施”为交换链（Swap Chain）。
    该设施必须被明确创建。它其实是一个队列，队列中存放着已经计算好的结果，这些结果将被依次放入framebuffer，并渲染
到屏幕空间。
    这个队列应该怎样工作，一张图片应该在何种条件下被展示到屏幕，这都有赖于我们如何去设置这个Swap Chain。但交换链
的一般目的是进行帧同步，使得图像的呈现与屏幕的刷新率同步。

*/

/*
        第四步，为当前的swap chain选择合适的配置
        如果刚刚的第三步已经满足，说明我们对swap chain更加细节层面的验证已经通过（感觉其实根本上是对显卡+操作系统+窗口
    界面这几者综合层面的考量）。但是这其中仍有很多可配置优化的方面（其实就是做不同的选择，决定用可用模式下的哪种组合），
    以下将编写一些函数来找到这些最优化组合。其中需要注意的主要有以下三点：
        1/界面格式（即：颜色的位深）
        2/显示（展示）模式（即：何种情况下将图像“交换”到屏幕进行显示）
        3/交换范围（swap-chain中的图像分辨率）

        对于以上的每一个可配置项，我们都要在心中预先设想一个最优选择，如果最优选择满足，则使用该配置；若不可用，则要写一
    套逻辑，寻找到下一个可用的次优选择
    */

// 这个函数对应第四步-第一项：选择合适的界面格式（颜色位深）
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    /*
        每个 VkSurfaceFormatKHR 类都有 format 和 colorSpace 两个成员变量。
        其中 format 变量指出每个pixel的RGBA通道排列顺序，以及各自占有的位宽，比如：
    VK_FORMAT_B8G8R8A8_SRGB 指出每个pixel中是以 B/G/R/A 顺序依次存储的各值，且每个占8位宽，共32位存储
        其中 colorSpace 变量指出 SRGB 色域是否支持使用 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 标志位
        具体配置如下：
    */
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // 当然，以上是最优的情况，我们预想的是其支持 VK_FORMAT_B8G8R8A8_SRGB 以及 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // 然而在其不支持以上或不同时支持的时候，需要我们给出一个可行的次优解，为方便考虑，我们就选择当前可用的第一个即可：
    return availableFormats[0];
}

// 这个函数对应第四步-第二项：显示（展示）模式（即：何种情况下将图像“交换”到屏幕进行显示）
// 注意，这几乎可以说是Vulkan交换链中最重要的设置了，因为它代表了向屏幕显示图像的实际条件（在哪些条件下进行展示）
/*
    在这里我们罗列其中可选的四种模式：
    VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
    这种情况下，你程序上传的图像会被立刻上传到屏幕上，但有可能造成“撕裂”现象。

    VK_PRESENT_MODE_MAILBOX_KHR = 1,
    仅在队列排满时与以下 VK_PRESENT_MODE_FIFO_KHR 存在一些差异。程序会简单的用较新的图片替代那些在队列中正
在排队等待的图片，而非停止。这种模式多用于尽可能快的渲染，同时仍能避免“撕裂”现象。这种技术多被称为“三重缓冲”。

    VK_PRESENT_MODE_FIFO_KHR = 2,
    交换链是一个队列的形式，程序提交的图片被插入到队尾，最前面的图片被逐次提交到屏幕，如果队列满了，程序会等待
这与现代游戏中的垂直同步最为相似，刷新显示的瞬间被称为“垂直空白”。

    VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
    仅在程序产生延迟时（队列为空，但下一张图片还没计算出来）与以上 VK_PRESENT_MODE_FIFO_KHR 存在一些差异。
下一张图片传入后会被立刻送往屏幕，而不是等待下一个用于同步的“垂直空白”到来后再进行图像上传。
*/
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    // VK_PRESENT_MODE_FIFO_KHR 模式无疑是最通用的，且最有保障的一项，然而在更好的情况下，如果不考虑能耗
    // 问题，使用 VK_PRESENT_MODE_MAILBOX_KHR 是一种更好的权衡。所以如果支持的话，我们选择它作为最优解
    // 而VK_PRESENT_MODE_FIFO_KHR作为备选的次优解，如下：
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

// 这个函数对应第四步-第三项：交换范围（swap-chain中的图像分辨率）
// 其中涉及一些关于“像素坐标”和“屏幕坐标”不相等的情况（例子是苹果电脑的屏幕以及高DPI的显示器）
// 不做过多解释
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

/*
    第五步，创建 swap chain
    有了第四步的基础，下面我们要使用其中的配置，创建我们自定义的交换链
*/

void createSwapChain()
{
    // 首先创建一个交换链支持情况结构体，并根据当前硬件设备查询出所支持的实际情况
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    // 其次根据实际情况，给出当前创建swap chain最优的配置选择（以下就是三个主项，在第四步中介绍过）
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats); // 这个在之后第七步存入了成员变量
    swapChainPresentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities); // 这个在之后第七步存入了成员变量

    // 除此之外，我们还需要确定在交换链中存储的图片数量，这里我们初始化为最小值
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    std::cout << "minimum image count in swap chain = " << imageCount << std::endl
              << std::endl;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    { // 当支持的最大值大于0且当前所设置的imageCount大于支持的最大值，则将imageCount置为所支持的最大值
        // 需要注意的是，0在这里带有特殊意义，代表“不限制最大交换链中图片数量”
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    // 这里看来应该以上执行完后，imageCount就还是所支持的最小值

    // Vulkan 对 swap chain 的创建同样是通过传入一个大型的参数结构体来实现的，这应该对我们来说相对熟悉了
    // 前面很多对象的创建都遵循这个流程
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapChainSurfaceFormat.format;
    createInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    // 通过注销掉下面这行，可以测试验证层的功能，会引起其报错提示！！
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    // imageArrayLayers指定每个图像包含的层的数量。除非您正在开发立体3D应用程序，否则此值始终为1。
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // imageUsage字段的解释较为麻烦，现在不好理解

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // 当前的队列有 “图形队列” 和 “演示队列”，在实际情况下，二者可能是不同的。
    // 在二者不同的情况下，我们需要制定一套调度方案，将图形队列中绘制交换链中的对象，然后将它们提交到演示队列中
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    // 以下有两种方案可以处理从多个队列访问得到的图像
    /*
        VK_SHARING_MODE_EXCLUSIVE：图像一次只能由一个队列系列拥有，在将其用于另一个队列系列之前，必须明确
    将其所有权转移。这种模式下提供最佳的性能。
        VK_SHARING_MODE_CONCURRENT：图像可以被多个队列系列同时拥有并操作，并不需要明确的所有权转移。
    */
    //    在发生冲突，二者不同时，我们选择“共有模式”，涉及该方面所有权转移的章节再详细进行说明
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    // 如果图形队列和展示队列相同（这在大多数硬件上都是如此），那么我们应该坚持选用“独占模式”
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // 如果交换链基本功能是支持的，那么这里应该可以指定图像的“预转换”功能
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // 下面这个用于忽略 alpha 通道混合
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = swapChainPresentMode;
    // 设为VK_TRUE，被遮挡的像素我们不进行考虑，不会花费多余的计算！提供最优性能
    createInfo.clipped = VK_TRUE;

    // 复杂主题，之后会详细讲
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Vulkan 交换链是在成员变量中声明的，这里无需声明

    // 若非创建成功，则抛出错误
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

void cleanupSwapChain()
{
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
