#include "graphic_pipeline/color_blending.h"


/**
 *  配置颜色混合：如何从fragment shader中得到的fragment值得到最终pixel的颜色值
 * */ 
void configure_color_blending(VkPipelineColorBlendStateCreateInfo &colorBlending)
{
    /*
        第六步：颜色混合
        当fragment shader将顶点颜色返回，并经过光栅化阶段后，需要一种策略将framebuffer中已有的颜色与
    当前新得到的对应像素的颜色进行混合，基本上有以下两种方式进行实现：
        1/混合新旧值得到新的颜色（应该是加权平均？）
        2/使用按位操作的方式混合新旧值。

        同时提供两种颜色混合策略：
        VkPipelineColorBlendAttachmentState 单独配置，包含每个附加缓冲区的配置
        VkPipelineColorBlendStateCreateInfo 全局配置
        目前只有一个 framebuffer 所以只配置第一个即可
    */

    // 这里有必要定义成static，避免在程序结束后自动释放这部分内存
    static VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // // 这里同样是只先给一个占位符，先不启用，这表示新的颜色将直接替换旧的颜色
    // colorBlendAttachment.blendEnable = VK_FALSE;
    // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional
    // 以下提供一种经典的 color blending 方法，即alpha混合，通过不透明度混合新旧两种颜色
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // color blend 的全局配置
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;   // 暂不启用
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

}
