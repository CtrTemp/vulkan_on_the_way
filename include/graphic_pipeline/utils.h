#ifndef GRAPHIC_PIPELINE_UTILS
#define GRAPHIC_PIPELINE_UTILS


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

#include "logical_device_queue.h"


// 第三步，将编译好的文件导入到当前渲染管线中，首先我们需要一个加载文件的函数
static std::vector<char> readFile(const std::string &filename)
{
    /*
        设置读文件模式：
        ate：从文件末尾开始读，
        binary：以二进制文件读取
    */
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    // 文件打开失败则抛出错误
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    // 从文件末尾开始读取的好处是，我们可以使用读取位置来确定文件的大小并分配缓冲区：
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    // 而后我们再回到文件开头，一次读完所有字节：
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


/*
    在进行渲染前，我们应该以同样的方式创建一个 shader module，创建方式大同小异：
都是引入一个参数结构体，进行配置并传入。
    这里传入的参数只有一个，就是刚刚我们从预编译文件中读入的二进制文件，由于是以二进制
方式读入，这里传入的参数类型就给定为char类型数组，刚好每个元素代表一个字节长度。
*/
static VkShaderModule createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    // 创建一个 Shader
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

#endif