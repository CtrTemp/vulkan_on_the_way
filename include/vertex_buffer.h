#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

// 首先应引入GLM矩阵库
#include <glm/glm.hpp>

#include <array>

// #include "physical_device_queue.h"
// #include "logical_device_queue.h"

// #include "command_buffer.h"

#include "buffers/buffers_operation.h"


/*
    Introduction 01：
    注意顶点着色器部分，在以前的章节中，我们都是直接将顶点信息硬编码在了文件中，但在接下来，我们将在内存中定义
并填充这些顶点缓冲区。我们将从最简单的方法开始，在CPU可见的缓冲区使用memcpy指令将顶点数据拷贝到顶点缓冲区中。
（推测顶点缓冲区应该是在GPU内部的显存中）之后我们将了解如何使用暂存缓冲区将顶点数据复制到高性能内存中。
*/

/*
    Introduction 02：
    Buffer，缓冲区，在Vulkan中被视为一块GPU可读取的存储区，可以存放任意数据。它们可以被用作存放顶点数据，这也
正是我们在这个章节即将要做的。然而它们也可以被用作一些其他的功能，这在我们之后的章节中会一一介绍。不同于之前我们
配置过的一些Vulkan对象，Buffer并不会自动为自身分配存储空间，在这个章节，我们将学会自己为其分配存储空间。

    （运行程序前请注意，在上一章我们修改了vertex shader文件，请注意是否使用compile.sh进行了编译）

    下一个章节中，我们将使用另一个方法来将顶点数据导入到顶点缓冲区中，性能会提高，但需要更多的工作要做。
*/


/*
    Introduction 03：
    当前的顶点缓冲区运行良好，但可被我们当前主机CPU访问的内存区域对于设备GPU来说却并不一定是最优的（按照读取速率来说），
对于GPU来说最优的内存类型应该是带有 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 标志的内存区域，而这块区域CPU一般无法在
专业图形卡上访问到。
    （其实这部分也是和写CUDA代码时候遇到的介绍类似，当时确实有介绍到有部分区域仅对设备可见，主机不可见，应该是运行时）
    在这个章节中，我们将创建两个顶点缓冲区。其中第一个叫做“分段缓冲区”（staging buffer），它被建立在CPU可访问的内存
上，我们同样将从定义好的顶点数组中把数据导入到这块区域；第二个“顶点缓冲区”（vertex buffer）建立在GPU本地内存上（CPU
应无法直接访问）。
    随后，我们要做的就是使用缓冲区拷贝命令来将“分段缓冲区”中的内容转移到真正显卡上的“顶点缓冲区”。
    其实观察以上的说明，很容易发现总共进行了两次数据转移：第一次是在CPU程序中定义好的“顶点数组”中的数据，使用一些命令将
其转移到CPU可访问的一块开辟好的内存区域；第二次将这块CPU可访问区域的内容转移到GPU本地内存区。


    在这里我们要执行的是“缓冲区复制”相关的指令，但这些指令同样需要我们将其放入一个队列中执行，而且这个队列必须支持
VK_QUEUE_TRANSFER_BIT 指示位。但现在很好的一点是，目前我们已经知晓的图形队列（VK_QUEUE_GRAPHICS_BIT）和计算队列
（VK_QUEUE_COMPUTE_BIT）均隐式地支持这些“缓冲区复制”操作。所以我们不需要显示地再去创建一个专门的队列支持这些操作。
    （这里留了一个引子，你可以挑战一下，创建专门的队列支持这些“缓冲区复制”指令，需要修改哪些可以看tutorial）


    总结一下，其实上一个小节我们的程序是GPU读取在CPU内存中开辟的“顶点缓冲区”来进行绘制渲染的，而这样的效率必然不如GPU
直接读取本地内存进行绘制渲染。所以这小节我们做的就是这么一回事，在GPU上开辟内存，将CPU创建的数据导入到该GPU本地缓存中，
从而大大提高数据访问效率，从而提高之后的绘制效率。
*/

