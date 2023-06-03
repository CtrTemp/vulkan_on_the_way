#include "graphic_pipeline/rasterizer.h"

void configure_rasterizer(VkPipelineRasterizationStateCreateInfo &rasterizer)
{
    /*
        第四步：光栅化部分
    */
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    /*
        depthClampEnable 设置为 VK_TRUE，则在近平面与远平面之间的物体将会被“clamp”而非丢弃掉
    （这里它说的意思应该是保留但不显示）在阴影贴图情况下，这个功能很有用，注意需要启用GPU功能才可以
    开启该部分。
    */
    rasterizer.depthClampEnable = VK_FALSE;
    /*
        rasterizerDiscardEnable 设置为 VK_TRUE，则几何体将永远不会通过光栅化器并输出到下一阶段，
    这将基本禁用对帧缓冲区（framebuffer）的任何输出。
    */
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    /*
        polygonMode 字段的各种模式解释：
        VK_POLYGON_MODE_FILL：多边形片元的中间部分将被填充显示（插值）
        VK_POLYGON_MODE_LINE：只光栅化多边形的边，中间部分不会进行插值填充
        VK_POLYGON_MODE_POINT：只光栅化多边形顶点，边/中间均不填充
        除了VK_POLYGON_MODE_FILL外，启用其他模式都要启用GPU功能
    */
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // 很简单，就是多边形边的宽度（单位猜测应该是pixel）
    // 除了1.0f外的其他值都需要启用GPU功能
    rasterizer.lineWidth = 1.0f;
    /*
        cullMode 决定剔除哪些面，可以是多边形正面/反面/正反面都剔除，这会提高效率，注意正反面
    是由多边形顶点的顺序决定的。这里我们剔除背面。
        并且以下会定义以顶点顺时针方向视为正面（应该是左手定则）
    */
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // 用于更改片元深度的，多用于阴影贴图，不过这里还不太用的到，设为VK_FALSE先跳过。
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional
}