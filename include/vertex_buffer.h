#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>


extern const std::string MODEL_PATH;
extern const std::string TEXTURE_PATH;

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

/**
 * vertex 结构体
 * */
struct Vertex
{
    glm::vec3 pos;      // 顶点位置坐标
    glm::vec3 color;    // 顶点颜色
    glm::vec2 texCoord; // 添加UV贴图对应的二维纹理

    /*
        第二步：告诉Vulkan如何将当前格式的数据上传到GPU显存上，并保证其可以正确传递到顶点着色器。这应该通过
    传递一个 VkVertexInputBindingDescription 类型的结构体来实现，于是我们在当前描述顶点的结构体中添加以
    下的成员函数来返回以上所需信息。
    */
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        // pos vec3
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // color vec3
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // texCoord vec2
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    // 算符重载
    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

/*
    第八步，
    同样是配合第七步中 unordered_map 的使用，需要为其创建一个特殊的哈希查找函数
*/
namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.color) << 1)) >>
                    1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

extern std::vector<Vertex> vertices;      // 声明 存储在CPU内存上的“顶点”源数据
extern VkBuffer vertexBuffer;             // 声明 vertex buffer 实例
extern VkDeviceMemory vertexBufferMemory; // 声明 vertex buffer 对应在 GPU device 上的内存

extern std::vector<uint32_t> indices;    // 声明 存储在CPU内存上的“顶点索引”源数据
extern VkBuffer indexBuffer;             // 声明 index buffer 实例
extern VkDeviceMemory indexBufferMemory; // 声明 index buffer 对应在 GPU device 上的内存

/**
 *  GPU上创建 Vertex Buffer，并导入顶点数据
 * */
void createVertexBuffer();

/**
 *  GPU上创建 Index Buffer，并导入顶点索引数据
 * */
void createIndexBuffer();

/**
 * 注销 Vertex Buffer，释放其对应的内存
 * */
void cleanupVertexBuffer();

/**
 * 注销 Index Buffer，释放其对应的内存
 * */
void cleanupIndexBuffer();

/**
 *  模型文件导入
 * */
void loadModel();

#endif