/*
    Introduction 04：
    开始进入实际的领域，我们不仅仅要绘制三角形，实际上，更多情况下，我们遇到的是三角形拼接成的多
边形，在这种情况下，很多三角形会共享顶点。我们在本小节就开始探讨这个问题：如何绘制三角形共享顶点
的情况？首先，我们从一个长方形/四边形开始探讨。

    画一个四边形需要两个三角形，这意味着我们需要六个顶点，然而其中必然有两个三角形共享顶点，所以
总共其实我们只有四个可以使用的顶点。如果我们一贯坚持使用六个顶点，那么就需要对其中共用的顶点进行
复制，这造成了大量冗余，在更复杂的3D网格（mesh）中更是如此！
    所以，我们的解决方法是：使用索引缓冲区（Index Buffer）。

    索引缓冲区本质上是指向顶点缓冲区指针的数组。它将允许你对顶点序列进行重新排布，从而对顶点数据
进行复用。具体我们还是进入代码看例子吧。
*/


/*
    第一步：使用GLM库创建顶点类型结构体，当前的结构体应至少包含顶点颜色和位置两个基本属性。
*/
struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    /*
        第二步：告诉Vulkan如何将当前格式的数据上传到GPU显存上，并保证其可以正确传递到顶点着色器。这应该通过
    传递一个 VkVertexInputBindingDescription 类型的结构体来实现，于是我们在当前描述顶点的结构体中添加以
    下的成员函数来返回以上所需信息。
    */
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        // stride 字段用来描述顶点之间的存储字节间隔，这里我们连续传递，所以间隔就恰好等于结构体大小
        bindingDescription.stride = sizeof(Vertex);
        /*
            inputRate 用于描述以上 stride 对应移动对象是什么？
            VK_VERTEX_INPUT_RATE_VERTEX：表示每次逐顶点进行移动读取
            VK_VERTEX_INPUT_RATE_INSTANCE：表示每次逐实例进行移动读取
            （我们这里还不涉及实例化渲染，所以使用前者）
        */
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    /*
        第三步：添加另一个用于描述如何处理输入顶点数据的函数，同样要返回一个对应的结构体，如下：
    */
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0; // 指明？？？这个没理解
        /*
            tutorial：The binding parameter tells Vulkan from which binding the per-vertex data comes.
            直译：binding参数告诉Vulkan每个顶点数据来自哪个绑定。
        */
        attributeDescriptions[0].location = 0; // 指明在当前结构体中的位置索引
        /*
            format 字段有以下可选项：
            VK_FORMAT_R32_SFLOAT：对应float类型
            VK_FORMAT_R32G32_SFLOAT：对应vec2类型，内部数值单元是float类型
            VK_FORMAT_R32G32B32_SFLOAT：对应vec3类型，内部数值单元是float类型
            VK_FORMAT_R32G32B32A32_SFLOAT：对应vec4类型，内部数值单元是float类型

            format 字段定义值应该与vertex shader中对应值的位宽/维度相一致，这里允许此处定义值的维度超出
        vertex shader中的维度，且这种情况下多出的维度会被默认弃用！当这里定义的维度小于vertex shader中
        对应值的时候，则BGA组件将使用默认值（0,0,1）（这里啥意思没看懂，，，）
            注意下标索引，这里的索引是0,对应的是pos的vec2,故选用 VK_FORMAT_R32G32_SFLOAT 进行配置
        */
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        /*
            offset 字段：指定当前属性与结构体开头的位置相差多少个字节，这里我们借助标准库中的offsetof()
        运算符进行计算。（或者对于当前属性 pos 直接写0就可以，因为它是本结构体的首个属性）
        */
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // 同样的，以下对color属性进行设置，其对应的索引为1
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;                        // location +1
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 对应vec3
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

// 借助定义的vertex结构体，预先定义一个三角形顶点序列，其中的数据与原先在.vert文件中的定义完全相同
extern const std::vector<Vertex> vertices;

// 添加 Vertex buffer 对应的成员变量。
extern VkBuffer vertexBuffer;

// GPU 上的内存创建相关的成员变量
extern VkDeviceMemory vertexBufferMemory;


extern const std::vector<uint16_t> indices;
extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;

void createVertexBuffer();

void createIndexBuffer();

void cleanupVertexBuffer();

#endif