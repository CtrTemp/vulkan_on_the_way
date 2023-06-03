#include "graphic_pipeline/vertex_shader.h"



VkShaderModule configure_vertex_shader(VkPipelineShaderStageCreateInfo &vertShaderStageInfo)
{
    // 文件读入，创建module
    auto vertShaderCode = readFile("../shaders/vert.spv");
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    // create info populate
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 这里告诉我们是在设置 vertex shader 阶段
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // main 名称含义未知

    return vertShaderModule;
    
}


