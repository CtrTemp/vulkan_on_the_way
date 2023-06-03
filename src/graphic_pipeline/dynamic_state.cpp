#include "graphic_pipeline/dynamic_state.h"

void configure_dynamic_state(VkPipelineDynamicStateCreateInfo &dynamicState)
{

    /*
        虽然之前提到渲染管线一经创建，运行时便不再可编辑，但实际上也可以在绘制时不重新创建管线对其进行有限的
    更改。比如：视口的大小/线宽(line width)/混合常数(blend constant)等。如果你的确想在运行时的状态下对
    这些可更改的状态进行更改，那么你需要创建并填充一个结构体进行配置，如下：
    */
    // 这里有必要定义成static，避免在程序结束后自动释放这部分内存
    static std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
}
