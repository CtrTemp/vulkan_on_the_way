#include "graphic_pipeline/multi_sampling.h"

VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;

/**
 *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
 * */
void configure_multi_sampling(VkPipelineMultisampleStateCreateInfo &multisampling)
{
    getMaxUsableSampleCount();

    /*
        这是经典有效的抗锯齿方法之一，工作原理是将多个多边形边缘的光栅化值进行组合考虑（应该就是求加权平均）
    主要对边缘产生的锯齿/伪影起作用。注意要启用多重采样来抗锯齿，则必须启用GPU功能。
    */
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // 由于多重采样是一个值得展开的话题，我们在下一章节再进行展开讨论，这里留个占位符，先设置为 VK_FALSE
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = .2f;          // Optional
    multisampling.rasterizationSamples = msaaSamples;
    // multisampling.pSampleMask = nullptr;            // Optional
    // multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    // multisampling.alphaToOneEnable = VK_FALSE;      // Optional
}

VkSampleCountFlagBits getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width,
                swapChainExtent.height,
                1,
                msaaSamples,
                colorFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                colorImage,
                colorImageMemory);

    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void cleanupMultiSampleColorResource()
{
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
}
