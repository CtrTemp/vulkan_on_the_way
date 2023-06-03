#include "graphic_pipeline/viewport.h"

void configure_viewport(VkPipelineViewportStateCreateInfo &viewportState)
{
    /*
        第三步：视口以及视口的截取
        图形展示窗口的大小，以及设置我们通过视口剪切决定留下哪些展示部分
    */
    VkViewport viewport{};
    // 设置视口在窗口中展示的起始点
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // 设置视口的长宽，注意这里并非窗口的长宽，而是选用交换链中的图像长宽，因为这才是真正framebuffer的长宽
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    // 帧缓冲区的深度值，如果并非有特殊设计考虑，那么这里应该使用惯用的0～1归一化值
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // 设置剪刀窗，剪刀窗决定我们展示视口的哪些部分，只要剪刀窗的大小大于视口，则都会展示完整图像
    // 如果我们想将剪刀窗设为在运行时可被更改，则应该将其加入 VkDynamicState 的配置中
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // 输入创建信息
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;      // 窗口中总共一个视口
    viewportState.pViewports = &viewport; // 刚刚配置的视口
    viewportState.scissorCount = 1;       // 窗口中总共一个剪切窗
    viewportState.pScissors = &scissor;   // 刚刚配置的剪切窗

    // viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // viewportState.viewportCount = 1;
    // viewportState.scissorCount = 1;
}
