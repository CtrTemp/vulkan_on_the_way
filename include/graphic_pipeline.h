#ifndef GRAPHIC_PIPELINE_H
#define GRAPHIC_PIPELINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

// 引入shader文件所必须
#include <fstream>

#include "logical_device_queue.h"

#include "swapchain.h"

#include "graphic_pipeline/vertex_shader.h"
#include "graphic_pipeline/fragment_shader.h"

#include "graphic_pipeline/dynamic_state.h"
#include "graphic_pipeline/vertex_input.h"
#include "graphic_pipeline/input_assembly.h"
#include "graphic_pipeline/viewport.h"
#include "graphic_pipeline/rasterizer.h"
#include "graphic_pipeline/multi_sampling.h"
#include "graphic_pipeline/color_blending.h"

#include "render_passes.h"



extern const int MAX_FRAMES_IN_FLIGHT;

/*
    Brief Introduction：
    下面我们开始对最基本的光栅化渲染管线的流程做一个概览：

    1/input assembler 输入汇编器：它将从你指定的缓冲区收集原始顶点数据，当然也可以使用index buffer间接的
对顶点数据进行收集；
    2/vertex shader 顶点着色器：对每个顶点执行，将使用矩阵变换将所有的顶点从模型空间转换到屏幕空间，做了一个
坐标系变换，并将这些用屏幕坐标系坐标定义的顶点数据继续传入管线的下一层；
    3/tessellation shader 细分着色器：应用于某些规则几何体，对其进行细分，从而提高网格质量。这将使得砖墙和
楼梯一类的模型表面看起来不是那么平坦规整；
    4/geometry shader 几何着色器：它在每个基元（如三角形/直线/点）上运行，可以将之弃用或在此基础上细分出更
多的基元，这与以上的细分着色器很类似，但更加灵活（可能就是体现在其可以可以缩减一些或添加一些基元，而不仅限于细
分）。不过并不常用，因为除了在Intel集成显卡上效果明显，大多数图形卡的性能都不太好。
    5/rasterization 光栅化：这个阶段不必细说，将基元离散化为更小的片段。这些就是在framebuffer中填充的像素
基本元素。屏幕外以及被遮挡的元素都将被丢弃，不再进行考虑。
    6/fragment shader 片段着色器：剩下没有被弃用的将输入片段着色器，并且确定将片段写入哪个framebuffer以及
颜色信息/深度信息都将被考虑。哪些在三角形片元内部位置的像素将被以插值的方式赋予具体值，插值将依据其所属的三角形
顶点的光照信息/法线信息/纹理值来决定
    7/color blending 颜色混合：最终的着色阶段，对于以上的fragment之间相互交叠的情况进行考虑，其颜色可以是
相互遮挡覆盖/基于透明度混合/简单相加。


    以上其中1/5/7的工作方式是预定义的，不可编程的，但你仍可以在有限的几种模式下选取你需要的配置。
    以上2/3/4/6是可编程的，这誉为这你可以将自己的代码上传到图形显卡中，以明确应用该如何具体操作。

    之前提到过一点，为了保证性能，Vulkan中的一条管线一旦被创建（既组以上1～7的组合），在程序运行中便不可随意进
行更改；这意味着如果你想使用多条图形管线，切换图形管线，请在开始阶段就定义创建多条。

    以上的1～7并不是所有步骤都是必须的，例如，你如果只想绘制简单的几何体，那么以上的3/4阶段（涉及表面细分的部分）
均可以抛弃。如果你只想得到一张深度图，完全可以禁用fragment-shader部分。

    在之后的 graphic pipeline 创建的过程中，每一个环节都会进行较为细致的剖析。
*/


extern VkPipelineLayout pipelineLayout; 
extern VkPipeline graphicsPipeline;

void createGraphicsPipeline();
void cleanupGraphicPipeline();


#endif
