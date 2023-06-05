#include "uniform_buffer.h"

// 第四步，为成员变量添加新成员，描述符布局相关
VkDescriptorSetLayout descriptorSetLayout;

/*
    第七步，配合第六步，添加一些成员变量
    注意，为什么这里我们以数组的形式添加这些“统一缓冲区”？首先要明确这些缓冲区中提供的是我们对场景
中的物体进行变换的矩阵。而且我们为了让渲染过程中场景中的物体运动起来，每一帧我们都要更新这些变换阵
中的具体值。而同一时间在交换链中渲染的图像又不止一帧，所以出于统一/效率考虑，我们必须要在这里以数组
的形式设置多个“统一缓冲区”，且数量应该与交换链中最大图像数量相一致。这个我们在之后的配置函数中将会
进行详细配置。
*/
std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;
std::vector<void *> uniformBuffersMapped; // 这个是做什么的？没有看懂

// 为 descriptorPool 创建一个成员变量用于配置
VkDescriptorPool descriptorPool;
// 交换链中的每帧图片都要有一个 descriptorSet
std::vector<VkDescriptorSet> descriptorSets;

/*
    第三步，创建管道描述符布局相关的信息
*/
void createDescriptorSetLayout()
{

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    // descriptorType字段：刚刚提到，是一个“统一缓冲区”类型，此处的设置与之对应
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // 我们只创建一个描述符
    uboLayoutBinding.descriptorCount = 1;
    // stageFlags字段描述我们在着色器的哪个阶段引用这个描述符（以下表示我们在顶点着色器阶段引入）
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // pImmutableSamplers字段与图像采样相关，这里不进行过多描述，也默认不进行配置
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    // 第四步，配置结构体以及真正创建描述符布局对象
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding; // 刚刚设置的绑定信息

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

// 第六步
void createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    // 重置数组大小，与预设当前交换链中最大图片数量相一致
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    /*
        为每个buffer进行配置，注意以下配置，我们这里直接将“统一缓冲区”定义在CPU可见的内存区域，并没有
    为其再在GPU上开辟显存。这是由于每次/帧我们都要对图像的mvp变换阵进行更新，并应用，如果再多一个“阶段
    缓冲区”到真正“统一缓冲区”的映射将拉低运行效率。
    */
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

/*
    第八步，创建更新mvp变换阵的函数，该函数将在每一帧进行调用，使得mvp变换阵随着“统一缓冲区”的变换而更新。
应用新的变换阵之后，显示的图像会随着进行位移/旋转等变换。输入的单数是当前帧。
*/
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
        因为没有“阶段缓冲区”，这里省略掉一步映射，可以直接将数据拷贝到开辟好的CPU可访问的内存地址，如下：
    */
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

/*
    第一步，创建描述符池（descriptor pool）
    描述符集（descriptor sets）并不能被直接创建，它们应该被从描述符池（descriptor pool）中
被分配得到。这个模式与之前学习中提到的命令池（Command pool）与创建命令缓冲区（Command Buffer）
中的对应关系十分类似。
    以下我们就将编写一个“描述符池”来设置它
*/
void createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    // type 字段指定池类型，这里对应前一个小节使用的“统一缓冲区”
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    /*
        descriptorCount 字段指定我们要一次性创建多少个池，由于我们将为在交换链中的每帧图像都预
    置一个池，所以按照 MAX_FRAMES_IN_FLIGHT 字段给出的值来确定创建数量
    */
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // 对应创建信息
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    /*
        类似于 descriptorCount 字段用于指示要创建的池的数量多少，maxSets 字段用来指示我们要为
    最多多少个池分配内存
    */
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

/*
    第二步，创建描述符集（descriptor sets）
    如同第一步所讲，描述符集是我们真正要使用的部分，它将从描述符池（descriptor pool）中分配得来。
*/
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

    // 设置循环，为每个描述符集配置信息
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        /*
            如果想要重写/覆盖整个缓冲区？？？
            就像我们在本例中一样，那么也可以使用VK_whole_SIZE值作为范围。使用vkUpdateDescriptorSet
        函数更新描述符的配置，该函数将VkWriteDescriptorSet结构数组作为参数。（直译，这里没看懂）
        */
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        // 没理解
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        // 没理解
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;       // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        // 没理解，以上这部分回来要重看
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void cleanupUniformBuffer()
{
    // 逐一销毁/释放“统一缓冲区”
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
}

void cleanupDescriptor()
{
    // 销毁描述符相关的成员变量
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
