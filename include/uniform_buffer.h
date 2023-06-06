#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
/*
    添加以下这行定义，它将使得vec2也强行占用16Bytes这样虽然空间占用变大，但可以保证在C++中不使用
alignas说明符也可以保证大部分情况下的兼容。在这里你可以尝试去除刚刚的修改，查看是否可以得到正确的结果。
    （不过这里我的好像不奏效）

    但请注意，另一些情况下该方法仍然会失效，比如使用嵌套结构定义的结构体类型（在此不过多探讨，，，）
*/
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "buffers/buffers_operation.h"

/*
    Introduction 01：
    在这个章节我们将转向学习3D图形绘制相关的一些知识，其中最重要的应该要数矩阵变换/视图投影了。当我们想要对矩阵
进行变换时，我们可以先将其存成数组形式并导入到顶点缓冲区中。但是这样似乎太耗内存了，且每次变换都需要我们更新一次
顶点缓冲区。
    Vulkan中解决这一资源浪费问题的方法是使用“资源描述符”。它是着色器自由访问缓冲区和图像等资源的一种方式。我们
将在这个章节学习和掌握这种方法。我们会让程序开辟一个包含变换阵的缓冲区，并让顶点着色器通过描述符访问它们描述符，
描述符的使用包括以下三个步骤：
    1/在管道创建期间指定描述符布局（本小节涉及的重点）
    2/从描述符池分配描述符集（直译，不过听起来使用方法有点像命令缓冲区的提交）
    3/在渲染期间绑定描述符集（直译，不过听起来使用方法有点像命令缓冲区的提交）


    注意，本节代码写完后还缺少一些东西，并不能直接运行。

    写完之后，这里应该声明：当前这种使用UBO（Uniform Buffer Object）的方式并非将频繁更改的值传递给着色器的
最好/有效做法。将一小部分数据缓冲区传递给着色器的一种更有效的方法是推送常量（push constants）。我们可以在未来
的章节中讨论这些问题。
*/

/*
    Introduction 02：
    上一章中的描述符布局描述了可以绑定的描述符类型。在本章中，我们将为每个VkBuffer资源创建一个描述符集，
以将其绑定到统一的缓冲区描述符。（直译）
    意思大概是：因为我们之前为存储顶点数据创建了顶点缓冲区以及其对应的索引缓冲区，这两个对应的都是VkBuffer
类型。那么在这个章节中，由于我们需要对顶点做一些矩阵变换操作，所以，这些“矩阵变换操作”就应该被视为是一些
应用于其身上的“描述符”。上一小节中我们已经创建好了这些描述符，那么在本节我们要做的就是将这些定义好的描述符
对应的操作绑定到对应的“被操作对象”身上，也就是顶点缓冲区/索引缓冲区。
*/

/*
    第一步，
    描述符的类型有许多种，但在当前章节中我们仅讨论“统一缓冲区对象”（Uniform Buffer Objects / UBO）；之后的
章节中我们还会讨论其他类型的描述符，但使用方法基本上大同小异。最开始我们使用一个C风格的结构体来创建这个描述符。
从以下的描述中不难看出，这就是基于rasterization方法渲染管线中最最常见/典型的MVP变换阵。
    在这个结构体中使用 glm 定义的矩阵将在之后的步骤中被存储到一个 VkBuffer 类型的对象中。
    接下来的一步，我们要在 Vertex Shader 顶点着色器中进行修改。
*/

// struct UniformBufferObject
// {
//     glm::mat4 model; // 模型变换阵
//     glm::mat4 view;  // 视口变换阵
//     glm::mat4 proj;  // 投影变换阵
// };

