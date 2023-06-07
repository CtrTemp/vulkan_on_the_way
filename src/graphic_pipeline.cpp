#include "graphic_pipeline.h"

VkPipelineLayout pipelineLayout; // 图形渲染管线布局实例（运行时对管线进行修改/参数传入的接口实例）
VkPipeline graphicsPipeline;     // 图形渲染管线实例

const int MAX_FRAMES_IN_FLIGHT = 6; // CPU可以不加等待向GPU提交的最多任务量（CPU提交任务最多领先GPU多少个Loop）

/**
 *  创建图形渲染管线
 * */
void createGraphicsPipeline()
{

    // 配置可编程部分的 vertex shader 和 fragment shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

    VkShaderModule vertShaderModule = configure_vertex_shader(vertShaderStageInfo);
    VkShaderModule fragShaderModule = configure_fragment_shader(fragShaderStageInfo);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // 下面开始对非可编程部分进行配置

    /**
     *  Dynamic Static：配置一些在pipeline确定后仍能在 draw time 更改的部分。
     * */
    VkPipelineDynamicStateCreateInfo dynamicState{};
    configure_dynamic_state(dynamicState);

    /**
     *  vertex input 应该算作是一个“前处理阶段”，配置顶点数据的传入规则，如何从文件中导入顶点数据
     * */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    configure_vertex_input(vertexInputInfo);

    /**
     *  input assembler 输入汇编器，配置顶点将以怎样的规则被组合/装配成几何图形
     * */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    configure_input_assembly(inputAssembly);

    /**
     *  配置视口以及视口窗的截取
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
     *  rasterizer 光栅化器：配置如何根据上阶段传入的几何信息对其进行光栅化
     * */
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    configure_rasterizer(rasterizer);

    /**
     *  depth stencil 深度图模板配置。
     * */
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    configure_depth_stencil(depthStencil);

    /**
     *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
     * */
    VkPipelineMultisampleStateCreateInfo multisampling{};
    configure_multi_sampling(multisampling);

    /**
     *  color blending颜色混合阶段：配置最终像素上的颜色如何进行叠加。
     * */
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    configure_color_blending(colorBlending);

    /**
     *  创建 pipeline layout
     *
     *  注意这并不是真正的 graphic pipeline，pipeline layout的作用在于给你提供一个接口，
     *  在后期想在run time（draw time）对渲染结果进行交互，修改参数的时候，可以通过它传入修改值。如：可以在draw time修改传入
     * 的MVP变换阵使得场景中的物体进行移动/翻转等效果，这是通过向pipeline layout传入descriptor来实现的。
     * */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // 指定描述符，在运行时通过更新MVP变换阵使得图像中的物体“动起来”

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    /**
     *  真正创建 pipeline
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
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    /*
        第三步，pipeline layout部分
    */
    pipelineInfo.layout = pipelineLayout;
    /*
        第四步，render pass部分
    */
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // 这里的subpass为什么是0呢？？没搞清楚
    /*
        第五步，用于从已有管线创建新的管线，暂时不需要
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

    // 注意，shader被以SPIR-V的字节码的形式被导入pipeline配置后，在程序中就是固定的了，也不会允许在运行时进行修改，所以这里可以直接销毁
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

/**
 *  销毁图形渲染管线
 * */
void cleanupGraphicPipeline()
{
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}