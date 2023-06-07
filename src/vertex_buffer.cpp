#include "vertex_buffer.h"

// 顶点源数据，这里我们写死在内存中

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

/**
 *  顶点索引源数据，由于同一个顶点可以是多个多边形的顶点，是可以被复用的，
 * 故此，索引数据用来指示哪些顶点将怎样将组成多边形，其中存的是顶点数组中的索引
 * */
const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

VkBuffer vertexBuffer;             // vertex buffer 实例
VkDeviceMemory vertexBufferMemory; // vertex buffer 对应在 GPU device 上的内存

VkBuffer indexBuffer;             // index buffer 实例
VkDeviceMemory indexBufferMemory; // index buffer 对应在 GPU device 上的内存

/**
 *  GPU上创建 Vertex Buffer，并导入顶点数据
 * */
void createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    /**
     *  GPU上访问较快的内存类型CPU无法直接访问到，所以这里我们不能“一步到位”的方式去从CPU直接将数据拷贝到这块GPU
     * 内存。合理的方式是：
     *  1、先在GPU上创建一个Buffer，并为其分配一块CPU可访问的内存空间作为“中间桥”。
     *  2、将源数据从CPU拷贝到以上Buffer对应的中。
     *  3、在GPU上创建一个Buffer,并为其分配一块CPU无法访问的内存（最终数据存储的位置，方便GPU快速访问，但CPU无法访问）。
     *  4、将1、中创建的内存中的数据以 device to device 的方式拷贝到3、创建的内存区域。
     *  5、注销1、创建的Buffer，并释放其对应的内存区域。
     * */

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // 1、在GPU上创建一个Buffer，并为其分配一块CPU可访问的内存空间作为“中间桥”。
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    // 2、将源数据从CPU拷贝到以上Buffer对应的中。
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // 3、在GPU上创建一个Buffer,并为其分配一块CPU无法访问的内存（最终数据存储的位置，方便GPU快速访问，但CPU无法访问）。
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer,
                 vertexBufferMemory);

    // 4、将1、中创建的内存中的数据以 device to device 的方式拷贝到3、创建的内存区域。
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // 5、注销1、创建的Buffer，并释放其对应的内存区域。
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/**
 *  GPU上创建 Index Buffer，并导入顶点索引数据
 * */
void createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

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

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer,
                 indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/**
 * 注销 Vertex Buffer，释放其对应的内存
 * */
void cleanupVertexBuffer()
{
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

/**
 * 注销 Index Buffer，释放其对应的内存
 * */
void cleanupIndexBuffer()
{
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
}