struct UniformBufferObject
{
    glm::vec2 foo;
    /*
        第五步，如果我们尝试添加以上的 foo 成员变量，并且在vertex shader中也添加相同的成员，
    我们会发现，编译后运行程序，无法得到之前在屏幕中央旋转的四边形。
        这就引出了一个问题，我们之前只是按照程序要求去写了，但是并没有去思考C++中定义的这个结构
    体是如何与vertex shader中的变量去一一对应的。目前我们添加了这个成员，并且在着色器文件中也
    是在首位添加相同类型的成员，但对应关系却错乱了，导致我们无法成功渲染出想要的结果。这就表明实
    际上二者对应关系并不是这样按照顺序默认对应，应该会有某种限制来进行约束。
        这引出了下一个我们要讨论的重点：数据对齐！
        Vulkan中的数据对齐方案如下：
        标量数据：占有4Bytes，并且必须按照每4字节为最小单位进行对齐，也就是offset必须为4Bytes
    的倍数（4Bytes=32bits floats）
        vec2：占有8Bytes，对齐所需8Bytes的倍数
        vec3/vec4：16Bytes，对齐所需16Bytes的倍数
        结构体（含有嵌套关系的数据）：这个比较特殊，它的最小对齐单位应该是其内部成员基础对齐单位
    之和的倍数，这个倍数不足16的按照16来计算
        mat4：相当于一个方阵，由4个vec4拼接而成故为 4×16Bytes = 64Bytes；占有64Bytes，对齐
    所需必须是16Bytes的倍数

        注意，以上说的都针对在 C++ 中的定义

        那么针对当前这个 UniformBufferObject 中的成员：
        由于定义了以上的foo为vec2类型，占用8Bytes，故：

        model模型变换阵：基础偏移为 8Bytes
        view视图变换阵：基础偏移为 8Bytes+64Bytes = 72Bytes
        proj投影变换阵：基础偏移为 72Bytes+64Bytes = 136Bytes

        可见以上没有一个数值是16的倍数，这不满足mat4类型的对齐要求，所以才会出现错误
    */
    // glm::mat4 model;
    // 如果你完成了第六步，可以打开以上的注释
    // glm::mat4 view;
    // glm::mat4 proj;
    /*
        但是以上的任何一个偏移量都不是16的倍数（这是由于在结构体最开始定义的vec2造成的，它有
    8Bytes的偏移），所以我们修改如下，使得model被强行置为16Bytes的偏移，可以预见到的是它之后
    的view和proj矩阵也同样满足了16Bytes倍数偏移的需求。
        （使用到C++11新特性alignas）
        如果现在运行程序，会发现得到了正确的结果
    */
    alignas(16) glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    /*
        然而除了以上的方法，我们还有另外的方法来解决这一问题，且不必总是费心考虑数据对齐的问题。
    详见第六步。
    */
};

extern VkDescriptorSetLayout descriptorSetLayout;

extern std::vector<VkBuffer> uniformBuffers;
extern std::vector<VkDeviceMemory> uniformBuffersMemory;
extern std::vector<void *> uniformBuffersMapped; // 这个是做什么的？没有看懂

extern VkDescriptorPool descriptorPool;
extern std::vector<VkDescriptorSet> descriptorSets;

/**
 *  创建 descriptorSetLayout，作为 draw time 更改MVP变换阵并影响 vertex buffer 的接口
 * */
void createDescriptorSetLayout();

/**
 *  创建 uniform buffer，为存储的 MVP 变换阵分配内存。
 * */
void createUniformBuffers();

/**
 *  创建 descriptor pool
 * */
void createDescriptorPool();

/**
 *  创建 descriptor sets
 * */
void createDescriptorSets();

/**
 * 根据当前帧信息，更新 MVP 变换阵。并将变换阵携带的数据拷贝到预先创建好的GPU内存上。
 * draw time 运行时函数。
 * */
void updateUniformBuffer(uint32_t currentImage);

/**
 *  注销 uniform buffer 并释放其对应的GPU内存
 * */
void cleanupUniformBuffer();

/**
 *  注销 descriptorSetLayout
 * */
void cleanupDescriptor();

#endif