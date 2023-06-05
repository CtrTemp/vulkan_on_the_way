#include "vertex_buffer.h"

// // 借助定义的vertex结构体，预先定义一个三角形顶点序列，其中的数据与原先在.vert文件中的定义完全相同
// const std::vector<Vertex> vertices = {
//     {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//     {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

/*
    第一步，首先我们应该创建一个用于构建四边形网格的顶点序列，当然，它是由两个三角形构成的。
但我们只需要四个顶点来表示它（其中的第二项与第三项是被复用的），如下：
*/
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

/*
    第二步，我们将再创建一个索引缓冲区（index buffer）用来指示这些顶点将如何排列，构建
一个四边形网格（其实是如何被两个三角形所使用），这里使用 uint16_t 或 uint32_t 均可，但
鉴于我们当前的网格顶点数量不会超过65536，所以在这里我们坚持使用 uint16_t 类型。
*/
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0};

// 添加 Vertex buffer 对应的成员变量。
VkBuffer vertexBuffer;
// GPU 上的内存创建相关的成员变量
VkDeviceMemory vertexBufferMemory;

/*
    第三步，
    与 vertexBuffer 的创建类似，我们在这里同样需要在成员变量中添加以下变量
用于索引缓冲区的创建，从而指示我们用更少的/不冗余顶点创建更复杂的多边形网格。
*/
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;


