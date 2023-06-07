#include "texture.h"
/**
 *  选用 stb_image 库进行文件导入，注意必须在源文件中导入
 * */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

VkImage textureImage;              // 纹理贴图实例
VkDeviceMemory textureImageMemory; // 纹理贴图设备内存
VkImageView textureImageView;      // 纹理图的 ImageView 实例
VkSampler textureSampler;          // 纹理图采样器实例

/**
 *  创建纹理贴图实例
 * */
void createTextureImage()
{
    int texWidth, texHeight, texChannels;
    // 借助 stb_image 库加载图片，并获取其尺寸/通道数等附加信息
    stbi_uc *pixels = stbi_load("../textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    /**
     *  图像占用内存空间计算
     * */
    // 这里的 channel 获取可能有问题，必须默认给4通道才行，而以上获取的channel数总为3，不知为何
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    // 验证是否加载成功
    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }
    /**
     *  与vertex buffer同样的流程，由于在仅GPU可见内存上访问速度更快，所以我们还是需要借助 staging buffer，先将数据导入
     * 到CPU可访问的GPU内存区域，再进行 device to device 的数据拷贝。
     * */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    // 将图片数据拷贝到GPU内存上之后就可以直接释放CPU上对图片存储的信息了。
    stbi_image_free(pixels);

    /**
     *  创建图像实例，并为其分配设备内存空间，注意这里我们使用的是 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT，这意味着图像对应
     * 的内存空间将不可被CPU访问，且对于GPU的访问有更高的效率。
     * */
    createImage(texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage,
                textureImageMemory);

    /**
     *  由于纹理图像相对较大，拷贝需要花费的时间不可忽略，我们有必要为该部分的拷贝做优化。以下函数将image纹理变换为最适合进行文件
     * 传输的格式（进行了格式优化）。
     * */
    // 将纹理图像转换为VK_image_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    /**
     *  device to device memory copy
     * */
    copyBufferToImage(stagingBuffer,
                      textureImage,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));

    /**
     *  拷贝完成后，将设备端的texture image再转换回最适合GPU读写的形式（之前被转换为了最适合传输的形式）。
     * */
    transitionImageLayout(textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /**
     *  CPU可访问的staging buffer可以直接注销了，连同释放其对应的设备内存
     * */
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/**
 *  图像布局转换，使得其内存排布更适合接下来要进行的操作。
 *  如优化为最适合数据传输的形式/数据读写的形式。
 * */
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    /**
     *  以单次命令的形式导入 command buffer 最终再提交到命令队列进行执行
     * 使用 beginSingleTimeCommands 和 endSingleTimeCommands 配合进行了封装
     * */
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // 原布局
    barrier.newLayout = newLayout; // 要转为的布局

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // 定义barrier前后要发生的事件
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // 分支情况处理
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

/**
 *  将texture数据拷贝到设备内存
 *  本质上是进行了一次device to device的拷贝，就是 staging buffer 到一块仅设备可访问的内存区
 * */
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 以下确定要作为数据源的 buffer 的一些信息
    VkBufferImageCopy region{};
    region.bufferOffset = 0;      // 开始拷贝的地址偏移
    region.bufferRowLength = 0;   // 布局方式，是否要在每列像素之间添加一些填充字节
    region.bufferImageHeight = 0; // 布局方式，是否要在每行像素之间添加一些填充字节

    // 以下确定要作为数据承接者的 图像对象 的一些信息
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // 同样，我们暂时不考虑mipmap
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    // 将buffer中的数据拷贝到图像对象中
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        // 以下这个参数指的是图像当前的布局，当然我们选择的是最适合作为数据传输中“承接者”的布局
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    endSingleTimeCommands(commandBuffer);
}

/**
 *  为纹理贴图创建配套的 ImageView
 * */
void createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

/**
 *  创建采样器：
 *  当然，你可以想到的最简单的方法就是从图像中直接读取像素信息并直接应用到ImageView，但更好更通用
 * 的做法仍然是为其创建一个采样器作为中间层。这是为了以下多种情况进行的考虑：
 *  1/当出现图像尺寸小于“片段（fragment）”尺寸时，直接应用会出现马赛克现象，这时我们需要使用采样
 * 器进行低通滤波
 *  2/当出现图像尺寸大于“片段”尺寸时，这时候我们称为“欠采样”情况，导致在尖锐边缘（高频信号）进行采
 * 样时会出现伪影，造成模糊。这时候使用采样器进行“各项异性”滤波可以解决
 *  3/其他情况，当你想读取纹理图尺寸之外的“纹素”信息时，可以考虑使用地址映射模式，从而可以实现图像
 * 的重复/镜像重复等多种显示模式。（这里就和UV贴图是一个概念了）
 * */
void createTextureSampler()
{

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // 对于放大/缩小texel，我们都统一使用线性插值采样的方式
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    /*
        以下字段对应以上情况3,当所请求的地址超出纹理图像的尺寸时，采样器应如何对该地址/索引进行映射
    使其仍然位于纹理图像区域内。
        这里涉及的其实就是UV贴图，对应的UVW就是笛卡尔坐标系的XYZ。
        该配置有以下击中可选模式：
        VK_SAMPLER_ADDRESS_MODE_REPAT：对超出的部分重复计数，就是取原本坐标轴对应图像尺寸取余
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT：尺寸减去余数
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE：获取图像尺寸外最接近坐标部分的边缘颜色（对边缘
    颜色进行扩展填充到整个画面）
        VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE：与以上相同，但取的边缘是当前超出边对
    面的边
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER：超出边缘后，使用指定的单一颜色进行填充
        以下我们选用“重复模式”。不过其实也无所谓了，因为当前我们使用的图像对象与纹理基本上是匹配的，
    基本不会出现超出图像边缘的情况。
    */
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // 纯黑边缘填充色

    samplerInfo.unnormalizedCoordinates = VK_FALSE; // 选择不归一化UV坐标

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // MipMap 这里暂时不进行展开
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    // 创建纹理采样器
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

/**
 *  注销纹理贴图相关的组件
 * */
void cleanupTextureRelated()
{
    // 注销纹理采样器
    vkDestroySampler(device, textureSampler, nullptr);
    // 将 textureImageView 释放内存
    vkDestroyImageView(device, textureImageView, nullptr);
    // 将 textureImage 释放内存
    vkDestroyImage(device, textureImage, nullptr);
}
