#include "vertex_buffer.h"

// 为了方便的导入Obj格式模型文件，我们引入这个第三方库，注意要在源文件中引入
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// 指明模型文件/模型对应贴图文件路径
// const std::string MODEL_PATH = "../models/viking_room.obj";
// const std::string MODEL_PATH = "../models/bunny_low_resolution.obj";
const std::string MODEL_PATH = "../models/fish.obj";
// const std::string TEXTURE_PATH = "../textures/viking_room.png";

// const std::vector<Vertex> vertices = {
//     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

//     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

// const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

// 我们从 model 中导入顶点相关的数据，而非写死在内存中
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

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

/******************************************** 以下是模型导入部分 ********************************************/

/**
 *  模型文件导入
 * */
void loadModel()
{

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    /*
        从我们预定义的文件路径中读入，OBJ 文件由顶点位置信息/顶点法线信息/纹理坐标信息/表面组成，分别
    由v/vn/vt/f几个字段进行标识。
        以下使用 attrib 字段作为v/vn/vt三者的存储器，并使用attrib中的vertices/normals/texcoords
    几个字段分别指示；使用 shapes 字段作为f的存储器。
    */
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
    {
        // 内置报错信息，如果有错误会自动抛出对应提示信息
        throw std::runtime_error(warn + err);
    }

    std::cout << "shapes size = " << shapes.size() << std::endl;
    std::cout << "shapes = "
              << shapes[0].mesh.indices[0].vertex_index << "; "
              << shapes[0].mesh.indices[1].vertex_index << "; "
              << shapes[0].mesh.indices[2].vertex_index << "; "
              << std::endl;
    // /*
    //     然而对于以上Obj文件中的face字段，我们需要一个单独的循环来将其中的多个表面拼接成一个多边形网格
    // */
    // for (const auto &shape : shapes)
    // {
    //     for (const auto &index : shape.mesh.indices)
    //     {
    //         Vertex vertex{};

    //         // 每个三角形表面对应的顶点坐标
    //         vertex.pos = {
    //             attrib.vertices[3 * index.vertex_index + 0],
    //             attrib.vertices[3 * index.vertex_index + 1],
    //             attrib.vertices[3 * index.vertex_index + 2]};

    //         // 每个三角形表面对应的UV贴图坐标
    //         // vertex.texCoord = {
    //         //     attrib.texcoords[2 * index.texcoord_index + 0],
    //         //     attrib.texcoords[2 * index.texcoord_index + 1]};
    //         vertex.texCoord = {
    //             attrib.texcoords[2 * index.texcoord_index + 0],
    //             1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

    //         // 由于之后要应用贴图中的颜色，所以这里颜色可以先设置为一个常量
    //         vertex.color = {1.0f, 1.0f, 1.0f};

    //         vertices.push_back(vertex);
    //         indices.push_back(indices.size());
    //     }
    // }
    /*
        以上的步骤执行完毕后，我们就能看到一个完整的结果了。真的是激动人心！！！！
        但是仔细观察还是会发现一些错误：纹理贴图在一些部分是完全丢失/错误的。这还是由于当前的Obj格式
    文件中坐标轴翻转的问题，这次是UV方向上的反战，如上修改（其实对应的还是Y轴方向反了，看来Obj格式还
    是默认沿用当时的OpenGL标准，Y/V轴向下是正方向，起点在图片/屏幕左下角）。
    */

    /*
        另外：As the model rotates you may notice that the rear (backside of the walls) looks
    a bit funny. This is normal and is simply because the model is not really designed to be
    viewed from that side.
        （这里说的是，如果你从背面看这个“兽人小屋”，会发现一部分是“镂空”的，看起来有些滑稽，这是因为这个模型
    当初的设计也没想让你从那个角度看过去）
    */

    /*
        第七步，
        不过不幸的是，到目前为止，我们还没有用到顶点缓冲区，换句话说，这里还有大量的顶点重用可以被优化。
    于是我们使用C++中的map来做这个优化，注释以上的代码，并作如下修改：
    */

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            // 顶点坐标
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            // UV贴图坐标
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            // 顶点对应颜色（预设为常量）
            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}
