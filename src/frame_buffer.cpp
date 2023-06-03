#include "frame_buffer.h"

std::vector<VkFramebuffer> swapChainFramebuffers;

/**
 *  创建 framebuffer
 * */ 
void createFramebuffers()
{
    /*
        第三步，调整Framebuffers的大小，
        如同刚刚在创建相关的成员变量时提到的，Framebuffer要创建多个，保证与swapchain中图像的一一对应关系。
    所以这里要先将 swapChainFramebuffers 的大小与 swapChainImageViews调整一致
    */
    swapChainFramebuffers.resize(swapChainImageViews.size());
    /*
        第四步，在一个循环中创建所有的 Framebuffers
    */
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // 首先要指定帧缓冲区需要与哪个Render Pass兼容
        framebufferInfo.renderPass = renderPass;
        // 指示当前的framebuffer应该绑定到哪个swapchain中的ImageView，从而与之保持一一对应的关系
        // 显然，这里是可以有一对多的关系的，但当前我们还用不到
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        // 宽高值不言自明
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        // 层数值得是图像阵列中的层数，我们交换链图像是单图的，故设为1
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

/**
 *  销毁 framebuffer
 * */ 
void cleanupFramebuffer()
{
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
