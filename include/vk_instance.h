#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// 容器
#include <string>
#include <vector>

#include "vaildation_layer.h"

extern VkInstance instance; // 声明全局vulkan实例

void checkExtension(); // 查看当前vulkan所支持的拓展
void createInstance(); // 创建vulkan实例

void vkInstanceCleanUp(); // 销毁vulkan实例

#endif
