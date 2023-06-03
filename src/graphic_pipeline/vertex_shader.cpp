#include "graphic_pipeline/vertex_shader.h"


/**
 *  配置 vertex shader 部分
 *  vertex shader 是整个vulkan graphic pipeline中的第二阶段
 *  进入函数前保证已经使用GLSL完成了对 vertex shader 的编辑，并已经使用glslc工具将其编译为了vulkan可直接识别
 * 的SPIR-V二进制码文件。
 * */ 
VkShaderModule configure_vertex_shader(VkPipelineShaderStageCreateInfo &vertShaderStageInfo)
{
    // 读取编译好的二进制码文件，并使用返回的二进制串构建 vertex shader module
    auto vertShaderCode = readFile("../shaders/vert.spv");
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    // 修改配置变量
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 这里告诉我们是在设置 vertex shader 阶段
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // main 名称含义未知

    return vertShaderModule;
    
}


