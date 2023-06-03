#include "graphic_pipeline.h"

VkPipelineLayout pipelineLayout; // 添加图形渲染管线
VkPipeline graphicsPipeline;

// 创建渲染图形管线
void createGraphicsPipeline()
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

    VkShaderModule vertShaderModule = configure_vertex_shader(vertShaderStageInfo);
    VkShaderModule fragShaderModule = configure_fragment_shader(fragShaderStageInfo);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // 下面开始对非可编程部分进行配置
    /*
        Introduction：
        在上一章节中，我们已经完成了渲染管线中的“可编程部分”，即顶点着色器/片段着色器两部分。在这个章节中，
    我们将目光放在管线中剩余的“不可编程部分”。注意，虽然不可编程，但我们仍可以通过设置，在有限的几种选择中
    设置其工作模式。

        在这部分的开头指出，在一些比较旧的图形API中，渲染管线为大部分阶段提供了默认的状态配置。而在Vulkan
    中，你必须明确设置这些状态，并将所有的配置组合为一个“一旦运行就不可变更”的管线对象，从而最优化效率。这
    也是Vulkan的不同之处，优势所在。

    */

    VkPipelineDynamicStateCreateInfo dynamicState{};
    configure_dynamic_state(dynamicState);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    configure_vertex_input(vertexInputInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    configure_input_assembly(inputAssembly);

    VkPipelineViewportStateCreateInfo viewportState{};
    configure_viewport(viewportState);

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    configure_rasterizer(rasterizer);

    VkPipelineMultisampleStateCreateInfo multisampling{};
    configure_multi_sampling(multisampling);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    configure_color_blending(colorBlending);

    // 第七步，创建图形管线

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    /*
        使用前面一切设置好的信息，开始创建图形渲染管线
    */
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    /*
        第一步，从可编程的部分开始配置
        所以这一步是将shader相关的配置信息传入
    */
    pipelineInfo.pStages = shaderStages;
    /*
        第二步，配置“非可编程”的部分，也就是fixed stage
        同样是传入之前配置好的结构体
    */
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    /*
        第三步，pipeline layout部分
    */
    pipelineInfo.layout = pipelineLayout;
    /*
        第四步，render pass部分，以上完全按照前几个章节的顺序一一配置
    */
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // 这里的subpass为什么是0呢？？没搞清楚
    /*
        第五步，其实这里还有两个额外的配置项：当两个管线的大多数配置相同时，Vulkan允许编程者可以方便地从
    当前已经配置好的管线创建新的管线（从而减少配置量，只对一些二者有出入的地方进行配置修改）
        basePipelineHandle 字段用于指定现有渲染管线的句柄（从而让其他创建的管线可以使用这个已创建管线
    的信息？？应该对应了其本身的索引）
        basePipelineIndex 字段用于将现有管线索引到另一个已经创建好的管线上（则当前的配置仅视为在原有已
    创建好的管线上的修改？？是这样吗？）
        由于我们当前只有一条渲染管线，所以其实不需要以下的这些配置。我们将其设为空句柄，并给一个无效索引。
        但注意，仅当 VkGraphicsPipelineCreateInfo 的标志字段 sType 为 VK_PIPELINE_CREATE_DERIVATIVE_BIT
    的时候，以下两个字段的配置才会生效！
    */
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    /*
        第六步，创建渲染管线
        需要注意 vkCreateGraphicsPipelines 实际是被设计为一次调用创建多个图形渲染管线，但这里没有体现；
    后面的章节中（缓存章节），我们将会看到，使用缓存机制同时创建多管线要比逐个创建要高效的多！
    */
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // 注意，shader的销毁往往在最后
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void cleanupGraphicPipeline()
{
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}