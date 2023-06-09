#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
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

#include "texture.h"
#include "buffers/buffers_operation.h"

#include "interaction/camera.h"

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
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
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