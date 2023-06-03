#include "graphic_pipeline/vertex_input.h"

void configure_vertex_input(VkPipelineVertexInputStateCreateInfo &vertexInputInfo)
{
    /*
    从文件中读取顶点信息并进行输入：
    图形管线的第一步是 Vertex Input，顶点输入，创建以下的结构体来决定对其设置，该步的输出在图形渲染
管线中将直接传入顶点着色器，故以下字段将考虑这方面的衔接：
    Bindings：导入的数据间距，以及数据是按照顶点传递还是按照实例传递
    Arrtibute：传递给顶点着色器的属性类型，从哪些绑定的量加载这些属性以及偏移量
    （由于当前还不涉及从文件中读入数据，故这部分的知识在涉及模型导入的时候可能会理解的更好）
    （目前的数据是硬编码在shader文件中的）
    （以下我们先留一个配置进行占位，实际上以下的配置在渲染管线中并不起作用）
*/
    // VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
}
