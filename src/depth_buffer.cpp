#include "depth_buffer.h"

VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;

/**
 *  创建深度图实例，以及对应的ImageView实例，并为其在GPU上分配内存空间，并将其布局优化为最适合的深度图模板布局
 * */
void createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(
        swapChainExtent.width,
        swapChainExtent.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, // 表示创建的是深度图模板实例
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage,
        depthImageMemory);

    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    // // 显式地将图像布局转换为深度图附件，虽然这将在RenderPass中被处理，但为了方便理解，我们还是这样做
    // transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    // 然而实际运行时这里报错了，，，不允许在这里进行这一步转化，或者说不允许这种转化
}

/**
 *  根据当前显卡支持，找到深度图最佳像素格式，为以下之一：
 *  VK_FORMAT_D32_SFLOAT32
 *  VK_FORMAT_D32_SFLOAT_S8_UINT
 *  VK_FORMAT_D24_UNORM_S8_UINT
 * */
VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;

        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

/**
 *  辅助函数，为深度图寻找最佳支持格式
 * */

VkFormat findDepthFormat()
{
    // 传入参数是如下希望得到支持的格式
    // VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// 直译：添加一个简单的助手函数，告诉我们所选的深度格式是否包含模具组件：
bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
