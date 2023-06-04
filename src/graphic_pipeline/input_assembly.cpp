#include "graphic_pipeline/input_assembly.h"

/**
 *  配置输入“装配器”：描述按照何种规则将顶点连成几何
 * */ 
void configure_input_assembly(VkPipelineInputAssemblyStateCreateInfo &inputAssembly)
{
    
    /*
        输入“装配器”：
        通过 VkPipelineInputAssemblyStateCreateInfo 结构体进行描述，它主要描述两件事情：
        1/将从顶点绘制什么样的几何体（是否要将点连成线？或线连成面）
        2/是否启用基元“复用”（是否支持顶点重用）
    */
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    /*
        topology 字段有以下可选项，表示含义分别进行解释：
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST：输入的顶点将作为点被绘制
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST：输入的顶点将进行两两组合，绘制线段，且每个顶点不会被重复使用
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP：同样绘制线段，但顶点会被重用，每条结束点将会被视为下一条线段的起始点
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST：输入的顶点将三点一组，绘制三角形，顶点不会被重用
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP：同样绘制三角形，每个三角形第二三个顶点将被用作临接三角形的第一二个顶点
        由于目前只需要进行单个三角形的绘制，所以我们选择使用 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST 配置
    */
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // 如果这里设置为 VK_TRUE 则允许你在 topology 的 "_STRIP" 模式下将三角形或线段进行拆分（细分）
    inputAssembly.primitiveRestartEnable = VK_FALSE;

}
