#ifndef INIT_VK_H
#define INIT_VK_H




#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// 容器
#include <string>
#include <vector>


#include "vaildation_layer.h"

// 声明全局vulkan实例
extern VkInstance instance;

// 只有这个初始化函数的接口暴露给其他文件调用
void initVulkan();


#endif