// 第一步，在这里我们对 Vertex Buffer 进行创建
void createVertexBuffer()
{
    // VkBufferCreateInfo bufferInfo{};
    // bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // /*
    //     size字段：直接用于指示要创建的顶点缓冲区的大小（Bytes）。以下的 vertices 为我们预先定义在
    // 文件中的全局变量，使用sizeof运算符计算其单个大小再乘以其向量长度。
    // */
    // bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    // /*
    //     usage字段：用于指示创建Buffer的用处，始终要注意，我们是在创建一个buffer用作顶点缓冲区，而非
    // 使用一个专门的为vertex buffer配置的结构体为其配置！所以以下的字段表示我们的buffer是用于顶点缓冲
    // 区的。
    // */
    // bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    // /*
    //     sharingMode字段：这部分缓冲区是否独占内存，与交换链中的图像类似，它也可以选择被多个队列所共享，
    // 这里我们只将其用于图形队列，不共享。
    // */
    // bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // // 创建！记得在成员变量中添加 vertexBuffer 成员变量
    // if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create vertex buffer!");
    // }

    // /*
    //     第二步。
    //     以上顶点缓冲区创建完之后，我们实际上还是没有为其分配内存，以下为其进行具体分配。
    // */
    // /*
    //     首先我们应该获取需要多少内存的信息，并将其保存在一个类型为 VkMemoryRequirements 的结构体中。
    // VkMemoryRequirements 结构体应该有以下字段：
    //     size：总共需要多少字节空间
    //     alignment：存储起始点偏移量，单位仍然是字节
    //     memoryTypeBits：适用于缓冲区的内存类型的位字段（直译）
    // */
    // VkMemoryRequirements memRequirements;
    // vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    // /*
    //     第四步，使用刚刚创建的寻找内存分配策略的函数，并以此为依据分配内存
    // */
    // VkMemoryAllocateInfo allocInfo{};
    // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // allocInfo.allocationSize = memRequirements.size;
    // // 选择GPU上一块host端可访问的位置开辟内存，存储顶点信息
    // // 疑问？这是在GPU上开的还是CPU上开的？
    // allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // // 注意，正是 vkAllocateMemory() 函数，为我们在GPU设备上分配了相关内存
    // if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to allocate vertex buffer memory!");
    // }
    // /*
    //     之后将本地主机（CPU）上的 vertexBuffer 与设备（GPU）上分配的内存空间 vertexBufferMemory 建立
    // 链接，前三个参数不用说，第四个参数指的是地址偏移。如果偏移不是0，则其被要求可以被 memRequirements 对象
    // 的 aligence 字段整除。
    // */
    // vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    // /*
    //     第五步，将CPU设备上的vertex buffer中的内容拷贝到GPU设备上（存在GPU设备上我们已经开辟好的空间中）
    // 感觉这步和写CUDA代码十分相似～ 也是必须要先指定内存分配，之后再进行拷贝。不谋而合，不过深层次上的含义也
    // 确实相符，CUDA的确是更底层的对GPU的实际操作。那么在Vulkan这种相对顶层的API想要操作底层，也必须符合这
    // 个层面的规范，并获取充足的信息。
    //     参数2为设备地址，也就是目标地址dst
    //     参数4为本机地址，也就是源数据地址src
    //     参数3/5分别表示偏移量，此处都是0
    //     最后一个参数指示映射后指定的内存地址的指针？？？这没理解

    //     // 下面这部分就类似于CUDA上的内存拷贝，也比较容易理解
    // */
    // void *data;
    // // 首先是建立映射，并找到要传入的地址，将要传入的地址保存在data中
    // vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    // // 之后进行数据传输，将数据输送到GPU设备上目标位点
    // memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    // // 最后取消映射（这里为何取消映射？）
    // vkUnmapMemory(device, vertexBufferMemory);

    // vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    // 第一步，将以上代码段的信息放入以下函数中进行封装（暂时的）

    // VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    // createBuffer(bufferSize,
    //              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //              vertexBuffer,
    //              vertexBufferMemory);

    /*
        第二步，
        进行更改后， createVertexBuffer() 函数将用于创建对于主机CPU可见的临时缓冲区，以及对GPU设备可见的真正的
    顶点缓冲区。
    */

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    /*
        注意这里相较于第一步进行的修改，首先这里我们是要创建临时的“分段缓冲区”，目的是在CPU可见的内存上开辟一块空间
    来格式化存储本地数组中定义的顶点数据。而后这部分数据将被拷贝到仅GPU可见的内存中。所以这里的标志为被设置成了：
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT，表示它之后将会被用做拷贝操作的源数据。
        另外下面两个用于表示其属性的标志位被设置成了 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT：表示主机CPU可见的；
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT：参见上一小节（应该是为了解决一个操作与实际不一致的现象，这是由于驱动
    为了兼容caching预存取的问题造成的，当复制命令下达后，驱动并不一定会立刻将数据进行拷贝，而是等到有人访问且未命中
    之后再考虑取）
    */
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    /*
        以上这些操作将本地数组中的数据拷贝到本机可见的/刚刚开辟好的/暂时存在的“分段缓冲区”中
        （上一小节也提到过，但这小节进行印象加深）
    */

    // 第二步，创建GPU可见的真正“顶点缓冲区”
    /*
        以下的 VK_BUFFER_USAGE_TRANSFER_DST_BIT 表示它用作拷贝操作的数据目标地址/承接者，而
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 表示它作为真正的“顶点缓冲区”使用，最后 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    表示它作为仅被GPU设备可见的本地内存被开辟。
    */
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer,
                 vertexBufferMemory);
    /*
        注意，由于被开辟缓冲区的位置CPU不能直接访问，所以我们也就不能直接使用 vkMapMemory 命令来进行数据映射以及之后的
    拷贝操作，我们将借助以下的函数来进行实现。
    */
    /*
        第四步，使用特制函数将“分段缓冲区”中的数据拷贝到“顶点缓冲区”中。
    */
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // 同样记得销毁（不再成员函数中定义的都要在程序内部进行销毁，而不是cleanup函数中）

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/*
    第三步，创建顶点缓冲区，并为其填充数据
    可以看到，这与以上的 Vertex Buffer 的创建过程几乎完全一致，有了createBuffer()和copyBuffer()
等函数作为辅助后，这个过程变得更为简单。
*/
void createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    // 对索引缓冲区的创建同样借助了“分段缓冲区”，先在CPU可访问区创建，而后导入到GPU本地内存
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // 仅仅是以下的 VK_BUFFER_USAGE_INDEX_BUFFER_BIT 字段稍有不同
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer,
                 indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void cleanupVertexBuffer()
{
    // 在交换链销毁后，销毁顶点缓冲区，注意这里释放的应该是 CPU 设备上的空间
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    // 释放内存，注意这里释放的是GPU设备上的显存
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

