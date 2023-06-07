#include "graphic_pipeline/depth_stencil.h"

void configure_depth_stencil(VkPipelineDepthStencilStateCreateInfo &depthStencil)
{
    /*
        在渲染管线中使能深度图相关附件
    */
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    /*
        depthTestEnable字段：使能深度测试，不通过的fragment将被丢弃，通过的将保留，并被存入深度
    缓冲区对应坐标处；
        depthWriteEnable字段：是否将通过深度测试的fragment的新深度值写入深度缓冲区。
    */
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    /*
        depthCompareOp字段指示用于深度测试的操作符，我们的设定中“离我们更近的深度值越小”，所以会舍弃
    哪些深度值更大的，保留那些深度值更小的。于是选择 VK_COMPARE_OP_LESS 操作符。
    */
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    /*
        以下三个字段用于指示保留哪些深度区域的对象（对深度进行区域筛选，通过筛选的才能被保留，其余的均被
    舍弃掉，保留下来的也同样要遵循以上的深度测试）。这里我们暂不使用（depth范围设为0～1说明保留所有的）
    */
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    // 模板缓冲区相关的字段，这里我们也暂时不用考虑
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {};  // Optional
}