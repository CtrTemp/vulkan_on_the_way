#include "graphic_pipeline/fragment_shader.h"



VkShaderModule configure_fragment_shader(VkPipelineShaderStageCreateInfo &fragShaderStageInfo)
{
    // 文件读入，创建module
    auto fragShaderCode = readFile("../shaders/frag.spv");
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // create info populate
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 这里告诉我们是在设置 fragment shader 阶段
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // main 名称含义未知

    return fragShaderModule;
    
}
