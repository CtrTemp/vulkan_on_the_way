#include "graphic_pipeline.h"

VkPipelineLayout pipelineLayout; // 添加图形渲染管线
VkPipeline graphicsPipeline;

// 创建渲染图形管线
void createGraphicsPipeline()
{

    /**
     *  最开始的阶段，我们对可编程的顶点着色器（vertex shader）以及片段着色器（fragment shader）进行配置。
     *  基本流程就是：我们先分别写一段vertex shader和fragment shader代码，使用GLSL语言，并通过预先编译，
     * 将其编译成vulkan可以直接识别的机器码(SPIR-V)，并在运行时通过读文件的形式加载到我们的程序中。
     * */ 

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};  // 预定义vertex shader配置变量
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};  // 预定义fragment shader配置变量

    /**
     *  将预定义变量输入到自定义函数中进行配置，在最后真正组建 graphic pipeline 的时候作为配置变量传入。
     * 后面无论是pipeline中的可编程管线组件还是不可编程组件，都将遵循这种规则进行配置。
     * */ 

    // vertex shader 是整个vulkan graphic pipeline中的第二阶段
    VkShaderModule vertShaderModule = configure_vertex_shader(vertShaderStageInfo); 
    // fragment shader 是整个vulkan graphic pipeline中的第六阶段
    VkShaderModule fragShaderModule = configure_fragment_shader(fragShaderStageInfo);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // 下面开始对非可编程部分进行配置
    /**
     *  注意，虽然不可编程，但我们仍可以通过设置，在有限的几种选择中设置其工作模式。`
     *  在这部分的开头指出，在一些比较旧的图形API中，渲染管线为大部分阶段提供了默认的状态配置。而在Vulkan中，
     * 你必须明确设置这些状态，并将所有的配置组合为一个“一旦运行就不可变更”的管线对象，从而最优化效率。这也是
     * Vulkan的不同之处，优势所在。
    */


    /**
     *  Dynamic Static中可以配置一些在pipeline确定后仍能更改的部分。虽然这十分有限。
     * */ 
    VkPipelineDynamicStateCreateInfo dynamicState{};
    configure_dynamic_state(dynamicState);

    /**
     *  vertex input 应该算作是一个“前处理阶段”，在 graphic pipeline 的最前端，位于所有组件之前。
     * 它将作为模型的顶点数据传入至 graphic pipeline 中第一个组件之间的桥梁
     * */ 
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    configure_vertex_input(vertexInputInfo);


    /**
     *  input assembler 输入汇编器，作为 graphic pipeline 中的首个组件，它将从你指定的缓冲区收集原始顶点数据，
     * 当然也可以使用index buffer间接的对顶点数据进行收集。
     * */ 
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    configure_input_assembly(inputAssembly);

    /**
     *  视口以及视口窗的截取，
     *  1、“视口”将决定你绘制出的图像将在多大范围内显示
     *  2、“视口窗的截取”将决定当前视口有多大范围是可见的
     * */ 
    VkPipelineViewportStateCreateInfo viewportState{};
    configure_viewport(viewportState);


    /**
     *  跳过第三阶段 tessellation 细分着色器（可选的）
     *  ？？？ 不可编程且不可设置？？？
     * */ 

    /**
     *  跳过第四阶段 geometry shader 几何着色器部分（可选的）
     * ？？？ 不可编程且不可设置？？？
     * */ 

    /**
     *  rasterization光栅化：这处于整个 graphic pipeline 的第五个阶段，将几何面元（基元）离散为更小的“片段”，说白了
     * 就是变成像素，然后填充framebuffer.屏幕range之外的部分以及被遮挡的部分将被舍弃掉。
     * */ 
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    configure_rasterizer(rasterizer);


    /**
     *  跳过fragment shader片段着色器：整个 graphic pipeline 中的第六个阶段，图元（三角形）内部的部分将被以插值的方式赋值，
     * 这将由其所属图元（三角形）顶点的光照信息/法线信息/纹理值决定。
     * ？？？ 不可编程且不可设置？？？
     * */ 




    /**
     *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
     * */ 
    VkPipelineMultisampleStateCreateInfo multisampling{};
    configure_multi_sampling(multisampling);


    /**
     *  color blending颜色混合阶段：作为整个 graphic pipeline 中的最后一个阶段（第七阶段），它将以上fragment之间相互
     * 交疊的情况进行考虑，其最终输出就是我们看到的每一个像素的颜色值。它将基本根据遮挡关系/透明度等进行颜色混合/合成。
     * */ 
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    configure_color_blending(colorBlending);

    /**
     * 创建图形管线
     * 
     * ？？？ pipelineLayout 和 pipeline 有啥区别？？？
     * */ 
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