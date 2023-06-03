#include "graphic_pipeline/fragment_shader.h"



/**
 *  配置 fragment shader 部分
 *  fragment shader 是整个vulkan graphic pipeline中的第六阶段
 *  进入函数前保证已经使用GLSL完成了对 fragment shader 的编辑，并已经使用glslc工具将其编译为了vulkan可直接识别
 * 的SPIR-V二进制码文件。
 * */ 
VkShaderModule configure_fragment_shader(VkPipelineShaderStageCreateInfo &fragShaderStageInfo)
{
    // 读取编译好的二进制码文件，并使用返回的二进制串构建 fragment shader module
    auto fragShaderCode = readFile("../shaders/frag.spv");
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // 修改配置变量
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 这里告诉我们是在设置 fragment shader 阶段
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // main 名称含义未知

    return fragShaderModule;
    
}
