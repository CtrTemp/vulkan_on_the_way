#include "uniform_buffer.h"

/**
 * 定义描述符接口，将uniform buffer中mvp变换阵应用到vertex buffer上的接口
 * */
VkDescriptorSetLayout descriptorSetLayout;   // 创建 descriptorSetLayout
VkDescriptorPool descriptorPool;             // 创建 descriptorPool
std::vector<VkDescriptorSet> descriptorSets; // render loop中每帧图片都要有一个 descriptorSet

std::vector<VkBuffer> uniformBuffers;             // render loop 中每一帧图都应该对应一个操作 vertex buffer 的 uniform buffer
std::vector<VkDeviceMemory> uniformBuffersMemory; // uniform buffer 对应的GPU内存分配
std::vector<void *> uniformBuffersMapped;         // 这个是做什么的？没有看懂

/**
 *  descriptorSetLayout 作为运行时接口，在 draw time 更改变换阵并应用到 graphic pipeline 中。
 * */
void createDescriptorSetLayout()
{

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 在顶点着色器阶段引入描述符
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

/**
 *  创建 uniform buffer，其中存储的应该是 MVP 变换阵，应用于 vertex buffer 的变换矩阵。当前 RenderLoop 中的
 * 容许的最大image数量，每个image都应该配备一个 uniform buffer（与 command buffer 等同）
 * */
void createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    /*
        为每个buffer进行配置，注意以下配置，我们这里直接将“统一缓冲区”定义在CPU可见的内存区域，并没有
    为其再在GPU上开辟显存。这是由于每次/帧我们都要对图像的mvp变换阵进行更新，并应用，如果再多一个“阶段
    缓冲区”到真正“统一缓冲区”的映射将拉低运行效率。
    */

    /**
     *  逐个创建uniform buffer：
     *  1、注意这里将uniform直接创建在了CPU可访问的内存上，并将其映射到uniformBuffersMapped，没有使用staging buffer
     * 作二次映射以及device to device的数据拷贝。
     *  2、注意这里只进行了映射，却没有急着进行数据拷贝，因为真正要拷贝的数据是mvp变换阵，需要等到运行时才能确定。
     * */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffers[i],
                     uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

/**
 * 根据当前帧信息，更新 MVP 变换阵。并将变换阵携带的数据拷贝到预先创建好的GPU内存上。
 * draw time 运行时函数。
 * */
void updateUniformBuffer(uint32_t currentImage)
{

    /*
        使用chrono库中的精确时钟为每帧进行计时，这保证了我们可以精确控制每单位时间进行矩阵变换（比如旋转）
    操作的位移/角度。在之后的代码中，我们精确控制每秒图形旋转的角度，而不是每一帧的旋转角度。
    */
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    /*
        使用glm::rotate函数对图形进行“旋转”操作，time * glm::radians(90.0f)保证每秒旋转90度（这里
    应该对应的是沿图形的 y 轴坐标进行旋转），注意这里进行的是 M -> 模型变换阵
    */
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    /*
        视口变换阵相关操作：以下的操作使得我们并非沿着正冲着表面的方向观察，而是在其斜上方45度的位置进行观察，
    这个是固定的，并不随着每帧的变化而变化。
    */
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    /*
        投影变换阵操作：这里选用透视投影法（远小近大）从而获得更加真实的视图（与之对应的是平行投影法），同样是
    45度斜侧观察。考虑当前视图交换链中的图像大小。
    */
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    /*
        因为GLM最初是为OpenGL设计的，而OpenGL中的Y轴坐标和Vulkan中的Y轴计算方式恰好相反，所以这里应该考虑
    使用以下的操作进行矫正（当然你也可以暂时注销这里，查看是否会获得Y轴镜像的效果）
    */
    ubo.proj[1][1] *= -1;

    /*
        因为没有使用staging buffer，这里省略掉一步映射，可以直接将数据拷贝到开辟好的CPU可访问的GPU内存地址，如下：
    */
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

/**
 *  创建descriptor pool
 *  描述符集（descriptor sets）并不能被直接创建，它们应该被从描述符池（descriptor pool）中被分配得到。这个模式与
 * 之前提到的命令池（Command pool）与创建命令缓冲区（Command Buffer）中的对应关系十分类似。
 * */
void createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // 最大 descriptor sets 数量

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

/**
 *  创建描述符集（descriptor sets）
 * */
void createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    // 以下定义分配信息，用到的信息均是之前分配好的。
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // 为每个描述符集配置信息
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;       // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

/**
 *  注销 uniform buffer 并释放其对应的GPU内存
 * */
void cleanupUniformBuffer()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
}

/**
 *  注销 descriptorSetLayout
 *  descriptorSetLayout 注销后 descriptorPool和descriptorSet也会被自动注销
 * */
void cleanupDescriptor()
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
