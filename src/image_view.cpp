#include "image_view.h"

/*
    Introduction：
    为了使用包括在交换链中的任何的VkImage，在整个渲染管线中，我们需要创建一个VkImageView对象。
该对象将描述如何访问图像以及要访问图像的哪些部分。例如，如果应该将其视为2D深度纹理，mipmap就不会被使用。
*/


// 创建 ImageView
void createImageViews()
{
    swapChainImageFormat = swapChainSurfaceFormat.format; // image 格式要与swap chain选择的格式保持一致
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // 第一步要做的就是将 ImageView 的size设置成交换链中图像的大小，保持同步统一
    swapChainImageViews.resize(swapChainImages.size());

    // 第二步，对所有当前交换链中的图片进行遍历：
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        // 似乎对每一个 Image 都要为其创建一个对应的 ImageView
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        // 以下两个字段定义如何解释图像数据
        // viewType 可以指定你将图像视为 1D纹理/2D纹理/3D纹理/立方体贴图
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // 以下的字段应该是允许你取对这四个pixel通道进行任意映射，而不采用其默认值
        // 比如强制映射为某个常量，或通道值的交错映射互换
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // subresourceRange 字段描述图像的用途以及应访问图像的哪些部分。
        // 以下我们将图像作为颜色目标进行访问，无需任何 mipmap
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        // 如果你正在使用立体3D应用程序，这里允许你设置不同的图层访问（比如VR的左右眼图层）
        createInfo.subresourceRange.layerCount = 1;

        // 根据以上的配置创建 ImageView
        // 如果失败则抛出错误
        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void cleanupImageView()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
}
