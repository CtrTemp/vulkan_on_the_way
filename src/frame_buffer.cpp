#include "frame_buffer.h"

std::vector<VkFramebuffer> swapChainFramebuffers; // 交换链中的framebuffers

/**
 *  为交换链中的每一个 Image View 都要创建一个 framebuffer
 * */
void createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        // VkImageView attachments[] = {swapChainImageViews[i]};
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;                                     // 绑定render pass，render pass 就是对framebuffer的配置
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // 将 framebuffer/depthbuffer 与 swapchain 中的 imageview 一一对应
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

/**
 *  销毁交换链中所有的 framebuffer
 * */
void cleanupFramebuffer()
{
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
