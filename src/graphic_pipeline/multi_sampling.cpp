#include "graphic_pipeline/multi_sampling.h"

/**
 *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
 * */
void configure_multi_sampling(VkPipelineMultisampleStateCreateInfo &multisampling)
{
    /*
        这是经典有效的抗锯齿方法之一，工作原理是将多个多边形边缘的光栅化值进行组合考虑（应该就是求加权平均）
    主要对边缘产生的锯齿/伪影起作用。注意要启用多重采样来抗锯齿，则必须启用GPU功能。
    */
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // 由于多重采样是一个值得展开的话题，我们在下一章节再进行展开讨论，这里留个占位符，先设置为 VK_FALSE
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional
}
