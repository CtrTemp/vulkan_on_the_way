#include "image_view.h"

// 每个swap chain中的Image都要对应一个ImageView
std::vector<VkImageView> swapChainImageViews;
/**
 *  创建 ImageView
 * */
void createImageViews()
{
    // image 基本信息要与swap chain中设置的保持一致
    swapChainImageFormat = swapChainSurfaceFormat.format;
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // swap chain中的每一个Image都要配备一个ImageView
    swapChainImageViews.resize(swapChainImages.size());
    // 对vector遍历创建imageview
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        // viewType 字段可以指定你将图像视为 1D纹理/2D纹理/3D纹理/立方体贴图
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // 以下的字段应该是允许你取对这四个pixel通道进行任意映射，而不采用其默认值
        // 比如强制映射为某个常量，或通道值的交错映射互换
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // subresourceRange 字段描述图像的用途以及应访问图像的哪些部分。以下我们将图像作为颜色目标进行访问，且无需任何 mipmap
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        // 如果你正在使用立体3D应用程序，这里允许你设置不同的图层访问（比如VR的左右眼图层）
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

/**
 *  注销交换链中所有的ImageView
 * */
void cleanupImageView()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
}
